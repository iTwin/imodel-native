/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/DbMappingTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "MapContext.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                  Affan.Khan                          05/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, Simple_MixIn)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("Simple_MixIn.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName='TestSchema' alias='ts' version='01.00.00' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                   <ECSchemaReference name='ECDbMap' version='02.00.00' alias='ecdbmap' />
                   <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA'/>
                    <ECEntityClass typeName='MyMixin' modifier='Abstract'>
                       <ECCustomAttributes>
                           <IsMixin xmlns='CoreCustomAttributes.01.00'>
                               <AppliesToEntityClass>BaseClass</AppliesToEntityClass>
                           </IsMixin>
                       </ECCustomAttributes>
                       <ECProperty propertyName='M1' typeName='long' />
                       <ECProperty propertyName='M2' typeName='long' />
                       <ECProperty propertyName='M3' typeName='long' />
                   </ECEntityClass>
                   <ECEntityClass typeName='BaseClass' modifier='Abstract' >
                       <ECCustomAttributes>
                           <ClassMap xmlns='ECDbMap.02.00'>
                               <MapStrategy>TablePerHierarchy</MapStrategy>
                           </ClassMap>
                           <ShareColumns xmlns='ECDbMap.02.00'>
                               <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                               <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>
                           </ShareColumns>
                       </ECCustomAttributes>
                       <ECProperty propertyName='P1' typeName='long' />
                       <ECProperty propertyName='P2' typeName='long' />
                       <ECProperty propertyName='P3' typeName='long' />
                   </ECEntityClass>
                   <ECEntityClass typeName='ChildA'> 
                       <BaseClass>BaseClass</BaseClass>
                       <BaseClass>MyMixin</BaseClass>
                       <ECProperty propertyName='A1' typeName='long' />
                       <ECProperty propertyName='A2' typeName='long' />
                       <ECProperty propertyName='A3' typeName='long' />
                   </ECEntityClass>
                   <ECEntityClass typeName='ChildB'> 
                       <BaseClass>BaseClass</BaseClass>
                       <ECProperty propertyName='B1' typeName='long' />
                       <ECProperty propertyName='B2' typeName='long' />
                       <ECProperty propertyName='B3' typeName='long' />
                   </ECEntityClass>
                   <ECEntityClass typeName='ChildC'> 
                       <BaseClass>ChildB</BaseClass>
                       <ECProperty propertyName='C1' typeName='long' />
                       <ECProperty propertyName='C2' typeName='long' />
                       <ECProperty propertyName='C3' typeName='long' />
                   </ECEntityClass>
                   <ECEntityClass typeName='ChildD'> 
                       <BaseClass>ChildB</BaseClass>
                       <ECProperty propertyName='D1' typeName='long' />
                       <ECProperty propertyName='D2' typeName='long' />
                       <ECProperty propertyName='D3' typeName='long' />
                   </ECEntityClass>
                   <ECEntityClass typeName='ChildE'> 
                       <BaseClass>ChildD</BaseClass>
                       <ECProperty propertyName='E1' typeName='long' />
                       <ECProperty propertyName='E2' typeName='long' />
                       <ECProperty propertyName='E3' typeName='long' />
                   </ECEntityClass>
                   <ECEntityClass typeName='ChildF'> 
                       <BaseClass>ChildD</BaseClass>
                       <BaseClass>MyMixin</BaseClass>
                       <ECProperty propertyName='F1' typeName='long' />
                       <ECProperty propertyName='F2' typeName='long' />
                       <ECProperty propertyName='F3' typeName='long' />
                   </ECEntityClass>
                 </ECSchema>)xml")));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT M1, M2, M3 FROM ts.MyMixin")); stmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Affan.Khan                          05/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, SimpleFK_Dervied)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("SimpleFK.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                <ECEntityClass typeName="A">
                    <ECCustomAttributes>
                        <ClassMap xlmns="ECDbMap.02.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                    </ECCustomAttributes>
                    <ECProperty propertyName="Price" typeName="double" />
                </ECEntityClass>
                <ECEntityClass typeName="A1">
                    <BaseClass>A</BaseClass>
                    <ECProperty propertyName="Tag" typeName="double" />
                </ECEntityClass>
                <ECEntityClass typeName="B">
                    <ECProperty propertyName="Cost" typeName="double" />
                    <ECNavigationProperty propertyName="A" relationshipName="AHasB" direction="Backward">
                        <ECCustomAttributes>
                            <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                        </ECCustomAttributes>
                    </ECNavigationProperty>
                </ECEntityClass>
                <ECEntityClass typeName="B1">
                    <BaseClass>B</BaseClass>
                    <ECProperty propertyName="Tag" typeName="double" />
                </ECEntityClass>
               <ECRelationshipClass typeName="AHasB" strength="Referencing" modifier="Abstract" strengthDirection="Backward">
                  <Source multiplicity="(0..1)" polymorphic="True" roleLabel="A">
                      <Class class ="A" />
                  </Source>
                  <Target multiplicity="(0..*)" polymorphic="True" roleLabel="B">
                      <Class class ="B" />
                  </Target>
               </ECRelationshipClass>
               <ECRelationshipClass typeName="A1HasB1" strength="Referencing" modifier="Sealed" strengthDirection="Backward">
                  <BaseClass>AHasB</BaseClass>
                  <Source multiplicity="(0..1)" polymorphic="True" roleLabel="A1">
                      <Class class ="A1" />
                  </Source>
                  <Target multiplicity="(0..*)" polymorphic="True" roleLabel="B1">
                      <Class class ="B1" />
                  </Target>
               </ECRelationshipClass>
            </ECSchema>)xml")));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId, ECClassId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM ts.AHasB")); stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId, ECClassId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM ts.A1HasB1")); stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId, ECClassId, Price FROM ts.A")); stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId, ECClassId, Cost, A FROM ts.B")); stmt.Finalize();


    MapContext ctx(m_ecdb);

    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "A", "ECInstanceId"), "ts_A", "Id");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "A", "ECClassId"), "ts_A", "ECClassId");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "A", "Price"), "ts_A", "Price");

    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "A1", "ECInstanceId"), "ts_A", "Id");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "A1", "ECClassId"), "ts_A", "ECClassId");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "A1", "Price"), "ts_A", "Price");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "A1", "Tag"), "ts_A", "Tag");
    ////
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "AHasB", "ECInstanceId"), "ts_B", "Id");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "AHasB", "ECClassId"), "ts_B", "ARelECClassId");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "AHasB", "SourceECInstanceId"), "ts_B", "AId");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "AHasB", "SourceECClassId"), "ts_A", "ECClassId");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "AHasB", "TargetECInstanceId"), "ts_B", "Id");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "AHasB", "TargetECClassId"), "ts_B", "ECClassId");
    ////
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "B", "ECInstanceId"), "ts_B", "Id");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "B", "ECClassId"), "ts_B", "ECClassId");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "B", "Cost"), "ts_B", "Cost");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "B", "A.Id"), "ts_B", "Id");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "B", "A.RelECClassId"), "ts_B", "ARelECClassId");
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                  Affan.Khan                          05/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, SimpleFK)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("SimpleFK.ecdb", SchemaItem(
      R"xml(<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />
                <ECEntityClass typeName='A'>
                    <ECProperty propertyName='Price' typeName='double' />
                </ECEntityClass>
                <ECEntityClass typeName='B'>
                    <ECProperty propertyName='Cost' typeName='double' />
                    <ECNavigationProperty propertyName='A' relationshipName='AHasB' direction='Backward'>
                        <ECCustomAttributes>
                            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>
                        </ECCustomAttributes>
                    </ECNavigationProperty>
                </ECEntityClass>
               <ECRelationshipClass typeName='AHasB' strength='Referencing' modifier='Sealed' strengthDirection='Backward'>
                  <Source multiplicity='(0..1)' polymorphic='False' roleLabel='A'>
                      <Class class ='A' />
                  </Source>
                  <Target multiplicity='(0..*)' polymorphic='False' roleLabel='B'>
                      <Class class ='B' />
                  </Target>
               </ECRelationshipClass>
            </ECSchema>)xml")));
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId, ECClassId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM ts.AHasB")); stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId, ECClassId, Price FROM ts.A")); stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId, ECClassId, Cost, A FROM ts.B")); stmt.Finalize();


    MapContext ctx(m_ecdb);
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "A", "ECInstanceId"), "ts_A", "Id");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "A", "ECClassId"), "ts_A", "ECClassId");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "A", "Price"), "ts_A", "Price");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "AHasB", "ECInstanceId"), "ts_B", "Id");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "AHasB", "ECClassId"), "ts_B", "ARelECClassId");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "AHasB", "SourceECInstanceId"), "ts_B", "AId");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "AHasB", "SourceECClassId"), "ts_A", "ECClassId");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "AHasB", "TargetECInstanceId"), "ts_B", "Id");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "AHasB", "TargetECClassId"), "ts_B", "ECClassId");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "B", "ECInstanceId"), "ts_B", "Id");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "B", "ECClassId"), "ts_B", "ECClassId");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "B", "Cost"), "ts_B", "Cost");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "B", "A.Id"), "ts_B", "Id");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "B", "A.RelECClassId"), "ts_B", "ARelECClassId");
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                  Affan.Khan                          05/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, SimpleTest)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ECClassIdColumnVirtuality.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="SimpleTable" modifier="Abstract">
                <ECProperty propertyName="Prop1" typeName="string" />
            </ECEntityClass>
          </ECSchema>)xml")));

    MapContext ctx(m_ecdb);
    ASSERT_EXISTS_CLASSMAP(ctx, "TestSchema", "SimpleTable");
    ASSERT_NOTEXISTS_CLASSMAP(ctx, "TestSchemaBoo", "SimpleTableBoo");
    ASSERT_EXISTS_PROPERTYMAP(ctx, PropertyAccessString("TestSchema", "SimpleTable", "Prop1"));
    ASSERT_NOTEXISTS_PROPERTYMAP(ctx, PropertyAccessString("TestSchemaBoo", "SimpleTableBoo", "Prop1Boo"));
    ASSERT_EXISTS_PROPERTYMAP_TABLE(ctx, PropertyAccessString("TestSchema", "SimpleTable", "Prop1"), "ts_SimpleTable");
    ASSERT_NOTEXISTS_PROPERTYMAP_TABLE(ctx, PropertyAccessString("TestSchemaBoo", "SimpleTableBoo", "Prop1Boo"), "ts_SimpleTableBoo");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "SimpleTable", "Prop1"), "ts_SimpleTable", "Prop1");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "SimpleTable", "ECInstanceId"), "ts_SimpleTable", "Id");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "SimpleTable", "ECClassId"), "ts_SimpleTable", "ECClassId");

    ASSERT_PROPERTYMAP_COUNT(ctx, 3, "TestSchema", "SimpleTable");

    ASSERT_TABLE_TYPE(ctx, MapContext::Table::Type::Virtual, "ts_SimpleTable");
    ASSERT_COLUMN_COUNT(ctx, 3, "ts_SimpleTable");

    ASSERT_COLUMN_IS_NOT_VIRTUAL(ctx, "ts_SimpleTable", "Prop1");
    ASSERT_COLUMN_TYPE(ctx, MapContext::Column::Type::Text, "ts_SimpleTable", "Prop1");

    ASSERT_COLUMN_TYPE(ctx, MapContext::Column::Type::Integer, "ts_SimpleTable", "Id");
    ASSERT_COLUMN_IS_NOT_VIRTUAL(ctx, "ts_SimpleTable", "Id");

    ASSERT_COLUMN_IS_VIRTUAL(ctx, "ts_SimpleTable", "ECClassId");
    ASSERT_COLUMN_TYPE(ctx, MapContext::Column::Type::Integer, "ts_SimpleTable", "ECClassId");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Affan.Khan                          05/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, OverflowComplex_TPH_Overflow_Max_15)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("SimpleTreeWithTablePerHierarchySharedTable.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name='CoreCustomAttributes' version='01.00' alias='CoreCA' />
                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                <ECStructClass typeName="GreekApha">
                    <ECProperty propertyName="Alpha" typeName="double"/>
                    <ECProperty propertyName="Beta" typeName="double"/>
                    <ECProperty propertyName="Gamma" typeName="double"/>
                    <ECProperty propertyName="Delta" typeName="double"/>
                    <ECProperty propertyName="Epsilon" typeName="double"/>
                    <ECProperty propertyName="Zeta" typeName="double"/>
                    <ECProperty propertyName="Eta" typeName="double"/>
                    <ECProperty propertyName="Theta" typeName="double"/>
                    <ECProperty propertyName="Iota" typeName="double"/>
                    <ECProperty propertyName="Kappa" typeName="double"/>
                    <ECProperty propertyName="Lamda" typeName="double"/>
                    <ECProperty propertyName="Mu" typeName="double"/>
                </ECStructClass>
                <ECEntityClass typeName="IP" modifier="Abstract">
                    <ECCustomAttributes>
                        <IsMixin xmlns='CoreCustomAttributes.01.00'>
                            <AppliesToEntityClass>BaseClass</AppliesToEntityClass>
                        </IsMixin>
                    </ECCustomAttributes>
                    <ECProperty propertyName="I" typeName="int"/>
                    <ECProperty propertyName="L" typeName="long"/>
                    <ECProperty propertyName="B" typeName="binary"/>
                    <ECProperty propertyName="BOOL" typeName="boolean"/>
                    <ECProperty propertyName="P2D" typeName="point2d"/>
                    <ECProperty propertyName="P3D" typeName="point3d"/>
                    <ECStructProperty propertyName="ST" typeName="GreekApha"/>
                    <ECProperty propertyName="G" typeName="Bentley.Geometry.Common.IGeometry"/>
                    <ECProperty propertyName="S" typeName="string"/>
                    <ECProperty propertyName="DT" typeName="dateTime"/>
                    <ECProperty propertyName="D" typeName="double"/>
                </ECEntityClass>
                <ECEntityClass typeName="IA" modifier="Abstract">
                    <ECCustomAttributes>
                        <IsMixin xmlns='CoreCustomAttributes.01.00'>
                            <AppliesToEntityClass>BaseClass</AppliesToEntityClass>
                        </IsMixin>
                    </ECCustomAttributes>
                    <ECArrayProperty propertyName="ArS" typeName="string" minOccurs="0" maxOccurs="unbounded"/>
                    <ECArrayProperty propertyName="ArI" typeName="int" minOccurs="0" maxOccurs="unbounded"/>
                    <ECArrayProperty propertyName="ArL" typeName="long" minOccurs="0" maxOccurs="unbounded"/>
                    <ECArrayProperty propertyName="ArD" typeName="double" minOccurs="0" maxOccurs="unbounded"/>
                    <ECArrayProperty propertyName="ArDT" typeName="dateTime" minOccurs="0" maxOccurs="unbounded"/>
                    <ECArrayProperty propertyName="ArB" typeName="binary" minOccurs="0" maxOccurs="unbounded"/>
                    <ECArrayProperty propertyName="ArBOOL" typeName="boolean" minOccurs="0" maxOccurs="unbounded"/>
                    <ECArrayProperty propertyName="ArP2D" typeName="point2d" minOccurs="0" maxOccurs="unbounded"/>
                    <ECArrayProperty propertyName="ArP3D" typeName="point3d" minOccurs="0" maxOccurs="unbounded"/>
                    <ECArrayProperty propertyName="ArG" typeName="Bentley.Geometry.Common.IGeometry" minOccurs="0" maxOccurs="unbounded"/>
                    <ECArrayProperty propertyName="ArST" typeName="GreekApha" minOccurs="0" maxOccurs="unbounded"/>
                </ECEntityClass>
                <ECEntityClass typeName="BaseClass" modifier="Abstract">
                    <ECCustomAttributes>
                        <ClassMap xlmns="ECDbMap.02.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                        <ShareColumns xmlns='ECDbMap.02.00'>
                            <MaxSharedColumnsBeforeOverflow>15</MaxSharedColumnsBeforeOverflow>
                            <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                        </ShareColumns>"
                    </ECCustomAttributes>
                    <ECProperty propertyName="M_S" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="ChildClass">
                    <BaseClass>BaseClass</BaseClass>
                    <BaseClass>IP</BaseClass>
                    <BaseClass>IA</BaseClass>
                </ECEntityClass>
            </ECSchema>)xml")));

    MapContext ctx(m_ecdb);

    ASSERT_TABLE_TYPE(ctx, MapContext::Table::Type::Virtual, "ts_IA");
    ASSERT_TABLE_TYPE(ctx, MapContext::Table::Type::Virtual, "ts_IP");
    ASSERT_TABLE_TYPE(ctx, MapContext::Table::Type::Primary, "ts_BaseClass");
    ASSERT_TABLE_TYPE(ctx, MapContext::Table::Type::Overflow, "ts_BaseClass_Overflow");

    ASSERT_COLUMN_COUNT(ctx, 11 + 2, "ts_IA");
    ASSERT_COLUMN_COUNT(ctx, 25 + 2, "ts_IP");
    ASSERT_COLUMN_COUNT(ctx, 15 + 2 + 1, "ts_BaseClass");
    ASSERT_COLUMN_COUNT(ctx, 21 + 2, "ts_BaseClass_Overflow");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "M_S"), "ts_BaseClass", "M_S");
    //SharedColumn
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "I"), "ts_BaseClass", "ps1");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "L"), "ts_BaseClass", "ps2");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "B"), "ts_BaseClass", "ps3");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "BOOL"), "ts_BaseClass", "ps4");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "P2D.X"), "ts_BaseClass", "ps5");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "P2D.Y"), "ts_BaseClass", "ps6");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "P3D.X"), "ts_BaseClass", "ps7");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "P3D.Y"), "ts_BaseClass", "ps8");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "P3D.Z"), "ts_BaseClass", "ps9");
    //Struct is pused into overflow
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ST.Alpha"), "ts_BaseClass_Overflow", "os1");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ST.Beta"), "ts_BaseClass_Overflow", "os2");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ST.Gamma"), "ts_BaseClass_Overflow", "os3");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ST.Delta"), "ts_BaseClass_Overflow", "os4");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ST.Epsilon"), "ts_BaseClass_Overflow", "os5");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ST.Zeta"), "ts_BaseClass_Overflow", "os6");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ST.Eta"), "ts_BaseClass_Overflow", "os7");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ST.Theta"), "ts_BaseClass_Overflow", "os8");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ST.Iota"), "ts_BaseClass_Overflow", "os9");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ST.Kappa"), "ts_BaseClass_Overflow", "os10");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ST.Lamda"), "ts_BaseClass_Overflow", "os11");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ST.Mu"), "ts_BaseClass_Overflow", "os12");
    //Back to primary table
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "G"), "ts_BaseClass", "ps10");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "S"), "ts_BaseClass", "ps11");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "DT"), "ts_BaseClass", "ps12");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "D"), "ts_BaseClass", "ps13");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ArS"), "ts_BaseClass", "ps14");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ArI"), "ts_BaseClass", "ps15");
    //back to overflow
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ArL"), "ts_BaseClass_Overflow", "os13");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ArD"), "ts_BaseClass_Overflow", "os14");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ArDT"), "ts_BaseClass_Overflow", "os15");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ArB"), "ts_BaseClass_Overflow", "os16");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ArBOOL"), "ts_BaseClass_Overflow", "os17");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ArP2D"), "ts_BaseClass_Overflow", "os18");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ArP3D"), "ts_BaseClass_Overflow", "os19");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ArG"), "ts_BaseClass_Overflow", "os20");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ArST"), "ts_BaseClass_Overflow", "os21");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Affan.Khan                          05/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, OverflowComplex_TPH_Overflow_Default)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("SimpleTreeWithTablePerHierarchySharedTable.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name='CoreCustomAttributes' version='01.00' alias='CoreCA' />
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECStructClass typeName="GreekApha">
                <ECProperty propertyName="Alpha" typeName="double"/>
                <ECProperty propertyName="Beta" typeName="double"/>
                <ECProperty propertyName="Gamma" typeName="double"/>
                <ECProperty propertyName="Delta" typeName="double"/>
                <ECProperty propertyName="Epsilon" typeName="double"/>
                <ECProperty propertyName="Zeta" typeName="double"/>
                <ECProperty propertyName="Eta" typeName="double"/>
                <ECProperty propertyName="Theta" typeName="double"/>
                <ECProperty propertyName="Iota" typeName="double"/>
                <ECProperty propertyName="Kappa" typeName="double"/>
                <ECProperty propertyName="Lamda" typeName="double"/>
                <ECProperty propertyName="Mu" typeName="double"/>
            </ECStructClass>
            <ECEntityClass typeName="IP" modifier="Abstract">
                <ECCustomAttributes>
                    <IsMixin xmlns='CoreCustomAttributes.01.00'>
                        <AppliesToEntityClass>BaseClass</AppliesToEntityClass>
                    </IsMixin>
                </ECCustomAttributes>
                <ECProperty propertyName="I" typeName="int"/>
                <ECProperty propertyName="L" typeName="long"/>
                <ECProperty propertyName="B" typeName="binary"/>
                <ECProperty propertyName="BOOL" typeName="boolean"/>
                <ECProperty propertyName="P2D" typeName="point2d"/>
                <ECProperty propertyName="P3D" typeName="point3d"/>
                <ECStructProperty propertyName="ST" typeName="GreekApha"/>
                <ECProperty propertyName="G" typeName="Bentley.Geometry.Common.IGeometry"/>
                <ECProperty propertyName="S" typeName="string"/>
                <ECProperty propertyName="DT" typeName="dateTime"/>
                <ECProperty propertyName="D" typeName="double"/>
            </ECEntityClass>
            <ECEntityClass typeName="IA" modifier="Abstract">
                <ECCustomAttributes>
                    <IsMixin xmlns='CoreCustomAttributes.01.00'>
                        <AppliesToEntityClass>BaseClass</AppliesToEntityClass>
                    </IsMixin>
                </ECCustomAttributes>
                <ECArrayProperty propertyName="ArS" typeName="string" minOccurs="0" maxOccurs="unbounded"/>
                <ECArrayProperty propertyName="ArI" typeName="int" minOccurs="0" maxOccurs="unbounded"/>
                <ECArrayProperty propertyName="ArL" typeName="long" minOccurs="0" maxOccurs="unbounded"/>
                <ECArrayProperty propertyName="ArD" typeName="double" minOccurs="0" maxOccurs="unbounded"/>
                <ECArrayProperty propertyName="ArDT" typeName="dateTime" minOccurs="0" maxOccurs="unbounded"/>
                <ECArrayProperty propertyName="ArB" typeName="binary" minOccurs="0" maxOccurs="unbounded"/>
                <ECArrayProperty propertyName="ArBOOL" typeName="boolean" minOccurs="0" maxOccurs="unbounded"/>
                <ECArrayProperty propertyName="ArP2D" typeName="point2d" minOccurs="0" maxOccurs="unbounded"/>
                <ECArrayProperty propertyName="ArP3D" typeName="point3d" minOccurs="0" maxOccurs="unbounded"/>
                <ECArrayProperty propertyName="ArG" typeName="Bentley.Geometry.Common.IGeometry" minOccurs="0" maxOccurs="unbounded"/>
                <ECArrayProperty propertyName="ArST" typeName="GreekApha" minOccurs="0" maxOccurs="unbounded"/>
            </ECEntityClass>
            <ECEntityClass typeName="BaseClass" modifier="Abstract">
                <ECCustomAttributes>
                    <ClassMap xlmns="ECDbMap.02.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <ShareColumns xmlns='ECDbMap.02.00'>
                        <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                    </ShareColumns>"
                </ECCustomAttributes>
                <ECProperty propertyName="M_S" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="ChildClass">
                <BaseClass>BaseClass</BaseClass>
                <BaseClass>IP</BaseClass>
                <BaseClass>IA</BaseClass>
            </ECEntityClass>
        </ECSchema>)xml")));

    MapContext ctx(m_ecdb);
    ASSERT_TABLE_TYPE(ctx, MapContext::Table::Type::Virtual, "ts_IA");
    ASSERT_TABLE_TYPE(ctx, MapContext::Table::Type::Virtual, "ts_IP");
    ASSERT_TABLE_TYPE(ctx, MapContext::Table::Type::Primary, "ts_BaseClass");
    ASSERT_NOTEXISTS_TABLE(ctx, "ts_BaseClass_Overflow");

    ASSERT_COLUMN_COUNT(ctx, 11 + 2, "ts_IA");
    ASSERT_COLUMN_COUNT(ctx, 25 + 2, "ts_IP"); 
    ASSERT_COLUMN_COUNT(ctx, 39, "ts_BaseClass"); 
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ECInstanceId"), "ts_BaseClass", "Id");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ECClassId"), "ts_BaseClass", "ECClassId");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "M_S"), "ts_BaseClass", "M_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "I"), "ts_BaseClass", "ps1");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "L"), "ts_BaseClass", "ps2");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "B"), "ts_BaseClass", "ps3");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "BOOL"), "ts_BaseClass", "ps4");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "P2D.X"), "ts_BaseClass", "ps5");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "P2D.Y"), "ts_BaseClass", "ps6");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "P3D.X"), "ts_BaseClass", "ps7");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "P3D.Y"), "ts_BaseClass", "ps8");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "P3D.Z"), "ts_BaseClass", "ps9");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ST.Alpha"), "ts_BaseClass", "ps10");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ST.Beta"), "ts_BaseClass", "ps11");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ST.Gamma"), "ts_BaseClass", "ps12");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ST.Delta"), "ts_BaseClass", "ps13");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ST.Epsilon"), "ts_BaseClass", "ps14");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ST.Zeta"), "ts_BaseClass", "ps15");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ST.Eta"), "ts_BaseClass", "ps16");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ST.Theta"), "ts_BaseClass", "ps17");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ST.Iota"), "ts_BaseClass", "ps18");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ST.Kappa"), "ts_BaseClass", "ps19");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ST.Lamda"), "ts_BaseClass", "ps20");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ST.Mu"), "ts_BaseClass", "ps21");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "G"), "ts_BaseClass", "ps22");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "S"), "ts_BaseClass", "ps23");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "DT"), "ts_BaseClass", "ps24");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "D"), "ts_BaseClass", "ps25");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ArS"), "ts_BaseClass", "ps26");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ArI"), "ts_BaseClass", "ps27");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ArL"), "ts_BaseClass", "ps28");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ArD"), "ts_BaseClass", "ps29");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ArDT"), "ts_BaseClass", "ps30");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ArB"), "ts_BaseClass", "ps31");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ArBOOL"), "ts_BaseClass", "ps32");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ArP2D"), "ts_BaseClass", "ps33");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ArP3D"), "ts_BaseClass", "ps34");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ArG"), "ts_BaseClass", "ps35");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Affan.Khan                          05/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, OverflowComplex_TPH_Overflow_0)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("SimpleTreeWithTablePerHierarchySharedTable.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECSchemaReference name='CoreCustomAttributes' version='01.00' alias='CoreCA' />
        <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
        <ECStructClass typeName="GreekApha">
            <ECProperty propertyName="Alpha" typeName="double"/>
            <ECProperty propertyName="Beta" typeName="double"/>
            <ECProperty propertyName="Gamma" typeName="double"/>
            <ECProperty propertyName="Delta" typeName="double"/>
            <ECProperty propertyName="Epsilon" typeName="double"/>
            <ECProperty propertyName="Zeta" typeName="double"/>
            <ECProperty propertyName="Eta" typeName="double"/>
            <ECProperty propertyName="Theta" typeName="double"/>
            <ECProperty propertyName="Iota" typeName="double"/>
            <ECProperty propertyName="Kappa" typeName="double"/>
            <ECProperty propertyName="Lamda" typeName="double"/>
            <ECProperty propertyName="Mu" typeName="double"/>
        </ECStructClass>
        <ECEntityClass typeName="IP" modifier="Abstract">
            <ECCustomAttributes>
                <IsMixin xmlns='CoreCustomAttributes.01.00'>
                    <AppliesToEntityClass>BaseClass</AppliesToEntityClass>
                </IsMixin>
            </ECCustomAttributes>
            <ECProperty propertyName="I" typeName="int"/>
            <ECProperty propertyName="L" typeName="long"/>
            <ECProperty propertyName="B" typeName="binary"/>
            <ECProperty propertyName="BOOL" typeName="boolean"/>
            <ECProperty propertyName="P2D" typeName="point2d"/>
            <ECProperty propertyName="P3D" typeName="point3d"/>
            <ECStructProperty propertyName="ST" typeName="GreekApha"/>
            <ECProperty propertyName="G" typeName="Bentley.Geometry.Common.IGeometry"/>
            <ECProperty propertyName="S" typeName="string"/>
            <ECProperty propertyName="DT" typeName="dateTime"/>
            <ECProperty propertyName="D" typeName="double"/>
        </ECEntityClass>
        <ECEntityClass typeName="IA" modifier="Abstract">
            <ECCustomAttributes>
                <IsMixin xmlns='CoreCustomAttributes.01.00'>
                    <AppliesToEntityClass>BaseClass</AppliesToEntityClass>
                </IsMixin>
            </ECCustomAttributes>
            <ECArrayProperty propertyName="ArS" typeName="string" minOccurs="0" maxOccurs="unbounded"/>
            <ECArrayProperty propertyName="ArI" typeName="int" minOccurs="0" maxOccurs="unbounded"/>
            <ECArrayProperty propertyName="ArL" typeName="long" minOccurs="0" maxOccurs="unbounded"/>
            <ECArrayProperty propertyName="ArD" typeName="double" minOccurs="0" maxOccurs="unbounded"/>
            <ECArrayProperty propertyName="ArDT" typeName="dateTime" minOccurs="0" maxOccurs="unbounded"/>
            <ECArrayProperty propertyName="ArB" typeName="binary" minOccurs="0" maxOccurs="unbounded"/>
            <ECArrayProperty propertyName="ArBOOL" typeName="boolean" minOccurs="0" maxOccurs="unbounded"/>
            <ECArrayProperty propertyName="ArP2D" typeName="point2d" minOccurs="0" maxOccurs="unbounded"/>
            <ECArrayProperty propertyName="ArP3D" typeName="point3d" minOccurs="0" maxOccurs="unbounded"/>
            <ECArrayProperty propertyName="ArG" typeName="Bentley.Geometry.Common.IGeometry" minOccurs="0" maxOccurs="unbounded"/>
            <ECArrayProperty propertyName="ArST" typeName="GreekApha" minOccurs="0" maxOccurs="unbounded"/>
        </ECEntityClass>
        <ECEntityClass typeName="BaseClass" modifier="Abstract">
            <ECCustomAttributes>
                <ClassMap xlmns="ECDbMap.02.00">
                    <MapStrategy>TablePerHierarchy</MapStrategy>
                </ClassMap>
                <ShareColumns xmlns='ECDbMap.02.00'>
                    <MaxSharedColumnsBeforeOverflow>0</MaxSharedColumnsBeforeOverflow>
                    <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                </ShareColumns>"
            </ECCustomAttributes>
            <ECProperty propertyName="M_S" typeName="string" />
        </ECEntityClass>
        <ECEntityClass typeName="ChildClass">
            <BaseClass>BaseClass</BaseClass>
            <BaseClass>IP</BaseClass>
            <BaseClass>IA</BaseClass>
        </ECEntityClass>
    </ECSchema>)xml")));

    MapContext ctx(m_ecdb);
    ASSERT_TABLE_TYPE(ctx, MapContext::Table::Type::Virtual, "ts_IA");
    ASSERT_TABLE_TYPE(ctx, MapContext::Table::Type::Virtual, "ts_IP");
    ASSERT_TABLE_TYPE(ctx, MapContext::Table::Type::Primary, "ts_BaseClass");
    ASSERT_TABLE_TYPE(ctx, MapContext::Table::Type::Overflow, "ts_BaseClass_Overflow");

    ASSERT_COLUMN_COUNT(ctx, 11 + 2, "ts_IA");
    ASSERT_COLUMN_COUNT(ctx, 25 + 2, "ts_IP");
    ASSERT_COLUMN_COUNT(ctx, 3, "ts_BaseClass");
    ASSERT_COLUMN_COUNT(ctx, 38, "ts_BaseClass_Overflow");

    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "M_S"), "ts_BaseClass", "M_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "I"), "ts_BaseClass_Overflow", "os1");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "L"), "ts_BaseClass_Overflow", "os2");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "B"), "ts_BaseClass_Overflow", "os3");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "BOOL"), "ts_BaseClass_Overflow", "os4");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "P2D.X"), "ts_BaseClass_Overflow", "os5");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "P2D.Y"), "ts_BaseClass_Overflow", "os6");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "P3D.X"), "ts_BaseClass_Overflow", "os7");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "P3D.Y"), "ts_BaseClass_Overflow", "os8");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "P3D.Z"), "ts_BaseClass_Overflow", "os9");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ST.Alpha"), "ts_BaseClass_Overflow", "os10");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ST.Beta"), "ts_BaseClass_Overflow", "os11");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ST.Gamma"), "ts_BaseClass_Overflow", "os12");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ST.Delta"), "ts_BaseClass_Overflow", "os13");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ST.Epsilon"), "ts_BaseClass_Overflow", "os14");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ST.Zeta"), "ts_BaseClass_Overflow", "os15");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ST.Eta"), "ts_BaseClass_Overflow", "os16");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ST.Theta"), "ts_BaseClass_Overflow", "os17");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ST.Iota"), "ts_BaseClass_Overflow", "os18");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ST.Kappa"), "ts_BaseClass_Overflow", "os19");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ST.Lamda"), "ts_BaseClass_Overflow", "os20");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ST.Mu"), "ts_BaseClass_Overflow", "os21");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "G"), "ts_BaseClass_Overflow", "os22");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "S"), "ts_BaseClass_Overflow", "os23");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "DT"), "ts_BaseClass_Overflow", "os24");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "D"), "ts_BaseClass_Overflow", "os25");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ArS"), "ts_BaseClass_Overflow", "os26");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ArI"), "ts_BaseClass_Overflow", "os27");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ArL"), "ts_BaseClass_Overflow", "os28");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ArD"), "ts_BaseClass_Overflow", "os29");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ArDT"), "ts_BaseClass_Overflow", "os30");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ArB"), "ts_BaseClass_Overflow", "os31");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ArBOOL"), "ts_BaseClass_Overflow", "os32");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ArP2D"), "ts_BaseClass_Overflow", "os33");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ArP3D"), "ts_BaseClass_Overflow", "os34");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString("TestSchema", "ChildClass", "ArG"), "ts_BaseClass_Overflow", "os35");
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                  Affan.Khan                          05/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, SimpleTree_TPH_JT)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("SimpleTreeWithTablePerHierarchySharedTable.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                <ECEntityClass typeName="M" modifier="Abstract">
                    <ECCustomAttributes>
                        <ClassMap xlmns="ECDbMap.02.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                    <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                    </ECCustomAttributes>
                    <ECProperty propertyName="M_S" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="ML" modifier="Abstract">
                    <BaseClass>M</BaseClass>
                    <ECProperty propertyName="ML_S" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="L" modifier="none">
                    <BaseClass>ML</BaseClass>
                    <ECProperty propertyName="L_S" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="LL" modifier="sealed">
                    <BaseClass>L</BaseClass>
                    <ECProperty propertyName="LL_S" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="LR" modifier="sealed">
                    <BaseClass>L</BaseClass>
                    <ECProperty propertyName="LR_S" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="MR" modifier="Abstract">
                    <BaseClass>M</BaseClass>
                    <ECProperty propertyName="MR_S" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="R" modifier="none">
                    <BaseClass>MR</BaseClass>
                    <ECProperty propertyName="R_S" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="RL" modifier="sealed">
                    <BaseClass>R</BaseClass>
                    <ECProperty propertyName="RL_S" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="RR" modifier="sealed">
                    <BaseClass>R</BaseClass>
                    <ECProperty propertyName="RR_S" typeName="string" />
                </ECEntityClass>
            </ECSchema>)xml")));
    MapContext ctx(m_ecdb);
    Utf8CP testSchema = "TestSchema";
    //==================================================================================
    ASSERT_TABLE_TYPE(ctx, MapContext::Table::Type::Primary, "ts_M");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "M", "ECInstanceId"), "ts_M", "Id");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "M", "ECClassId"), "ts_M", "ECClassId");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "M", "M_S"), "ts_M", "M_S");

    ASSERT_TABLE_TYPE(ctx, MapContext::Table::Type::Joined, "ts_ML");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "ML", "ML_S"), "ts_ML", "ML_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "L", "ML_S"), "ts_ML", "ML_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "L", "L_S"), "ts_ML", "L_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "LL", "ML_S"), "ts_ML", "ML_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "LL", "L_S"), "ts_ML", "L_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "LL", "LL_S"), "ts_ML", "LL_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "LR", "ML_S"), "ts_ML", "ML_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "LR", "L_S"), "ts_ML", "L_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "LR", "LR_S"), "ts_ML", "LR_S");

    //==================================================================================
    ASSERT_TABLE_TYPE(ctx, MapContext::Table::Type::Joined, "ts_MR");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "MR", "MR_S"), "ts_MR", "MR_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "R", "MR_S"), "ts_MR", "MR_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "R", "R_S"), "ts_MR", "R_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "RL", "MR_S"), "ts_MR", "MR_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "RL", "R_S"), "ts_MR", "R_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "RL", "RL_S"), "ts_MR", "RL_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "RR", "MR_S"), "ts_MR", "MR_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "RR", "R_S"), "ts_MR", "R_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "RR", "RR_S"), "ts_MR", "RR_S");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Affan.Khan                          05/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, SimpleTree_TPH)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("SimpleTree.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                <ECEntityClass typeName="M" modifier="Abstract">
                    <ECCustomAttributes>
                        <ClassMap xlmns="ECDbMap.02.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                    </ECCustomAttributes>
                    <ECProperty propertyName="M_S" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="ML" modifier="Abstract">
                    <BaseClass>M</BaseClass>
                    <ECProperty propertyName="ML_S" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="L" modifier="none">
                    <BaseClass>ML</BaseClass>
                    <ECProperty propertyName="L_S" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="LL" modifier="sealed">
                    <BaseClass>L</BaseClass>
                    <ECProperty propertyName="LL_S" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="LR" modifier="sealed">
                    <BaseClass>L</BaseClass>
                    <ECProperty propertyName="LR_S" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="MR" modifier="Abstract">
                    <BaseClass>M</BaseClass>
                    <ECProperty propertyName="MR_S" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="R" modifier="none">
                    <BaseClass>MR</BaseClass>
                    <ECProperty propertyName="R_S" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="RL" modifier="sealed">
                    <BaseClass>R</BaseClass>
                    <ECProperty propertyName="RL_S" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="RR" modifier="sealed">
                    <BaseClass>R</BaseClass>
                    <ECProperty propertyName="RR_S" typeName="string" />
                </ECEntityClass>
            </ECSchema>)xml")));
    
    MapContext ctx(m_ecdb);
    Utf8CP testSchema = "TestSchema";
    Utf8CP tableName = "ts_M";
    //==================================================================================
    ASSERT_TABLE_TYPE(ctx, MapContext::Table::Type::Primary, tableName);
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "M", "M_S"), tableName, "M_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "ML", "M_S"), tableName, "M_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "ML", "ML_S"), tableName, "ML_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "L", "M_S"), tableName, "M_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "L", "ML_S"), tableName, "ML_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "L", "L_S"), tableName, "L_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "LL", "M_S"), tableName, "M_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "LL", "ML_S"), tableName, "ML_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "LL", "L_S"), tableName, "L_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "LL", "LL_S"), tableName, "LL_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "LR", "M_S"), tableName, "M_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "LR", "ML_S"), tableName, "ML_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "LR", "L_S"), tableName, "L_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "LR", "LR_S"), tableName, "LR_S");
    //==================================================================================
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "M", "M_S"), tableName, "M_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "MR", "M_S"), tableName, "M_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "MR", "MR_S"), tableName, "MR_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "R", "M_S"), tableName, "M_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "R", "MR_S"), tableName, "MR_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "R", "R_S"), tableName, "R_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "RL", "M_S"), tableName, "M_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "RL", "MR_S"), tableName, "MR_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "RL", "R_S"), tableName, "R_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "RL", "RL_S"), tableName, "RL_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "RR", "M_S"), tableName, "M_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "RR", "MR_S"), tableName, "mR_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "RR", "R_S"), tableName, "R_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "RR", "RR_S"), tableName, "RR_S");
    //==================================================================================
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Affan.Khan                          05/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, SimpleTree)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("SimpleTreeWithTablePerClass.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECEntityClass typeName="M" modifier="Abstract">
                    <ECProperty propertyName="M_S" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="ML" modifier="Abstract">
                    <BaseClass>M</BaseClass>
                    <ECProperty propertyName="ML_S" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="L" modifier="none">
                    <BaseClass>ML</BaseClass>
                    <ECProperty propertyName="L_S" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="LL" modifier="sealed">
                    <BaseClass>L</BaseClass>
                    <ECProperty propertyName="LL_S" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="LR" modifier="sealed">
                    <BaseClass>L</BaseClass>
                    <ECProperty propertyName="LR_S" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="MR" modifier="Abstract">
                    <BaseClass>M</BaseClass>
                    <ECProperty propertyName="MR_S" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="R" modifier="none">
                    <BaseClass>MR</BaseClass>
                    <ECProperty propertyName="R_S" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="RL" modifier="sealed">
                    <BaseClass>R</BaseClass>
                    <ECProperty propertyName="RL_S" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="RR" modifier="sealed">
                    <BaseClass>R</BaseClass>
                    <ECProperty propertyName="RR_S" typeName="string" />
                </ECEntityClass>
            </ECSchema>)xml")));
    MapContext ctx(m_ecdb);
    const Utf8CP testSchema = "TestSchema";
    //==================================================================================
    ASSERT_TABLE_TYPE(ctx, MapContext::Table::Type::Virtual, "ts_M");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "M", "M_S"), "ts_M", "M_S");
    ASSERT_TABLE_TYPE(ctx, MapContext::Table::Type::Virtual, "ts_ML");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "ML", "M_S"), "ts_ML", "M_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "ML", "ML_S"), "ts_ML", "ML_S");
    ASSERT_TABLE_TYPE(ctx, MapContext::Table::Type::Primary, "ts_L");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "L", "M_S"), "ts_L", "M_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "L", "ML_S"), "ts_L", "ML_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "L", "L_S"), "ts_L", "L_S");
    ASSERT_TABLE_TYPE(ctx, MapContext::Table::Type::Primary, "ts_LL");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "LL", "M_S"), "ts_LL", "M_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "LL", "ML_S"), "ts_LL", "ML_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "LL", "L_S"), "ts_LL", "L_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "LL", "LL_S"), "ts_LL", "LL_S");
    ASSERT_TABLE_TYPE(ctx, MapContext::Table::Type::Primary, "ts_LR");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "LR", "M_S"), "ts_LR", "M_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "LR", "ML_S"), "ts_LR", "ML_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "LR", "L_S"), "ts_LR", "L_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "LR", "LR_S"), "ts_LR", "LR_S");
    //==================================================================================
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "M", "M_S"), "ts_M", "M_S");
    ASSERT_TABLE_TYPE(ctx, MapContext::Table::Type::Virtual, "ts_MR");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "MR", "M_S"), "ts_MR", "M_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "MR", "MR_S"), "ts_MR", "MR_S");
    ASSERT_TABLE_TYPE(ctx, MapContext::Table::Type::Primary, "ts_R");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "R", "M_S"), "ts_R", "M_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "R", "MR_S"), "ts_R", "MR_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "R", "R_S"), "ts_R", "R_S");
    ASSERT_TABLE_TYPE(ctx, MapContext::Table::Type::Primary, "ts_RL");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "RL", "M_S"), "ts_RL", "M_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "RL", "MR_S"), "ts_RL", "MR_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "RL", "R_S"), "ts_RL", "R_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "RL", "RL_S"), "ts_RL", "RL_S");
    ASSERT_TABLE_TYPE(ctx, MapContext::Table::Type::Primary, "ts_RR");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "RR", "M_S"), "ts_RR", "M_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "RR", "MR_S"), "ts_RR", "mR_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "RR", "R_S"), "ts_RR", "R_S");
    ASSERT_EXISTS_PROPERTYMAP_COLUMN(ctx, PropertyAccessString(testSchema, "RR", "RR_S"), "ts_RR", "RR_S");
    //==================================================================================
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      05/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, InvalidMapStrategyCATests)
    {
    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='ClassA'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>bla</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='ClassAB'>"
        "        <BaseClass>ClassA</BaseClass>"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "Invalid MapStrategy";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='ClassA'>"
        "        <ECCustomAttributes>"
        "            <ShareColumns xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "ShareColumnsCA cannot be used without a strategy";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='ClassA'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>None</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "MapStrategy None not allowed";

    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='ClassA'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "               <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>")));

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Foo" modifier="Sealed">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
                <ECProperty propertyName="Price" typeName="double" />
            </ECEntityClass>
        </ECSchema>)xml"))) << "TablePerHierarchy on sealed class is not allowed";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='ClassA'>"
        "        <ECCustomAttributes>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "JoinedTablePerDirectSubclass cannot be used without a strategy";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>NotMapped</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "ShareColumnsCA not allowed with Strategy NotMapped";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>None</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "sharedTable only allowed with SharedTable strategy";
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      05/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, UpdatableViews)
    {
        {
        ECDb ecdb;
        ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, SchemaItem(
            R"xml(<ECSchema schemaName="TestSchema" alias="ts1" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECEntityClass typeName="Base" modifier="Abstract">
                    <ECProperty propertyName="BaseProp1" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="Sub1" modifier="Abstract">
                    <BaseClass>Base</BaseClass>
                    <ECProperty propertyName="Sub1Prop1" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="Sub2">
                    <BaseClass>Base</BaseClass>
                    <ECProperty propertyName="Sub2Prop1" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="Sub21">
                    <BaseClass>Sub2</BaseClass>
                    <ECProperty propertyName="Sub21Prop1" typeName="string" />
                </ECEntityClass>
                </ECSchema>)xml"), "updatableviews1.ecdb"));

        ASSERT_FALSE(ecdb.TableExists("ts1_Base")) << "abstract class";
        ASSERT_TRUE(ecdb.TableExists("_ts1_Base")) << "expects updatable view as it has at least one concrete subclass";

        ASSERT_FALSE(ecdb.TableExists("ts1_Sub1")) << "abstract class";
        ASSERT_FALSE(ecdb.TableExists("_ts1_Sub1")) << "no updatable view expected as it doesn't have concrete subclasses";

        ASSERT_TRUE(ecdb.TableExists("ts1_Sub2")) << "concrete class";
        ASSERT_TRUE(ecdb.TableExists("_ts1_Sub2")) << "expects updatable view as it has at least one concrete subclass";

        ASSERT_TRUE(ecdb.TableExists("ts1_Sub21")) << "concrete class";
        ASSERT_FALSE(ecdb.TableExists("_ts1_Sub21")) << "no updatable view expected as it is leaf class";
        }

        {
        ECDb ecdb;
        ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, SchemaItem(
            R"xml(<ECSchema schemaName="TestSchema" alias="ts2" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="CoreCustomAttributes" version="01.00" alias="CoreCA" />
                        <ECEntityClass typeName="Base" modifier="Abstract">
                            <ECProperty propertyName="BaseProp1" typeName="string" />
                        </ECEntityClass>
                        <ECEntityClass typeName="IMixin" modifier="Abstract">
                            <ECCustomAttributes>
                                <IsMixin xlmns="CoreCustomAttributes.01.00">
                                    <AppliesToEntityClass>Base</AppliesToEntityClass>
                                </IsMixin>
                            </ECCustomAttributes>
                         </ECEntityClass>
                        <ECEntityClass typeName="Sub1" modifier="Abstract">
                            <BaseClass>Base</BaseClass>
                            <BaseClass>IMixin</BaseClass>
                            <ECProperty propertyName="Sub1Prop1" typeName="string" />
                        </ECEntityClass>
                        <ECEntityClass typeName="Sub2" modifier="Abstract">
                            <BaseClass>Base</BaseClass>
                            <ECProperty propertyName="Sub2Prop1" typeName="string" />
                        </ECEntityClass>
                        <ECEntityClass typeName="Sub21">
                            <BaseClass>Sub2</BaseClass>
                            <ECProperty propertyName="Sub21Prop1" typeName="string" />
                        </ECEntityClass>
                </ECSchema>)xml"), "updatableviews2.ecdb"));

        ASSERT_FALSE(ecdb.TableExists("ts2_Base")) << "abstract class";
        ASSERT_TRUE(ecdb.TableExists("_ts2_Base")) << "expects updatable view as it has at least one concrete subclass";

        ASSERT_FALSE(ecdb.TableExists("ts2_IMixin")) << "abstract class";
        ASSERT_FALSE(ecdb.TableExists("_ts2_IMixin")) << "mixins never have updatable views as they are not updatable";

        ASSERT_FALSE(ecdb.TableExists("ts2_Sub1")) << "abstract class";
        ASSERT_FALSE(ecdb.TableExists("_ts2_Sub1")) << "no updatable view expected as it doesn't have concrete subclasses";

        ASSERT_FALSE(ecdb.TableExists("ts2_Sub2")) << "abstract class";
        ASSERT_TRUE(ecdb.TableExists("_ts2_Sub2")) << "expects updatable view as it has at least one concrete subclass";

        ASSERT_TRUE(ecdb.TableExists("ts2_Sub21")) << "concrete class";
        ASSERT_FALSE(ecdb.TableExists("_ts2_Sub21")) << "no updatable view expected as it is leaf class";
        }

        {
        ECDb ecdb;
        ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, SchemaItem(
            R"xml(<ECSchema schemaName="TestSchema" alias="ts3" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECEntityClass typeName="Base" modifier="Abstract">
                    <ECProperty propertyName="BaseProp1" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="Sub1" modifier="Abstract">
                    <BaseClass>Base</BaseClass>
                    <ECProperty propertyName="Sub1Prop1" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="Sub2" modifier="Abstract">
                    <BaseClass>Base</BaseClass>
                    <ECProperty propertyName="Sub2Prop1" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="Sub21" modifier="Abstract">
                    <BaseClass>Sub2</BaseClass>
                    <ECProperty propertyName="Sub21Prop1" typeName="string" />
                </ECEntityClass>
                </ECSchema>)xml"), "updatableviews3.ecdb"));

        ASSERT_FALSE(ecdb.TableExists("ts3_Base")) << "No tables expected as all classes are abstract";
        ASSERT_FALSE(ecdb.TableExists("_ts3_Base")) << "No updatable view expected as all classes are abstract";

        ASSERT_FALSE(ecdb.TableExists("ts3_Sub1")) << "No tables expected as all classes are abstract";
        ASSERT_FALSE(ecdb.TableExists("_ts3_Sub1")) << "No updatable view expected as all classes are abstract";

        ASSERT_FALSE(ecdb.TableExists("ts3_Sub2")) << "No tables expected as all classes are abstract";
        ASSERT_FALSE(ecdb.TableExists("_ts3_Sub2")) << "No updatable view expected as all classes are abstract";

        ASSERT_FALSE(ecdb.TableExists("ts3_Sub21")) << "No tables expected as all classes are abstract";
        ASSERT_FALSE(ecdb.TableExists("_ts3_Sub21")) << "No updatable view expected as all classes are abstract";
        }

        {
        ECDb ecdb;
        ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, SchemaItem(
            R"xml(<ECSchema schemaName="TestSchema" alias="ts4" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECEntityClass typeName="Base" modifier="Abstract">
                    <ECProperty propertyName="BaseProp1" typeName="string" />
                </ECEntityClass>
                </ECSchema>)xml"), "updatableviews4.ecdb"));

        ASSERT_FALSE(ecdb.TableExists("ts4_Base")) << "abstract class";
        ASSERT_FALSE(ecdb.TableExists("_ts4_Base")) << "No updatable view expected as abstract class doesn't have subclasses";
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      05/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, ECClassIdColumnVirtuality)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ECClassIdColumnVirtuality.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />

    <ECEntityClass typeName="Base_Abstract_OwnTable" modifier="Abstract">
        <ECProperty propertyName="Prop1" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Sub_Of_Base_Abstract_OwnTable">
        <BaseClass>Base_Abstract_OwnTable</BaseClass>
        <ECProperty propertyName="Prop2" typeName="string" />
    </ECEntityClass>

    <ECEntityClass typeName="Base_OwnTable">
        <ECProperty propertyName="Prop1" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Sub_Of_Base_OwnTable">
        <BaseClass>Base_OwnTable</BaseClass>
        <ECProperty propertyName="Prop2" typeName="string" />
    </ECEntityClass>

    <ECEntityClass typeName="Base_Abstract_NoSubclass_OwnTable" modifier="Abstract">
        <ECProperty propertyName="Prop1" typeName="string" />
    </ECEntityClass>

    <ECEntityClass typeName="Base_Abstract_NoSubclass_TPH" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xlmns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="Prop1" typeName="string" />
    </ECEntityClass>

    <ECEntityClass typeName="Base_Abstract_TPH" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xlmns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="Prop1" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Sub_Of_Base_Abstract_TPH">
        <BaseClass>Base_Abstract_TPH</BaseClass>
        <ECProperty propertyName="Prop2" typeName="string" />
    </ECEntityClass>

    <ECEntityClass typeName="Base_NoSubclass_TPH">
        <ECCustomAttributes>
            <ClassMap xlmns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="Prop1" typeName="string" />
    </ECEntityClass>

    <ECEntityClass typeName="Base_TPH">
        <ECCustomAttributes>
            <ClassMap xlmns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="Prop1" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Sub_Of_Base_TPH">
        <BaseClass>Base_TPH</BaseClass>
        <ECProperty propertyName="Prop2" typeName="string" />
    </ECEntityClass>

    </ECSchema>)xml")));

    ASSERT_FALSE(m_ecdb.TableExists("ts_Base_Abstract_OwnTable")) << "is expected to be virtual";

    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Base_Abstract_OwnTable", "ECInstanceId"), ColumnInfo("ts_Base_Abstract_OwnTable","Id"));
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Base_Abstract_OwnTable", "ECClassId"), ColumnInfo("ts_Base_Abstract_OwnTable","ECClassId", true));
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Base_Abstract_OwnTable", "Prop1"), ColumnInfo("ts_Base_Abstract_OwnTable","Prop1"));

    AssertColumnNames(m_ecdb, "ts_Sub_Of_Base_Abstract_OwnTable", {"Id","Prop1","Prop2"});
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Sub_Of_Base_Abstract_OwnTable", "ECInstanceId"), ColumnInfo("ts_Sub_Of_Base_Abstract_OwnTable","Id"));
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Sub_Of_Base_Abstract_OwnTable", "ECClassId"), ColumnInfo("ts_Sub_Of_Base_Abstract_OwnTable","ECClassId", true));
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Sub_Of_Base_Abstract_OwnTable", "Prop1"), ColumnInfo("ts_Sub_Of_Base_Abstract_OwnTable","Prop1"));
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Sub_Of_Base_Abstract_OwnTable", "Prop2"), ColumnInfo("ts_Sub_Of_Base_Abstract_OwnTable","Prop2"));



    AssertColumnNames(m_ecdb, "ts_Base_OwnTable", {"Id","Prop1"});
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Base_OwnTable", "ECInstanceId"), ColumnInfo("ts_Base_OwnTable","Id"));
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Base_OwnTable", "ECClassId"), ColumnInfo("ts_Base_OwnTable","ECClassId", true));
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Base_OwnTable", "Prop1"), ColumnInfo("ts_Base_OwnTable","Prop1"));

    AssertColumnNames(m_ecdb, "ts_Sub_Of_Base_OwnTable", {"Id","Prop1","Prop2"});
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Sub_Of_Base_OwnTable", "ECInstanceId"), ColumnInfo("ts_Sub_Of_Base_OwnTable","Id"));
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Sub_Of_Base_OwnTable", "ECClassId"), ColumnInfo("ts_Sub_Of_Base_OwnTable","ECClassId", true));
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Sub_Of_Base_OwnTable", "Prop1"), ColumnInfo("ts_Sub_Of_Base_OwnTable","Prop1"));
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Sub_Of_Base_OwnTable", "Prop2"), ColumnInfo("ts_Sub_Of_Base_OwnTable","Prop2"));



    ASSERT_FALSE(m_ecdb.TableExists("ts_Base_Abstract_NoSubclass_OwnTable")) << "is expected to be virtual";
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Base_Abstract_NoSubclass_OwnTable", "ECInstanceId"), ColumnInfo("ts_Base_Abstract_NoSubclass_OwnTable","Id"));
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Base_Abstract_NoSubclass_OwnTable", "ECClassId"), ColumnInfo("ts_Base_Abstract_NoSubclass_OwnTable","ECClassId", true));
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Base_Abstract_NoSubclass_OwnTable", "Prop1"), ColumnInfo("ts_Base_Abstract_NoSubclass_OwnTable","Prop1"));



    AssertColumnNames(m_ecdb, "ts_Base_Abstract_NoSubclass_TPH", {"Id","ECClassId","Prop1"});
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Base_Abstract_NoSubclass_TPH", "ECInstanceId"), ColumnInfo("ts_Base_Abstract_NoSubclass_TPH","Id"));
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Base_Abstract_NoSubclass_TPH", "ECClassId"), ColumnInfo("ts_Base_Abstract_NoSubclass_TPH","ECClassId"));
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Base_Abstract_NoSubclass_TPH", "Prop1"), ColumnInfo("ts_Base_Abstract_NoSubclass_TPH","Prop1"));



    AssertColumnNames(m_ecdb, "ts_Base_Abstract_TPH", {"Id","ECClassId","Prop1","Prop2"});
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Base_Abstract_TPH", "ECInstanceId"), ColumnInfo("ts_Base_Abstract_TPH","Id"));
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Base_Abstract_TPH", "ECClassId"), ColumnInfo("ts_Base_Abstract_TPH","ECClassId"));
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Base_Abstract_TPH", "Prop1"), ColumnInfo("ts_Base_Abstract_TPH","Prop1"));

    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Sub_Of_Base_Abstract_TPH", "ECInstanceId"), ColumnInfo("ts_Base_Abstract_TPH","Id"));
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Sub_Of_Base_Abstract_TPH", "ECClassId"), ColumnInfo("ts_Base_Abstract_TPH","ECClassId"));
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Sub_Of_Base_Abstract_TPH", "Prop1"), ColumnInfo("ts_Base_Abstract_TPH","Prop1"));
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Sub_Of_Base_Abstract_TPH", "Prop2"), ColumnInfo("ts_Base_Abstract_TPH","Prop2"));



    AssertColumnNames(m_ecdb, "ts_Base_NoSubclass_TPH", {"Id","ECClassId","Prop1"});
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Base_NoSubclass_TPH", "ECInstanceId"), ColumnInfo("ts_Base_NoSubclass_TPH","Id"));
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Base_NoSubclass_TPH", "ECClassId"), ColumnInfo("ts_Base_NoSubclass_TPH","ECClassId"));
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Base_NoSubclass_TPH", "Prop1"), ColumnInfo("ts_Base_NoSubclass_TPH","Prop1"));


    AssertColumnNames(m_ecdb, "ts_Base_TPH", {"Id","ECClassId","Prop1","Prop2"});
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Base_TPH", "ECInstanceId"), ColumnInfo("ts_Base_TPH","Id"));
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Base_TPH", "ECClassId"), ColumnInfo("ts_Base_TPH","ECClassId"));
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Base_TPH", "Prop1"), ColumnInfo("ts_Base_TPH","Prop1"));

    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Sub_Of_Base_TPH", "ECInstanceId"), ColumnInfo("ts_Base_TPH","Id"));
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Sub_Of_Base_TPH", "ECClassId"), ColumnInfo("ts_Base_TPH","ECClassId"));
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Sub_Of_Base_TPH", "Prop1"), ColumnInfo("ts_Base_TPH","Prop1"));
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Sub_Of_Base_TPH", "Prop2"), ColumnInfo("ts_Base_TPH","Prop2"));
    }


//---------------------------------------------------------------------------------------
// @bsiMethod                                                   Krischan.Eberle   02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, UnsupportedNavigationPropertyCases)
    {
    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECEntityClass typeName='A'>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "        <ECNavigationProperty propertyName='Bs' relationshipName='AHasB' direction='Forward' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='B'>"
        "        <ECProperty propertyName='Cost' typeName='double' />"
        "        <ECNavigationProperty propertyName='A' relationshipName='AHasB' direction='Backward' />"
        "    </ECEntityClass>"
        "   <ECRelationshipClass typeName='AHasB' strength='Referencing' modifier='Sealed'>"
        "      <Source multiplicity='(0..1)' polymorphic='False' roleLabel='A'>"
        "          <Class class ='A' />"
        "      </Source>"
        "      <Target multiplicity='(0..*)' polymorphic='False' roleLabel='Bs'>"
        "          <Class class ='B' />"
        "      </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>"))) << "NavigationProperty to 'Many' end of relationship is not supported";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECEntityClass typeName='A'>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='B'>"
        "        <ECProperty propertyName='Cost' typeName='double' />"
        "        <ECNavigationProperty propertyName='A' relationshipName='AHasB' direction='Backward' />"
        "    </ECEntityClass>"
        "   <ECRelationshipClass typeName='AHasB' strength='Referencing' modifier='Sealed'>"
        "      <Source multiplicity='(0..*)' polymorphic='False' roleLabel='As'>"
        "          <Class class ='A' />"
        "      </Source>"
        "      <Target multiplicity='(0..*)' polymorphic='False' roleLabel='Bs'>"
        "          <Class class ='B' />"
        "      </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>"))) << "NavigationProperty for link table relationships is not supported";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECEntityClass typeName='A'>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "        <ECNavigationProperty propertyName='B' relationshipName='AHasB' direction='Forward' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='B'>"
        "        <ECProperty propertyName='Cost' typeName='double' />"
        "        <ECNavigationProperty propertyName='A' relationshipName='AHasB' direction='Backward' />"
        "    </ECEntityClass>"
        "   <ECRelationshipClass typeName='AHasB' strength='Referencing' modifier='Sealed'>"
        "      <Source multiplicity='(0..1)' polymorphic='False' roleLabel='A'>"
        "          <Class class ='A' />"
        "      </Source>"
        "      <Target multiplicity='(0..1)' polymorphic='False' roleLabel='B'>"
        "          <Class class ='B' />"
        "      </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>"))) << "NavigationProperty on class which is not on FK end of relationship is not supported";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECEntityClass typeName='A'>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "        <ECNavigationProperty propertyName='B' relationshipName='AHasB' direction='Forward' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='B'>"
        "        <ECProperty propertyName='Cost' typeName='double' />"
        "        <ECNavigationProperty propertyName='A' relationshipName='AHasB' direction='Backward' />"
        "    </ECEntityClass>"
        "   <ECRelationshipClass typeName='AHasB' strength='Referencing' modifier='Sealed' strengthDirection='Backward'>"
        "      <Source multiplicity='(0..1)' polymorphic='False' roleLabel='A'>"
        "          <Class class ='A' />"
        "      </Source>"
        "      <Target multiplicity='(0..1)' polymorphic='False' roleLabel='B'>"
        "          <Class class ='B' />"
        "      </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>"))) << "NavigationProperty on class which is not on FK end of relationship is not supported";
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, OwnTableCATests)
    {
    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='ClassA'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>OwnTable</MapStrategy>"
                                   "            </ClassMap>"
                                   "            <ShareColumns xmlns='ECDbMap.02.00'/>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='Price' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>"))) << "Option ShareColumnsCA can only be used with strategy SharedTable";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='ClassA' modifier='None'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>OwnTable</MapStrategy>"
                                   "            </ClassMap>"
                                   "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='Price' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>"))) << "Option JoinedTablePerDirectSubclass can only be used with strategy SharedTable (applied to subclasses)";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='Class' modifier='None'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>OwnTable</MapStrategy>"
                                   "                <TableName>bla</TableName>"
                                   "            </ClassMap>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='Price' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>"))) << "MapStrategy OwnTable doesn't allow TableName to be set.";

   
    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Base' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>OwnTable</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P0' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>NotMapped</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "NotMapped within Class Hierarchy is supported";

    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='ParentA' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>OwnTable</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='ParentB' modifier='None'>"
        "        <BaseClass>ParentA</BaseClass>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "OwnTable allows a child class to have it's own strategy.";

    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>OwnTable</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Child' modifier='None'>"
        "        <BaseClass>Parent</BaseClass>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P3' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "OwnTable allows a child class to have it's own strategy.";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='ClassA' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>OwnTable</MapStrategy>"
        "                <TableName>bla</TableName>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "Own Table doesn't allows a user Defined table name.";
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, TablePerHierarchyCATests)
    {
    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='Class' modifier='None'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                   "                <TableName>bla</TableName>"
                                   "            </ClassMap>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='Price' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>"))) << "MapStrategy TablePerHierarchy doesn't allow TableName to be set.";


    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='Base' modifier='None'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                   "            </ClassMap>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='P0' typeName='int' />"
                                   "    </ECEntityClass>"
                                   "    <ECEntityClass typeName='Sub' modifier='None'>"
                                   "        <BaseClass>Base</BaseClass>"
                                   "        <ECProperty propertyName='P1' typeName='int' />"
                                   "    </ECEntityClass>"
                                   "    <ECEntityClass typeName='SubSub' modifier='None'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                   "            </ClassMap>"
                                   "        </ECCustomAttributes>"
                                   "        <BaseClass>Sub</BaseClass>"
                                   "        <ECProperty propertyName='P2' typeName='int' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>"))) << "MapStrategy TablePerHierarchy on child class where base has TablePerHierarchy is not supported.";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='Base1' modifier='None'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                   "            </ClassMap>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='P1' typeName='int' />"
                                   "    </ECEntityClass>"
                                   "    <ECEntityClass typeName='Base2' modifier='None'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                   "            </ClassMap>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='P2' typeName='int' />"
                                   "    </ECEntityClass>"
                                   "    <ECEntityClass typeName='Sub' modifier='None'>"
                                   "        <BaseClass>Base1</BaseClass>"
                                   "        <BaseClass>Base2</BaseClass>"
                                   "        <ECProperty propertyName='P3' typeName='int' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>"))) << "Child class has two base classes which both have MapStrategy TablePerHierarchy. This is not expected to be supported.";

    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Base' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P0' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub' modifier='None'>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='SubSub' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>NotMapped</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Sub</BaseClass>"
        "        <ECProperty propertyName='P2' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='SubSubSub' Modifier='None'>"
        "        <BaseClass>SubSub</BaseClass>"
        "        <ECProperty propertyName='P3' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "MapStrategy NotMapped on child class where base has TablePerHierarchy is supported.";

    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TeststructClassInPolymorphicSharedTable' nameSpacePrefix='tph' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='BaseClass' modifier='Abstract'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                   "            </ClassMap>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='p1' typeName='string' />"
                                   "    </ECEntityClass>"
                                   "    <ECEntityClass typeName='AbstractClass' modifier='Abstract'>"
                                   "        <BaseClass>BaseClass</BaseClass>"
                                   "        <ECProperty propertyName='p2' typeName='string' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>"))) << "Abstract Class in a Hierarchy with TablePerHierarchy CA is expected to be supported.";

    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Base' modifier='None'>"
        "        <ECProperty propertyName='P0' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub' modifier='None'>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='SubSub' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Sub</BaseClass>"
        "        <ECProperty propertyName='P2' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "TablePerHierarchy within Class Hierarchy is expected to be supported where Root class has default MapStrategy";

    {
    ECDb ecdb;
    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                          "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                          "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                          "    <ECEntityClass typeName='Parent' modifier='None'>"
                          "        <ECCustomAttributes>"
                          "            <ClassMap xmlns='ECDbMap.02.00'>"
                          "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                          "            </ClassMap>"
                          "        </ECCustomAttributes>"
                          "        <ECProperty propertyName='P1' typeName='int' />"
                          "    </ECEntityClass>"
                          "    <ECEntityClass typeName='Child1'>"
                          "        <BaseClass>Parent</BaseClass>"
                          "        <ECProperty propertyName='Price' typeName='double' />"
                          "    </ECEntityClass>"
                          "    <ECEntityClass typeName='Child2'>"
                          "        <BaseClass>Parent</BaseClass>"
                          "        <ECProperty propertyName='Price' typeName='double' />"
                          "    </ECEntityClass>"
                          "</ECSchema>"), "tableperhierarchycatests.ecdb"));

    Utf8CP tableName = "ts_Parent";
    bvector<Utf8String> columnNames;
    ecdb.GetColumns(columnNames, tableName);
    ASSERT_EQ(4, columnNames.size()) << "Table " << tableName;
    ASSERT_TRUE(std::find(columnNames.begin(), columnNames.end(), "Id") != columnNames.end()) << "Table " << tableName;
    ASSERT_TRUE(std::find(columnNames.begin(), columnNames.end(), "ECClassId") != columnNames.end()) << "Table " << tableName;
    ASSERT_TRUE(std::find(columnNames.begin(), columnNames.end(), "P1") != columnNames.end()) << "Table " << tableName;
    //As property type is the same, both Child1.Price and Child2.Price share the same column
    ASSERT_TRUE(std::find(columnNames.begin(), columnNames.end(), "Price") != columnNames.end()) << "Table " << tableName;
    }
    }


//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, ExistingTableCATests)
    {
    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='Class' modifier='None'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>ExistingTable</MapStrategy>"
                                   "            </ClassMap>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='Price' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>"))) << "MapStrategy ExistingTable expects TableName to be set";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='ClassA' modifier='None'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>ExistingTable</MapStrategy>"
                                   "            </ClassMap>"
                                   "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='Price' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>"))) << "Option JoinedTablePerDirectSubclass can only be used with strategy SharedTable (applied to subclasses)";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='Class' modifier='None'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>ExistingTable</MapStrategy>"
                                   "                <TableName>idontexist</TableName>"
                                   "            </ClassMap>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='Price' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>"))) << "MapStrategy ExistingTable expects table specified by TableName to preexist";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='BePropInfo' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>ExistingTable</MapStrategy>"
        "                <TableName>be_Prop</TableName>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Namespace' typeName='string' />"
        "        <ECProperty propertyName='PropNotMappedToAnExistingCol' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "Cannot add new column to existing table";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbFileInfo' version='02.00' prefix='ecdbf' />"
        "    <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "   <ECRelationshipClass typeName='ParentHasEmbeddedFile' strength='Referencing' modifier='Sealed'>"
        "      <Source cardinality='(0,1)' polymorphic='False'>"
        "          <Class class ='Parent' />"
        "      </Source>"
        "      <Target cardinality='(0,N)' polymorphic='False'>"
        "          <Class class ='ecdb.EmbeddedFileInfo' />"
        "      </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>"))) << "Cannot add FK column to existing table";

 

    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("existingtablecatests.ecdb"));

    bmap<Utf8String, bool> testDataset;
    testDataset["SELECT * FROM ecdbf.ExternalFileInfo"] = true;
    testDataset["INSERT INTO ecdbf.ExternalFileInfo(Name) VALUES('Foo')"] = true;
    testDataset["UPDATE ecdbf.ExternalFileInfo SET Name='Foo' WHERE ECInstanceId=1"] = true;
    testDataset["DELETE FROM ecdbf.ExternalFileInfo"] = true;

    testDataset["SELECT * FROM ecdbf.EmbeddedFileInfo"] = true;
    testDataset["INSERT INTO ecdbf.EmbeddedFileInfo(Name) VALUES('Foo')"] = false;
    testDataset["UPDATE ecdbf.EmbeddedFileInfo SET Name='Foo' WHERE ECInstanceId=1"] = false;
    testDataset["DELETE FROM ecdbf.EmbeddedFileInfo"] = false;
    //polymorphic update/delete where subclass maps to existing table
    testDataset["UPDATE ecdbf.FileInfo SET Name='Foo' WHERE ECInstanceId=1"] = false;
    testDataset["DELETE FROM ecdbf.FileInfo"] = false;

    testDataset["SELECT * FROM meta.ECClassDef"] = true;
    testDataset["INSERT INTO meta.ECClassDef(SchemaId, Name, DisplayLabel) VALUES(1, 'Foo', 'Foo')"] = false;
    testDataset["UPDATE meta.ECClassDef SET DisplayLabel='Foo' WHERE ECInstanceId=1"] = false;
    testDataset["DELETE FROM meta.ECClassDef"] = false;

    for (bpair<Utf8String, bool> const& testItem : testDataset)
        {
        const bool expectedSuccess = testItem.second;
        Utf8CP ecsql = testItem.first.c_str();
        ECSqlStatement stmt;
        const ECSqlStatus stat = stmt.Prepare(m_ecdb, ecsql);

        if (expectedSuccess)
            ASSERT_EQ(ECSqlStatus::Success, stat) << ecsql;
        else
            ASSERT_EQ(ECSqlStatus::InvalidECSql, stat) << ecsql;
        }

    ECClassCP testClass = m_ecdb.Schemas().GetClass("ECDbMeta", "ECClassDef");
    ASSERT_TRUE(testClass != nullptr);

    {
    ECInstanceInserter inserter(m_ecdb, *testClass, nullptr);
    ASSERT_FALSE(inserter.IsValid());
    ECInstanceUpdater updater(m_ecdb, *testClass, nullptr);
    ASSERT_FALSE(updater.IsValid());
    ECInstanceDeleter deleter(m_ecdb, *testClass, nullptr);
    ASSERT_FALSE(deleter.IsValid());
    }

    {
    JsonInserter inserter(m_ecdb, *testClass, nullptr);
    ASSERT_FALSE(inserter.IsValid());
    JsonUpdater updater(m_ecdb, *testClass, nullptr);
    ASSERT_FALSE(updater.IsValid());
    JsonDeleter deleter(m_ecdb, *testClass, nullptr);
    ASSERT_FALSE(deleter.IsValid());
    }
    }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                           Maha Nasir                         03/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, RelationshipMappingTests)
    {
    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECEntityClass typeName='A'>"
        "        <ECProperty propertyName='Price' typeName='double'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='B'>"
        "        <ECProperty propertyName='Name' typeName='string'/>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='AHasB' modifier='None' strength='referencing' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>OwnTable</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "       <Source multiplicity='(0,1)' polymorphic='True' roleLabel='A'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0,N)' polymorphic='True' roleLabel='B'>"
        "           <Class class='B' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='AHasB2' modifier='None' strength='referencing' >"
        "       <BaseClass>AHasB</BaseClass>"
        "       <Source multiplicity='(0,1)' polymorphic='True' roleLabel='A'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0,N)' polymorphic='True' roleLabel='B'>"
        "           <Class class='B' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>"))) << "BaseRelationshipClass having OwnTable mapping strategy is not supported in ECRelationshipClassHeirarchy.";

    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECEntityClass typeName='A'>"
        "        <ECProperty propertyName='Price' typeName='double'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='B'>"
        "        <ECProperty propertyName='Name' typeName='string'/>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='AHasB' modifier='None' strength='referencing' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "       <Source multiplicity='(0,1)' polymorphic='True' roleLabel='A'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0,N)' polymorphic='True' roleLabel='B'>"
        "           <Class class='B' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='AHasB2' modifier='None' strength='referencing' >"
        "       <BaseClass>AHasB</BaseClass>"
        "       <Source multiplicity='(0,1)' polymorphic='True' roleLabel='A'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0,N)' polymorphic='True' roleLabel='B'>"
        "           <Class class='B' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>"))) << "BaseRelationshipClass having TablePerHeirarchy mapping strategy is expected to be supported in ECRelationshipClassHeirarchy.";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECEntityClass typeName='A' modifier='Abstract'>"
        "        <ECProperty propertyName='Price' typeName='double'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='B'>"
        "        <ECProperty propertyName='Name' typeName='string'/>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='AHasB' modifier='None' strength='referencing' >"
        "       <Source multiplicity='(0,1)' polymorphic='True' roleLabel='A'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0,N)' polymorphic='True' roleLabel='B'>"
        "           <Class class='B' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>"))) << "Source or target constraint classes are abstract without subclasses. Apply the MapStrategy 'TablePerHierarchy' to the abstract constraint class";
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, NotMappedCATests)
    {
    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                        "    <ECEntityClass typeName='Class' modifier='None'>"
                                        "        <ECCustomAttributes>"
                                        "            <ClassMap xmlns='ECDbMap.02.00'>"
                                        "                <MapStrategy>NotMapped</MapStrategy>"
                                        "                <TableName>bla</TableName>"
                                        "            </ClassMap>"
                                        "        </ECCustomAttributes>"
                                        "        <ECProperty propertyName='Price' typeName='double' />"
                                        "    </ECEntityClass>"
                                        "</ECSchema>"))) << "MapStrategy NotMapped doesn't allow TableName to be set.";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Base' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>NotMapped</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P0' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "Conflicting MapStrategy TablePerHierarchy within Class Hierarchy not supported where Root has Strategy NotMapped";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Base' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>NotMapped</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P0' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>OwnTable</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "Conflicting mapStrategies OwnTable within Class Hierarchy not supported where Root has MapStrategy NotMapped";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='A' modifier='None'>"
        "        <ECProperty propertyName='AProp' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='B' modifier='None'>"
        "        <ECProperty propertyName='BProp' typeName='int' />"
        "        <ECNavigationProperty propertyName='A' relationshipName='Rel' direction='Backward'/>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='referencing'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "               <MapStrategy>OwnTable</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "       <Source cardinality='(0,1)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' polymorphic='True'>"
        "           <Class class='B' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>"))) << "ECRelationshipClass with FK mapping must not have a ClassMap CA unless it has MapStrategy NotMapped";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                         "<ECSchema schemaName='Test' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                         "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                                         "    <ECEntityClass typeName='A' modifier='None'>"
                                         "        <ECProperty propertyName='AProp' typeName='int' />"
                                         "    </ECEntityClass>"
                                         "    <ECEntityClass typeName='B' modifier='None'>"
                                         "        <ECProperty propertyName='BProp' typeName='int' />"
                                         "        <ECNavigationProperty propertyName='A' relationshipName='Rel' direction='Backward'/>"
                                         "    </ECEntityClass>"
                                         "    <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='referencing'>"
                                         "        <ECCustomAttributes>"
                                         "            <ClassMap xmlns='ECDbMap.02.00'>"
                                         "                <MapStrategy>NotMapped</MapStrategy>"
                                         "            </ClassMap>"
                                         "        </ECCustomAttributes>"
                                         "       <Source multiplicity='(0..1)' polymorphic='True' roleLabel='references'>"
                                         "           <Class class='A' />"
                                         "       </Source>"
                                         "       <Target multiplicity='(0..*)' polymorphic='True' roleLabel='is referenced by'>"
                                         "           <Class class='B' />"
                                         "       </Target>"
                                         "     </ECRelationshipClass>"
                                         "</ECSchema>"))) << "ECRelationshipClass with FK mapping can have a ClassMap CA with MapStrategy NotMapped only if nav prop class has it too";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                         "<ECSchema schemaName='Test' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                         "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                                         "    <ECEntityClass typeName='A' modifier='None'>"
                                         "        <ECProperty propertyName='AProp' typeName='int' />"
                                         "    </ECEntityClass>"
                                         "    <ECEntityClass typeName='B' modifier='None'>"
                                         "        <ECCustomAttributes>"
                                         "            <ClassMap xmlns='ECDbMap.02.00'>"
                                         "                <MapStrategy>NotMapped</MapStrategy>"
                                         "            </ClassMap>"
                                         "        </ECCustomAttributes>"
                                         "        <ECProperty propertyName='BProp' typeName='int' />"
                                         "        <ECNavigationProperty propertyName='A' relationshipName='Rel' direction='Backward'/>"
                                         "    </ECEntityClass>"
                                         "    <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='referencing'>"
                                         "       <Source multiplicity='(0..1)' polymorphic='True' roleLabel='references'>"
                                         "           <Class class='A' />"
                                         "       </Source>"
                                         "       <Target multiplicity='(0..*)' polymorphic='True' roleLabel='is referenced by'>"
                                         "           <Class class='B' />"
                                         "       </Target>"
                                         "     </ECRelationshipClass>"
                                         "</ECSchema>"))) << "ECRelationshipClass with FK mapping if constraint class has NotMapped strategy";


    {
    ECDb ecdb;
    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Base' modifier='None'>"
        "        <ECProperty propertyName='P0' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub' modifier='None'>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='SubSub' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>NotMapped</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Sub</BaseClass>"
        "        <ECProperty propertyName='P2' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='SubSubSub' modifier='None'>"
        "        <BaseClass>SubSub</BaseClass>"
        "        <ECProperty propertyName='P3' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>"), "notmappedcatests.ecdb"));

    MapStrategyInfo mapStrategy;
    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, ecdb.Schemas().GetClass("Test", "Sub")->GetId()));
    ASSERT_EQ(MapStrategyInfo::Strategy::OwnTable, mapStrategy.m_strategy);

    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, ecdb.Schemas().GetClass("Test", "SubSub")->GetId()));
    ASSERT_EQ(MapStrategyInfo::Strategy::NotMapped, mapStrategy.m_strategy);

    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, ecdb.Schemas().GetClass("Test", "SubSubSub")->GetId()));
    ASSERT_EQ(MapStrategyInfo::Strategy::NotMapped, mapStrategy.m_strategy);
    }

    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Base' modifier='None'>"
        "        <ECProperty propertyName='P0' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub' modifier='None'>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='SubSub' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>NotMapped</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Sub</BaseClass>"
        "        <ECProperty propertyName='P2' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>"), "notmappedcatests.ecdb")) << "NotMapped within Class Hierarchy is expected to be supported where Root class has default MapStrategy";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                                          "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                                          "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                                          "    <ECEntityClass typeName='Base' modifier='None'>"
                                                          "        <ECCustomAttributes>"
                                                          "            <ClassMap xmlns='ECDbMap.02.00'>"
                                                          "                <MapStrategy>NotMapped</MapStrategy>"
                                                          "            </ClassMap>"
                                                          "        </ECCustomAttributes>"
                                                          "        <ECProperty propertyName='P0' typeName='int' />"
                                                          "    </ECEntityClass>"
                                                          "    <ECEntityClass typeName='Sub' modifier='None'>"
                                                          "        <BaseClass>Base</BaseClass>"
                                                          "        <ECProperty propertyName='P1' typeName='int' />"
                                                          "    </ECEntityClass>"
                                                          "    <ECEntityClass typeName='SubSub' modifier='None'>"
                                                          "        <ECCustomAttributes>"
                                                          "            <ClassMap xmlns='ECDbMap.02.00'>"
                                                          "                <MapStrategy>NotMapped</MapStrategy>"
                                                          "            </ClassMap>"
                                                          "        </ECCustomAttributes>"
                                                          "        <BaseClass>Sub</BaseClass>"
                                                          "        <ECProperty propertyName='P2' typeName='int' />"
                                                          "    </ECEntityClass>"
                                                          "    <ECEntityClass typeName='SubSubSub' modifier='None'>"
                                                          "        <BaseClass>SubSub</BaseClass>"
                                                          "        <ECProperty propertyName='P3' typeName='int' />"
                                                          "    </ECEntityClass>"
                                                          "</ECSchema>"))) << "NotMapped cannot be set on subclass if base class already defined it";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                                          "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                                          "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                                          "    <ECEntityClass typeName='Base' modifier='None'>"
                                                          "        <ECCustomAttributes>"
                                                          "            <ClassMap xmlns='ECDbMap.02.00'>"
                                                          "                <MapStrategy>NotMapped</MapStrategy>"
                                                          "            </ClassMap>"
                                                          "        </ECCustomAttributes>"
                                                          "        <ECProperty propertyName='P0' typeName='int' />"
                                                          "    </ECEntityClass>"
                                                          "    <ECEntityClass typeName='Sub' modifier='None'>"
                                                          "        <ECCustomAttributes>"
                                                          "            <ClassMap xmlns='ECDbMap.02.00'>"
                                                          "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                                          "            </ClassMap>"
                                                          "        </ECCustomAttributes>"
                                                          "        <BaseClass>Base</BaseClass>"
                                                          "        <ECProperty propertyName='P1' typeName='int' />"
                                                          "    </ECEntityClass>"
                                                          "    <ECEntityClass typeName='SubSub' modifier='None'>"
                                                          "        <BaseClass>Sub</BaseClass>"
                                                          "        <ECProperty propertyName='P2' typeName='int' />"
                                                          "    </ECEntityClass>"
                                                          "</ECSchema>"))) << "TPH within Class Hierarchy is not supported where Root has Strategy NotMapped";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                                  "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                                  "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                                  "    <ECEntityClass typeName='Base' modifier='None'>"
                                                  "        <ECCustomAttributes>"
                                                  "            <ClassMap xmlns='ECDbMap.02.00'>"
                                                  "                <MapStrategy>NotMapped</MapStrategy>"
                                                  "            </ClassMap>"
                                                  "        </ECCustomAttributes>"
                                                  "        <ECProperty propertyName='P0' typeName='int' />"
                                                  "    </ECEntityClass>"
                                                  "    <ECEntityClass typeName='Sub' modifier='None'>"
                                                  "        <ECCustomAttributes>"
                                                  "            <ClassMap xmlns='ECDbMap.02.00'>"
                                                  "                <MapStrategy>OwnTable</MapStrategy>"
                                                  "            </ClassMap>"
                                                  "        </ECCustomAttributes>"
                                                  "        <BaseClass>Base</BaseClass>"
                                                  "        <ECProperty propertyName='P1' typeName='int' />"
                                                  "    </ECEntityClass>"
                                                  "    <ECEntityClass typeName='SubSub' modifier='None'>"
                                                  "        <BaseClass>Sub</BaseClass>"
                                                  "        <ECProperty propertyName='P2' typeName='int' />"
                                                  "    </ECEntityClass>"
                                                  "</ECSchema>"))) << "OwnTable within Class Hierarchy is not supported where Root has Strategy NotMapped";

    {
    ECDb ecdb;
    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                                  "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                                  "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                                  "    <ECEntityClass typeName='Base' modifier='None'>"
                                                  "        <ECCustomAttributes>"
                                                  "            <ClassMap xmlns='ECDbMap.02.00'>"
                                                  "                <MapStrategy>NotMapped</MapStrategy>"
                                                  "            </ClassMap>"
                                                  "        </ECCustomAttributes>"
                                                  "        <ECProperty propertyName='P0' typeName='int' />"
                                                  "    </ECEntityClass>"
                                                  "    <ECEntityClass typeName='Sub' modifier='None'>"
                                                  "        <BaseClass>Base</BaseClass>"
                                                  "        <ECProperty propertyName='P1' typeName='int' />"
                                                  "    </ECEntityClass>"
                                                  "    <ECEntityClass typeName='SubSub' modifier='None'>"
                                                  "        <BaseClass>Sub</BaseClass>"
                                                  "        <ECProperty propertyName='P2' typeName='int' />"
                                                  "    </ECEntityClass>"
                                                  "    <ECEntityClass typeName='SubSubSub' modifier='None'>"
                                                  "        <BaseClass>SubSub</BaseClass>"
                                                  "        <ECProperty propertyName='P3' typeName='int' />"
                                                  "    </ECEntityClass>"
                                                  "</ECSchema>"), "notmappedcatests.ecdb")) << "NotMapped applied to non-sealed classes means to apply to subclasses, too";

    MapStrategyInfo mapStrategy;
    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, ecdb.Schemas().GetClass("Test", "Base")->GetId()));
    ASSERT_EQ(MapStrategyInfo::Strategy::NotMapped, mapStrategy.m_strategy);

    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, ecdb.Schemas().GetClass("Test", "Sub")->GetId()));
    ASSERT_EQ(MapStrategyInfo::Strategy::NotMapped, mapStrategy.m_strategy);

    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, ecdb.Schemas().GetClass("Test", "SubSub")->GetId()));
    ASSERT_EQ(MapStrategyInfo::Strategy::NotMapped, mapStrategy.m_strategy);

    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, ecdb.Schemas().GetClass("Test", "SubSubSub")->GetId()));
    ASSERT_EQ(MapStrategyInfo::Strategy::NotMapped, mapStrategy.m_strategy);

    }

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                                                "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                                                "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                                                "    <ECEntityClass typeName='Base' modifier='None'>"
                                                                "        <ECCustomAttributes>"
                                                                "            <ClassMap xmlns='ECDbMap.02.00'>"
                                                                "                <MapStrategy>NotMapped</MapStrategy>"
                                                                "            </ClassMap>"
                                                                "        </ECCustomAttributes>"
                                                                "        <ECProperty propertyName='P0' typeName='int' />"
                                                                "    </ECEntityClass>"
                                                                "    <ECEntityClass typeName='Sub' modifier='None'>"
                                                                "        <ECCustomAttributes>"
                                                                "            <ClassMap xmlns='ECDbMap.02.00'>"
                                                                "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                                                "            </ClassMap>"
                                                                "        </ECCustomAttributes>"
                                                                "        <BaseClass>Base</BaseClass>"
                                                                "        <ECProperty propertyName='P1' typeName='int' />"
                                                                "    </ECEntityClass>"
                                                                "    <ECEntityClass typeName='SubSub' modifier='None'>"
                                                                "        <BaseClass>Sub</BaseClass>"
                                                                "        <ECProperty propertyName='P2' typeName='int' />"
                                                                "    </ECEntityClass>"
                                                                "</ECSchema>"))) << "TablePerHierarchy cannot be applied to subclass if base class has NotMapped";

    {
    ECDb ecdb;
    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                                  "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                                  "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                                  "    <ECEntityClass typeName='Base' modifier='Abstract'>"
                                                  "        <ECCustomAttributes>"
                                                  "            <ClassMap xmlns='ECDbMap.02.00'>"
                                                  "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                                  "            </ClassMap>"
                                                  "        </ECCustomAttributes>"
                                                  "        <ECProperty propertyName='P0' typeName='int' />"
                                                  "    </ECEntityClass>"
                                                  "    <ECEntityClass typeName='Sub' modifier='Sealed'>"
                                                  "        <ECCustomAttributes>"
                                                  "            <ClassMap xmlns='ECDbMap.02.00'>"
                                                  "                <MapStrategy>NotMapped</MapStrategy>"
                                                  "            </ClassMap>"
                                                  "        </ECCustomAttributes>"
                                                  "        <BaseClass>Base</BaseClass>"
                                                  "        <ECProperty propertyName='P1' typeName='int' />"
                                                  "    </ECEntityClass>"
                                                  "</ECSchema>"), "notmappedcatests.ecdb"));

    MapStrategyInfo mapStrategy;
    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, ecdb.Schemas().GetClass("Test", "Base")->GetId()));
    ASSERT_EQ((int) MapStrategyInfo::Strategy::TablePerHierarchy, (int) mapStrategy.m_strategy);

    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, ecdb.Schemas().GetClass("Test", "Sub")->GetId()));
    ASSERT_EQ((int) MapStrategyInfo::Strategy::NotMapped, (int) mapStrategy.m_strategy);
    }

    {
    ECDb ecdb;
    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='Test' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                "    <ECEntityClass typeName='A' modifier='None'>"
                "        <ECProperty propertyName='AProp' typeName='int' />"
                "    </ECEntityClass>"
                "    <ECEntityClass typeName='B' modifier='None'>"
                "        <ECCustomAttributes>"
                "            <ClassMap xmlns='ECDbMap.02.00'>"
                "                <MapStrategy>NotMapped</MapStrategy>"
                "            </ClassMap>"
                "        </ECCustomAttributes>"
                "        <ECProperty propertyName='BProp' typeName='int' />"
                "        <ECNavigationProperty propertyName='A' relationshipName='Rel' direction='Backward'/>"
                "    </ECEntityClass>"
                "    <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='referencing'>"
                "        <ECCustomAttributes>"
                "            <ClassMap xmlns='ECDbMap.02.00'>"
                "                <MapStrategy>NotMapped</MapStrategy>"
                "            </ClassMap>"
                "        </ECCustomAttributes>"
                "       <Source multiplicity='(0..1)' polymorphic='True' roleLabel='references'>"
                "           <Class class='A' />"
                "       </Source>"
                "       <Target multiplicity='(0..*)' polymorphic='True' roleLabel='is referenced by'>"
                "           <Class class='B' />"
                "       </Target>"
                "     </ECRelationshipClass>"
                "</ECSchema>"), "notmappedcatests.ecdb")) << "Both rel and nav prop class have NotMapped strategy";

    MapStrategyInfo mapStrategy;
    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, ecdb.Schemas().GetClass("Test", "B")->GetId()));
    ASSERT_EQ((int) MapStrategyInfo::Strategy::NotMapped, (int) mapStrategy.m_strategy);

    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, ecdb.Schemas().GetClass("Test", "Rel")->GetId()));
    ASSERT_EQ((int) MapStrategyInfo::Strategy::NotMapped, (int) mapStrategy.m_strategy);
    }

    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, JoinedTableCATests)
    {
    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='ClassA' modifier='None'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                   "            </ClassMap>"
                                   "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='Price' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>"))) << "Option JoinedTablePerDirectSubclass is expected to work with strategy TablePerHierarchy";

    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='ClassA' modifier='None'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                   "            </ClassMap>"
                                   "            <ShareColumns xmlns='ECDbMap.02.00'>"
                                   "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
                                   "            </ShareColumns>"
                                   "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='Price' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>"))) << "Combination of options JoinedTablePerDirectSubclass and ShareColumns(applytosubclassesonly) is expected to work with strategy TablePerHierarchy";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='ClassA' modifier='None'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                   "            </ClassMap>"
                                   "            <ShareColumns xmlns='ECDbMap.02.00'/>"
                                   "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='Price' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>"))) << "Combination of options JoinedTablePerDirectSubclass and ShareColumns on same class is expected to fail with strategy TablePerHierarchy";

    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='ClassA' modifier='None'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                   "            </ClassMap>"
                                   "            <ShareColumns xmlns='ECDbMap.02.00'>"
                                   "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
                                   "            </ShareColumns>"
                                   "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='Price' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>"))) << "Combination of options ShareColumns and JoinedTablePerDirectSubclass is expected to work with strategy TablePerHierarchy";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='ClassA' modifier='None'>"
                                   "        <ECCustomAttributes>"
                                   "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='Price' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>"))) << "JoinedTablePerDirectSubclass cannot be applied without MapStrategy TablePerHierarchy";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='ClassA' modifier='None'>"
                                   "        <ECCustomAttributes>"
                                   "            <ShareColumns xmlns='ECDbMap.02.00'/>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='Price' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>"))) << "ShareColumns cannot be applied without MapStrategy TablePerHierarchy";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
                          {"<?xml version='1.0' encoding='utf-8'?>"
                          "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                          "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                          "    <ECEntityClass typeName='ClassA' modifier='None'>"
                          "        <ECCustomAttributes>"
                          "            <ClassMap xmlns='ECDbMap.02.00'>"
                          "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                          "            </ClassMap>"
                          "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                          "        </ECCustomAttributes>"
                          "        <ECProperty propertyName='Price' typeName='double' />"
                          "    </ECEntityClass>"
                          "    <ECEntityClass typeName='ClassB' modifier='None'>"
                          "        <BaseClass>ClassA</BaseClass>"
                          "        <ECProperty propertyName='Cost' typeName='double' />"
                          "    </ECEntityClass>"
                          "</ECSchema>",

                        "<?xml version='1.0' encoding='utf-8'?>"
                          "<ECSchema schemaName='TestSchema2' nameSpacePrefix='ts2' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                          "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                          "    <ECSchemaReference name='TestSchema' version='01.00' prefix='ts' />"
                          "    <ECEntityClass typeName='ClassC' modifier='None'>"
                          "        <ECCustomAttributes>"
                          "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                          "        </ECCustomAttributes>"
                          "        <BaseClass>ts:ClassB</BaseClass>"
                          "        <ECProperty propertyName='Name' typeName='string' />"
                          "    </ECEntityClass>"
                          "</ECSchema>"}))) << "JoinedTablePerDirectSubclass cannot be applied if it was already specified higher up in the hierarchy";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='ClassA' modifier='None'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>OwnTable</MapStrategy>"
                                   "            </ClassMap>"
                                   "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='Price' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>"))) << "Option JoinedTablePerDirectSubclass can only be used with strategy TablePerHierarchy";

    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Base' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P0' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub' modifier='None'>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='SubSub' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Sub</BaseClass>"
        "        <ECProperty propertyName='P2' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='SubSubSub' modifier='None'>"
        "        <BaseClass>SubSub</BaseClass>"
        "        <ECProperty propertyName='P3' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "Option JoinedTablePerDirectSubclass can be applied to subclass where base has TablePerHierarchy.";

    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='ClassA' modifier='None'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                   "            </ClassMap>"
                                   "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                                   "        </ECCustomAttributes>"
                                   "    </ECEntityClass>"
                                   "    <ECEntityClass typeName='ClassB' modifier='None'>"
                                   "        <BaseClass>ClassA</BaseClass>"
                                   "        <ECProperty propertyName='Cost' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>"))) << "JoinedTable on a class without any property is expected to be successful";

    }

//---------------------------------------------------------------------------------------
// @bsiMethod                              Maha Nasir                         03/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, ECPropertyCATests)
    {
    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECEntityClass typeName='A'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double'>"
        "        <ECCustomAttributes>"
        "            <PropertyMap xmlns='ECDbMap.02.00'>"
        "                <ColumnName>bla</ColumnName>"
        "            </PropertyMap>"
        "        </ECCustomAttributes>"
        "        </ECProperty>"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "ColumnName can only be set for ExistingTable map strategy.";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECEntityClass typeName='A'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double'>"
        "        <ECCustomAttributes>"
        "            <PropertyMap xmlns='ECDbMap.02.00'>"
        "                <Collation>bla</Collation>"
        "            </PropertyMap>"
        "        </ECCustomAttributes>"
        "        </ECProperty>"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "ECProperty has invalid value for Collation.";
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Krischan.Eberle             03/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, IdNameCollisions)
    {
    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(R"xml(
                            <ECSchema schemaName="TestSchema" alias="ts1" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                              <ECEntityClass typeName="Foo">
                                <ECProperty propertyName="ECInstanceId" typeName="string" />
                              </ECEntityClass>
                            </ECSchema>)xml"))) << "ECInstanceId is a system property";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(R"xml(
                            <ECSchema schemaName="TestSchema" alias="ts2" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                              <ECEntityClass typeName="Foo">
                                <ECProperty propertyName="Id" typeName="string" />
                              </ECEntityClass>
                            </ECSchema>)xml"))) << "Id is an alias for the ECInstanceId system property";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(R"xml(
                            <ECSchema schemaName="TestSchema" alias="ts3" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                              <ECEntityClass typeName="A">
                                <ECProperty propertyName="Name" typeName="string" />
                              </ECEntityClass>
                              <ECEntityClass typeName="B">
                                <ECProperty propertyName="Name" typeName="string" />
                              </ECEntityClass>
                              <ECRelationshipClass typeName="Rel">
                                    <Source cardinality="(0..*)" polymorphic="True">
                                       <Class class="A" />
                                    </Source>
                                    <Target cardinality="(0..*)" polymorphic="True">
                                       <Class class="B"/>
                                     </Target>
                                <ECProperty propertyName="SourceECInstanceId" typeName="string" />
                              </ECRelationshipClass>
                            </ECSchema>)xml"))) << "SourceECInstanceId is a system property and therefore a reserved name";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(R"xml(
                            <ECSchema schemaName="TestSchema" alias="ts4" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                              <ECEntityClass typeName="A">
                                <ECProperty propertyName="Name" typeName="string" />
                              </ECEntityClass>
                              <ECEntityClass typeName="B">
                                <ECProperty propertyName="Name" typeName="string" />
                              </ECEntityClass>
                              <ECRelationshipClass typeName="Rel">
                                    <Source cardinality="(0..*)" polymorphic="True">
                                       <Class class="A" />
                                    </Source>
                                    <Target cardinality="(0..*)" polymorphic="True">
                                       <Class class="B"/>
                                     </Target>
                                <ECProperty propertyName="SourceId" typeName="string" />
                              </ECRelationshipClass>
                            </ECSchema>)xml"))) << "SourceId is an alias for the SourceECInstanceId system property and therefore a reserved name";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(R"xml(
            <ECSchema schemaName="TestSchema" alias="ts5" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECEntityClass typeName="A">
                <ECProperty propertyName="Name" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="B">
                <ECProperty propertyName="Name" typeName="string" />
                </ECEntityClass>
                <ECRelationshipClass typeName="Rel">
                    <Source cardinality="(0..*)" polymorphic="True">
                        <Class class="A" />
                    </Source>
                    <Target cardinality="(0..*)" polymorphic="True">
                        <Class class="B"/>
                        </Target>
                <ECProperty propertyName="TargetECInstanceId" typeName="string" />
                </ECRelationshipClass>
            </ECSchema>)xml"))) << "TargetECInstanceId is a system property and therefore a reserved name";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(R"xml(
            <ECSchema schemaName="TestSchema" alias="ts6" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECEntityClass typeName="A">
                <ECProperty propertyName="Name" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="B">
                <ECProperty propertyName="Name" typeName="string" />
                </ECEntityClass>
                <ECRelationshipClass typeName="Rel">
                    <Source cardinality="(0..*)" polymorphic="True">
                        <Class class="A" />
                    </Source>
                    <Target cardinality="(0..*)" polymorphic="True">
                        <Class class="B"/>
                        </Target>
                <ECProperty propertyName="TargetId" typeName="string" />
                </ECRelationshipClass>
            </ECSchema>)xml"))) << "TargetId is the alias for the TargetECInstanceId system property and therefore a reserved name";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(R"xml(
                            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                              <ECEntityClass typeName="Foo">
                                <ECProperty propertyName="ECClassId" typeName="string" />
                              </ECEntityClass>
                            </ECSchema>)xml"))) << "ECClassId is a system property";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(R"xml(
                            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                               <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                              <ECEntityClass typeName="A">
                                <ECProperty propertyName="Name" typeName="string" />
                              </ECEntityClass>
                              <ECEntityClass typeName="B">
                                <ECProperty propertyName="Name" typeName="string" />
                              </ECEntityClass>
                              <ECRelationshipClass typeName="Rel" modifier="Sealed">
                                    <Source multiplicity="(0..*)" polymorphic="True" roleLabel="A">
                                       <Class class="A" />
                                    </Source>
                                    <Target multiplicity="(0..*)" polymorphic="True" roleLabel="B">
                                       <Class class="B"/>
                                     </Target>
                                <ECProperty propertyName="SourceECClassId" typeName="string" />
                              </ECRelationshipClass>
                            </ECSchema>)xml"))) << "SourceECClassId is a system property and therefore a reserved name";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(R"xml(
                            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                               <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                              <ECEntityClass typeName="A">
                                <ECProperty propertyName="Name" typeName="string" />
                              </ECEntityClass>
                              <ECEntityClass typeName="B">
                                <ECProperty propertyName="Name" typeName="string" />
                              </ECEntityClass>
                              <ECRelationshipClass typeName="Rel" modifier="Sealed">
                                    <Source multiplicity="(0..*)" polymorphic="True" roleLabel="A">
                                       <Class class="A" />
                                    </Source>
                                    <Target multiplicity="(0..*)" polymorphic="True" roleLabel="B">
                                       <Class class="B"/>
                                     </Target>
                                <ECProperty propertyName="TargetECClassId" typeName="string" />
                              </ECRelationshipClass>
                            </ECSchema>)xml"))) << "TargetECClassId is a system property and therefore a reserved name";


    {
    ECDb ecdb;
    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, SchemaItem(R"xml(
                            <ECSchema schemaName="TestSchema1" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                               <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                              <ECEntityClass typeName="Foo">
                                <ECCustomAttributes>
                                    <ClassMap xmlns="ECDbMap.02.00">
                                        <ECInstanceIdColumn>MyId</ECInstanceIdColumn>
                                    </ClassMap>
                                </ECCustomAttributes>
                                <ECProperty propertyName="MyId" typeName="string" />
                              </ECEntityClass>
                            </ECSchema>)xml"), "idnamecollisions.ecdb"));

    ASSERT_PROPERTYMAPPING(ecdb, PropertyAccessString("ts", "Foo", "MyId"), {ColumnInfo("ts_Foo","MyId1")});
    }


    {
    ECDb ecdb;
    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, SchemaItem(R"xml(
                            <ECSchema schemaName="TestSchema2" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                               <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                              <ECEntityClass typeName="Base">
                                <ECCustomAttributes>
                                    <ClassMap xmlns="ECDbMap.02.00">
                                        <MapStrategy>TablePerHierarchy</MapStrategy>
                                    </ClassMap>
                                    <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00"/>
                                </ECCustomAttributes>
                              </ECEntityClass>
                              <ECEntityClass typeName="Sub">
                                <BaseClass>Base</BaseClass>
                                <ECProperty propertyName="BaseId" typeName="string" />
                              </ECEntityClass>
                            </ECSchema>)xml"), "idnamecollisions.ecdb"));

    ASSERT_PROPERTYMAPPING(ecdb, PropertyAccessString("ts", "Sub", "BaseId"), {ColumnInfo("ts_Sub","BaseId1")});
    }


    {
    ECDb ecdb;
    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, SchemaItem(R"xml(
                            <ECSchema schemaName="TestSchema3" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                               <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                              <ECEntityClass typeName="A">
                                <ECProperty propertyName="Name" typeName="string" />
                              </ECEntityClass>
                              <ECEntityClass typeName="B">
                                <ECProperty propertyName="Name" typeName="string" />
                              </ECEntityClass>
                              <ECRelationshipClass typeName="Rel" modifier="Sealed">
                                <ECCustomAttributes>
                                    <LinkTableRelationshipMap xmlns="ECDbMap.02.00">
                                        <SourceECInstanceIdColumn>MySourceId</SourceECInstanceIdColumn>
                                    </LinkTableRelationshipMap>
                                </ECCustomAttributes>
                                    <Source multiplicity="(0..*)" polymorphic="True" roleLabel="A">
                                       <Class class="A" />
                                    </Source>
                                    <Target multiplicity="(0..*)" polymorphic="True" roleLabel="B">
                                       <Class class="B"/>
                                     </Target>
                                <ECProperty propertyName="MySourceId" typeName="string" />
                              </ECRelationshipClass>
                            </ECSchema>)xml"), "idnamecollisions.ecdb"));

    ASSERT_PROPERTYMAPPING(ecdb, PropertyAccessString("ts", "Rel", "MySourceId"), {ColumnInfo("ts_Rel","MySourceId1")});
    }


    {
    ECDb ecdb;
    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, SchemaItem(R"xml(
                              <ECSchema schemaName="TestSchema4" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                               <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                              <ECEntityClass typeName="A">
                                <ECProperty propertyName="Name" typeName="string" />
                              </ECEntityClass>
                              <ECEntityClass typeName="B">
                                <ECProperty propertyName="Name" typeName="string" />
                              </ECEntityClass>
                              <ECRelationshipClass typeName="Rel" modifier="Sealed">
                                <ECCustomAttributes>
                                    <LinkTableRelationshipMap xmlns="ECDbMap.02.00">
                                        <TargetECInstanceIdColumn>MyTargetId</TargetECInstanceIdColumn>
                                    </LinkTableRelationshipMap>
                                </ECCustomAttributes>
                                    <Source multiplicity="(0..*)" polymorphic="True" roleLabel="A">
                                       <Class class="A" />
                                    </Source>
                                    <Target multiplicity="(0..*)" polymorphic="True" roleLabel="B">
                                       <Class class="B"/>
                                     </Target>
                                <ECProperty propertyName="MyTargetId" typeName="string" />
                              </ECRelationshipClass>
                            </ECSchema>)xml"), "idnamecollisions.ecdb"));

    ASSERT_PROPERTYMAPPING(ecdb, PropertyAccessString("ts", "Rel", "MyTargetId"), {ColumnInfo("ts_Rel","MyTargetId1")});
    }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, ForeignKeyMapCATests)
    {
    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TestSchema1' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='A' modifier='None'>"
                                   "        <ECProperty propertyName='AA' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "    <ECEntityClass typeName='B' modifier='None'>"
                                   "        <ECProperty propertyName='BB' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "    <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='referencing'>"
                                   "        <ECCustomAttributes>"
                                   "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'>"
                                   "            </ForeignKeyConstraint>"
                                   "        </ECCustomAttributes>"
                                   "       <Source cardinality='(0,1)' polymorphic='True'>"
                                   "           <Class class='A' />"
                                   "       </Source>"
                                   "       <Target cardinality='(0,N)' polymorphic='True'>"
                                   "           <Class class='B' />"
                                   "       </Target>"
                                   "     </ECRelationshipClass>"
                                   "</ECSchema>"))) << "ForeignKeyConstraint on relationship is not supported";

    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TestSchema2' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='A' modifier='None'>"
                                   "        <ECProperty propertyName='AA' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "    <ECEntityClass typeName='B' modifier='None'>"
                                   "        <ECProperty propertyName='BB' typeName='double' />"
                                   "        <ECNavigationProperty propertyName='A' relationshipName='Rel' direction='Backward'>"
                                   "          <ECCustomAttributes>"
                                   "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'>"
                                   "               <OnDeleteAction>Cascade</OnDeleteAction>"
                                   "            </ForeignKeyConstraint>"
                                   "          </ECCustomAttributes>"
                                   "        </ECNavigationProperty>"
                                   "    </ECEntityClass>"
                                   "    <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='embedding'>"
                                   "       <Source cardinality='(0,1)' polymorphic='True'>"
                                   "           <Class class='A' />"
                                   "       </Source>"
                                   "       <Target cardinality='(0,N)' polymorphic='True'>"
                                   "           <Class class='B' />"
                                   "       </Target>"
                                   "     </ECRelationshipClass>"
                                   "</ECSchema>"))) << "Cascading delete only supported for embedding relationships";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TestSchema3' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='A' modifier='None'>"
                                   "        <ECProperty propertyName='AA' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "    <ECEntityClass typeName='B' modifier='None'>"
                                   "        <ECProperty propertyName='BB' typeName='double' />"
                                   "        <ECNavigationProperty propertyName='A' relationshipName='Rel' direction='Backward'>"
                                   "          <ECCustomAttributes>"
                                   "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'>"
                                   "               <OnDeleteAction>Cascade</OnDeleteAction>"
                                   "            </ForeignKeyConstraint>"
                                   "          </ECCustomAttributes>"
                                   "        </ECNavigationProperty>"
                                   "    </ECEntityClass>"
                                   "    <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='referencing'>"
                                   "       <Source cardinality='(0,1)' polymorphic='True'>"
                                   "           <Class class='A' />"
                                   "       </Source>"
                                   "       <Target cardinality='(0,N)' polymorphic='True'>"
                                   "           <Class class='B' />"
                                   "       </Target>"
                                   "     </ECRelationshipClass>"
                                   "</ECSchema>"))) << "Cascading delete only supported for embedding relationships";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TestSchema4' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='A' modifier='None'>"
                                   "        <ECProperty propertyName='AA' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "    <ECEntityClass typeName='B' modifier='None'>"
                                   "        <ECProperty propertyName='BB' typeName='double' />"
                                   "        <ECNavigationProperty propertyName='A' relationshipName='Rel' direction='Backward'>"
                                   "          <ECCustomAttributes>"
                                   "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'>"
                                   "               <OnDeleteAction>Cascade</OnDeleteAction>"
                                   "            </ForeignKeyConstraint>"
                                   "          </ECCustomAttributes>"
                                   "        </ECNavigationProperty>"
                                   "    </ECEntityClass>"
                                   "    <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='holding'>"
                                   "       <Source cardinality='(0,1)' polymorphic='True'>"
                                   "           <Class class='A' />"
                                   "       </Source>"
                                   "       <Target cardinality='(0,N)' polymorphic='True'>"
                                   "           <Class class='B' />"
                                   "       </Target>"
                                   "     </ECRelationshipClass>"
                                   "</ECSchema>"))) << "Cascading delete only supported for embedding relationships";
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, ShareColumnsCA)
    {
    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' modifier='None'>"
        "        <BaseClass>Parent</BaseClass>"
        "        <ECCustomAttributes>"
        "            <ShareColumns xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Cost' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub11' modifier='None'>"
        "        <BaseClass>Sub1</BaseClass>"
        "        <ECCustomAttributes>"
        "            <ShareColumns xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Diameter' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "Defining ShareColumns multiple times in class hierarchy is expected to fail";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' modifier='None'>"
        "        <BaseClass>Parent</BaseClass>"
        "        <ECCustomAttributes>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Cost' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub11' modifier='None'>"
        "        <BaseClass>Sub1</BaseClass>"
        "        <ECCustomAttributes>"
        "            <ShareColumns xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Diameter' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "Defining ShareColumns multiple times in class hierarchy is expected to fail";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' modifier='None'>"
        "        <BaseClass>Parent</BaseClass>"
        "        <ECCustomAttributes>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Cost' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub11' modifier='None'>"
        "        <BaseClass>Sub1</BaseClass>"
        "        <ECCustomAttributes>"
        "            <ShareColumns xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Diameter' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "Defining ShareColumns multiple times in class hierarchy is expected to fail";

    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <MaxSharedColumnsBeforeOverflow>100</MaxSharedColumnsBeforeOverflow>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' modifier='None'>"
        "        <BaseClass>Parent</BaseClass>"
        "        <ECProperty propertyName='Cost' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub11' modifier='None'>"
        "        <BaseClass>Sub1</BaseClass>"
        "        <ECProperty propertyName='Diameter' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>")));

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' modifier='None'>"
        "        <BaseClass>Parent</BaseClass>"
        "        <ECProperty propertyName='Cost' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub11' modifier='None'>"
        "        <BaseClass>Sub1</BaseClass>"
        "        <ECCustomAttributes>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <MaxSharedColumnsBeforeOverflow>100</MaxSharedColumnsBeforeOverflow>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Diameter' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "MaxSharedColumnsBeforeOverflow can only be defined on first occurrence of SharedColumn option in a hierarchy";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' modifier='None'>"
        "        <BaseClass>Parent</BaseClass>"
        "        <ECProperty propertyName='Cost' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub11' modifier='None'>"
        "        <BaseClass>Sub1</BaseClass>"
        "        <ECCustomAttributes>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <MaxSharedColumnsBeforeOverflow>100</MaxSharedColumnsBeforeOverflow>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Diameter' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "MaxSharedColumnsBeforeOverflow can only be defined on first occurrence of SharedColumn option in a hierarchy";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' modifier='None'>"
        "        <BaseClass>Parent</BaseClass>"
        "        <ECProperty propertyName='Cost' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub11' modifier='None'>"
        "        <BaseClass>Sub1</BaseClass>"
        "        <ECCustomAttributes>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <MaxSharedColumnsBeforeOverflow>100</MaxSharedColumnsBeforeOverflow>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Diameter' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "MaxSharedColumnsBeforeOverflow can only be defined on first occurrence of SharedColumn option in a hierarchy";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                         03/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, StatementBindingInvalidScenarios)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("incompletePoints.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'> "
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Element' modifier='None'>"
        "        <ECProperty propertyName='Code' typeName='string'/>"
        "        <ECProperty propertyName='P2D' typeName='point2d'/>"
        "        <ECProperty propertyName='P3D' typeName='point3d'/>"
        "    </ECEntityClass>"
        "</ECSchema>")));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Element (P2D,P3D,Code) VALUES (?,?,'C1')"));
    //Binding Point 2d & 3d
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint2d(1, DPoint2d::From(-21, 22.1)));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint3d(2, DPoint3d::From(-12.53, 21.76, -32.22)));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT P2D,P3D FROM ts.Element WHERE Code='C1'"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    //GetValuePoint3d can't be called for a 2d column.
    ECValue val3d(stmt.GetValuePoint3d(0));
    ASSERT_FALSE(val3d.IsPoint2d());

    //GetValuePoint2d can't be called for a 3d column.
    ECValue val2d(stmt.GetValuePoint2d(1));
    ASSERT_FALSE(val2d.IsPoint3d());

    //GetValueBoolean can't be called for a 2d/3d column.
    ASSERT_EQ(false, stmt.GetValueBoolean(0));

    //GetValueBlob can't be called for a 2d/3d column.
    ASSERT_TRUE(nullptr == stmt.GetValueBlob(1));

    //GetValueText can't be called for a 2d/3d column.
    ASSERT_TRUE(nullptr == stmt.GetValueText(0));

    //GetValueInt can't be called for a 2d/3d column.
    ASSERT_EQ(0, stmt.GetValueInt(1));
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Krischan.Eberle                  08/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, ShareColumnsCAWithoutTPH)
    {
    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ShareColumns xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' modifier='None'>"
        "        <BaseClass>Parent</BaseClass>"
        "        <ECProperty propertyName='Cost' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "ShareColumns without MapStrategy is not supported";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>OwnTable</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' modifier='None'>"
        "        <BaseClass>Parent</BaseClass>"
        "        <ECProperty propertyName='Cost' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "ShareColumns with MapStrategy OwnTable is not supported";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>ExistingTable</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' modifier='None'>"
        "        <BaseClass>Parent</BaseClass>"
        "        <ECProperty propertyName='Cost' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "ShareColumns with MapStrategy ExistingTable is not supported";
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Krischan.Eberle                  05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, ShareColumnsCAAndPerColumnConstraints)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("sharecolumnsandcolumnconstraints.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='TestClass' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "               <MaxSharedColumnsBeforeOverflow>10</MaxSharedColumnsBeforeOverflow>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "        <ECProperty propertyName='NotNullableProp' typeName='double'>"
        "           <ECCustomAttributes>"
        "            <PropertyMap xmlns='ECDbMap.02.00'>"
        "                <IsNullable>false</IsNullable>"
        "            </PropertyMap>"
        "           </ECCustomAttributes>"
        "        </ECProperty>"
        "        <ECProperty propertyName='NullableProp' typeName='string'>"
        "           <ECCustomAttributes>"
        "            <PropertyMap xmlns='ECDbMap.02.00'>"
        "                <IsNullable>true</IsNullable>"
        "            </PropertyMap>"
        "           </ECCustomAttributes>"
        "        </ECProperty>"
        "        <ECProperty propertyName='UniqueProp' typeName='string'>"
        "           <ECCustomAttributes>"
        "            <PropertyMap xmlns='ECDbMap.02.00'>"
        "                <IsUnique>true</IsUnique>"
        "            </PropertyMap>"
        "           </ECCustomAttributes>"
        "        </ECProperty>"
        "        <ECProperty propertyName='NotUniqueProp' typeName='string'>"
        "           <ECCustomAttributes>"
        "            <PropertyMap xmlns='ECDbMap.02.00'>"
        "                <IsUnique>false</IsUnique>"
        "            </PropertyMap>"
        "           </ECCustomAttributes>"
        "        </ECProperty>"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "Column constraints on property that maps to shared column";

    Utf8String ddl = RetrieveDdl(m_ecdb, "ts_TestClass");
    ASSERT_FALSE(ddl.empty());

    bvector<Utf8String> columnDdlList;
    BeStringUtilities::Split(ddl.c_str(), ",", columnDdlList);

    for (Utf8StringR columnDdl : columnDdlList)
        {
        columnDdl.ToLower();

        if (!columnDdl.StartsWithI("[sc") || columnDdl.StartsWithI("sc"))
            continue;

        ASSERT_TRUE(columnDdl.find("not null") == columnDdl.npos) << columnDdl.c_str();
        ASSERT_TRUE(columnDdl.find("unique") == columnDdl.npos) << columnDdl.c_str();
        ASSERT_TRUE(columnDdl.find("collat") == columnDdl.npos) << columnDdl.c_str();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, MaxSharedColumnsBeforeOverflow)
    {
    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                          "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                          "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
                                          "   <ECEntityClass typeName='Parent' modifier='None' >"
                                          "        <ECCustomAttributes>"
                                          "            <ClassMap xmlns='ECDbMap.02.00'>"
                                          "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                          "            </ClassMap>"
                                          "            <ShareColumns xmlns='ECDbMap.02.00'>"
                                          "              <MaxSharedColumnsBeforeOverflow>-3</MaxSharedColumnsBeforeOverflow>"
                                          "            </ShareColumns>"
                                          "        </ECCustomAttributes>"
                                          "       <ECProperty propertyName='P1' typeName='int' />"
                                          "   </ECEntityClass>"
                                          "</ECSchema>"))) << "MaxSharedColumnsBeforeOverflow must not be negative. It must be >= 1";
    {
    ASSERT_EQ(SUCCESS, SetupECDb("maxsharedcolumnsbeforeoverflow.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "   <ECEntityClass typeName='Parent' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "       <ECProperty propertyName='P1' typeName='int' />"
        "   </ECEntityClass>"
        "</ECSchema>")));

    std::vector<std::pair<Utf8String, int>> testItems;
    testItems.push_back(std::make_pair("ts_Parent", 3));
    AssertColumnCount(m_ecdb, testItems, "MaxSharedColumnsBeforeOverflow");
    }

            {
    ASSERT_EQ(SUCCESS, SetupECDb("maxsharedcolumnsbeforeoverflow.ecdb", SchemaItem(
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
                "   <ECEntityClass typeName='Parent' modifier='None' >"
                "        <ECCustomAttributes>"
                "            <ClassMap xmlns='ECDbMap.02.00'>"
                "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                "            </ClassMap>"
                "            <ShareColumns xmlns='ECDbMap.02.00'>"
                "              <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>"
                "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
                "            </ShareColumns>"
                "        </ECCustomAttributes>"
                "       <ECProperty propertyName='P1' typeName='int' />"
                "   </ECEntityClass>"
                "</ECSchema>")));

            std::vector<std::pair<Utf8String, int>> testItems;
            testItems.push_back(std::make_pair("ts_Parent", 3));
            AssertColumnCount(m_ecdb, testItems, "MaxSharedColumnsBeforeOverflow");
            }

            {
            ASSERT_EQ(SUCCESS, SetupECDb("maxsharedcolumnsbeforeoverflow.ecdb", SchemaItem(
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                "    <ECEntityClass typeName='Parent' modifier='None'>"
                "        <ECCustomAttributes>"
                "            <ClassMap xmlns='ECDbMap.02.00'>"
                "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                "            </ClassMap>"
                "            <ShareColumns xmlns='ECDbMap.02.00'>"
                "              <MaxSharedColumnsBeforeOverflow>100</MaxSharedColumnsBeforeOverflow>"
                "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
                "            </ShareColumns>"
                "        </ECCustomAttributes>"
                "        <ECProperty propertyName='Price' typeName='double' />"
                "    </ECEntityClass>"
                "    <ECEntityClass typeName='Sub1' modifier='None'>"
                "        <BaseClass>Parent</BaseClass>"
                "        <ECProperty propertyName='Cost' typeName='double' />"
                "    </ECEntityClass>"
                "    <ECEntityClass typeName='Sub2' modifier='None'>"
                "        <BaseClass>Parent</BaseClass>"
                "        <ECProperty propertyName='DoubleProp' typeName='double' />"
                "    </ECEntityClass>"
                "    <ECEntityClass typeName='Sub11' modifier='None'>"
                "        <BaseClass>Sub1</BaseClass>"
                "        <ECProperty propertyName='Diameter' typeName='double' />"
                "    </ECEntityClass>"
                "</ECSchema>")));

            std::vector<std::pair<Utf8String, int>> testItems;
            testItems.push_back(std::make_pair("ts_Parent", 5));
            AssertColumnCount(m_ecdb, testItems, "After first schema import");

            ASSERT_EQ(SUCCESS, ImportSchema(m_ecdb, SchemaItem(
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema2' nameSpacePrefix='ts2' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                "    <ECSchemaReference name='TestSchema' version='01.00' prefix='ts' />"
                "    <ECEntityClass typeName='Sub3'>"
                "        <BaseClass>ts:Parent</BaseClass>"
                "        <ECProperty propertyName='Prop3' typeName='double' />"
                "    </ECEntityClass>"
                "    <ECEntityClass typeName='Sub111'>"
                "        <BaseClass>ts:Sub11</BaseClass>"
                "        <ECProperty propertyName='Prop111' typeName='double' />"
                "    </ECEntityClass>"
                "</ECSchema>")));

            testItems.clear();
            testItems.push_back(std::make_pair("ts_Parent", 6));
            AssertColumnCount(m_ecdb, testItems, "After second schema import");
            }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, MaxSharedColumnsBeforeOverflowWithJoinedTable_SubsequentSchemaImports)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("maxsharedcolumnsbeforeoverflow.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "    <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <MaxSharedColumnsBeforeOverflow>100</MaxSharedColumnsBeforeOverflow>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' modifier='None'>"
        "        <BaseClass>Parent</BaseClass>"
        "        <ECProperty propertyName='Cost' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub2' modifier='None'>"
        "        <BaseClass>Parent</BaseClass>"
        "        <ECProperty propertyName='DoubleProp' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub11' modifier='None'>"
        "        <BaseClass>Sub1</BaseClass>"
        "        <ECProperty propertyName='Diameter' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>")));

    std::vector<std::pair<Utf8String, int>> testItems;
    testItems.push_back(std::make_pair("ts_Parent", 3));
    testItems.push_back(std::make_pair("ts_Sub1", 4));
    testItems.push_back(std::make_pair("ts_Sub2", 3));
    AssertColumnCount(m_ecdb, testItems, "After first schema import");

    ASSERT_EQ(SUCCESS, ImportSchema(m_ecdb, SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema2' alias='ts2' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='TestSchema' version='01.00' alias='ts' />"
        "    <ECEntityClass typeName='Sub3'>"
        "        <BaseClass>ts:Parent</BaseClass>"
        "        <ECProperty propertyName='Prop3' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub111'>"
        "        <BaseClass>ts:Sub11</BaseClass>"
        "        <ECProperty propertyName='Prop111' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>")));

    testItems.clear();
    testItems.push_back(std::make_pair("ts_Parent", 3));
    testItems.push_back(std::make_pair("ts_Sub1", 5));
    testItems.push_back(std::make_pair("ts_Sub2", 3));
    testItems.push_back(std::make_pair("ts2_Sub3", 3));
    AssertColumnCount(m_ecdb, testItems, "After second schema import");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, SharedColumnJoinedTable_VariousScenarios)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("maxsharedcolumnsbeforeoverflow.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "    <ECEntityClass typeName='Base1' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='PropBase1_1' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Base1</BaseClass>"
        "        <ECProperty propertyName='PropSub1_1' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub11' modifier='None'>"
        "        <BaseClass>Sub1</BaseClass>"
        "        <ECProperty propertyName='PropSub11_1' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub12' modifier='None'>"
        "        <BaseClass>Sub1</BaseClass>"
        "        <ECProperty propertyName='PropSub12_1' typeName='double' />"
        "    </ECEntityClass>"

        "    <ECEntityClass typeName='Base2' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='PropBase2_1' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub2' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Base2</BaseClass>"
        "        <ECProperty propertyName='PropSub2_1' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub21' modifier='None'>"
        "        <BaseClass>Sub2</BaseClass>"
        "        <ECProperty propertyName='PropSub21_1' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub22' modifier='None'>"
        "        <BaseClass>Sub2</BaseClass>"
        "        <ECProperty propertyName='PropSub22_1' typeName='double' />"
        "    </ECEntityClass>"

        "    <ECEntityClass typeName='Base5' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='PropBase5_1' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub5' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Base5</BaseClass>"
        "        <ECProperty propertyName='PropSub5_1' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub51' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Sub5</BaseClass>"
        "        <ECProperty propertyName='PropSub51_1' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub511' modifier='None'>"
        "        <BaseClass>Sub51</BaseClass>"
        "        <ECProperty propertyName='PropSub511_1' typeName='double' />"
        "    </ECEntityClass>"

        "    <ECEntityClass typeName='Base6' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='PropBase6_1' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub6' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Base6</BaseClass>"
        "        <ECProperty propertyName='PropSub6_1' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub61' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>"
        "              <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Sub6</BaseClass>"
        "        <ECProperty propertyName='PropSub61_1' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub611' modifier='None'>"
        "        <BaseClass>Sub61</BaseClass>"
        "        <ECProperty propertyName='PropSub611_1' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>")));

    std::vector<std::pair<Utf8CP, std::vector<Utf8CP>>> expectedTableLayouts {
            //Base1 hierarchy
            {"ts_Base1", {"Id", "ECClassId", "PropBase1_1", "PropSub1_1"}},
            {"ts_Sub11", {"Base1Id", "ECClassId", "js1"}},
            {"ts_Sub12", {"Base1Id", "ECClassId", "js1"}},

            //Base2 hierarchy
            {"ts_Base2", {"Id", "ECClassId", "PropBase2_1", "PropSub2_1"}},
            {"ts_Sub21", {"Base2Id", "ECClassId", "js1"}},
            {"ts_Sub22", {"Base2Id", "ECClassId", "js1"}},

            //Base5 hierarchy
            {"ts_Base5", {"Id", "ECClassId", "PropBase5_1", "PropSub5_1"}},
            {"ts_Sub51", {"Base5Id", "ECClassId", "PropSub51_1", "js1"}},

            //Base6 hierarchy
            {"ts_Base6", {"Id", "ECClassId", "PropBase6_1", "PropSub6_1"}},
            {"ts_Sub61", {"Base6Id", "ECClassId", "js1", "js2"}},

        };

    for (std::pair<Utf8CP, std::vector<Utf8CP>> const& expectedTableLayout : expectedTableLayouts)
        {
        Utf8CP tableName = expectedTableLayout.first;
        std::vector<Utf8CP> const& expectedColNames = expectedTableLayout.second;
        bvector<Utf8String> actualColNames;
        ASSERT_TRUE(m_ecdb.GetColumns(actualColNames, tableName));
        ASSERT_EQ(expectedColNames.size(), actualColNames.size()) << tableName;
        for (size_t i = 0; i < expectedColNames.size(); i++)
            {
            ASSERT_STRCASEEQ(expectedColNames[i], actualColNames[i].c_str()) << tableName;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         11/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, Overflow_InsertWithNoParameters)
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
        "              <MaxSharedColumnsBeforeOverflow>0</MaxSharedColumnsBeforeOverflow>"
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
        "        <ECProperty propertyName='Geom' typeName='Bentley.Geometry.Common.IGeometry'/>"
        "    </ECEntityClass>"
        "</ECSchema>")));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.TestElement (Code, S, I, L, D, DT, B, P2D.X, P2D.Y, P3D.X, P3D.Y, P3D.Z, ST1P.D1, ST1P.P2D.X, ST1P.P2D.Y, ST1P.ST2P.D2, ST1P.ST2P.P3D.X, ST1P.ST2P.P3D.Y, ST1P.ST2P.P3D.Z, arrayOfP3d, arrayOfST1, BIN, Geom) "
                                                 "VALUES ('C1', 'Str', 123, 12345, 23.5453, TIMESTAMP '2013-02-09T12:00:00', true, 12.34, 45.45, 56.34, 67.44, 14.44, 22312.34, 34.14, 86.54, 34.23, 23.55, 64.34, 34.45, null, null, null, null)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    m_ecdb.SaveChanges();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT Code, S, I, L, D, DT, B, P2D.X, P2D.Y, P3D.X, P3D.Y, P3D.Z, ST1P.D1, ST1P.P2D.X, ST1P.P2D.Y, ST1P.ST2P.D2, ST1P.ST2P.P3D.X, ST1P.ST2P.P3D.Y, ST1P.ST2P.P3D.Z, arrayOfP3d, arrayOfST1, BIN, Geom FROM  ts.TestElement WHERE Code = 'C1'"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    int idx = 0;
    ASSERT_STREQ("C1", stmt.GetValueText(idx++));   //Code
    ASSERT_STREQ("Str", stmt.GetValueText(idx++));  //S
    ASSERT_EQ(123, stmt.GetValueInt(idx++));        //I
    ASSERT_EQ(12345, stmt.GetValueInt64(idx++));    //L
    ASSERT_EQ(23.5453, stmt.GetValueDouble(idx++)); //D
    auto dt = DateTime(DateTime::Kind::Unspecified, 2013, 2, 9, 12, 0);
    ASSERT_EQ(dt, stmt.GetValueDateTime(idx++));    //DT
    ASSERT_EQ(true, stmt.GetValueBoolean(idx++));   //B
    ASSERT_EQ(12.34, stmt.GetValueDouble(idx++));   //P2D.X
    ASSERT_EQ(45.45, stmt.GetValueDouble(idx++));   //P2D.Y
    ASSERT_EQ(56.34, stmt.GetValueDouble(idx++));   //P3D.X
    ASSERT_EQ(67.44, stmt.GetValueDouble(idx++));   // P3D.Y
    ASSERT_EQ(14.44, stmt.GetValueDouble(idx++));   // P3D.Z
    ASSERT_EQ(22312.34, stmt.GetValueDouble(idx++));//ST1P.D1
    ASSERT_EQ(34.14, stmt.GetValueDouble(idx++));   //ST1P.P2D.X
    ASSERT_EQ(86.54, stmt.GetValueDouble(idx++));   //ST1P.P2D.Y
    ASSERT_EQ(34.23, stmt.GetValueDouble(idx++));   //ST1P.ST2P.D2
    ASSERT_EQ(23.55, stmt.GetValueDouble(idx++));   //ST1P.ST2P.P3D.X
    ASSERT_EQ(64.34, stmt.GetValueDouble(idx++));   //ST1P.ST2P.P3D.Y
    ASSERT_EQ(34.45, stmt.GetValueDouble(idx++));   //ST1P.ST2P.P3D.Z
    ASSERT_EQ(0, stmt.GetValue(idx).GetArrayLength());//==[]
    ASSERT_EQ(true, stmt.IsValueNull(idx++));       //arrayOfP3d
    ASSERT_EQ(0, stmt.GetValue(idx).GetArrayLength());// ==[]
    ASSERT_EQ(true, stmt.IsValueNull(idx++));       //arrayOfST1
    ASSERT_EQ(true, stmt.IsValueNull(idx++));       //BIN is null
    ASSERT_EQ(true, stmt.IsValueNull(idx++));       //Geom is null
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         11/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, Overflow_InsertExplicitNullsUsingECSql)
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
        "</ECSchema>")));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.TestElement (Code, S, I, L, D, DT, B, P2D.X, P2D.Y, P3D.X, P3D.Y, P3D.Z, ST1P.D1, ST1P.P2D.X, ST1P.P2D.Y, ST1P.ST2P.D2, ST1P.ST2P.P3D.X, ST1P.ST2P.P3D.Y, ST1P.ST2P.P3D.Z, arrayOfP3d, arrayOfST1, BIN) "
                                                 "VALUES ('C2', null, null, null, null, null, null, null,null, null, null, null, null, null, null, null, null, null, null, null, null, null)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    m_ecdb.SaveChanges();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT Code, S, I, L, D, DT, B, P2D.X, P2D.Y, P3D.X, P3D.Y, P3D.Z, ST1P.D1, ST1P.P2D.X, ST1P.P2D.Y, ST1P.ST2P.D2, ST1P.ST2P.P3D.X, ST1P.ST2P.P3D.Y, ST1P.ST2P.P3D.Z, arrayOfP3d, arrayOfST1, BIN FROM  ts.TestElement WHERE Code ='C2'"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    int idx = 0;
    ASSERT_STREQ("C2", stmt.GetValueText(idx++));   //Code
    ASSERT_EQ(true, stmt.IsValueNull(idx++));   //S
    ASSERT_EQ(true, stmt.IsValueNull(idx++));   //I
    ASSERT_EQ(true, stmt.IsValueNull(idx++));   //L
    ASSERT_EQ(true, stmt.IsValueNull(idx++));   //D
    ASSERT_EQ(true, stmt.IsValueNull(idx++));   //DT
    ASSERT_EQ(true, stmt.IsValueNull(idx++));   //B
    ASSERT_EQ(true, stmt.IsValueNull(idx++));   //P2D.X
    ASSERT_EQ(true, stmt.IsValueNull(idx++));   //P2D.Y
    ASSERT_EQ(true, stmt.IsValueNull(idx++));   //P3D.X
    ASSERT_EQ(true, stmt.IsValueNull(idx++));;  //P3D.Y
    ASSERT_EQ(true, stmt.IsValueNull(idx++));   //P3D.Z
    ASSERT_EQ(true, stmt.IsValueNull(idx++));   //ST1P.D1
    ASSERT_EQ(true, stmt.IsValueNull(idx++));   //ST1P.P2D.X
    ASSERT_EQ(true, stmt.IsValueNull(idx++));   //ST1P.P2D.Y
    ASSERT_EQ(true, stmt.IsValueNull(idx++));   //ST1P.ST2P.D2
    ASSERT_EQ(true, stmt.IsValueNull(idx++));   //ST1P.ST2P.P3D.X
    ASSERT_EQ(true, stmt.IsValueNull(idx++));   //ST1P.ST2P.P3D.Y
    ASSERT_EQ(true, stmt.IsValueNull(idx++));   //ST1P.ST2P.P3D.Z
    ASSERT_EQ(0, stmt.GetValue(idx).GetArrayLength());//==[]
    ASSERT_EQ(true, stmt.IsValueNull(idx++));  //arrayOfP3d
    ASSERT_EQ(0, stmt.GetValue(idx).GetArrayLength());// ==[]
    ASSERT_EQ(true, stmt.IsValueNull(idx++));  //arrayOfST1
    ASSERT_EQ(true, stmt.IsValueNull(idx++));   //BIN is null
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         11/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, Overflow_SharedColumns2)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("overflowProperties.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='TestSchema1' nameSpacePrefix='ts1' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'> "
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Element' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <MaxSharedColumnsBeforeOverflow>3</MaxSharedColumnsBeforeOverflow>"
        "              <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='D1' modifier='None'>"
        "        <BaseClass>Element</BaseClass>"
        "        <ECProperty propertyName='D1A' typeName='int'/>"
        "        <ECProperty propertyName='D1B' typeName='int'/>"
        "        <ECProperty propertyName='D1C' typeName='int'/>"
        "        <ECProperty propertyName='D1D' typeName='int'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='D2' modifier='None'>"
        "        <BaseClass>Element</BaseClass>"
        "        <ECProperty propertyName='D2A' typeName='int'/>"
        "        <ECProperty propertyName='D2B' typeName='int'/>"
        "        <ECProperty propertyName='D2C' typeName='int'/>"
        "        <ECProperty propertyName='D2D' typeName='int'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='D11' modifier='None'>"
        "        <BaseClass>D1</BaseClass>"
        "        <ECProperty propertyName='D11A' typeName='int'/>"
        "        <ECProperty propertyName='D11B' typeName='int'/>"
        "        <ECProperty propertyName='D11C' typeName='int'/>"
        "        <ECProperty propertyName='D11D' typeName='int'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='D21' modifier='None'>"
        "        <BaseClass>Element</BaseClass>"
        "        <ECProperty propertyName='D21A' typeName='int'/>"
        "        <ECProperty propertyName='D21B' typeName='int'/>"
        "        <ECProperty propertyName='D21C' typeName='int'/>"
        "        <ECProperty propertyName='D21D' typeName='int'/>"
        "    </ECEntityClass>"
        "</ECSchema>")));
    

    ASSERT_EQ(SUCCESS, ImportSchema(m_ecdb, SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema2' nameSpacePrefix='ts2' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='TestSchema1' version='01.00' prefix='ts1' />"
        "    <ECEntityClass typeName='D111'>"
        "        <BaseClass>ts1:D11</BaseClass>"
        "        <ECProperty propertyName='Sub32Prop1' typeName='double' />"
        "        <ECProperty propertyName='Sub32Prop2' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='D211'>"
        "        <BaseClass>ts1:D21</BaseClass>"
        "        <ECProperty propertyName='Sub32Prop1' typeName='double' />"
        "        <ECProperty propertyName='Sub32Prop2' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>")));

    m_ecdb.Schemas().CreateClassViewsInDb();
    m_ecdb.SaveChanges();
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         11/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, Overflow_SharedColumns)
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
        "              <MaxSharedColumnsBeforeOverflow>3</MaxSharedColumnsBeforeOverflow>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='D0' modifier='None'>"
        "        <BaseClass>Element</BaseClass>"
        "        <ECProperty propertyName='D0_I' typeName='int'/>"
        "        <ECProperty propertyName='MyId' typeName='long'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='D1' modifier='None'>"
        "        <BaseClass>Element</BaseClass>"
        "        <ECProperty propertyName='D1_I' typeName='int'/>"
        "        <ECProperty propertyName='MyId' typeName='long'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='D2' modifier='None'>"
        "        <BaseClass>Element</BaseClass>"
        "        <ECProperty propertyName='D2_I' typeName='int'/>"
        "        <ECProperty propertyName='MyId' typeName='long'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='D3' modifier='None'>"
        "        <BaseClass>Element</BaseClass>"
        "        <ECProperty propertyName='D3_I' typeName='int'/>"
        "        <ECProperty propertyName='MyId' typeName='long'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='D4' modifier='None'>"
        "        <BaseClass>Element</BaseClass>"
        "        <ECProperty propertyName='D4_I' typeName='int'/>"
        "        <ECProperty propertyName='MyId' typeName='long'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='D5' modifier='None'>"
        "        <BaseClass>Element</BaseClass>"
        "        <ECProperty propertyName='D5_I' typeName='int'/>"
        "        <ECProperty propertyName='MyId' typeName='long'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='D6' modifier='None'>"
        "        <BaseClass>Element</BaseClass>"
        "        <ECProperty propertyName='D6_I' typeName='int'/>"
        "        <ECProperty propertyName='MyId' typeName='long'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='D7' modifier='None'>"
        "        <BaseClass>Element</BaseClass>"
        "        <ECProperty propertyName='D7_I' typeName='int'/>"
        "        <ECProperty propertyName='MyId' typeName='long'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='D8' modifier='None'>"
        "        <BaseClass>Element</BaseClass>"
        "        <ECProperty propertyName='D8_I' typeName='int'/>"
        "        <ECProperty propertyName='MyId' typeName='long'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='D9' modifier='None'>"
        "        <BaseClass>Element</BaseClass>"
        "        <ECProperty propertyName='D9_I' typeName='int'/>"
        "        <ECProperty propertyName='MyId' typeName='long'/>"
        "    </ECEntityClass>"
        "</ECSchema>")));
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         11/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, Overflow_InsertImplicitNullsUsingECSql)
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
        "</ECSchema>")));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.TestElement (Code) VALUES ('C3')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    m_ecdb.SaveChanges();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT Code, S, I, L, D, DT, B, P2D.X, P2D.Y, P3D.X, P3D.Y, P3D.Z, ST1P.D1, ST1P.P2D.X, ST1P.P2D.Y, ST1P.ST2P.D2, ST1P.ST2P.P3D.X, ST1P.ST2P.P3D.Y, ST1P.ST2P.P3D.Z, arrayOfP3d, arrayOfST1, BIN FROM  ts.TestElement WHERE Code = 'C3'"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    int idx = 0;
    ASSERT_STREQ("C3", stmt.GetValueText(idx++));   //Code
    ASSERT_EQ(true, stmt.IsValueNull(idx++));   //S
    ASSERT_EQ(true, stmt.IsValueNull(idx++));   //I
    ASSERT_EQ(true, stmt.IsValueNull(idx++));   //L
    ASSERT_EQ(true, stmt.IsValueNull(idx++));   //D
    ASSERT_EQ(true, stmt.IsValueNull(idx++));   //DT
    ASSERT_EQ(true, stmt.IsValueNull(idx++));   //B
    ASSERT_EQ(true, stmt.IsValueNull(idx++));   //P2D.X
    ASSERT_EQ(true, stmt.IsValueNull(idx++));   //P2D.Y
    ASSERT_EQ(true, stmt.IsValueNull(idx++));   //P3D.X
    ASSERT_EQ(true, stmt.IsValueNull(idx++));;  //P3D.Y
    ASSERT_EQ(true, stmt.IsValueNull(idx++));   //P3D.Z
    ASSERT_EQ(true, stmt.IsValueNull(idx++));   //ST1P.D1
    ASSERT_EQ(true, stmt.IsValueNull(idx++));   //ST1P.P2D.X
    ASSERT_EQ(true, stmt.IsValueNull(idx++));   //ST1P.P2D.Y
    ASSERT_EQ(true, stmt.IsValueNull(idx++));   //ST1P.ST2P.D2
    ASSERT_EQ(true, stmt.IsValueNull(idx++));   //ST1P.ST2P.P3D.X
    ASSERT_EQ(true, stmt.IsValueNull(idx++));   //ST1P.ST2P.P3D.Y
    ASSERT_EQ(true, stmt.IsValueNull(idx++));   //ST1P.ST2P.P3D.Z
    ASSERT_EQ(0, stmt.GetValue(idx).GetArrayLength());//==[]
    ASSERT_EQ(true, stmt.IsValueNull(idx++));  //arrayOfP3d
    ASSERT_EQ(0, stmt.GetValue(idx).GetArrayLength());// ==[]
    ASSERT_EQ(true, stmt.IsValueNull(idx++));  //arrayOfST1
    ASSERT_EQ(true, stmt.IsValueNull(idx++));   //BIN is null
    
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         11/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, Overflow_InsertComplexTypesWithUnNamedParametersAndMixOrder)
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
        "              <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>"
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
        "        <ECProperty propertyName='Geom' typeName='Bentley.Geometry.Common.IGeometry'/>"
        "    </ECEntityClass>"
        "</ECSchema>")));


    //Point2D(3,4) (23,43,32)
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.TestElement (Code, S, arrayOfST1, I, arrayOfP3d, L, ST1P, D, DT, B, P2D, P3D, BIN, Geom) "
                                                 "VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?)"));
    int idx = 1;
    Utf8CP pCode = "C8";
    Utf8CP pS = "SampleText";
    int    pI = 123452;
    int64_t pL = 123324234234;
    double pD = 1232.343234;
    DateTime pDt = DateTime(DateTime::Kind::Unspecified, 2016, 11, 23, 0, 0);
    bool pB = true;
    DPoint2d pP2D = DPoint2d::From(12.33, -12.34);
    DPoint3d pP3D = DPoint3d::From(22.13, -62.34, -13.12);
    void const* bin = &pP2D;
    size_t binSize = sizeof(pP2D);
    double pST1P_D1 = 431231.3432;
    DPoint2d pST1P_P2D = DPoint2d::From(-212.34, 2112.314);
    double pST1P_ST2P_D2 = 431231.3432;
    DPoint3d pST1P_ST2P_P3D = DPoint3d::From(-123.434, 3217.3, -1.03);
    DPoint3d pArrayOfP3d[] = {DPoint3d::From(-41.33, 41.13, -12.25), DPoint3d::From(-23.37, 53.54, -34.31), DPoint3d::From(-33.41, 11.13, -99.11)};
    double pArrayOfST1_D1[] = {123.3434, 345.223,-532.123};
    DPoint2d pArrayOfST1_P2D[] = {DPoint2d::From(-21, 22.1),DPoint2d::From(-85.34, 35.36),DPoint2d::From(-31.34, 12.35)};
    double pArrayOfST1_D2[] = {12.3, -45.72, -31.11};
    DPoint3d pArrayOfST1_P3D[] = {DPoint3d::From(-12.11, -74.1, 12.3),DPoint3d::From(-12.53, 21.76, -32.22),DPoint3d::From(-41.14, -22.45, -31.16)};
    IGeometryPtr geom = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, 1.0, 1.0, 1.0)));

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(idx++, pCode, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(idx++, pS, IECSqlBinder::MakeCopy::No));
    IECSqlBinder& arrayOfST1 = stmt.GetBinder(idx++);
    {
    IECSqlBinder& arrayElementBinder = arrayOfST1.AddArrayElement();
    ASSERT_EQ(ECSqlStatus::Success, arrayElementBinder["D1"].BindDouble(pArrayOfST1_D1[0]));
    ASSERT_EQ(ECSqlStatus::Success, arrayElementBinder["P2D"].BindPoint2d(pArrayOfST1_P2D[0]));
    ASSERT_EQ(ECSqlStatus::Success, arrayElementBinder["ST2P"]["D2"].BindDouble(pArrayOfST1_D2[0]));
    ASSERT_EQ(ECSqlStatus::Success, arrayElementBinder["ST2P"]["P3D"].BindPoint3d(pArrayOfST1_P3D[0]));
    }
    {
    IECSqlBinder& arrayElementBinder = arrayOfST1.AddArrayElement();
    ASSERT_EQ(ECSqlStatus::Success, arrayElementBinder["D1"].BindDouble(pArrayOfST1_D1[1]));
    ASSERT_EQ(ECSqlStatus::Success, arrayElementBinder["P2D"].BindPoint2d(pArrayOfST1_P2D[1]));
    ASSERT_EQ(ECSqlStatus::Success, arrayElementBinder["ST2P"]["D2"].BindDouble(pArrayOfST1_D2[1]));
    ASSERT_EQ(ECSqlStatus::Success, arrayElementBinder["ST2P"]["P3D"].BindPoint3d(pArrayOfST1_P3D[1]));
    }
    {
    IECSqlBinder& arrayElementBinder = arrayOfST1.AddArrayElement();
    ASSERT_EQ(ECSqlStatus::Success, arrayElementBinder["D1"].BindDouble(pArrayOfST1_D1[2]));
    ASSERT_EQ(ECSqlStatus::Success, arrayElementBinder["P2D"].BindPoint2d(pArrayOfST1_P2D[2]));
    ASSERT_EQ(ECSqlStatus::Success, arrayElementBinder["ST2P"]["D2"].BindDouble(pArrayOfST1_D2[2]));
    ASSERT_EQ(ECSqlStatus::Success, arrayElementBinder["ST2P"]["P3D"].BindPoint3d(pArrayOfST1_P3D[2]));
    }
    
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(idx++, pI));

    IECSqlBinder& arrayOfP3d = stmt.GetBinder(idx++);
    for (size_t i = 0; i < 3; i++)
        {
        ASSERT_EQ(ECSqlStatus::Success, arrayOfP3d.AddArrayElement().BindPoint3d(pArrayOfP3d[i]));
        }

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(idx++, pL));
    IECSqlBinder& st1p = stmt.GetBinder(idx++);
    ASSERT_EQ(ECSqlStatus::Success, st1p["D1"].BindDouble(pST1P_D1));
    ASSERT_EQ(ECSqlStatus::Success, st1p["P2D"].BindPoint2d(pST1P_P2D));
    ASSERT_EQ(ECSqlStatus::Success, st1p["ST2P"]["D2"].BindDouble(pST1P_ST2P_D2));
    ASSERT_EQ(ECSqlStatus::Success, st1p["ST2P"]["P3D"].BindPoint3d(pST1P_ST2P_P3D));

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(idx++, pD));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDateTime(idx++, pDt));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindBoolean(idx++, pB));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint2d(idx++, pP2D));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint3d(idx++, pP3D));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindBlob(idx++, bin, (int) binSize, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindGeometry(idx++, *geom));

    //SELECT * .. []
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    m_ecdb.SaveChanges();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT Code, S, I, L, D, DT, B, P2D, P3D, ST1P, arrayOfP3d, arrayOfST1, BIN, Geom FROM  ts.TestElement WHERE Code = 'C8'"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    idx = 0;
    ASSERT_STREQ(pCode, stmt.GetValueText(idx++));  //Code
    ASSERT_STREQ(pS, stmt.GetValueText(idx++));     //S
    ASSERT_EQ(pI, stmt.GetValueInt(idx++));         //I
    ASSERT_EQ(pL, stmt.GetValueInt64(idx++));       //L
    ASSERT_EQ(pD, stmt.GetValueDouble(idx++));      //D
    ASSERT_EQ(pDt, stmt.GetValueDateTime(idx++));   //DT
    ASSERT_EQ(pB, stmt.GetValueBoolean(idx++));     //B
    ASSERT_EQ(pP2D, stmt.GetValuePoint2d(idx++));   //P2D
    ASSERT_EQ(pP3D, stmt.GetValuePoint3d(idx++));   //P3D

    IECSqlValue const& st1pv = stmt.GetValue(idx++);    //ST1P
    ASSERT_EQ(pST1P_D1, st1pv["D1"].GetDouble());
    ASSERT_EQ(pST1P_P2D, st1pv["P2D"].GetPoint2d());

    IECSqlValue const& st2pv = st1pv["ST2P"];  //ST1P.ST2P
    ASSERT_EQ(pST1P_ST2P_D2, st2pv["D2"].GetDouble());
    ASSERT_EQ(pST1P_ST2P_P3D, st2pv["P3D"].GetPoint3d());
    IECSqlValue const& arrayOfP3dv = stmt.GetValue(idx++); // //arrayOfP3d
    int i = 0;
    for (IECSqlValue const& arrayElement : arrayOfP3dv.GetArrayIterable())
        {
        ASSERT_TRUE(pArrayOfP3d[i].AlmostEqual(arrayElement.GetPoint3d())) << i;
        i++;
        }
    ASSERT_EQ(3, i);

    IECSqlValue const& arrayOfST1v = stmt.GetValue(idx++);  //arrayOfST1
    i = 0;
    for (IECSqlValue const& arrayElement : arrayOfST1v.GetArrayIterable())
        {
        ASSERT_EQ(pArrayOfST1_D1[i], arrayElement["D1"].GetDouble());//ST1P.D1
        ASSERT_TRUE(pArrayOfST1_P2D[i].AlmostEqual(arrayElement["P2D"].GetPoint2d()));//ST1P.P2D
        ASSERT_EQ(pArrayOfST1_D2[i], arrayElement["ST2P"]["D2"].GetDouble());//ST1P.STP2.D2
        ASSERT_TRUE(pArrayOfST1_P3D[i].AlmostEqual(arrayElement["ST2P"]["P3D"].GetPoint3d()));//ST1P.STP2.P3D
        i++;
        }
    ASSERT_EQ(3, i);
    ASSERT_EQ(0, memcmp(bin, stmt.GetValueBlob(idx++), binSize));  //Bin
    IGeometryPtr actualGeom = stmt.GetValueGeometry(idx++);
    ASSERT_TRUE(actualGeom->IsSameStructureAndGeometry(*geom));
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         11/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, Overflow_InsertComplexTypes)
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
        "</ECSchema>")));

    //Point2D(3,4) (23,43,32)
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.TestElement (Code, S, I, L, D, DT, B, P2D, P3D, BIN, ST1P, arrayOfP3d, arrayOfST1) "
                                                 "VALUES (:code, :s, :i, :l, :d, :dt, :b, :p2d, :p3d, :bin, :st1p, :arrayOfP3d, :arrayOfST1)"));
    Utf8CP pCode = "C8";
    Utf8CP pS = "SampleText";
    int    pI = 123452;
    int64_t pL = 123324234234;
    double pD = 1232.343234;
    DateTime pDt = DateTime(DateTime::Kind::Unspecified, 2016, 11, 23, 0, 0);
    bool pB = true;
    DPoint2d pP2D = DPoint2d::From(12.33, -12.34);
    DPoint3d pP3D = DPoint3d::From(22.13, -62.34, -13.12);
    void const* bin = &pL;
    size_t binSize = sizeof(pL);
    double pST1P_D1 = 431231.3432;
    DPoint2d pST1P_P2D = DPoint2d::From(-212.34, 2112.314);
    double pST1P_ST2P_D2 = 431231.3432;
    DPoint3d pST1P_ST2P_P3D = DPoint3d::From(-123.434, 3217.3, -1.03);
    DPoint3d pArrayOfP3d[] = {DPoint3d::From(-41.33, 41.13, -12.25), DPoint3d::From(-23.37, 53.54, -34.31), DPoint3d::From(-33.41, 11.13, -99.11)};
    double pArrayOfST1_D1[] = {123.3434, 345.223,-532.123};
    DPoint2d pArrayOfST1_P2D[] = {DPoint2d::From(-21, 22.1),DPoint2d::From(-85.34, 35.36),DPoint2d::From(-31.34, 12.35)};
    double pArrayOfST1_D2[] = {12.3, -45.72, -31.11};
    DPoint3d pArrayOfST1_P3D[] = {DPoint3d::From(-12.11, -74.1, 12.3),DPoint3d::From(-12.53, 21.76, -32.22),DPoint3d::From(-41.14, -22.45, -31.16)};

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, pCode, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(2, pS, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(3, pI));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(4, pL));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(5, pD));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDateTime(6, pDt));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindBoolean(7, pB));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint2d(8, pP2D));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint3d(9, pP3D));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindBlob(10, bin, (int) binSize, IECSqlBinder::MakeCopy::No));

    IECSqlBinder& st1p = stmt.GetBinder(11);
    ASSERT_EQ(ECSqlStatus::Success, st1p["D1"].BindDouble(pST1P_D1));
    ASSERT_EQ(ECSqlStatus::Success, st1p["P2D"].BindPoint2d(pST1P_P2D));
    ASSERT_EQ(ECSqlStatus::Success, st1p["ST2P"]["D2"].BindDouble(pST1P_ST2P_D2));
    ASSERT_EQ(ECSqlStatus::Success, st1p["ST2P"]["P3D"].BindPoint3d(pST1P_ST2P_P3D));

    IECSqlBinder& arrayOfP3d = stmt.GetBinder(12);
    for (size_t i = 0; i < 3; i++)
        {
        ASSERT_EQ(ECSqlStatus::Success, arrayOfP3d.AddArrayElement().BindPoint3d(pArrayOfP3d[i]));
        }

    IECSqlBinder& arrayOfST1 = stmt.GetBinder(13);
    for (size_t i = 0; i < 3; i++)
        {
        IECSqlBinder& elementBinder = arrayOfST1.AddArrayElement();
        ASSERT_EQ(ECSqlStatus::Success, elementBinder["D1"].BindDouble(pArrayOfST1_D1[i]));
        ASSERT_EQ(ECSqlStatus::Success, elementBinder["P2D"].BindPoint2d(pArrayOfST1_P2D[i]));
        ASSERT_EQ(ECSqlStatus::Success, elementBinder["ST2P"]["D2"].BindDouble(pArrayOfST1_D2[i]));
        ASSERT_EQ(ECSqlStatus::Success, elementBinder["ST2P"]["P3D"].BindPoint3d(pArrayOfST1_P3D[i]));
        }

    //SELECT * .. []
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    m_ecdb.SaveChanges();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT Code, S, I, L, D, DT, B, P2D, P3D, ST1P, arrayOfP3d, arrayOfST1, BIN FROM  ts.TestElement WHERE Code = 'C8'"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_STREQ(pCode, stmt.GetValueText(0)) << stmt.GetECSql(); 
    ASSERT_STREQ(pS, stmt.GetValueText(1)) << stmt.GetECSql();
    ASSERT_EQ(pI, stmt.GetValueInt(2)) << stmt.GetECSql();

    ASSERT_EQ(pL, stmt.GetValueInt64(3)) << stmt.GetECSql();
    ASSERT_EQ(pD, stmt.GetValueDouble(4)) << stmt.GetECSql();
    ASSERT_EQ(pDt, stmt.GetValueDateTime(5)) << stmt.GetECSql();
    ASSERT_EQ(pB, stmt.GetValueBoolean(6)) << stmt.GetECSql();
    ASSERT_EQ(pP2D, stmt.GetValuePoint2d(7)) << stmt.GetECSql();
    ASSERT_EQ(pP3D, stmt.GetValuePoint3d(8)) << stmt.GetECSql();

    IECSqlValue const& st1pv = stmt.GetValue(9);
    ASSERT_EQ(pST1P_D1, st1pv["D1"].GetDouble());
    ASSERT_EQ(pST1P_P2D, st1pv["P2D"].GetPoint2d());

    IECSqlValue const& st2pv = st1pv["ST2P"];  //ST1P.ST2P
    ASSERT_EQ(pST1P_ST2P_D2, st2pv["D2"].GetDouble());
    ASSERT_EQ(pST1P_ST2P_P3D, st2pv["P3D"].GetPoint3d());
    IECSqlValue const& arrayOfP3dv = stmt.GetValue(10); // //arrayOfP3d
    int i = 0;
    for (IECSqlValue const& element : arrayOfP3dv.GetArrayIterable())
        {
        ASSERT_EQ(pArrayOfP3d[i++], element.GetPoint3d());
        }
    ASSERT_EQ(3, i);

    IECSqlValue const& arrayOfST1v = stmt.GetValue(11);  //arrayOfST1
    i = 0;
    for (IECSqlValue const& arrayElement : arrayOfST1v.GetArrayIterable())
        {
        ASSERT_EQ(pArrayOfST1_D1[i], arrayElement["D1"].GetDouble());//ST1P.D1
        ASSERT_EQ(pArrayOfST1_P2D[i], arrayElement["P2D"].GetPoint2d());//ST1P.P2D
        ASSERT_EQ(pArrayOfST1_D2[i], arrayElement["ST2P"]["D2"].GetDouble());//ST1P.ST2P.D2
        ASSERT_EQ(pArrayOfST1_P3D[i], arrayElement["ST2P"]["P3D"].GetPoint3d());//ST1P.ST2P.P3D
        i++;
        }
    ASSERT_EQ(3, i);
    ASSERT_EQ(0, memcmp(bin, stmt.GetValueBlob(12), binSize)) << stmt.GetECSql();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         11/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, Overflow_UpdateComplexTypes)
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
        "</ECSchema>")));

    //Point2D(3,4) (23,43,32)

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.TestElement (Code) VALUES ('C9')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE ts.TestElement SET Code = :code, S = :s, I = :i, L = :l, D = :d, DT = :dt, B = :b, P2D = :p2d, P3D = :p3d, BIN = :bin, ST1P = :st1p, arrayOfP3d = :arrayOfP3d, arrayOfST1 = :arrayOfST1  WHERE Code = 'C9'"));
    int idx = 1;
    Utf8CP pCode = "C9";
    Utf8CP pS = "SampleText";
    int    pI = 123452;
    int64_t pL = 123324234234;
    double pD = 1232.343234;
    DateTime pDt = DateTime(DateTime::Kind::Unspecified, 2016, 11, 23, 0, 0);
    bool pB = true;
    DPoint2d pP2D = DPoint2d::From(12.33, -12.34);
    DPoint3d pP3D = DPoint3d::From(22.13, -62.34, -13.12);
    void const* bin = &pP3D;
    size_t binSize = sizeof(pP3D);
    double pST1P_D1 = 431231.3432;
    DPoint2d pST1P_P2D = DPoint2d::From(-212.34, 2112.314);
    double pST1P_ST2P_D2 = 431231.3432;
    DPoint3d pST1P_ST2P_P3D = DPoint3d::From(-123.434, 3217.3, -1.03);
    DPoint3d pArrayOfP3d[] = {DPoint3d::From(-41.33, 41.13, -12.25), DPoint3d::From(-23.37, 53.54, -34.31), DPoint3d::From(-33.41, 11.13, -99.11)};
    double pArrayOfST1_D1[] = {123.3434, 345.223,-532.123};
    DPoint2d pArrayOfST1_P2D[] = {DPoint2d::From(-21, 22.1),DPoint2d::From(-85.34, 35.36),DPoint2d::From(-31.34, 12.35)};
    double pArrayOfST1_D2[] = {12.3, -45.72, -31.11};
    DPoint3d pArrayOfST1_P3D[] = {DPoint3d::From(-12.11, -74.1, 12.3),DPoint3d::From(-12.53, 21.76, -32.22),DPoint3d::From(-41.14, -22.45, -31.16)};

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(idx++, pCode, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(idx++, pS, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(idx++, pI));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(idx++, pL));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(idx++, pD));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDateTime(idx++, pDt));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindBoolean(idx++, pB));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint2d(idx++, pP2D));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint3d(idx++, pP3D));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindBlob(idx++, bin, (int) binSize, IECSqlBinder::MakeCopy::No));

    IECSqlBinder& st1p = stmt.GetBinder(idx++);
    ASSERT_EQ(ECSqlStatus::Success, st1p["D1"].BindDouble(pST1P_D1));
    ASSERT_EQ(ECSqlStatus::Success, st1p["P2D"].BindPoint2d(pST1P_P2D));
    ASSERT_EQ(ECSqlStatus::Success, st1p["ST2P"]["D2"].BindDouble(pST1P_ST2P_D2));
    ASSERT_EQ(ECSqlStatus::Success, st1p["ST2P"]["P3D"].BindPoint3d(pST1P_ST2P_P3D));

    IECSqlBinder& arrayOfP3d = stmt.GetBinder(idx++);
    for (size_t i = 0; i < 3; i++)
        {
        ASSERT_EQ(ECSqlStatus::Success, arrayOfP3d.AddArrayElement().BindPoint3d(pArrayOfP3d[i]));
        }

    IECSqlBinder& arrayOfST1 = stmt.GetBinder(idx++);
    for (size_t i = 0; i < 3; i++)
        {
        IECSqlBinder& elementBinder = arrayOfST1.AddArrayElement();
        ASSERT_EQ(ECSqlStatus::Success, elementBinder["D1"].BindDouble(pArrayOfST1_D1[i]));
        ASSERT_EQ(ECSqlStatus::Success, elementBinder["P2D"].BindPoint2d(pArrayOfST1_P2D[i]));
        ASSERT_EQ(ECSqlStatus::Success, elementBinder["ST2P"]["D2"].BindDouble(pArrayOfST1_D2[i]));
        ASSERT_EQ(ECSqlStatus::Success, elementBinder["ST2P"]["P3D"].BindPoint3d(pArrayOfST1_P3D[i]));
        }

    //SELECT * .. []
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    m_ecdb.SaveChanges();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT Code, S, I, L, D, DT, B, P2D, P3D, ST1P, arrayOfP3d, arrayOfST1, BIN FROM  ts.TestElement WHERE Code = 'C9'"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    idx = 0;
    ASSERT_STREQ(pCode, stmt.GetValueText(idx++));  //Code
    ASSERT_STREQ(pS, stmt.GetValueText(idx++));     //S
    ASSERT_EQ(pI, stmt.GetValueInt(idx++));         //I
    ASSERT_EQ(pL, stmt.GetValueInt64(idx++));       //L
    ASSERT_EQ(pD, stmt.GetValueDouble(idx++));      //D
    ASSERT_EQ(pDt, stmt.GetValueDateTime(idx++));   //DT NOT SURE WHY COMPARE FAIL
    ASSERT_EQ(pB, stmt.GetValueBoolean(idx++));     //B
    ASSERT_EQ(pP2D, stmt.GetValuePoint2d(idx++));   //P2D
    ASSERT_EQ(pP3D, stmt.GetValuePoint3d(idx++));   //P3D

    IECSqlValue const& st1pv = stmt.GetValue(idx++);    //ST1P
    ASSERT_EQ(pST1P_D1, st1pv["D1"].GetDouble());
    ASSERT_EQ(pST1P_P2D, st1pv["P2D"].GetPoint2d());

    IECSqlValue const& st2pv = st1pv["ST2P"];  //ST1P.ST2P
    ASSERT_EQ(pST1P_ST2P_D2, st2pv["D2"].GetDouble());
    ASSERT_EQ(pST1P_ST2P_P3D, st2pv["P3D"].GetPoint3d());
    IECSqlValue const& arrayOfP3dv = stmt.GetValue(idx++); // //arrayOfP3d
    int i = 0;
    for (IECSqlValue const& element : arrayOfP3dv.GetArrayIterable())
        {
        ASSERT_EQ(pArrayOfP3d[i++], element.GetPoint3d());
        }
    ASSERT_EQ(3, i);
    IECSqlValue const& arrayOfST1v = stmt.GetValue(idx++);  //arrayOfST1
    i = 0;
    for (IECSqlValue const& arrayElement : arrayOfST1v.GetArrayIterable())
        {
        ASSERT_EQ(pArrayOfST1_D1[i], arrayElement["D1"].GetDouble());//ST1P.D1
        ASSERT_EQ(pArrayOfST1_P2D[i], arrayElement["P2D"].GetPoint2d());//ST1P.P2D
        ASSERT_EQ(pArrayOfST1_D2[i], arrayElement["ST2P"]["D2"].GetDouble());//ST1P.STP2.D2
        ASSERT_EQ(pArrayOfST1_P3D[i], arrayElement["ST2P"]["P3D"].GetPoint3d());//ST1P.STP2.P3D
        i++;
        }
    ASSERT_EQ(3, i);

    ASSERT_EQ(0, memcmp(bin, stmt.GetValueBlob(idx++), binSize));  //Bin
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                         02/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, ForeignKeyMappingOnJoinedTable_FailingScenarios)
    {
    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "  <ECEntityClass typeName='Model' >"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "        </ClassMap>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "    <ECProperty propertyName='Name' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='Element' >"
        "    <BaseClass>Model</BaseClass>"
        "    <ECProperty propertyName='Code' typeName='string' />"
        "    <ECNavigationProperty propertyName='Model' relationshipName='ModelHasElements' direction='Backward'>"
        "      <ECCustomAttributes>"
        "        <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
        "      </ECCustomAttributes>"
        "    </ECNavigationProperty>"
        "  </ECEntityClass>"
        "  <ECRelationshipClass typeName='ModelHasElements' modifier='None' strength='embedding' direction='Forward'>"
        "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Model Has Elements'>"
        "      <Class class='Model' />"
        "    </Source>"
        "    <Target multiplicity='(0..N)' polymorphic='True' roleLabel='Model Has Elements (Reversed)'>"
        "      <Class class='Element' />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>"))) << "ForeignKey End of a relationship mapped to a Joined Table, doesn't allow the strength to be 'embedding'.";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "  <ECEntityClass typeName='Model' >"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "        </ClassMap>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "    <ECProperty propertyName='Name' typeName='string' />" 
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='Element' >"
        "    <BaseClass>Model</BaseClass>"
        "    <ECProperty propertyName='Prop' typeName='string' />"
        "    <ECNavigationProperty propertyName='Model' relationshipName='ModelHasElements' direction='Backward'>"
        "      <ECCustomAttributes>"
        "        <ForeignKeyConstraint xmlns='ECDbMap.02.00'>"
        "         <OnDeleteAction>Cascade</OnDeleteAction>"
        "        </ForeignKeyConstraint>"
        "      </ECCustomAttributes>"
        "    </ECNavigationProperty>"
        "  </ECEntityClass>"
        "  <ECRelationshipClass typeName='ModelHasElements' modifier='None' strength='referencing' direction='Forward'>"
        "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Model Has Elements'>"
        "      <Class class='Model' />"
        "    </Source>"
        "    <Target multiplicity='(0..N)' polymorphic='True' roleLabel='Model Has Elements (Reversed)'>"
        "      <Class class='Element' />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>"))) << "The ForeignKeyConstraintCA can only define 'Cascade' as OnDeleteAction if the relationship strength is 'Embedding'.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, MaxSharedcolumnsbeforeoverflowBisScenario)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("MaxSharedColumnsBeforeOverflow.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Element' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='InformationElement' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Element</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='DefinitionElement' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <MaxSharedColumnsBeforeOverflow>50</MaxSharedColumnsBeforeOverflow>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <BaseClass>InformationElement</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='GeometricElement' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Element</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='GeometricElement2d' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <MaxSharedColumnsBeforeOverflow>1</MaxSharedColumnsBeforeOverflow>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <BaseClass>GeometricElement</BaseClass>"
        "        <ECProperty propertyName='GeometryStream' typeName='binary' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='GeometricElement3d' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <BaseClass>GeometricElement</BaseClass>"
        "        <ECProperty propertyName='GeometryStream' typeName='binary' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' modifier='None'>"
        "        <BaseClass>DefinitionElement</BaseClass>"
        "        <ECProperty propertyName='Sub1Prop1' typeName='double' />"
        "        <ECProperty propertyName='Sub1Prop2' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub11' modifier='None'>"
        "        <BaseClass>Sub1</BaseClass>"
        "        <ECProperty propertyName='Sub11Prop' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub2' modifier='None'>"
        "        <BaseClass>GeometricElement2d</BaseClass>"
        "        <ECProperty propertyName='Sub2Prop' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub3' modifier='None'>"
        "        <BaseClass>GeometricElement3d</BaseClass>"
        "        <ECProperty propertyName='Sub3Prop1' typeName='double' />"
        "        <ECProperty propertyName='Sub3Prop2' typeName='double' />"
        "        <ECProperty propertyName='Sub3Prop3' typeName='double' />"
        "        <ECProperty propertyName='Sub3Prop4' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>")));

    const int elementExpectedColCount = 3;
    const int definitionElementExpectedColCount = 5;
    int geometricElement2dExpectedColCount = 4;
    int geometricElement3dExpectedColCount = 7;

    std::vector<std::pair<Utf8String, int>> testItems;
    testItems.push_back(std::make_pair("ts_Element", elementExpectedColCount));
    testItems.push_back(std::make_pair("ts_DefinitionElement", definitionElementExpectedColCount));
    testItems.push_back(std::make_pair("ts_GeometricElement2d", geometricElement2dExpectedColCount));
    testItems.push_back(std::make_pair("ts_GeometricElement3d", geometricElement3dExpectedColCount));

    AssertColumnCount(m_ecdb, testItems, "after first schema import");

    ASSERT_EQ(SUCCESS, ImportSchema(m_ecdb, SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema2' nameSpacePrefix='ts2' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='TestSchema' version='01.00' prefix='ts' />"
        "    <ECEntityClass typeName='Sub4'>"
        "        <BaseClass>ts:InformationElement</BaseClass>"
        "        <ECProperty propertyName='Sub4Prop' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub21'>"
        "        <BaseClass>ts:Sub2</BaseClass>"
        "        <ECProperty propertyName='Sub21Prop' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub31'>"
        "        <BaseClass>ts:Sub3</BaseClass>"
        "        <ECProperty propertyName='Sub31Prop1' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>")));
    m_ecdb.SaveChanges();

    int geometricElement2dOverflowExpectedColCount = 3;
    int geometricElement3dOverflowExpectedColCount = 3;
    const int sub4ExpectedColCount = 3;

    testItems.clear();
    testItems.push_back(std::make_pair("ts_Element", elementExpectedColCount));
    testItems.push_back(std::make_pair("ts_DefinitionElement", definitionElementExpectedColCount));
    testItems.push_back(std::make_pair("ts_GeometricElement2d", geometricElement2dExpectedColCount));
    testItems.push_back(std::make_pair("ts_GeometricElement2d_Overflow", geometricElement2dOverflowExpectedColCount));
    testItems.push_back(std::make_pair("ts_GeometricElement3d", geometricElement3dExpectedColCount));
    testItems.push_back(std::make_pair("ts_GeometricElement3d_Overflow", geometricElement3dOverflowExpectedColCount));
    testItems.push_back(std::make_pair("ts2_Sub4", sub4ExpectedColCount));
    AssertColumnCount(m_ecdb, testItems, "after second schema import");

    ASSERT_EQ(SUCCESS, ImportSchema(m_ecdb, SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema3' nameSpacePrefix='ts3' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='TestSchema' version='01.00' prefix='ts' />"
        "    <ECEntityClass typeName='Sub32'>"
        "        <BaseClass>ts:Sub3</BaseClass>"
        "        <ECProperty propertyName='Sub32Prop1' typeName='double' />"
        "        <ECProperty propertyName='Sub32Prop2' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>")));
    m_ecdb.SaveChanges();
    
    geometricElement3dOverflowExpectedColCount+=1;
    testItems.clear();
    testItems.push_back(std::make_pair("ts_Element", elementExpectedColCount));
    testItems.push_back(std::make_pair("ts_DefinitionElement", definitionElementExpectedColCount));
    testItems.push_back(std::make_pair("ts_GeometricElement2d", geometricElement2dExpectedColCount));
    testItems.push_back(std::make_pair("ts_GeometricElement2d_Overflow", geometricElement2dOverflowExpectedColCount));
    testItems.push_back(std::make_pair("ts_GeometricElement3d", geometricElement3dExpectedColCount));
    testItems.push_back(std::make_pair("ts_GeometricElement3d_Overflow", geometricElement3dOverflowExpectedColCount));
    testItems.push_back(std::make_pair("ts2_Sub4", sub4ExpectedColCount));

    AssertColumnCount(m_ecdb, testItems, "after third schema import");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, ShareColumnsCA_TableLayout)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ShareColumnsCA.ecdb",SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                        "<ECSchema schemaName='SchemaWithShareColumnsCA' nameSpacePrefix='rc' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.00' prefix='bsca' />"
                        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                        "    <ECEntityClass typeName='BaseClass' modifier='None'>"
                        "        <ECCustomAttributes>"
                        "            <ClassMap xmlns='ECDbMap.02.00'>"
                        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                        "            </ClassMap>"
                        "            <ShareColumns xmlns='ECDbMap.02.00'>"
                        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
                        "            </ShareColumns>"
                        "        </ECCustomAttributes>"
                        "        <ECProperty propertyName='P1' typeName='string' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='ChildDomainClassA' modifier='None'>"
                        "        <BaseClass>BaseClass</BaseClass>"
                        "        <ECProperty propertyName='P2' typeName='double' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='ChildDomainClassB' modifier='None'>"
                        "        <BaseClass>BaseClass</BaseClass>"
                        "        <ECProperty propertyName='P3' typeName='int' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='DerivedA' modifier='None'>"
                        "        <BaseClass>ChildDomainClassA</BaseClass>"
                        "        <ECProperty propertyName='P4' typeName='double' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='DerivedB' modifier='None'>"
                        "        <BaseClass>ChildDomainClassA</BaseClass>"
                        "        <ECProperty propertyName='P5' typeName='string' />"
                        "    </ECEntityClass>"
                        "</ECSchema>")));

    //verify tables
    ASSERT_TRUE(m_ecdb.TableExists("rc_BaseClass"));
    ASSERT_FALSE(m_ecdb.TableExists("rc_ChildDomainClassA"));
    ASSERT_FALSE(m_ecdb.TableExists("rc_ChildDomainClassB"));
    ASSERT_FALSE(m_ecdb.TableExists("rc_DerivedA"));
    ASSERT_FALSE(m_ecdb.TableExists("rc_DerivedB"));

    //verify ECSqlStatments
    ECSqlStatement s1, s2, s3, s4, s5;
    ASSERT_EQ(s1.Prepare(m_ecdb, "INSERT INTO rc.BaseClass (P1) VALUES('HelloWorld')"), ECSqlStatus::Success);
    ASSERT_EQ(BE_SQLITE_DONE, s1.Step());
    ASSERT_EQ(s2.Prepare(m_ecdb, "INSERT INTO rc.ChildDomainClassA (P1, P2) VALUES('ChildClassA', 10.002)"), ECSqlStatus::Success);
    ASSERT_EQ(BE_SQLITE_DONE, s2.Step());
    ASSERT_EQ(s3.Prepare(m_ecdb, "INSERT INTO rc.ChildDomainClassB (P1, P3) VALUES('ChildClassB', 2)"), ECSqlStatus::Success);
    ASSERT_EQ(BE_SQLITE_DONE, s3.Step());
    ASSERT_EQ(s4.Prepare(m_ecdb, "INSERT INTO rc.DerivedA (P1, P2, P4) VALUES('DerivedA', 11.003, 12.004)"), ECSqlStatus::Success);
    ASSERT_EQ(BE_SQLITE_DONE, s4.Step());
    ASSERT_EQ(s5.Prepare(m_ecdb, "INSERT INTO rc.DerivedB (P1, P2, P5) VALUES('DerivedB', 11.003, 'DerivedB')"), ECSqlStatus::Success);
    ASSERT_EQ(BE_SQLITE_DONE, s5.Step());

    //verify No of Columns in BaseClass
    Statement statement;
    ASSERT_EQ(BE_SQLITE_OK, statement.Prepare(m_ecdb, "SELECT * FROM rc_BaseClass"));
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(5, statement.GetColumnCount());

    //verify that the columns generated are same as expected
    Utf8CP expectedColumnNames = "IdECClassIdP1ps1ps2";
    Utf8String actualColumnNames;
    for (int i = 0; i < statement.GetColumnCount(); i++)
        {
        actualColumnNames.append(statement.GetColumnName(i));
        }
    ASSERT_STREQ(expectedColumnNames, actualColumnNames.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   09/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, TablePrefix)
    {
            {
            ECDb ecdb;
            ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb,
                                                         SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                                                    "<ECSchema schemaName='test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                                                    "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                                                    "        <ECCustomAttributes>"
                                                                    "            <SchemaMap xmlns='ECDbMap.02.00'>"
                                                                    "                <TablePrefix>myownprefix</TablePrefix>"
                                                                    "            </SchemaMap>"
                                                                    "        </ECCustomAttributes>"
                                                                    "    <ECEntityClass typeName='A' modifier='None'>"
                                                                    "        <ECProperty propertyName='P1' typeName='string' />"
                                                                    "    </ECEntityClass>"
                                                                    "    <ECEntityClass typeName='B' modifier='None'>"
                                                                    "        <ECProperty propertyName='P2' typeName='string' />"
                                                                    "    </ECEntityClass>"
                                                                    "</ECSchema>"), "tableprefix.ecdb"));

            //verify tables
            ASSERT_TRUE(ecdb.TableExists("myownprefix_A"));
            ASSERT_TRUE(ecdb.TableExists("myownprefix_B"));
            ASSERT_FALSE(ecdb.TableExists("ts_A"));
            ASSERT_FALSE(ecdb.TableExists("ts_B"));
            }

            {
            ECDb ecdb;
            ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb,
                                                         SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                "<ECSchema schemaName='test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                "        <ECCustomAttributes>"
                                "            <SchemaMap xmlns='ECDbMap.02.00'>"
                                "            </SchemaMap>"
                                "        </ECCustomAttributes>"
                                "    <ECEntityClass typeName='A' modifier='None'>"
                                "        <ECProperty propertyName='P1' typeName='string' />"
                                "    </ECEntityClass>"
                                "    <ECEntityClass typeName='B' modifier='None'>"
                                "        <ECProperty propertyName='P2' typeName='string' />"
                                "    </ECEntityClass>"
                                "</ECSchema>"), "tableprefix.ecdb"));

            //verify tables
            ASSERT_TRUE(ecdb.TableExists("ts_A"));
            ASSERT_TRUE(ecdb.TableExists("ts_B"));
            }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     11/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, ShareColumnsCAAcrossMultipleSchemaImports)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ShareColumnsCAForSubclasses.ecdb",SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                        "<ECSchema schemaName='ReferredSchema' nameSpacePrefix='rs' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                        "    <ECEntityClass typeName='Base' modifier='None'>"
                        "        <ECCustomAttributes>"
                        "            <ClassMap xmlns='ECDbMap.02.00'>"
                        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                        "            </ClassMap>"
                        "            <ShareColumns xmlns='ECDbMap.02.00'>"
                        "              <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>"
                        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
                        "            </ShareColumns>"
                        "        </ECCustomAttributes>"
                        "        <ECProperty propertyName='P0' typeName='int' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='Sub1' modifier='None'>"
                        "         <BaseClass>Base</BaseClass>"
                        "        <ECProperty propertyName='P1' typeName='int' />"
                        "    </ECEntityClass>"
                        "</ECSchema>"))) << "Mapstrategy Option SharedColumnForSubClasses (applied to subclasses) is expected to succeed";

    ASSERT_EQ(SUCCESS, ImportSchema(m_ecdb, SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                              "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                              "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                              "    <ECSchemaReference name='ReferredSchema' version='01.00' prefix='rs' />"
                              "    <ECEntityClass typeName='Sub2' modifier='None'>"
                              "         <BaseClass>rs:Sub1</BaseClass>"
                              "        <ECProperty propertyName='P2' typeName='int' />"
                              "    </ECEntityClass>"
                              "</ECSchema>"))) << "MapStrategy Option SharedColumnForSubClasses (applied to subclasses) is expected to be honored from base Class of Refered schema";

    //verify Number and Names of Columns in BaseClass
    Statement statement;
    const int expectedColCount = 5;
    ASSERT_EQ(BE_SQLITE_OK, statement.Prepare(m_ecdb, "SELECT * FROM rs_Base"));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    ASSERT_EQ(expectedColCount, statement.GetColumnCount());

    //verify that the columns generated are same as expected
    Utf8CP expectedColumnNames = "IdECClassIdP0ps1ps2";
    Utf8String actualColumnNames;
    for (int i = 0; i < expectedColCount; i++)
        {
        actualColumnNames.append(statement.GetColumnName(i));
        }
    ASSERT_STREQ(expectedColumnNames, actualColumnNames.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, AbstractClass)
    {
    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='AbstractClassTest' nameSpacePrefix='t' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='AbstractClass' modifier='Abstract'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                   "            </ClassMap>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='Prop' typeName='int' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>"))) << "Abstract class with TablePerHierarchy MapStrategy";

    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='AbstractClassTest' nameSpacePrefix='t' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='AbstractClass' modifier='Abstract'>"
                                   "        <ECProperty propertyName='Prop' typeName='int' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>"))) << "Abstract class with no MapStrategy specified";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='AbstractClassTest' nameSpacePrefix='t' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='AbstractClass' modifier='Abstract'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>ExistingTable</MapStrategy>"
                                   "                <TableName>MyName</TableName>"
                                   "            </ClassMap>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='Prop' typeName='int' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>"))) << "Abstract class can only have TablePerHierarchy MapStrategy";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='AbstractClassTest' nameSpacePrefix='t' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='AbstractClass' modifier='Abstract'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>OwnTable</MapStrategy>"
                                   "            </ClassMap>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='Prop' typeName='int' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>"))) << "Abstract class can only have TablePerHierarchy MapStrategy";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, PropertyWithSameNameAsStructMemberColumn)
    {
            {
            ECDb ecdb;
            ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, SchemaItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
                                "  <ECStructClass typeName='ElementCode' modifier='None'>"
                                "    <ECProperty propertyName='Name' typeName='string' />"
                                "  </ECStructClass>"
                                "  <ECEntityClass typeName='Foo' modifier='None'>"
                                "    <ECProperty propertyName='Code_Name' typeName='string' />"
                                "    <ECStructProperty propertyName='Code' typeName='ElementCode' />"
                                "  </ECEntityClass>"
                                "</ECSchema>"), "propertywithsamenameasstructmembercol.ecdb"));

            ASSERT_TRUE(ecdb.ColumnExists("ts_Foo", "Code_Name"));

            ECClassId fooClassId = ecdb.Schemas().GetClassId("TestSchema", "Foo");
            ASSERT_TRUE(fooClassId.IsValid());

            Utf8String expectedColumnName;
            expectedColumnName.Sprintf("c%s_Code_Name", fooClassId.ToString().c_str());
            ASSERT_TRUE(ecdb.ColumnExists("ts_Foo", expectedColumnName.c_str()));
            }

            {
            //now flip order of struct prop and prim prop
            ECDb ecdb;
            ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, SchemaItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
                                "  <ECStructClass typeName='ElementCode' modifier='None'>"
                                "    <ECProperty propertyName='Name' typeName='string' />"
                                "  </ECStructClass>"
                                "  <ECEntityClass typeName='Foo' modifier='None'>"
                                "    <ECStructProperty propertyName='Code' typeName='ElementCode' />"
                                "    <ECProperty propertyName='Code_Name' typeName='string' />"
                                "  </ECEntityClass>"
                                "</ECSchema>"), "propertywithsamenameasstructmembercol.ecdb"));


            ASSERT_TRUE(ecdb.ColumnExists("ts_Foo", "Code_Name"));
            ECClassId fooClassId = ecdb.Schemas().GetClassId("TestSchema", "Foo");
            ASSERT_TRUE(fooClassId.IsValid());

            Utf8String expectedColumnName;
            expectedColumnName.Sprintf("c%s_Code_Name", fooClassId.ToString().c_str());
            ASSERT_TRUE(ecdb.ColumnExists("ts_Foo", expectedColumnName.c_str()));
            }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, InstanceInsertionForClassMappedToExistingTable)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("VerifyInstanceInsertionForClassMappedToExistingTable.ecdb"));

    ASSERT_FALSE(m_ecdb.TableExists("TestTable"));
    m_ecdb.CreateTable("TestTable", "Id INTEGER PRIMARY KEY, Name TEXT, Date INTEGER");
    ASSERT_TRUE(m_ecdb.TableExists("TestTable"));
    m_ecdb.SaveChanges();

    ASSERT_EQ(SUCCESS, ImportSchema(m_ecdb, SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='t' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "<ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "<ECEntityClass typeName='TestClass' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>ExistingTable</MapStrategy>"
        "                <TableName>TestTable</TableName>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "  <ECProperty propertyName='Name' typeName='string'/>"
        "  <ECProperty propertyName='Date' typeName='int'/>"
        "</ECEntityClass>"
        "</ECSchema>")));

    //Verifying that the class is not mapped to any table other than the Existing Table.
    ASSERT_FALSE(m_ecdb.TableExists("t_Class"));

    ECClassCP testClass = m_ecdb.Schemas().GetClass("TestSchema", "TestClass");
    ASSERT_TRUE(testClass != nullptr);

    ECInstanceInserter inserter(m_ecdb, *testClass, nullptr);
    ASSERT_FALSE(inserter.IsValid());

    ECInstanceUpdater updater(m_ecdb, *testClass, nullptr);
    ASSERT_FALSE(updater.IsValid());

    ECInstanceDeleter deleter(m_ecdb, *testClass, nullptr);
    ASSERT_FALSE(deleter.IsValid());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, MapRelationshipsToExistingTable)
    {
    /*-----------------------------------------------
    LinkTable relationship against existing link table
    ------------------------------------------------*/
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("linktablerelationshipmappedtoexistinglinktable.ecdb"));

    m_ecdb.CreateTable("TestTable", "Id INTEGER PRIMARY KEY, relProp INTEGER, SourceId INTEGER, TargetId INTEGER");
    ASSERT_TRUE(m_ecdb.TableExists("TestTable"));
    m_ecdb.SaveChanges();

    ASSERT_EQ(SUCCESS, ImportSchema(m_ecdb, SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='t' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "<ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "<ECEntityClass typeName='Foo' modifier='None' >"
        "   <ECProperty propertyName='FooProp' typeName='int' />"
        "</ECEntityClass>"
        "<ECEntityClass typeName='Goo' modifier='None' >"
        "   <ECProperty propertyName='GooProp' typeName='int' />"
        "</ECEntityClass>"
        "<ECRelationshipClass typeName='FooHasGoo' modifier='Sealed' strength='referencing'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>ExistingTable</MapStrategy>"
        "                <TableName>TestTable</TableName>"
        "            </ClassMap>"
        "            <LinkTableRelationshipMap xmlns = 'ECDbMap.02.00'>"
        "            </LinkTableRelationshipMap>"
        "        </ECCustomAttributes>"
        "    <Source cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Foo' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Goo' />"
        "    </Target>"
        "   <ECProperty propertyName='relProp' typeName='int' />"
        "</ECRelationshipClass>"
        "</ECSchema>"))) << "link table mapping to existing table";

    //Verify ECSql
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO t.Foo(ECInstanceId, FooProp) VALUES(1, 1)")) << " link table mapping to existing table";
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << " link table mapping to existing table";
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO t.Goo(ECInstanceId, GooProp) VALUES(1, 1)")) << " link table mapping to existing table";
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << " link table mapping to existing table";
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT count(*) FROM t.FooHasGoo")) << " link table mapping to existing table";
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " link table mapping to existing table";
    ASSERT_EQ(0, stmt.GetValueInt(0)) << stmt.GetECSql() << " link table mapping to existing table";
    stmt.Finalize();

    //cannot modify classes with 'existing table' 
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "INSERT INTO t.FooHasGoo (SourceECInstanceId, TargetECInstanceId) VALUES(1, 1)")) << " link table mapping to existing table";
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "UPDATE t.FooHasGoo set relProp=10 WHERE ECInstanceId=1")) << " link table mapping to existing table";
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "DELETE FROM t.FooHasGoo")) << " link table mapping to existing table";
    stmt.Finalize();
    }

    /*----------------------------------------------------------------------------------------------------------------------------
    CA linkTableRelationshipMap not applied nor the ExistingTable Contains Columns named SourceId and TargetId
    -----------------------------------------------------------------------------------------------------------------------------*/
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("linktablerelationshipmappedtoexistinglinktable.ecdb"));

    m_ecdb.CreateTable("TestTable", "Id INTEGER PRIMARY KEY, relProp INTEGER, MySourceId INTEGER, MyTargetId INTEGER");
    ASSERT_TRUE(m_ecdb.TableExists("TestTable"));
    m_ecdb.SaveChanges();

    ASSERT_EQ(ERROR, ImportSchema(m_ecdb, SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='t' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "<ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "<ECEntityClass typeName='Foo' modifier='None' >"
        "   <ECProperty propertyName='FooProp' typeName='int' />"
        "</ECEntityClass>"
        "<ECEntityClass typeName='Goo' modifier='None' >"
        "   <ECProperty propertyName='GooProp' typeName='int' />"
        "</ECEntityClass>"
        "<ECRelationshipClass typeName='FooHasGoo' modifier='Sealed' strength='referencing'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>ExistingTable</MapStrategy>"
        "                <TableName>TestTable</TableName>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "    <Source cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Foo' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Goo' />"
        "    </Target>"
        "   <ECProperty propertyName='relProp' typeName='int' />"
        "</ECRelationshipClass>")));
    }

    /*---------------------------------------------
    FK relationship class mapping to existing table
    ----------------------------------------------*/
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("fkrelationshipclassmappedtoexistingtable.ecdb"));

    m_ecdb.CreateTable("TestTable", "Id INTEGER PRIMARY KEY, GooProp INTEGER, ForeignKeyId INTEGER");
    ASSERT_TRUE(m_ecdb.TableExists("TestTable"));
    m_ecdb.SaveChanges();

    ASSERT_EQ(SUCCESS, ImportSchema(m_ecdb, SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='t' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "<ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "<ECEntityClass typeName='Foo' modifier='None' >"
        "   <ECProperty propertyName='FooProp' typeName='int' />"
        "</ECEntityClass>"
        "<ECEntityClass typeName='Goo' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>ExistingTable</MapStrategy>"
        "                <TableName>TestTable</TableName>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "   <ECProperty propertyName='GooProp' typeName='int' />"
        "   <ECNavigationProperty propertyName='ForeignKeyId' relationshipName='FooHasGoo' direction='Backward' />"
        "</ECEntityClass>"
        "<ECRelationshipClass typeName='FooHasGoo' modifier='Sealed' strength='referencing'>"
        "    <Source cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'Foo' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Goo' />"
        "    </Target>"
        "</ECRelationshipClass>"
        "</ECSchema>"))) << "FK mapping to existing table";

    //verify ECSql
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO t.Foo(ECInstanceId, FooProp) VALUES(1, 1)")) << "FK mapping to existing table";
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << " FK mapping to existing table";
    stmt.Finalize();

    //ECSql on Classes mapped to existing table
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM t.Goo")) << "FK mapping to existing table";
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " FK mapping to existing table";
    ASSERT_EQ(0, stmt.GetValueInt(0)) << stmt.GetECSql() << " FK mapping to existing table";
    stmt.Finalize();

    //cannot modify classes mapped to existing table
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "INSERT INTO t.Goo(ECInstanceId, GooProp) VALUES(2, 2)")) << "FK mapping to existing table";
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "UPDATE t.Goo SET GooProp=3 WHERE GooProp=2 AND ECInstanceId=2")) << "FK mapping to existing table";
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "DELETE FROM t.Goo")) << "FK mapping to existing table";
    stmt.Finalize();

    //ECSql on FK relationship mapped to existing table
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM t.FooHasGoo")) << "FK mapping to existing table";
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " FK mapping to existing table";
    ASSERT_EQ(0, stmt.GetValueInt(0)) << stmt.GetECSql() << " FK mapping to existing table";
    stmt.Finalize();

    //cannot modify relationship mapped to existing table
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "INSERT INTO t.FooHasGoo(SourceECInstanceId, TargetECInstanceId) VALUES(1, 1)")) << "FK mapping to existing table";
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "DELETE FROM t.FooHasGoo")) << "FK mapping to existing table";
    stmt.Finalize();
    }

    
    /*--------------------------------------------------------------------------------------------------------------------
    CA ForiegnKeyRelationshipMap not applied to relationshp class nor the Existing table contains column with required name
    ---------------------------------------------------------------------------------------------------------------------*/
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("fkrelationshipclassmappedtoexistingtable.ecdb"));
    m_ecdb.CreateTable("TestTable", "Id INTEGER PRIMARY KEY, GooProp INTEGER, ForeignKeyId INTEGER");
    ASSERT_TRUE(m_ecdb.TableExists("TestTable"));
    m_ecdb.SaveChanges();

    ASSERT_EQ(ERROR, ImportSchema(m_ecdb, SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='t' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "<ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "<ECEntityClass typeName='Foo' modifier='None' >"
        "   <ECProperty propertyName='FooProp' typeName='int' />"
        "</ECEntityClass>"
        "<ECEntityClass typeName='Goo' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>ExistingTable</MapStrategy>"
        "                <TableName>TestTable</TableName>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "   <ECProperty propertyName='GooProp' typeName='int' />"
        "   <ECNavigationProperty propertyName='Foo' relationshipName='FooHasGoo' direction='Backward'/>"
        "</ECEntityClass>"
        "<ECRelationshipClass typeName='FooHasGoo' modifier='Sealed' strength='referencing'>"
        "    <Source cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'Foo' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Goo' />"
        "    </Target>"
        "</ECRelationshipClass>"
        "</ECSchema>"))) << "ForeignKeyRelationshipMap CA not found nor the Existing table contains column with required name";
    }

    

    //Cardinality implying NotNull or FK Column should not be allowed to map to existing table.
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("existingtablenavproperty.ecdb"));

    m_ecdb.CreateTable("TestTable", "Id INTEGER PRIMARY KEY, GooProp INTEGER, navProp INTEGER");
    ASSERT_TRUE(m_ecdb.TableExists("TestTable"));
    m_ecdb.SaveChanges();

    ASSERT_EQ(ERROR, ImportSchema(m_ecdb, SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='t' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "<ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "<ECEntityClass typeName='Foo' modifier='None' >"
        "   <ECProperty propertyName='FooProp' typeName='int' />"
        "</ECEntityClass>"
        "<ECEntityClass typeName='Goo' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>ExistingTable</MapStrategy>"
        "                <TableName>TestTable</TableName>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "   <ECProperty propertyName='GooProp' typeName='int' />"
        "   <ECNavigationProperty propertyName = 'navProp' relationshipName = 'FooHasManyGoo' direction = 'backward' />"
        "</ECEntityClass>"
        "<ECRelationshipClass typeName='FooHasManyGoo' modifier='Sealed' strength='embedding'>"
        "    <Source cardinality='(1,1)' polymorphic='false'>"
        "      <Class class = 'Foo' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='true'>"
        "      <Class class = 'Goo' />"
        "    </Target>"
        "</ECRelationshipClass>"
        "</ECSchema>"))) << "Cardinality implying NotNull or FK Column is expected to be not supported";
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     11/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, NotNullConstraint)
    {

            //NotNull constraint on FK Relationship for OwnTable
            {
            ECDb db;
            ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(db, SchemaItem(
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                "<ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                "<ECEntityClass typeName='Foo' modifier='None' >"
                "   <ECProperty propertyName='FooProp' typeName='int' />"
                "</ECEntityClass>"
                "<ECEntityClass typeName='Goo' modifier='None' >"
                "   <ECProperty propertyName='GooProp' typeName='int' />"
                "   <ECNavigationProperty propertyName='Foo' relationshipName='FooHasGoo' direction='Backward'>"
                "        <ECCustomAttributes>"
                "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                "        </ECCustomAttributes>"
                "    </ECNavigationProperty>"
                "</ECEntityClass>"
                "<ECRelationshipClass typeName='FooHasGoo' modifier='Sealed' strength='referencing'>"
                "    <Source multiplicity='(0..1)' polymorphic='false' roleLabel='Foo'>"
                "      <Class class = 'Foo' />"
                "    </Source>"
                "    <Target multiplicity='(1..1)' polymorphic='false' roleLabel='Goo'>"
                "      <Class class = 'Goo' />"
                "    </Target>"
                "</ECRelationshipClass>"
                "</ECSchema>"), "NotNull.ecdb")) << "NotNull constraint is honoured when a single class is mapped to a table.";

            Statement sqlstmt;
            ASSERT_EQ(BE_SQLITE_OK, sqlstmt.Prepare(db, "SELECT NotNullConstraint FROM ec_Column WHERE Name='FooId'"));
            ASSERT_EQ(BE_SQLITE_ROW, sqlstmt.Step());
            ASSERT_EQ(0, sqlstmt.GetValueInt(0));
            }

            //NotNull constraint on FK Relationship for SharedTable CA 
            {
            ECDb db;
            ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(db, SchemaItem(
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                "<ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                "<ECEntityClass typeName='Parent' modifier='None' >"
                "        <ECCustomAttributes>"
                "            <ClassMap xmlns='ECDbMap.02.00'>"
                "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                "            </ClassMap>"
                "        </ECCustomAttributes>"
                "   <ECProperty propertyName='ParentProp' typeName='int' />"
                "</ECEntityClass>"
                "<ECEntityClass typeName='Child' modifier='None' >"
                "   <BaseClass>Parent</BaseClass>"
                "   <ECProperty propertyName='ChildAProp' typeName='int' />"
                "   <ECNavigationProperty propertyName='Parent' relationshipName='ParentHasChild' direction='Backward'>"
                "        <ECCustomAttributes>"
                "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                "        </ECCustomAttributes>"
                "   </ECNavigationProperty>"
                "</ECEntityClass>"
                "<ECRelationshipClass typeName='ParentHasChild' modifier='Sealed' strength='referencing'>"
                "    <Source cardinality='(0,1)' polymorphic='false'>"
                "      <Class class = 'Parent' />"
                "    </Source>"
                "    <Target cardinality='(1,N)' polymorphic='false'>"
                "      <Class class = 'Child' />"
                "    </Target>"
                "</ECRelationshipClass>"
                "</ECSchema>"), "NotNull.ecdb")) << "NotNull constraint is honoured when multiple classes are mapped to a same table.";

            Statement sqlstmt;
            ASSERT_EQ(BE_SQLITE_OK, sqlstmt.Prepare(db, "SELECT NotNullConstraint FROM ec_Column WHERE Name='ParentId'"));
            ASSERT_EQ(BE_SQLITE_ROW, sqlstmt.Step());
            ASSERT_EQ(0, sqlstmt.GetValueInt(0));
            }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     12/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, NotNullConstraintForRelationshipClassId)
    {
    //NotNull determination on RelClassIdColumn for FK Relationship
    ASSERT_EQ(SUCCESS, SetupECDb("NotNullConstraintOnRelClassIdColumn.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "<ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "<ECEntityClass typeName='Foo' modifier='None' >"
        "   <ECProperty propertyName='FooProp' typeName='int' />"
        "</ECEntityClass>"
        "<ECEntityClass typeName='Goo' modifier='None' >"
        "   <ECProperty propertyName='GooProp' typeName='int' />"
        "   <ECNavigationProperty propertyName='Foo' relationshipName='FooHasGoo' direction='Backward'/>"
        "</ECEntityClass>"
        "<ECRelationshipClass typeName='FooHasGoo' modifier='Sealed' strength='referencing'>"
        "    <Source multiplicity='(0..1)' polymorphic='false' roleLabel='Foo'>"
        "      <Class class = 'Foo' />"
        "    </Source>"
        "    <Target multiplicity='(1..1)' polymorphic='false' roleLabel='Goo'>"
        "      <Class class = 'Goo' />"
        "    </Target>"
        "</ECRelationshipClass>"
        "</ECSchema>"))) << "NotNull constraint is honoured for an FK Relationship.";

    Statement sqlstmt;
    ASSERT_EQ(BE_SQLITE_OK, sqlstmt.Prepare(m_ecdb, "SELECT NotNullConstraint FROM ec_Column WHERE Name='FooRelECClassId'"));
    ASSERT_EQ(BE_SQLITE_ROW, sqlstmt.Step());
    ASSERT_EQ(0, sqlstmt.GetValueInt(0));
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     12/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, ConstraintCheckOnProperties)
    {
            {
            ECDb db;
            ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(db, SchemaItem(
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                "<ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                "<ECEntityClass typeName='Foo' modifier='None' >"
                "           <ECCustomAttributes>"
                "            <ClassMap xmlns='ECDbMap.02.00'>"
                "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                "            </ClassMap>"
                "           </ECCustomAttributes>"
                "  <ECProperty propertyName='P1' typeName='int'>"
                "           <ECCustomAttributes>"
                "            <PropertyMap xmlns='ECDbMap.02.00'>"
                "               <IsUnique>true</IsUnique>"
                "            </PropertyMap>"
                "           </ECCustomAttributes>"
                "  </ECProperty>"
                "  <ECProperty propertyName='P2' typeName='int'>"
                "           <ECCustomAttributes>"
                "            <PropertyMap xmlns='ECDbMap.02.00'>"
                "               <IsNullable>false</IsNullable>"
                "            </PropertyMap>"
                "           </ECCustomAttributes>"
                "  </ECProperty>"
                "</ECEntityClass>"
                "<ECEntityClass typeName='Goo' modifier='None' >"
                "<BaseClass>Foo</BaseClass>"
                "   <ECProperty propertyName='GooProp' typeName='string' />"
                "</ECEntityClass>"
                "</ECSchema>"), "ConstraintCheckOnProperties.ecdb")) << "NotNull and IsUnique constraints on a subclass are expected to succeed.";

            //Verifying IsUnique constraint.
            ECSqlStatement stmt;

            //On the class itself
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "INSERT INTO ts.Foo(P1, P2) VALUES (1, 11)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "INSERT INTO ts.Foo(P1, P2) VALUES (1, 12)"));
            ASSERT_EQ(BE_SQLITE_CONSTRAINT_UNIQUE, stmt.Step());
            stmt.Finalize();

            //On subclass
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "INSERT INTO ts.Goo(P1, P2, GooProp) VALUES (2, 12, 'val1')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "INSERT INTO ts.Goo(P1, P2, GooProp) VALUES (2, 13, 'val2')"));
            ASSERT_EQ(BE_SQLITE_CONSTRAINT_UNIQUE, stmt.Step()); // As P1 has CA IsUnique applied to it.
            stmt.Finalize();

            //Verifying IsNullable constraint

            //On the class itself
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "INSERT INTO ts.Foo(P1) VALUES (3)"));
            ASSERT_EQ(BE_SQLITE_CONSTRAINT_NOTNULL, stmt.Step()); // As P2 has CA IsNullable applied to it.
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "INSERT INTO ts.Foo(P2) VALUES (null)"));
            ASSERT_EQ(BE_SQLITE_CONSTRAINT_NOTNULL, stmt.Step());
            stmt.Finalize();

            //On subclass
            //Implicitly inserting null for P2
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "INSERT INTO ts.Goo(GooProp) VALUES (11)"));
            ASSERT_EQ(BE_SQLITE_CONSTRAINT_NOTNULL, stmt.Step());
            stmt.Finalize();

            //Explicitly inserting null for P2
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "INSERT INTO ts.Goo(P2, GooProp) VALUES (null, 11)"));
            ASSERT_EQ(BE_SQLITE_CONSTRAINT_NOTNULL, stmt.Step());
            stmt.Finalize();
            }

            //Verification of IsUnique/IsNullable properties on Shared Columns
            {
            ECDb db;
            ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(db, SchemaItem(
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                "<ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                "<ECEntityClass typeName='Foo' modifier='None' >"
                "           <ECCustomAttributes>"
                "            <ClassMap xmlns='ECDbMap.02.00'>"
                "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                "            </ClassMap>"
                "            <ShareColumns xmlns='ECDbMap.02.00'>"
                "            </ShareColumns>"
                "           </ECCustomAttributes>"
                "  <ECProperty propertyName='P1' typeName='int'>"
                "           <ECCustomAttributes>"
                "            <PropertyMap xmlns='ECDbMap.02.00'>"
                "               <IsUnique>true</IsUnique>"
                "            </PropertyMap>"
                "           </ECCustomAttributes>"
                "  </ECProperty>"
                "  <ECProperty propertyName='P2' typeName='int'>"
                "           <ECCustomAttributes>"
                "            <PropertyMap xmlns='ECDbMap.02.00'>"
                "               <IsNullable>false</IsNullable>"
                "            </PropertyMap>"
                "           </ECCustomAttributes>"
                "  </ECProperty>"
                "</ECEntityClass>"
                "<ECEntityClass typeName='Goo' modifier='None' >"
                "<BaseClass>Foo</BaseClass>"
                "   <ECProperty propertyName='GooProp' typeName='string' />"
                "</ECEntityClass>"
                "</ECSchema>"),"ConstraintCheckOnProperties.ecdb")) << "NotNull and IsUnique constraints on a subclass are expected to succeed.";

            //IsUnique/IsNull constraints are ignored as the properties are mapped to SharedColumn, so insertion should be successfull
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "INSERT INTO ts.Goo(P1, P2, GooProp) VALUES (2, 12, 'val1')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "INSERT INTO ts.Goo(P1, P2, GooProp) VALUES (2, 13, 'val2')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "INSERT INTO ts.Goo(GooProp) VALUES (11)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            //Explicitly inserting null for P2
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "INSERT INTO ts.Goo(P2, GooProp) VALUES (null, 11)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();
            }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     10/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, DuplicateRelationshipsFlagForSubClassesInLinkTable)
    {
    auto assertECSql = [] (Utf8CP ecsql, ECDbR ecdb, ECSqlStatus sqlStatus = ECSqlStatus::InvalidECSql, DbResult dbResult = DbResult::BE_SQLITE_ERROR)
        {
        ECSqlStatement statement;
        ASSERT_EQ(sqlStatus, statement.Prepare(ecdb, ecsql));
        ASSERT_EQ(dbResult, statement.Step());
        statement.Finalize();
        };

    ASSERT_EQ(SUCCESS, SetupECDb("DuplicateRelationshipsFlagForSubClassesInLinkTable.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='t' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "<ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "<ECEntityClass typeName='A' modifier='Abstract' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "   <ECProperty propertyName='A_Prop' typeName='string' />"
        "</ECEntityClass>"
        "<ECEntityClass typeName='B' modifier='None' >"
        "   <BaseClass>A</BaseClass>"
        "   <ECProperty propertyName='B_Prop' typeName='string' />"
        "</ECEntityClass>"
        "<ECEntityClass typeName='C' modifier='None' >"
        "   <BaseClass>B</BaseClass>"
        "   <ECProperty propertyName='C_Prop' typeName='string' />"
        "</ECEntityClass>"
        "<ECRelationshipClass typeName='AHasA' modifier='Abstract' strength='referencing'>"
        "        <ECCustomAttributes>"
        "            <LinkTableRelationshipMap xmlns='ECDbMap.02.00'>"
        "               <AllowDuplicateRelationships>True</AllowDuplicateRelationships>"
        "            </LinkTableRelationshipMap>"
        "        </ECCustomAttributes>"
        "    <Source multiplicity='(1..*)' polymorphic='true' roleLabel='A'>"
        "      <Class class='A' />"
        "    </Source>"
        "    <Target multiplicity='(1..*)' polymorphic='true' roleLabel='A'>"
        "      <Class class='A' />"
        "    </Target>"
        "</ECRelationshipClass>"
        "<ECRelationshipClass typeName='BHasB' modifier='Sealed' strength='referencing'>"
        "   <BaseClass>AHasA</BaseClass>"
        "    <Source multiplicity='(1..*)' polymorphic='true' roleLabel='B'>"
        "      <Class class='B' />"
        "    </Source>"
        "    <Target multiplicity='(1..*)' polymorphic='true' roleLabel='B'>"
        "      <Class class='B' />"
        "    </Target>"
        "</ECRelationshipClass>"
        "<ECRelationshipClass typeName='CHasC' modifier='Sealed' strength='referencing'>"
        "   <BaseClass>AHasA</BaseClass>"
        "    <Source multiplicity='(1..*)' polymorphic='true' roleLabel='C'>"
        "      <Class class='C' />"
        "    </Source>"
        "    <Target multiplicity='(1..*)' polymorphic='true' roleLabel='C'>"
        "      <Class class='C' />"
        "    </Target>"
        "</ECRelationshipClass>"
        "</ECSchema>")));

    ECSqlStatement stmt;
    assertECSql("INSERT INTO t.B(ECInstanceId, B_Prop) VALUES(1, 'B1')", m_ecdb, ECSqlStatus::Success, DbResult::BE_SQLITE_DONE);
    assertECSql("INSERT INTO t.C(ECInstanceId, C_Prop) VALUES(2, 'C1')", m_ecdb, ECSqlStatus::Success, DbResult::BE_SQLITE_DONE);
    assertECSql("INSERT INTO t.BHasB(ECInstanceId, SourceECInstanceId, TargetECInstanceId) VALUES(1, 1, 2)", m_ecdb, ECSqlStatus::Success, DbResult::BE_SQLITE_DONE);
    assertECSql("INSERT INTO t.BHasB(ECInstanceId, SourceECInstanceId, TargetECInstanceId) VALUES(2, 1, 2)", m_ecdb, ECSqlStatus::Success, DbResult::BE_SQLITE_DONE);
    assertECSql("INSERT INTO t.CHasC(ECInstanceId, SourceECInstanceId, TargetECInstanceId) VALUES(3, 1, 2)", m_ecdb, ECSqlStatus::Success, DbResult::BE_SQLITE_DONE);
    assertECSql("INSERT INTO t.CHasC(ECInstanceId, SourceECInstanceId, TargetECInstanceId) VALUES(4, 1, 2)", m_ecdb, ECSqlStatus::Success, DbResult::BE_SQLITE_DONE);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     1/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, IndexSkippedForIdSpecificationCA)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("IdSpecification.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "<ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.13' alias='ecdbmap' />"
        "<ECEntityClass typeName='ClassWithBusinessKey' modifier='None' >"
        "           <ECCustomAttributes>"
        "            <BusinessKeySpecification xmlns='Bentley_Standard_CustomAttributes.01.13'>"
        "               <PropertyName>Name</PropertyName>"
        "            </BusinessKeySpecification>"
        "           </ECCustomAttributes>"
        "   <ECProperty propertyName='Name' typeName='string' />"
        "</ECEntityClass>"
        "<ECEntityClass typeName='ClassWithSyncId' modifier='None' >"
        "           <ECCustomAttributes>"
        "            <SyncIDSpecification xmlns='Bentley_Standard_CustomAttributes.01.13'>"
        "               <Property>Name</Property>"
        "            </SyncIDSpecification>"
        "           </ECCustomAttributes>"
        "   <ECProperty propertyName='Name' typeName='string' />"
        "</ECEntityClass>"
        "<ECEntityClass typeName='ClassWithGlobalId' modifier='None' >"
        "           <ECCustomAttributes>"
        "            <GlobalIdSpecification xmlns='Bentley_Standard_CustomAttributes.01.13'>"
        "               <PropertyName>Name</PropertyName>"
        "            </GlobalIdSpecification>"
        "           </ECCustomAttributes>"
        "   <ECProperty propertyName='Name' typeName='string' />"
        "</ECEntityClass>"
        "</ECSchema>")));

    ASSERT_EQ(1, m_ecdb.Schemas().GetSchema("TestSchema", true)->GetClassCP("ClassWithBusinessKey")->GetPropertyCount(false));
    ASSERT_EQ(1, m_ecdb.Schemas().GetSchema("TestSchema", true)->GetClassCP("ClassWithGlobalId")->GetPropertyCount(false));
    ASSERT_EQ(1, m_ecdb.Schemas().GetSchema("TestSchema", true)->GetClassCP("ClassWithSyncId")->GetPropertyCount(false));

    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT * FROM sqlite_master WHERE name='ix_test_ClassWithBusinessKey_BusinessKeySpecification_Name' AND type='index'"));
    ASSERT_NE(BE_SQLITE_ROW, stmt.Step()) << "Index for BusinessKeyCA should'nt be created";
    stmt.Finalize();

    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT * FROM sqlite_master WHERE name='ix_test_ClassWithGlobalId_GlobalIdSpecification_Name' AND type='index'"));
    ASSERT_NE(BE_SQLITE_ROW, stmt.Step()) << "Index for GlobalIdCA should'nt be created";
    stmt.Finalize();

    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT * FROM sqlite_master WHERE name='ix_test_ClassWithSyncId_SyncIDSpecification_Name' AND type='index'"));
    ASSERT_NE(BE_SQLITE_ROW, stmt.Step()) << "Index for SyncIdCA should'nt be created";
    }

//---------------------------------------------------------------------------------------
//*Test to verify the CRUD operations for a schema having similar Class and Property name
// @bsimethod                                   Maha Nasir                     08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, ClassAndPropertyWithSameName)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("InstanceCRUD.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Product' modifier='None'>"
        "        <ECProperty propertyName='Product' typeName='string' />"
        "        <ECProperty propertyName='Price' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>")));

    //Inserts Instances.
    ECSqlStatement stmt, s1, s2, s3, s4;
    ASSERT_EQ(ECSqlStatus::Success, s1.Prepare(m_ecdb, "INSERT INTO ts.Product (Product,Price) VALUES('Book',100)"));
    ASSERT_EQ(BE_SQLITE_DONE, s1.Step());
    ASSERT_EQ(ECSqlStatus::Success, s2.Prepare(m_ecdb, "INSERT INTO ts.Product (Product,Price) VALUES('E-Reader',200)"));
    ASSERT_EQ(BE_SQLITE_DONE, s2.Step());
    ASSERT_EQ(ECSqlStatus::Success, s3.Prepare(m_ecdb, "INSERT INTO ts.Product (Product,Price) VALUES('I-Pod',700)"));
    ASSERT_EQ(BE_SQLITE_DONE, s3.Step());
    ASSERT_EQ(ECSqlStatus::Success, s4.Prepare(m_ecdb, "INSERT INTO ts.Product (Product,Price) VALUES('Goggles',500)"));
    ASSERT_EQ(BE_SQLITE_DONE, s4.Step());

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM ts.Product"));
    ASSERT_TRUE(BE_SQLITE_ROW == stmt.Step());
    ASSERT_EQ(4, stmt.GetValueInt(0));
    stmt.Finalize();

    //Deletes the instance
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "DELETE FROM ts.Product WHERE Price=200"));
    ASSERT_TRUE(BE_SQLITE_DONE == stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM ts.Product"));
    ASSERT_TRUE(BE_SQLITE_ROW == stmt.Step());
    ASSERT_EQ(3, stmt.GetValueInt(0));
    stmt.Finalize();

    //Updates the instance
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE ts.Product SET Product='Watch' WHERE Price=500"));
    ASSERT_TRUE(BE_SQLITE_DONE == stmt.Step());
    stmt.Finalize();

    //Select the instance matching the query.
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT Product.Product FROM ts.Product WHERE Price=700"));
    ASSERT_TRUE(BE_SQLITE_ROW == stmt.Step());
    ASSERT_STREQ("I-Pod", stmt.GetValueText(0));
    stmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     11/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, ECSqlForUnmappedClass)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ECSqlForUnmappedClass.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='Test' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "    <ECEntityClass typeName='Product' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>NotMapped</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Name'  typeName='string' />"
        "        <ECProperty propertyName='Price' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "Mapping Strategy NotMapped applied on subclasses is expected to succeed.";

    ECSqlStatement stmt;
    ASSERT_NE(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Product (Name,Price) VALUES('Book',100)"));
    ASSERT_NE(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT FROM ts.Product WHERE Name='Book'"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     11/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, ECClassIdAsVirtualColumn)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ECClassIdAsVirtualColumn.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='Test' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "    <ECEntityClass typeName='Product' modifier='None'>"
        "        <ECProperty propertyName='Name' typeName='string' />"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>")));

    Statement sqlstmt;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, sqlstmt.Prepare(m_ecdb, "INSERT INTO ts_Product(Name,Price) VALUES('Book',100)"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, sqlstmt.Step());
    sqlstmt.Finalize();

    ASSERT_EQ(DbResult::BE_SQLITE_OK, sqlstmt.Prepare(m_ecdb, "SELECT IsVirtual FROM ec_Column WHERE Name='ECClassId' AND TableId = (SELECT Id FROM ec_Table WHERE Name='ts_Product')"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, sqlstmt.Step());
    ASSERT_EQ(1, sqlstmt.GetValueInt(0));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     10/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, NotMappedCAForLinkTable)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("NotMappedCAForLinkTable.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='t' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "<ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "<ECEntityClass typeName='Foo' modifier='None' >"
        "   <ECProperty propertyName='FooProp' typeName='int' />"
        "</ECEntityClass>"
        "<ECEntityClass typeName='Goo' modifier='None' >"
        "   <ECProperty propertyName='GooProp' typeName='int' />"
        "</ECEntityClass>"
        "<ECRelationshipClass typeName='FooHasGoo' modifier='Sealed' strength='referencing'>"
        "        <ECCustomAttributes>"
        "            <LinkTableRelationshipMap xmlns='ECDbMap.02.00'>"
        "               <SourceECInstanceIdColumn>FooId</SourceECInstanceIdColumn>"
        "               <TargetECInstanceIdColumn>GooId</TargetECInstanceIdColumn>"
        "            </LinkTableRelationshipMap>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>NotMapped</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "    <Source multiplicity='(0..1)' polymorphic='true' roleLabel='Foo'>"
        "      <Class class = 'Foo' />"
        "    </Source>"
        "    <Target multiplicity='(0..*)' polymorphic='true' roleLabel='Goo'>"
        "      <Class class = 'Goo' />"
        "    </Target>"
        "</ECRelationshipClass>"
        "</ECSchema>"))) << "Mapping strategy NotMapped can be applied to LinkTable ECRelationship. ";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, MismatchDataTypesInExistingTable)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("DataTypeMismatchInExistingTableTest.ecdb"));

    ASSERT_FALSE(m_ecdb.TableExists("TestTable"));
    m_ecdb.CreateTable("TestTable", "ECInstanceId INTEGER PRIMARY KEY, Name TEXT, Date INTEGER");
    ASSERT_TRUE(m_ecdb.TableExists("TestTable"));
    m_ecdb.SaveChanges();

    ASSERT_EQ(ERROR, ImportSchema(m_ecdb, SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='t' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "<ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "<ECEntityClass typeName='Class' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>ExistingTable</MapStrategy>"
        "                <TableName>TestTable</TableName>"
        "                <ECInstanceIdColumn>ECInstanceId</ECInstanceIdColumn>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "  <ECProperty propertyName='Name' typeName='string'/>"
        "  <ECProperty propertyName='Date' typeName='double'/>"
        "</ECEntityClass>"
        "</ECSchema>")));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, ExistingTableWithOutECInstanceIdColumn)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("InvalidPrimaryKeyInExistingTable.ecdb"));

    ASSERT_FALSE(m_ecdb.TableExists("TestTable"));

    m_ecdb.CreateTable("TestTable", "MyId INTEGER PRIMARY KEY, Name TEXT, Date INTEGER");
    ASSERT_TRUE(m_ecdb.TableExists("TestTable"));

    ASSERT_EQ(ERROR, ImportSchema(m_ecdb, SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='t' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "<ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "<ECEntityClass typeName='TestClass' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>ExistingTable</MapStrategy>"
        "                <TableName>TestTable</TableName>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "  <ECProperty propertyName='Name' typeName='string'/>"
        "  <ECProperty propertyName='Date' typeName='int'/>"
        "</ECEntityClass>"
        "</ECSchema>")));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle   12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, PropertiesWithoutColumnsInExistingTable)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("existingtablemapstrategy.ecdb"));

    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.CreateTable("Foo", "Id INTEGER PRIMARY KEY, P1 TEXT, P2 INTEGER"));
    m_ecdb.SaveChanges();

    ASSERT_EQ(ERROR, ImportSchema(m_ecdb, SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='t' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "<ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "<ECEntityClass typeName='Foo'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>ExistingTable</MapStrategy>"
        "                <TableName>Foo</TableName>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "  <ECProperty propertyName='P1' typeName='string'/>"
        "  <ECProperty propertyName='P2' typeName='int'/>"
        "  <ECProperty propertyName='P3' typeName='int'/>"
        "  <ECProperty propertyName='P4' typeName='string'/>"
        "</ECEntityClass>"
        "</ECSchema>")));
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, IndexGenerationOnClassId)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("IndexGenerationOnClassId.ecdb",SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='ClassA' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P0' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='ClassB' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P0' typeName='string' />"
        "    </ECEntityClass>"
        "</ECSchema>")));

    Statement sqlstmt;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, sqlstmt.Prepare(m_ecdb, "SELECT Name FROM ec_Index WHERE Id=(SELECT IndexId FROM ec_IndexColumn WHERE ColumnId=(SELECT Id FROM ec_Column WHERE Name='ECClassId' AND TableId=(SELECT Id FROM ec_Table WHERE Name='ts_ClassA')))"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, sqlstmt.Step());
    ASSERT_STREQ("ix_ts_ClassA_ecclassid", sqlstmt.GetValueText(0));
    sqlstmt.Finalize();

    ASSERT_EQ(DbResult::BE_SQLITE_OK, sqlstmt.Prepare(m_ecdb, "SELECT Name FROM ec_Index WHERE Id=(SELECT IndexId FROM ec_IndexColumn WHERE ColumnId=(SELECT Id FROM ec_Column WHERE Name='ECClassId' AND TableId=(SELECT Id FROM ec_Table WHERE Name='ts_ClassB')))"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, sqlstmt.Step());
    ASSERT_STREQ("ix_ts_ClassB_ecclassid", sqlstmt.GetValueText(0));
    sqlstmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     07/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, NotMappedWithinClassHierarchy)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("NotMappedWithinClassHierarchy.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Base' modifier='None'>"
        "        <ECProperty propertyName='P0' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub' modifier='None' >"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='SubSub' modifier='Sealed' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>NotMapped</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Sub</BaseClass>"
        "        <ECProperty propertyName='P2' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>")));

    //verify tables
    ASSERT_TRUE(m_ecdb.TableExists("ts_Base"));
    ASSERT_TRUE(m_ecdb.TableExists("ts_Sub"));
    ASSERT_FALSE(m_ecdb.TableExists("ts_SubSub"));

    //verify ECSQL
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "INSERT INTO ts.SubSub (P1, P2) VALUES(1,2)")) << "INSERT not possible against unmapped class";
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT * FROM ts.SubSub")) << "SELECT not possible against unmapped class";
    }

    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     09/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, MultiInheritence_UnsupportedScenarios)
    {
    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='Test1' alias='ts1' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "    <ECEntityClass typeName='Base1' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "          <ClassMap xmlns='ECDbMap.02.00'>"
        "            <MapStrategy>TablePerHierarchy</MapStrategy>"
        "          </ClassMap>"
        "          <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Base2' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "          <ClassMap xmlns='ECDbMap.02.00'>"
        "            <MapStrategy>TablePerHierarchy</MapStrategy>"
        "          </ClassMap>"
        "          <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P2' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' modifier='None' >"
        "        <BaseClass>Base1</BaseClass>"
        "        <ECProperty propertyName='P11' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub2' modifier='None' >"
        "        <BaseClass>Base2</BaseClass>"
        "        <ECProperty propertyName='P21' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='MyClass' modifier='Sealed' >"
        "        <BaseClass>Sub1</BaseClass>"
        "        <BaseClass>Sub2</BaseClass>"
        "        <ECProperty propertyName='MyProp' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "Multi-inheritance with base classes mapped to different shared tables";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='Test2' alias='ts2' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "    <ECEntityClass typeName='Base' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "          <ClassMap xmlns='ECDbMap.02.00'>"
        "            <MapStrategy>TablePerHierarchy</MapStrategy>"
        "          </ClassMap>"
        "          <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P0' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' modifier='Abstract' >"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub2' modifier='Abstract' >"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='P2' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='MyClass' modifier='Sealed' >"
        "        <BaseClass>Sub1</BaseClass>"
        "        <BaseClass>Sub2</BaseClass>"
        "        <ECProperty propertyName='MyProp' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "Multi-inheritance with base classes mapped to different joined tables";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
            "<?xml version='1.0' encoding='utf-8'?>"
            "<ECSchema schemaName='Test3' alias='ts3' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
            "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
            "    <ECEntityClass typeName='Base1' modifier='Abstract'>"
            "        <ECProperty propertyName='P1' typeName='int' />"
            "    </ECEntityClass>"
            "    <ECEntityClass typeName='Base2' modifier='Abstract'>"
            "        <ECProperty propertyName='P2' typeName='int' />"
            "    </ECEntityClass>"
            "    <ECEntityClass typeName='Sub1' modifier='None' >"
            "        <ECCustomAttributes>"
            "          <ClassMap xmlns='ECDbMap.02.00'>"
            "                <MapStrategy>TablePerHierarchy</MapStrategy>"
            "          </ClassMap>"
            "          <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
            "        </ECCustomAttributes>"
            "        <BaseClass>Base1</BaseClass>"
            "        <ECProperty propertyName='P1' typeName='int' />"
            "    </ECEntityClass>"
            "    <ECEntityClass typeName='Sub2' modifier='None' >"
            "        <BaseClass>Base2</BaseClass>"
            "        <ECProperty propertyName='P2' typeName='int' />"
            "    </ECEntityClass>"
            "    <ECEntityClass typeName='MyClass' modifier='Sealed' >"
            "        <BaseClass>Sub1</BaseClass>"
            "        <BaseClass>Sub2</BaseClass>"
            "        <ECProperty propertyName='MyProp' typeName='int' />"
            "    </ECEntityClass>"
            "</ECSchema>"))) << "Multi-inheritance with one TPH base class and one OwnedTable base class";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='Test4' alias='ts4' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "    <ECEntityClass typeName='Base1' modifier='Abstract'>"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Base2' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>NotMapped</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P2' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' modifier='None' >"
        "        <ECCustomAttributes>"
        "          <ClassMap xmlns='ECDbMap.02.00'>"
        "            <MapStrategy>TablePerHierarchy</MapStrategy>"
        "          </ClassMap>"
        "          <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Base1</BaseClass>"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub2' modifier='None' >"
        "        <BaseClass>Base2</BaseClass>"
        "        <ECProperty propertyName='P2' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='MyClass' modifier='Sealed' >"
        "        <BaseClass>Sub1</BaseClass>"
        "        <BaseClass>Sub2</BaseClass>"
        "        <ECProperty propertyName='MyProp' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "Multi-inheritance with one TPH base class and one NotMapped base class";


    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='Test5' alias='ts5' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "    <ECEntityClass typeName='Base1' modifier='Abstract'>"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Base2' modifier='Abstract'>"
        "        <ECProperty propertyName='P2' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' modifier='None' >"
        "        <ECCustomAttributes>"
        "          <ClassMap xmlns='ECDbMap.02.00'>"
        "            <MapStrategy>TablePerHierarchy</MapStrategy>"
        "          </ClassMap>"
        "          <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Base1</BaseClass>"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub2' modifier='None' >"
        "        <ECCustomAttributes>"
        "          <ClassMap xmlns='ECDbMap.02.00'>"
        "            <MapStrategy>TablePerHierarchy</MapStrategy>"
        "          </ClassMap>"
        "          <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Base2</BaseClass>"
        "        <ECProperty propertyName='P2' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='MyClass' modifier='Sealed' >"
        "        <BaseClass>Sub1</BaseClass>"
        "        <BaseClass>Sub2</BaseClass>"
        "        <ECProperty propertyName='MyProp' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "Multi-inheritance with two TPH base classes";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     09/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, BaseClassAndMixins_TablePerHierarchyPlusVirtualTable)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("multiinheritance.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='Test' alias='ts1' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "    <ECSchemaReference name='CoreCustomAttributes' version='01.00' alias='CoreCA' />"
        "    <ECEntityClass typeName='Base1' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "          <ClassMap xmlns='ECDbMap.02.00'>"
        "            <MapStrategy>TablePerHierarchy</MapStrategy>"
        "          </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Base1_Prop1' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Base2' modifier='Abstract'>" // Mapped to virtual table
        "        <ECCustomAttributes>"
        "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
        "            <AppliesToEntityClass>Base1</AppliesToEntityClass>"
        "          </IsMixin>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Base2_Prop1' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' modifier='Abstract' >"
        "        <BaseClass>Base1</BaseClass>"
        "        <ECProperty propertyName='Sub1_Prop1' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='MyClass' modifier='Sealed' >"
        "        <BaseClass>Sub1</BaseClass>"
        "        <BaseClass>Base2</BaseClass>"
        "        <ECProperty propertyName='MyProp' typeName='string' />"
        "    </ECEntityClass>"
        "</ECSchema>")));
    ASSERT_EQ(SUCCESS, Populate(m_ecdb, 3));

    ECClassId myClassId = m_ecdb.Schemas().GetClassId("ts1", "MyClass", SchemaLookupMode::ByAlias);
    ASSERT_TRUE(myClassId.IsValid());

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECClassId, Base2_Prop1 FROM ts1.Base2"));
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        ASSERT_EQ(myClassId.GetValue(), stmt.GetValueId<ECClassId>(0).GetValue()) << stmt.GetECSql();
        ASSERT_FALSE(stmt.IsValueNull(1)) << stmt.GetECSql();
        }
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECClassId, Base2_Prop1 FROM ts1.MyClass"));
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        ASSERT_EQ(myClassId.GetValue(), stmt.GetValueId<ECClassId>(0).GetValue()) << stmt.GetECSql();
        ASSERT_FALSE(stmt.IsValueNull(1)) << stmt.GetECSql();
        }
    }

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan Eberle                     09/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, BaseClassAndMixins_Diamond)
    {
    bvector<SchemaItem> testSchemas;
    testSchemas.push_back(SchemaItem(
                                     "<?xml version = '1.0' encoding = 'utf-8'?>"
                                     "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                     "<ECSchemaReference name='CoreCustomAttributes' version='01.00' alias='CoreCA' />"
                                     "<ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                                     "  <ECEntityClass typeName='Base' modifier='Abstract' >"
                                     "    <ECCustomAttributes>"
                                     "        <ClassMap xmlns='ECDbMap.02.00'>"
                                     "            <MapStrategy>TablePerHierarchy</MapStrategy>"
                                     "        </ClassMap>"
                                     "    </ECCustomAttributes>"
                                     "    <ECProperty propertyName='Base_Prop1' typeName='string' />"
                                     "  </ECEntityClass>"
                                     "  <ECEntityClass typeName='Sub1' modifier='Abstract'>"
                                     "    <BaseClass>Base</BaseClass>"
                                     "    <ECProperty propertyName='Sub1_Prop1' typeName='string' />"
                                     "  </ECEntityClass>"
                                     "  <ECEntityClass typeName='IMixin' modifier='Abstract'>"
                                     "        <ECCustomAttributes>"
                                     "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
                                     "            <AppliesToEntityClass>Sub1</AppliesToEntityClass>"
                                     "          </IsMixin>"
                                     "        </ECCustomAttributes>"
                                     "    <ECProperty propertyName='IMixin_Prop1' typeName='string' />"
                                     "  </ECEntityClass>"
                                     "  <ECEntityClass typeName='Sub12' modifier='Abstract'>"
                                     "    <BaseClass>Sub1</BaseClass>"
                                     "    <ECProperty propertyName='Sub12_Prop1' typeName='string' />"
                                     "  </ECEntityClass>"
                                     "  <ECEntityClass typeName='MyClass' >"
                                     "    <BaseClass>Sub12</BaseClass>"
                                     "    <BaseClass>IMixin</BaseClass>"
                                     "    <ECProperty propertyName='MyClass_Prop1' typeName='string' />"
                                     "  </ECEntityClass>"
                                     "</ECSchema>"));

    testSchemas.push_back(SchemaItem(
                                     "<?xml version = '1.0' encoding = 'utf-8'?>"
                                     "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                     "<ECSchemaReference name='CoreCustomAttributes' version='01.00' alias='CoreCA' />"
                                     "<ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                                     "  <ECEntityClass typeName='Base' modifier='Abstract' >"
                                     "    <ECCustomAttributes>"
                                     "        <ClassMap xmlns='ECDbMap.02.00'>"
                                     "            <MapStrategy>TablePerHierarchy</MapStrategy>"
                                     "        </ClassMap>"
                                     "        <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                                     "    </ECCustomAttributes>"
                                     "    <ECProperty propertyName='Base_Prop1' typeName='string' />"
                                     "  </ECEntityClass>"
                                     "  <ECEntityClass typeName='Sub1' modifier='Abstract'>"
                                     "    <BaseClass>Base</BaseClass>"
                                     "    <ECProperty propertyName='Sub1_Prop1' typeName='string' />"
                                     "  </ECEntityClass>"
                                     "  <ECEntityClass typeName='IMixin' modifier='Abstract'>"
                                     "        <ECCustomAttributes>"
                                     "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
                                     "            <AppliesToEntityClass>Sub1</AppliesToEntityClass>"
                                     "          </IsMixin>"
                                     "        </ECCustomAttributes>"
                                     "    <ECProperty propertyName='IMixin_Prop1' typeName='string' />"
                                     "  </ECEntityClass>"
                                     "  <ECEntityClass typeName='Sub12' modifier='Abstract'>"
                                     "    <BaseClass>Sub1</BaseClass>"
                                     "    <ECProperty propertyName='Sub12_Prop1' typeName='string' />"
                                     "  </ECEntityClass>"
                                     "  <ECEntityClass typeName='MyClass' >"
                                     "    <BaseClass>Sub12</BaseClass>"
                                     "    <BaseClass>IMixin</BaseClass>"
                                     "    <ECProperty propertyName='MyClass_Prop1' typeName='string' />"
                                     "  </ECEntityClass>"
                                     "</ECSchema>"));

    testSchemas.push_back(SchemaItem(
                                     "<?xml version = '1.0' encoding = 'utf-8'?>"
                                     "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                     "<ECSchemaReference name='CoreCustomAttributes' version='01.00' alias='CoreCA' />"
                                     "<ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                                     "  <ECEntityClass typeName='Base' modifier='Abstract' >"
                                     "    <ECCustomAttributes>"
                                     "        <ClassMap xmlns='ECDbMap.02.00'>"
                                     "            <MapStrategy>TablePerHierarchy</MapStrategy>"
                                     "        </ClassMap>"
                                     "        <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                                     "        <ShareColumns xmlns='ECDbMap.02.00'>"
                                     "           <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
                                     "        </ShareColumns>"
                                     "    </ECCustomAttributes>"
                                     "    <ECProperty propertyName='Base_Prop1' typeName='string' />"
                                     "  </ECEntityClass>"
                                     "  <ECEntityClass typeName='Sub1' modifier='Abstract'>"
                                     "    <BaseClass>Base</BaseClass>"
                                     "    <ECProperty propertyName='Sub1_Prop1' typeName='string' />"
                                     "  </ECEntityClass>"
                                     "  <ECEntityClass typeName='IMixin' modifier='Abstract'>"
                                     "        <ECCustomAttributes>"
                                     "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
                                     "            <AppliesToEntityClass>Sub1</AppliesToEntityClass>"
                                     "          </IsMixin>"
                                     "        </ECCustomAttributes>"
                                     "    <ECProperty propertyName='IMixin_Prop1' typeName='string' />"
                                     "  </ECEntityClass>"
                                     "  <ECEntityClass typeName='Sub12' modifier='Abstract'>"
                                     "    <BaseClass>Sub1</BaseClass>"
                                     "    <ECProperty propertyName='Sub12_Prop1' typeName='string' />"
                                     "  </ECEntityClass>"
                                     "  <ECEntityClass typeName='MyClass' >"
                                     "    <BaseClass>Sub12</BaseClass>"
                                     "    <BaseClass>IMixin</BaseClass>"
                                     "    <ECProperty propertyName='MyClass_Prop1' typeName='string' />"
                                     "  </ECEntityClass>"
                                     "</ECSchema>"));


    for (SchemaItem const& testSchema : testSchemas)
        {
        ECDb ecdb;
        ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, testSchema, "multinheritance_diamond.ecdb"));

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.MyClass(Base_Prop1,Sub1_Prop1,IMixin_Prop1,Sub12_Prop1,MyClass_Prop1) "
                                                     "VALUES('base','sub1', 'imixin', 'sub12', 'myclass')"));
        ECInstanceKey key;
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << stmt.GetECSql();

        stmt.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT IMixin_Prop1 FROM ts.IMixin WHERE ECInstanceId=?"));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, key.GetInstanceId())) << stmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
        ASSERT_FALSE(stmt.IsValueNull(0)) << stmt.GetECSql();
        ASSERT_STREQ("imixin", stmt.GetValueText(0)) << stmt.GetECSql();
        }
    }





//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle         10/15
//+---------------+---------------+---------------+---------------+---------------+------
void AssertECInstanceIdAutoGeneration(ECDbCR ecdb, bool expectedToSucceed, Utf8CP fullyQualifiedTestClass, Utf8CP prop, Utf8CP val)
    {
    if (!expectedToSucceed)
        BeTest::SetFailOnAssert(false);

    //different ways to let ECDb auto-generated (if allowed)
    {
    Utf8String ecsql;
    ecsql.Sprintf("INSERT INTO %s (%s) VALUES(%s)", fullyQualifiedTestClass, prop, val);
    ECSqlStatement stmt;

    ECSqlStatus expectedStat = expectedToSucceed ? ECSqlStatus::Success : ECSqlStatus::InvalidECSql;
    ASSERT_EQ(expectedStat, stmt.Prepare(ecdb, ecsql.c_str())) << ecsql.c_str();

    if (expectedToSucceed)
        {
        ECInstanceKey newKey;
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey));
        ASSERT_TRUE(newKey.IsValid());
        }
    }

    {
    Utf8String ecsql;
    ecsql.Sprintf("INSERT INTO %s (ECInstanceId, %s) VALUES(NULL, %s)", fullyQualifiedTestClass, prop, val);
    ECSqlStatement stmt;

    //only fails at step time
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str())) << ecsql.c_str();

    if (expectedToSucceed)
        {
        ECInstanceKey newKey;
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey));
        ASSERT_TRUE(newKey.IsValid());
        }
    }

    ECInstanceId id;
    {
    Utf8String ecsql;
    ecsql.Sprintf("INSERT INTO %s (ECInstanceId, %s) VALUES(?, %s)", fullyQualifiedTestClass, prop, val);
    ECSqlStatement stmt;

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str())) << "Prepare should always succeed if ECInstanceId is bound via parameters: " << ecsql.c_str();

    DbResult expectedStat = expectedToSucceed ? BE_SQLITE_DONE : BE_SQLITE_ERROR;

    ECInstanceKey newKey;
    ASSERT_EQ(expectedStat, stmt.Step(newKey));
    ASSERT_EQ(expectedToSucceed, newKey.IsValid());

    id = newKey.GetInstanceId();
    }

    //now test when ECInstanceId is specified
    {
    id = ECInstanceId(id.GetValue() + 1);
    Utf8String ecsql;
    ecsql.Sprintf("INSERT INTO %s (ECInstanceId, %s) VALUES(%llu, %s)", fullyQualifiedTestClass, prop, id.GetValue(), val);
    ECSqlStatement stmt;

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str())) << ecsql.c_str();
    ECInstanceKey newKey;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey)) << "Disable flag does not affect case when ECInstanceId is specified";
    ASSERT_EQ(id.GetValue(), newKey.GetInstanceId().GetValue());
    }

    {
    Utf8String ecsql;
    ecsql.Sprintf("INSERT INTO %s (ECInstanceId, %s) VALUES(?, %s)", fullyQualifiedTestClass, prop, val);
    ECSqlStatement stmt;

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str())) << ecsql.c_str();

    id = ECInstanceId(id.GetValue() + 1);
    stmt.BindId(1, id);
    ECInstanceKey newKey;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey)) << "Disable flag does not affect case when ECInstanceId is specified";
    ASSERT_EQ(id.GetValue(), newKey.GetInstanceId().GetValue());
    }

    BeTest::SetFailOnAssert(true);
    }



//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, UserDefinedIndexTest)
    {
    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "    <ECEntityClass typeName='Element' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "DbIndexList CA with empty Indexes property is not supported";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "    <ECEntityClass typeName='Element' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "Empty DbIndexList CA is not supported";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "    <ECEntityClass typeName='Element' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                   </DbIndex>"
        "                  </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "DbIndexList CA with empty DbIndex is not supported";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "    <ECEntityClass typeName='Element' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                      <Properties>"
        "                      </Properties>"
        "                   </DbIndex>"
        "                  </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "DbIndexList CA with DbIndex with empty Properties is not supported";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Element' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>True</IsUnique>"
        "                       <Name>uix_element_code</Name>"
        "                       <Properties>"
        "                          <string>Bla</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "Property in index does not exist";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECStructClass typeName='ElementCode' modifier='None'>"
        "        <ECProperty propertyName='AuthorityId' typeName='long' />"
        "        <ECProperty propertyName='Namespace' typeName='string' />"
        "        <ECProperty propertyName='Val' typeName='string' />"
        "    </ECStructClass>"
        "    <ECEntityClass typeName='Element' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>True</IsUnique>"
        "                       <Name>uix_element_code</Name>"
        "                       <Properties>"
        "                          <string>Code</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECStructProperty propertyName='Code' typeName='ElementCode' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "Cannot define index on struct prop";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECStructClass typeName='ElementCode' modifier='None'>"
        "        <ECProperty propertyName='AuthorityId' typeName='long' />"
        "        <ECProperty propertyName='Namespace' typeName='string' />"
        "        <ECProperty propertyName='Val' typeName='string' />"
        "    </ECStructClass>"
        "    <ECEntityClass typeName='Element' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>True</IsUnique>"
        "                       <Name>uix_element_code</Name>"
        "                       <Properties>"
        "                          <string>Codes</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECStructArrayProperty propertyName='Codes' typeName='ElementCode' minOccurs='0' maxOccurs='unbounded' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "Cannot define index on struct array prop";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "    <ECEntityClass typeName='DgnModel' modifier='None' >"
        "        <ECProperty propertyName='Name' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='DgnElement' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>False</IsUnique>"
        "                       <Name>ix_model</Name>"
        "                       <Properties>"
        "                          <string>Model</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECNavigationProperty propertyName='Model' relationshipName='ModelHasElements' direction='Backward' />"
        "    </ECEntityClass>"
        "  <ECRelationshipClass typeName='ModelHasElements' strength='embedding'>"
        "    <Source multiplicity='(1..1)' polymorphic='true' roleLabel='Model'>"
        "      <Class class='DgnModel' />"
        "    </Source>"
        "    <Target multiplicity='(0..*)' polymorphic = 'true' roleLabel='Element'>"
        "      <Class class='DgnElement' />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>"))) << "Cannot define index on navigation prop";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Element' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>True</IsUnique>"
        "                       <Name>uix_element_code</Name>"
        "                       <Properties>"
        "                          <string>Codes</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </ClassMap>"
        "            </ShareColumns>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>True</IsUnique>"
        "                       <Name>uix_element_code</Name>"
        "                       <Properties>"
        "                          <string>Codes</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECArrayProperty propertyName='Codes' typeName='string' minOccurs='0' maxOccurs='unbounded' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "Cannot define index on primitive array prop";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='A' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>True</IsUnique>"
        "                       <Name>mypoorlynamedindex</Name>"
        "                       <Properties>"
        "                          <string>Code</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='string'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='B' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>False</IsUnique>"
        "                       <Name>mypoorlynamedindex</Name>"
        "                       <Properties>"
        "                          <string>BB</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='BB' typeName='string'/>"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "Duplicate indexes";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='A' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='AProp' typeName='string'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='B' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>False</IsUnique>"
        "                       <Name>MyIndex</Name>"
        "                       <Properties>"
        "                          <string>AProp</string>"
        "                          <string>BProp</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <BaseClass>A</BaseClass>"
        "        <ECProperty propertyName='BProp' typeName='string'/>"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "Index with properties that map to different tables is not supported";

    {
    ECDb ecdb;
    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts1' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Base' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <Name>ix_base_code</Name>"
        "                       <Properties>"
        "                          <string>Code</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "             </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' modifier='None'>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='Sub1_Prop' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub2' modifier='None'>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='Sub2_Prop' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>"), "userdefinedindextest1.ecdb"));


    AssertIndex(ecdb, "ix_base_code", false, "ts1_Base", {"Code"});
    }

    {
    ECDb ecdb;
    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts2' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Base' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <Name>ix_base_code</Name>"
        "                       <Properties>"
        "                          <string>Code</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "             </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <Name>ix_sub1_prop</Name>"
        "                       <Properties>"
        "                          <string>Sub1_Prop</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='Sub1_Prop' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub2' modifier='None'>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='Sub2_Prop' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>"), "userdefinedindextest2.ecdb"));

    AssertIndex(ecdb, "ix_sub1_prop", false, "ts2_Base", {"Sub1_Prop"});
    }

    {
    ECDb ecdb;
    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts3' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Base' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <Name>uix_sub1_code</Name>"
        "                       <IsUnique>true</IsUnique>"
        "                       <Properties>"
        "                          <string>Code</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='Sub1_Prop' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub2' modifier='None'>"
        "        <BaseClass>Sub1</BaseClass>"
        "        <ECProperty propertyName='Sub2_Prop' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>"), "userdefinedindextest3.ecdb"));

    ECClassId baseClassId = ecdb.Schemas().GetClassId("TestSchema", "Base");
    Utf8String indexWhereClause;
    indexWhereClause.Sprintf("ECClassId<>%s", baseClassId.ToString().c_str());
    AssertIndex(ecdb, "uix_sub1_code", true, "ts3_Base", {"Code"}, indexWhereClause.c_str());
    }

    {
    ECDb ecdb;
    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts4' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Base' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <Name>uix_base_code</Name>"
        "                       <IsUnique>True</IsUnique>"
        "                       <Properties>"
        "                          <string>Code</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' modifier='None'>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='Sub1_Prop' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub2' modifier='None'>"
        "        <BaseClass>Sub1</BaseClass>"
        "        <ECProperty propertyName='Sub2_Prop' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub3' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <Name>uix_sub3_prop</Name>"
        "                       <IsUnique>True</IsUnique>"
        "                       <Properties>"
        "                          <string>Sub3_Prop</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='Sub3_Prop' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>"), "userdefinedindextest4.ecdb"));


    ECClassId sub3ClassId = ecdb.Schemas().GetClassId("TestSchema", "Sub3");


    AssertIndex(ecdb, "uix_base_code", true, "ts4_Base", {"Code"});

    Utf8String indexWhereClause = "ECClassId=" + sub3ClassId.ToString();
    AssertIndex(ecdb, "uix_sub3_prop", true, "ts4_Base", {"Sub3_Prop"}, indexWhereClause.c_str());

    Utf8String ecdbFilePath(ecdb.GetDbFileName());
    ecdb.CloseDb();
    //after second import new subclass in hierarchy must be reflected by indices
    ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(ecdbFilePath.c_str(), ECDb::OpenParams(Db::OpenMode::ReadWrite)));

    ASSERT_EQ(SUCCESS, ImportSchema(ecdb, SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema2' nameSpacePrefix='ts42' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='TestSchema' version='01.00' prefix='ts4' />"
        "    <ECEntityClass typeName='Sub4' modifier='None'>"
        "        <BaseClass>ts4:Sub3</BaseClass>"
        "        <ECProperty propertyName='Sub4_Prop' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>")));

    //This index is not affected as index is still applying to entire hierarchy
    AssertIndex(ecdb, "uix_base_code", true, "ts4_Base", {"Code"});

    //This index must include the new subclass Sub4
    ECClassId sub4ClassId = ecdb.Schemas().GetClassId("TestSchema2", "Sub4");
    indexWhereClause = "ECClassId=" + sub3ClassId.ToString() + " OR ECClassId=" + sub4ClassId.ToString();
    AssertIndex(ecdb, "uix_sub3_prop", true, "ts4_Base", {"Sub3_Prop"}, indexWhereClause.c_str());
    }

    {
    ECDb ecdb;
    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts5' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Base' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <Name>ix_sub1_aid</Name>"
        "                       <Properties>"
        "                          <string>AId</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='AId' typeName='long' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub2' modifier='None'>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='Name' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1_1' modifier='None'>"
        "        <BaseClass>Sub1</BaseClass>"
        "        <ECProperty propertyName='Cost' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>"), "userdefinedindextest5.ecdb"));

    AssertIndex(ecdb, "ix_sub1_aid", false, "ts5_Base", {"ps1"});
    }

    {
    ECDb ecdb;
    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts6' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Base' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>True</IsUnique>"
        "                       <Name>uix_sub1_aid</Name>"
        "                       <Properties>"
        "                          <string>AId</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='AId' typeName='long' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub2' modifier='None'>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='Name' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub3' modifier='None'>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='Name2' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub11' modifier='None'>"
        "        <BaseClass>Sub1</BaseClass>"
        "        <ECProperty propertyName='Cost' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>"), "userdefinedindextest6.ecdb")) << "Unique indices on shared columns are supported.";

    ECClassId sub1ClassId = ecdb.Schemas().GetClassId("TestSchema", "Sub1");
    ECClassId sub11ClassId = ecdb.Schemas().GetClassId("TestSchema", "Sub11");
    Utf8String indexWhereClause = "ECClassId=" + sub1ClassId.ToString() + " OR ECClassId=" + sub11ClassId.ToString();
    AssertIndex(ecdb, "uix_sub1_aid", true, "ts6_Base", {"ps1"}, indexWhereClause.c_str());
    }

    {
    ECDb ecdb;
    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts7' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECStructClass typeName='ElementCode' modifier='None'>"
        "        <ECProperty propertyName='AuthorityId' typeName='long' />"
        "        <ECProperty propertyName='Namespace' typeName='string' />"
        "        <ECProperty propertyName='Val' typeName='string' />"
        "    </ECStructClass>"
        "    <ECEntityClass typeName='Element' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>True</IsUnique>"
        "                       <Name>uix_element_code</Name>"
        "                       <Properties>"
        "                          <string>Code.AuthorityId</string>"
        "                          <string>Code.Namespace</string>"
        "                          <string>Code.Val</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECStructProperty propertyName='Code' typeName='ElementCode' />"
        "    </ECEntityClass>"
        "</ECSchema>"), "userdefinedindextest7.ecdb"));

    AssertIndex(ecdb, "uix_element_code", true, "ts7_Element", {"Code_AuthorityId", "Code_Namespace", "Code_Val"});
    }

    {
    ECDb ecdb;
    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts8' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='CoreCustomAttributes' version='01.00' prefix='CoreCA' />"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Root' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>True</IsUnique>"
        "                       <Name>uix_root</Name>"
        "                       <Properties>"
        "                          <string>RootProp</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='RootProp' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Interface' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <IsMixin xmlns='CoreCustomAttributes.01.00'>"
        "               <AppliesToEntityClass>Root</AppliesToEntityClass>"
        "            </IsMixin>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>True</IsUnique>"
        "                       <Name>uix_interface</Name>"
        "                       <Properties>"
        "                          <string>InterfaceProp</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='InterfaceProp' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub'>"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>True</IsUnique>"
        "                       <Name>uix_sub</Name>"
        "                       <Properties>"
        "                          <string>SubProp</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "       <BaseClass>Root</BaseClass>"
        "       <BaseClass>Interface</BaseClass>"
        "        <ECProperty propertyName='SubProp' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='SubSub'>"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>True</IsUnique>"
        "                       <Name>uix_subsub</Name>"
        "                       <Properties>"
        "                          <string>SubSubProp</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "       <BaseClass>Sub</BaseClass>"
        "        <ECProperty propertyName='SubSubProp' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub2'>"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>True</IsUnique>"
        "                       <Name>uix_sub2</Name>"
        "                       <Properties>"
        "                          <string>Sub2Prop</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "       <BaseClass>Root</BaseClass>"
        "       <BaseClass>Interface</BaseClass>"
        "        <ECProperty propertyName='Sub2Prop' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='RootUnshared' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>True</IsUnique>"
        "                       <Name>uix_rootunshared</Name>"
        "                       <Properties>"
        "                          <string>RootUnsharedProp</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='RootUnsharedProp' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='SubUnshared'>"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>True</IsUnique>"
        "                       <Name>uix_subunshared</Name>"
        "                       <Properties>"
        "                          <string>SubUnsharedProp</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "       <BaseClass>RootUnshared</BaseClass>"
        "        <ECProperty propertyName='SubUnsharedProp' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>"), "userdefinedindextest8.ecdb")) << "Index on abstract classes - Schema 1";

    ASSERT_EQ(SUCCESS, ImportSchema(ecdb, SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema2' nameSpacePrefix='ts82' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECSchemaReference name='TestSchema' version='01.00' prefix='ts8' />"
        "    <ECEntityClass typeName='Sub3'>"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>True</IsUnique>"
        "                       <Name>uix_sub3</Name>"
        "                       <Properties>"
        "                          <string>Sub3Prop</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "       <BaseClass>ts8:Root</BaseClass>"
        "       <BaseClass>ts8:Interface</BaseClass>"
        "        <ECProperty propertyName='Sub3Prop' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub2Unshared'>"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>True</IsUnique>"
        "                       <Name>uix_sub2unshared</Name>"
        "                       <Properties>"
        "                          <string>Sub2UnsharedProp</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "       <BaseClass>ts8:RootUnshared</BaseClass>"
        "        <ECProperty propertyName='Sub2UnsharedProp' typeName='int' />"
        "    </ECEntityClass>"
        " </ECSchema>"))) << "Index on abstract classes - Schema 2";

    //class hierarchy with shared table
    AssertIndex(ecdb, "uix_root", true, "ts8_Root", {"RootProp"});

    //index from Interface class is applied to Sub and Sub2 which are stored in joined tables
    AssertIndex(ecdb, "uix_interface_ts8_Sub", true, "ts8_Sub", {"InterfaceProp"});
    AssertIndex(ecdb, "uix_interface_ts8_Sub2", true, "ts8_Sub2", {"InterfaceProp"});
    AssertIndex(ecdb, "uix_interface_ts82_Sub3", true, "ts82_Sub3", {"InterfaceProp"});
    AssertIndex(ecdb, "uix_sub", true, "ts8_Sub", {"SubProp"});
    AssertIndex(ecdb, "uix_sub2", true, "ts8_Sub2", {"Sub2Prop"});
    AssertIndex(ecdb, "uix_sub3", true, "ts82_Sub3", {"Sub3Prop"});

    ECClassCP subSubClass = ecdb.Schemas().GetClass("TestSchema", "SubSub");
    ASSERT_TRUE(subSubClass != nullptr);
    Utf8String indexWhere = "ECClassId=" + subSubClass->GetId().ToString();

    AssertIndex(ecdb, "uix_subsub", true, "ts8_Sub", {"SubSubProp"}, indexWhere.c_str());

    //class hierarchy without shared table
    AssertIndex(ecdb, "uix_rootunshared_ts8_SubUnshared", true, "ts8_SubUnshared", {"RootUnsharedProp"});
    AssertIndex(ecdb, "uix_rootunshared_ts82_Sub2Unshared", true, "ts82_Sub2Unshared", {"RootUnsharedProp"});
    AssertIndex(ecdb, "uix_subunshared", true, "ts8_SubUnshared", {"SubUnsharedProp"});
    AssertIndex(ecdb, "uix_sub2unshared", true, "ts82_Sub2Unshared", {"Sub2UnsharedProp"});
    }

        {
        ECDb ecdb;
        ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, SchemaItem(
            "<?xml version='1.0' encoding='utf-8'?>"
            "<ECSchema schemaName='TestSchema' alias='ts9' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
            "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
            "    <ECEntityClass typeName='DgnModel' modifier='None' >"
            "        <ECProperty propertyName='Name' typeName='string' />"
            "    </ECEntityClass>"
            "    <ECEntityClass typeName='DgnElement' modifier='None' >"
            "        <ECCustomAttributes>"
            "            <ClassMap xmlns='ECDbMap.02.00'>"
            "                <MapStrategy>TablePerHierarchy</MapStrategy>"
            "            </ClassMap>"
            "            <DbIndexList xmlns='ECDbMap.02.00'>"
            "                 <Indexes>"
            "                   <DbIndex>"
            "                       <IsUnique>False</IsUnique>"
            "                       <Name>ix_dgnelement_model_id_code</Name>"
            "                       <Properties>"
            "                          <string>Model.Id</string>"
            "                          <string>Code</string>"
            "                       </Properties>"
            "                   </DbIndex>"
            "                 </Indexes>"
            "            </DbIndexList>"
            "        </ECCustomAttributes>"
            "        <ECNavigationProperty propertyName='Model' relationshipName='ModelHasElements' direction='Backward' />"
            "        <ECProperty propertyName='Code' typeName='int' />"
            "    </ECEntityClass>"
            "  <ECRelationshipClass typeName='ModelHasElements' strength='embedding' modifier='Sealed'>"
            "    <Source multiplicity='(1..1)' polymorphic='true' roleLabel='Model'>"
            "      <Class class='DgnModel' />"
            "    </Source>"
            "    <Target multiplicity='(0..*)' polymorphic = 'true' roleLabel='Element'>"
            "      <Class class='DgnElement' />"
            "    </Target>"
            "  </ECRelationshipClass>"
            "</ECSchema>"), "userdefinedindextest9.ecdb"));

        AssertIndex(ecdb, "ix_dgnelement_model_id_code", false, "ts9_DgnElement", {"ModelId","Code"});
        }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                  02/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, UserDefinedIndexOnMixin)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("userdefinedindexonmixin.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='CoreCustomAttributes' version='01.00' prefix='CoreCA' />"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Root' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='RootProp' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Interface' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <IsMixin xmlns='CoreCustomAttributes.01.00'>"
        "               <AppliesToEntityClass>Root</AppliesToEntityClass>"
        "            </IsMixin>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "                 <Indexes>"
        "                   <DbIndex>"
        "                       <IsUnique>True</IsUnique>"
        "                       <Name>uix_interface</Name>"
        "                       <Properties>"
        "                          <string>InterfaceProp</string>"
        "                       </Properties>"
        "                   </DbIndex>"
        "                 </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='InterfaceProp' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub'>"
        "       <BaseClass>Root</BaseClass>"
        "       <BaseClass>Interface</BaseClass>"
        "        <ECProperty propertyName='SubProp' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='SubSub'>"
        "       <BaseClass>Sub</BaseClass>"
        "        <ECProperty propertyName='SubSubProp' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>"))) << "Index on mixin";

    AssertIndex(m_ecdb, "uix_interface_ts_Sub", true, "ts_Sub", {"InterfaceProp"});
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Majd.Uddin                         03/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DbMappingTestFixture, PartialIndex)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecdbmapindextest.ecdb", SchemaItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
            "   <ECSchemaReference name='ECDbMap' version='02.00' prefix ='ecdbmap' />"
            "   <ECEntityClass typeName = 'IndexClass' >"
            "       <ECCustomAttributes>"
            "       <DbIndexList xmlns='ECDbMap.02.00'>"
            "           <Indexes>"
            "               <DbIndex>"
            "                   <Name>IDX_Partial</Name>"
            "                   <IsUnique>False</IsUnique>"
            "                   <Properties>"
            "                       <string>PropertyPartialIndex</string>"
            "                   </Properties>"
            "                   <Where>IndexedColumnsAreNotNull</Where>"
            "               </DbIndex>"
            "               <DbIndex>"
            "                   <Name>IDX_Full</Name>"
            "                   <IsUnique>False</IsUnique>"
            "                   <Properties>"
            "                       <string>PropertyFullIndex</string>"
            "                   </Properties>"
            "               </DbIndex>"
            "               <DbIndex>"
            "                   <Name>IDX_PartialMissing</Name>"
            "                   <IsUnique>False</IsUnique>"
            "                   <Properties>"
            "                       <string>PropertyPartialIndex</string>"
            "                   </Properties>"
            "                   <Where></Where>"
            "               </DbIndex>"
            "           </Indexes>"
            "       </DbIndexList>"
            "   </ECCustomAttributes>"
            "   <ECProperty propertyName ='PropertyFullIndex' typeName = 'string' />"
            "   <ECProperty propertyName ='PropertyPartialIndex' typeName = 'string' />"
            "   </ECEntityClass>"
            "</ECSchema>")));
        //Verify that one Partial index was created
        BeSQLite::Statement stmt;
        ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT * FROM SQLITE_MASTER WHERE type='index' AND tbl_name='ts_IndexClass' AND name=?"));
        ASSERT_EQ(BE_SQLITE_OK, stmt.BindText(1, "IDX_Partial", Statement::MakeCopy::No));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        Utf8String sqlCmd = stmt.GetValueText(4);
        ASSERT_FALSE(sqlCmd.find("WHERE") == std::string::npos) << "IDX_Partial is a partial index and will have WHERE clause";
        //Verify that other index is not Partial as Where was not specified
        ASSERT_EQ(BE_SQLITE_OK, stmt.Reset());
        ASSERT_EQ(BE_SQLITE_OK, stmt.BindText(1, "IDX_Full", Statement::MakeCopy::No));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        sqlCmd = stmt.GetValueText(4);
        ASSERT_TRUE(sqlCmd.find("WHERE") == std::string::npos);

        //Verify that index with empty Where clause is treated as not-partial index
        ASSERT_EQ(BE_SQLITE_OK, stmt.Reset());
        ASSERT_EQ(BE_SQLITE_OK, stmt.BindText(1, "IDX_PartialMissing", Statement::MakeCopy::No));
        //IDX_PartialMissing will be skipped as it has empty WHERE clause
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

        stmt.Finalize();
        }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Majd.Uddin                         03/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DbMappingTestFixture, UniqueIndex)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecdbmapindextest.ecdb", SchemaItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
                                                                "   <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                                                "<ECEntityClass typeName='IndexClass2' >"
                                                                "   <ECCustomAttributes>"
                                                                "       <DbIndexList xmlns='ECDbMap.02.00'>"
                                                                "           <Indexes>"
                                                                "               <DbIndex>"
                                                                "                   <Name>IDX_Unique</Name>"
                                                                "                   <IsUnique>True</IsUnique>"
                                                                "                   <Properties>"
                                                                "                       <string>Property2</string>"
                                                                "                   </Properties>"
                                                                "               </DbIndex>"
                                                                "               <DbIndex>"
                                                                "                   <Name>IDX_NotUnique</Name>"
                                                                "                   <IsUnique>False</IsUnique>"
                                                                "                   <Properties>"
                                                                "                       <string>Property2</string>"
                                                                "                   </Properties>"
                                                                "               </DbIndex>"
                                                                "           </Indexes>"
                                                                "       </DbIndexList>"
                                                                "   </ECCustomAttributes>"
                                                                "   <ECProperty propertyName='Property1' typeName='string' />"
                                                                "   <ECProperty propertyName='Property2' typeName='string' />"
                                                                "</ECEntityClass>"
                                                                "</ECSchema>")));

    //Verify that one Unique index was created
    BeSQLite::Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT * FROM SQLITE_MASTER WHERE type='index' AND tbl_name='ts_IndexClass2' AND name=?"));
    ASSERT_EQ(BE_SQLITE_OK, stmt.BindText(1, "IDX_Unique", Statement::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    Utf8String sqlCmd = stmt.GetValueText(4);
    ASSERT_FALSE(sqlCmd.find("UNIQUE") == std::string::npos) << "IDX_Unique will have UNIQUE clause";

    //Verify that other indexes are not Unique
    ASSERT_EQ(BE_SQLITE_OK, stmt.Reset());
    ASSERT_EQ(BE_SQLITE_OK, stmt.BindText(1, "IDX_NotUnique", Statement::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    sqlCmd = stmt.GetValueText(4);
    ASSERT_TRUE(sqlCmd.find("UNIQUE") == std::string::npos);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Majd.Uddin                         03/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DbMappingTestFixture, IndexErrors)
    {
    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
                                     "   <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                     "<ECEntityClass typeName='IndexClass3' >"
                                     "   <ECCustomAttributes>"
                                     "       <DbIndexList xmlns='ECDbMap.02.00'>"
                                     "           <Indexes>"
                                     "               <DbIndex>"
                                     "                   <Name>IDX_NoProperty</Name>"
                                     "                   <IsUnique>False</IsUnique>"
                                     "                   <Properties>"
                                     "                       <string>Property1</string>"
                                     "                   </Properties>"
                                     "               </DbIndex>"
                                     "           </Indexes>"
                                     "       </DbIndexList>"
                                     "   </ECCustomAttributes>"
                                     "   <ECProperty propertyName='PropertyString' typeName='string' />"
                                     "   <ECProperty propertyName='PropertyInt' typeName='int' />"
                                     "   <ECProperty propertyName='PropertyDouble' typeName='double' />"
                                     "</ECEntityClass>"
                                     "</ECSchema>")));

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
        "   <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "<ECEntityClass typeName='IndexClass3' >"
        "   <ECCustomAttributes>"
        "       <DbIndexList xmlns='ECDbMap.02.00'>"
        "           <Indexes>"
        "               <DbIndex>"
        "                   <Name>IDX_WrongProperty</Name>"
        "                   <IsUnique>False</IsUnique>"
        "                   <Properties>"
        "                       <string>Property1</string>"
        "                   </Properties>"
        "               </DbIndex>"
        "           </Indexes>"
        "       </DbIndexList>"
        "   </ECCustomAttributes>"
        "   <ECProperty propertyName ='PropertyString' typeName = 'string' />"
        "   <ECProperty propertyName ='PropertyInt' typeName = 'int' />"
        "   <ECProperty propertyName ='PropertyDouble' typeName = 'double' />"
        "</ECEntityClass>"
        "</ECSchema>")));

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
        "   <ECSchemaReference name='ECDbMap' version='02.00' prefix ='ecdbmap' />"
        "<ECEntityClass typeName = 'IndexClass3' >"
        "   <ECCustomAttributes>"
        "       <DbIndexList xmlns='ECDbMap.02.00'>"
        "           <Indexes>"
        "               <DbIndex>"
        "                   <Name>IDX_WrongPropertyArray</Name>"
        "                   <IsUnique>False</IsUnique>"
        "                   <Properties>"
        "                       <int>PropertyInt</int>"
        "                       <string>Property1</string>"
        "                   </Properties>"
        "               </DbIndex>"
        "           </Indexes>"
        "       </DbIndexList>"
        "   </ECCustomAttributes>"
        "   <ECProperty propertyName ='PropertyString' typeName = 'string' />"
        "   <ECProperty propertyName ='PropertyInt' typeName = 'int' />"
        "   <ECProperty propertyName ='PropertyDouble' typeName = 'double' />"
        "</ECEntityClass>"
        "</ECSchema>")));

    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(SchemaItem(
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
        "   <ECSchemaReference name='ECDbMap' version='02.00' prefix ='ecdbmap' />"
        "<ECEntityClass typeName = 'IndexClass3' >"
        "   <ECCustomAttributes>"
        "       <DbIndexList xmlns='ECDbMap.02.00'>"
        "           <Indexes>"
        "               <DbIndex>"
        "                   <Name>SELECT</Name>"
        "                   <IsUnique>False</IsUnique>"
        "                   <Properties>"
        "                       <string>PropertyInt</string>"
        "                   </Properties>"
        "               </DbIndex>"
        "           </Indexes>"
        "       </DbIndexList>"
        "   </ECCustomAttributes>"
        "   <ECProperty propertyName ='PropertyString' typeName = 'string' />"
        "   <ECProperty propertyName ='PropertyInt' typeName = 'int' />"
        "   <ECProperty propertyName ='PropertyDouble' typeName = 'double' />"
        "</ECEntityClass>"
        "</ECSchema>")));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     12/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, PropertyMapCAOnNavigationProperty)
    {
    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts1' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='A' modifier='None'>"
        "        <ECProperty propertyName='MyId' typeName='long' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='B' modifier='None'>"
        "        <ECNavigationProperty propertyName='MyA' relationshipName='Rel' direction='Backward'>"
        "           <ECCustomAttributes>"
        "            <PropertyMap xmlns='ECDbMap.02.00'>"
        "            </PropertyMap>"
        "           </ECCustomAttributes>"
        "        </ECNavigationProperty>"
        "        <ECProperty propertyName='Name' typeName='string' />"
        "    </ECEntityClass>"
        "  <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='embedding'>"
        "    <Source cardinality='(0,1)' polymorphic='True'>"
        "      <Class class='A'/>"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class='B' />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>"))) << "PropertyMap CA not allowed on navigation property";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts2' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "    <ECEntityClass typeName='A' modifier='None'>"
        "        <ECProperty propertyName='MyId' typeName='long' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='B' modifier='None'>"
        "        <ECNavigationProperty propertyName='MyA' relationshipName='Rel' direction='Backward'>"
        "           <ECCustomAttributes>"
        "            <PropertyMap xmlns='ECDbMap.02.00'>"
        "               <IsNullable>false</IsNullable>"
        "            </PropertyMap>"
        "           </ECCustomAttributes>"
        "        </ECNavigationProperty>"
        "        <ECProperty propertyName='Name' typeName='string' />"
        "    </ECEntityClass>"
        "  <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='embedding'>"
        "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='A'>"
        "      <Class class='A'/>"
        "    </Source>"
        "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='B'>"
        "      <Class class='B' />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>"))) << "PropertyMap CA not allowed on navigation property";;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     12/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, PropertyMapCAColumnNameCollation)
    {
    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                                        <ECSchema schemaName="TestSchema" alias="ts0" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                                            <ECEntityClass typeName="Foo" modifier="None">
                                                <ECProperty propertyName="MyProp" typeName="long">
                                                   <ECCustomAttributes>
                                                    <PropertyMap xmlns="ECDbMap.02.00">
                                                       <ColumnName>c_prop</ColumnName>
                                                    </PropertyMap>
                                                   </ECCustomAttributes>
                                                </ECProperty>
                                            </ECEntityClass>
                                        </ECSchema>)xml"))) << "ColumnName without MapStrategy Existing Table";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                        "<ECSchema schemaName='TestSchema' alias='ts0' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                                        "    <ECEntityClass typeName='Base' modifier='None'>"
                                        "           <ECCustomAttributes>"
                                        "            <ClassMap xmlns='ECDbMap.02.00'>"
                                        "               <MapStrategy>TablePerHierarchy</MapStrategy>"
                                        "            </ClassMap>"
                                        "            <ShareColumns xmlns='ECDbMap.02.00'>"
                                        "                  <MaxSharedColumnsBeforeOverflow>2</MaxSharedColumnsBeforeOverflow>"
                                        "                  <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>"
                                        "             </ShareColumns>"
                                        "           </ECCustomAttributes>"
                                        "        <ECProperty propertyName='P_Base' typeName='long'>"
                                        "           <ECCustomAttributes>"
                                        "            <PropertyMap xmlns='ECDbMap.02.00'>"
                                        "               <ColumnName>c_base</ColumnName>"
                                        "            </PropertyMap>"
                                        "           </ECCustomAttributes>"
                                        "        </ECProperty>"
                                        "    </ECEntityClass>"
                                        "</ECSchema>"))) << "ColumnName without MapStrategy Existing Table and on shared column";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                        "<ECSchema schemaName='TestSchema' alias='ts2' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                                        "    <ECEntityClass typeName='Base' modifier='None'>"
                                        "           <ECCustomAttributes>"
                                        "            <ClassMap xmlns='ECDbMap.02.00'>"
                                        "               <MapStrategy>TablePerHierarchy</MapStrategy>"
                                        "            </ClassMap>"
                                        "            <ShareColumns xmlns='ECDbMap.02.00'>"
                                        "                  <MaxSharedColumnsBeforeOverflow>2</MaxSharedColumnsBeforeOverflow>"
                                        "                  <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>"
                                        "             </ShareColumns>"
                                        "           </ECCustomAttributes>"
                                        "        <ECProperty propertyName='P_Base' typeName='long'>"
                                        "           <ECCustomAttributes>"
                                        "            <PropertyMap xmlns='ECDbMap.02.00'>"
                                        "               <Collation>NoCase</Collation>"
                                        "            </PropertyMap>"
                                        "           </ECCustomAttributes>"
                                        "        </ECProperty>"
                                        "    </ECEntityClass>"
                                        "</ECSchema>"))) << "Collation on shared column";


    ASSERT_EQ(SUCCESS, SetupECDb("propertymapcacolumnnamecollationtests.ecdb",
                            SchemaItem(
                                "<?xml version='1.0' encoding='utf-8'?>"
                                "<ECSchema schemaName='TestSchema' alias='ts3' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                                "    <ECEntityClass typeName='Base' modifier='None'>"
                                "           <ECCustomAttributes>"
                                "            <ClassMap xmlns='ECDbMap.02.00'>"
                                "               <MapStrategy>TablePerHierarchy</MapStrategy>"
                                "            </ClassMap>"
                                "           </ECCustomAttributes>"
                                "        <ECProperty propertyName='P_Base' typeName='long'>"
                                "           <ECCustomAttributes>"
                                "            <PropertyMap xmlns='ECDbMap.02.00'>"
                                "               <IsUnique>True</IsUnique>"
                                "               <Collation>NoCase</Collation>"
                                "            </PropertyMap>"
                                "           </ECCustomAttributes>"
                                "        </ECProperty>"
                                "    </ECEntityClass>"
                                "    <ECEntityClass typeName='Sub1' modifier='None'>"
                                "        <BaseClass>Base</BaseClass>"
                                "        <ECProperty propertyName='P_Sub1' typeName='long'>"
                                "           <ECCustomAttributes>"
                                "            <PropertyMap xmlns='ECDbMap.02.00'>"
                                "               <IsUnique>True</IsUnique>"
                                "               <Collation>NoCase</Collation>"
                                "            </PropertyMap>"
                                "           </ECCustomAttributes>"
                                "        </ECProperty>"
                                "    </ECEntityClass>"
                                "    <ECEntityClass typeName='Sub2' modifier='None'>"
                                "           <ECCustomAttributes>"
                                "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                                "           </ECCustomAttributes>"
                                "        <BaseClass>Base</BaseClass>"
                                "        <ECProperty propertyName='P_Sub2' typeName='long'>"
                                "           <ECCustomAttributes>"
                                "            <PropertyMap xmlns='ECDbMap.02.00'>"
                                "               <IsUnique>True</IsUnique>"
                                "               <Collation>NoCase</Collation>"
                                "            </PropertyMap>"
                                "           </ECCustomAttributes>"
                                "        </ECProperty>"
                                "    </ECEntityClass>"
                                "    <ECEntityClass typeName='Sub2Sub' modifier='None'>"
                                "      <BaseClass>Sub2</BaseClass>"
                                "        <ECProperty propertyName='P_Sub2Sub' typeName='long'>"
                                "           <ECCustomAttributes>"
                                "            <PropertyMap xmlns='ECDbMap.02.00'>"
                                "               <IsUnique>True</IsUnique>"
                                "               <Collation>NoCase</Collation>"
                                "            </PropertyMap>"
                                "           </ECCustomAttributes>"
                                "        </ECProperty>"
                                "    </ECEntityClass>"
                                "    <ECEntityClass typeName='Sub2Sub2' modifier='None'>"
                                "      <ECCustomAttributes>"
                                "        <ShareColumns xmlns='ECDbMap.02.00'>"
                                "           <MaxSharedColumnsBeforeOverflow>2</MaxSharedColumnsBeforeOverflow>"
                                "           <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
                                "        </ShareColumns>"
                                "       </ECCustomAttributes>"
                                "       <BaseClass>Sub2</BaseClass>"
                                "        <ECProperty propertyName='P_Sub2Sub2' typeName='long'>"
                                "           <ECCustomAttributes>"
                                "            <PropertyMap xmlns='ECDbMap.02.00'>"
                                "               <IsUnique>True</IsUnique>"
                                "               <Collation>NoCase</Collation>"
                                "            </PropertyMap>"
                                "           </ECCustomAttributes>"
                                "        </ECProperty>"
                                "    </ECEntityClass>"
                                "</ECSchema>")));

    bvector<Utf8String> actualColNames;
    ASSERT_TRUE(m_ecdb.GetColumns(actualColNames, "ts3_Base"));
    ASSERT_EQ(5, actualColNames.size()) << "ts3_Base";
    ASSERT_STRCASEEQ("Id", actualColNames[0].c_str()) << "ts3_Base";
    ASSERT_STRCASEEQ("ECClassId", actualColNames[1].c_str()) << "ts3_Base";
    ASSERT_STRCASEEQ("P_Base", actualColNames[2].c_str()) << "ts3_Base";
    ASSERT_STRCASEEQ("P_Sub1", actualColNames[3].c_str()) << "ts3_Base";
    ASSERT_STRCASEEQ("P_Sub2", actualColNames[4].c_str()) << "ts3_Base";

    Utf8String tsBaseDdl = RetrieveDdl(m_ecdb, "ts3_Base");
    ASSERT_FALSE(tsBaseDdl.empty());

    ASSERT_TRUE(tsBaseDdl.ContainsI("[P_Base] INTEGER UNIQUE COLLATE NOCASE,")) << tsBaseDdl.c_str();
    ASSERT_TRUE(tsBaseDdl.ContainsI("[P_Sub1] INTEGER UNIQUE COLLATE NOCASE,")) << tsBaseDdl.c_str();
    ASSERT_TRUE(tsBaseDdl.ContainsI("[P_Sub2] INTEGER UNIQUE COLLATE NOCASE")) << tsBaseDdl.c_str();


    actualColNames.clear();
    ASSERT_TRUE(m_ecdb.GetColumns(actualColNames, "ts3_Sub2Sub"));
    ASSERT_EQ(3, actualColNames.size()) << "ts3_Sub2Sub";
    ASSERT_STRCASEEQ("BaseId", actualColNames[0].c_str()) << "ts3_Sub2Sub";
    ASSERT_STRCASEEQ("ECClassId", actualColNames[1].c_str()) << "ts3_Sub2Sub";
    ASSERT_STRCASEEQ("P_Sub2sub", actualColNames[2].c_str()) << "ts3_Sub2Sub";

    Utf8String tsSub2SubDdl = RetrieveDdl(m_ecdb, "ts3_Sub2Sub");
    ASSERT_FALSE(tsSub2SubDdl.empty());

    ASSERT_TRUE(tsSub2SubDdl.ContainsI("[P_Sub2sub] INTEGER UNIQUE COLLATE NOCASE,")) << tsSub2SubDdl.c_str();

    actualColNames.clear();
    ASSERT_TRUE(m_ecdb.GetColumns(actualColNames, "ts3_Sub2Sub2"));
    ASSERT_EQ(3, actualColNames.size()) << "ts3_Sub2Sub2";
    ASSERT_STRCASEEQ("BaseId", actualColNames[0].c_str()) << "ts3_Sub2Sub2";
    ASSERT_STRCASEEQ("ECClassId", actualColNames[1].c_str()) << "ts3_Sub2Sub2";
    ASSERT_STRCASEEQ("P_Sub2sub2", actualColNames[2].c_str()) << "ts3_Sub2Sub2";

    Utf8String tsSub2Sub2Ddl = RetrieveDdl(m_ecdb, "ts3_Sub2Sub2");
    ASSERT_FALSE(tsSub2Sub2Ddl.empty());

    ASSERT_TRUE(tsSub2Sub2Ddl.ContainsI("[P_Sub2sub2] INTEGER UNIQUE COLLATE NOCASE,")) << tsSub2Sub2Ddl.c_str();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     11/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, PropertyMapCAIsNullableIsUnique)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("propertymapcatests.ecdb",
                            SchemaItem(
                                "<?xml version='1.0' encoding='utf-8'?>"
                                "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                                "    <ECEntityClass typeName='Base' modifier='None'>"
                                "           <ECCustomAttributes>"
                                "            <ClassMap xmlns='ECDbMap.02.00'>"
                                "               <MapStrategy>TablePerHierarchy</MapStrategy>"
                                "            </ClassMap>"
                                "           </ECCustomAttributes>"
                                "        <ECProperty propertyName='P_Base' typeName='long'>"
                                "           <ECCustomAttributes>"
                                "            <PropertyMap xmlns='ECDbMap.02.00'>"
                                "               <IsNullable>false</IsNullable>"
                                "               <IsUnique>true</IsUnique>"
                                "            </PropertyMap>"
                                "           </ECCustomAttributes>"
                                "        </ECProperty>"
                                "    </ECEntityClass>"
                                "    <ECEntityClass typeName='Sub1' modifier='None'>"
                                "        <BaseClass>Base</BaseClass>"
                                "        <ECProperty propertyName='P_Sub1' typeName='long'>"
                                "           <ECCustomAttributes>"
                                "            <PropertyMap xmlns='ECDbMap.02.00'>"
                                "               <IsNullable>false</IsNullable>"
                                "               <IsUnique>true</IsUnique>"
                                "            </PropertyMap>"
                                "           </ECCustomAttributes>"
                                "        </ECProperty>"
                                "    </ECEntityClass>"
                                "    <ECEntityClass typeName='Sub2' modifier='None'>"
                                "           <ECCustomAttributes>"
                                "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                                "           </ECCustomAttributes>"
                                "        <BaseClass>Base</BaseClass>"
                                "        <ECProperty propertyName='P_Sub2' typeName='long'>"
                                "           <ECCustomAttributes>"
                                "            <PropertyMap xmlns='ECDbMap.02.00'>"
                                "               <IsNullable>false</IsNullable>"
                                "               <IsUnique>true</IsUnique>"
                                "            </PropertyMap>"
                                "           </ECCustomAttributes>"
                                "        </ECProperty>"
                                "    </ECEntityClass>"
                                "    <ECEntityClass typeName='Sub2Sub' modifier='None'>"
                                "      <BaseClass>Sub2</BaseClass>"
                                "        <ECProperty propertyName='P_Sub2Sub' typeName='long'>"
                                "           <ECCustomAttributes>"
                                "            <PropertyMap xmlns='ECDbMap.02.00'>"
                                "               <IsNullable>false</IsNullable>"
                                "               <IsUnique>true</IsUnique>"
                                "            </PropertyMap>"
                                "           </ECCustomAttributes>"
                                "        </ECProperty>"
                                "    </ECEntityClass>"
                                "    <ECEntityClass typeName='Sub2SubSub' modifier='None'>"
                                "        <BaseClass>Sub2Sub</BaseClass>"
                                "        <ECProperty propertyName='P_Sub2SubSub' typeName='long'>"
                                "           <ECCustomAttributes>"
                                "            <PropertyMap xmlns='ECDbMap.02.00'>"
                                "               <IsNullable>false</IsNullable>"
                                "               <IsUnique>true</IsUnique>"
                                "            </PropertyMap>"
                                "           </ECCustomAttributes>"
                                "        </ECProperty>"
                                "    </ECEntityClass>"
                                "    <ECEntityClass typeName='Sub2Sub2' modifier='None'>"
                                "      <ECCustomAttributes>"
                                "        <ShareColumns xmlns='ECDbMap.02.00'>"
                                "           <MaxSharedColumnsBeforeOverflow>2</MaxSharedColumnsBeforeOverflow>"
                                "           <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>"
                                "        </ShareColumns>"
                                "       </ECCustomAttributes>"
                                "      <BaseClass>Sub2</BaseClass>"
                                "        <ECProperty propertyName='P_Sub2Sub2' typeName='long'>"
                                "           <ECCustomAttributes>"
                                "            <PropertyMap xmlns='ECDbMap.02.00'>"
                                "               <IsNullable>false</IsNullable>"
                                "               <IsUnique>true</IsUnique>"
                                "            </PropertyMap>"
                                "           </ECCustomAttributes>"
                                "        </ECProperty>"
                                "    </ECEntityClass>"
                                "    <ECEntityClass typeName='Sub2Sub2Sub' modifier='None'>"
                                "        <BaseClass>Sub2Sub2</BaseClass>"
                                "        <ECProperty propertyName='P1_Sub2Sub2Sub' typeName='long'>"
                                "           <ECCustomAttributes>"
                                "            <PropertyMap xmlns='ECDbMap.02.00'>"
                                "               <IsNullable>false</IsNullable>"
                                "               <IsUnique>true</IsUnique>"
                                "            </PropertyMap>"
                                "           </ECCustomAttributes>"
                                "        </ECProperty>"
                                    "        <ECProperty propertyName='P2_Sub2Sub2Sub' typeName='long'>"
                                    "           <ECCustomAttributes>"
                                    "            <PropertyMap xmlns='ECDbMap.02.00'>"
                                    "               <IsNullable>false</IsNullable>"
                                    "               <IsUnique>true</IsUnique>"
                                    "            </PropertyMap>"
                                    "           </ECCustomAttributes>"
                                    "        </ECProperty>"
                                    "    </ECEntityClass>"
                                "</ECSchema>")));

    bvector<Utf8String> actualColNames;
    ASSERT_TRUE(m_ecdb.GetColumns(actualColNames, "ts_Base"));
    ASSERT_EQ(5, actualColNames.size()) << "ts_Base";
    ASSERT_STRCASEEQ("Id", actualColNames[0].c_str()) << "ts_Base";
    ASSERT_STRCASEEQ("ECClassId", actualColNames[1].c_str()) << "ts_Base";
    ASSERT_STRCASEEQ("P_Base", actualColNames[2].c_str()) << "ts_Base";
    ASSERT_STRCASEEQ("P_Sub1", actualColNames[3].c_str()) << "ts_Base";
    ASSERT_STRCASEEQ("P_Sub2", actualColNames[4].c_str()) << "ts_Base";


    Utf8String tsBaseDdl = RetrieveDdl(m_ecdb, "ts_Base");
    ASSERT_TRUE(tsBaseDdl.ContainsI("[P_Base] INTEGER NOT NULL UNIQUE,")) << tsBaseDdl.c_str();
    ASSERT_TRUE(tsBaseDdl.ContainsI("[P_Sub1] INTEGER UNIQUE,")) << tsBaseDdl.c_str();
    ASSERT_TRUE(tsBaseDdl.ContainsI("[P_Sub2] INTEGER UNIQUE")) << tsBaseDdl.c_str();


    actualColNames.clear();
    ASSERT_TRUE(m_ecdb.GetColumns(actualColNames, "ts_Sub2Sub"));
    ASSERT_EQ(4, actualColNames.size()) << "ts_Sub2Sub";
    ASSERT_STRCASEEQ("BaseId", actualColNames[0].c_str()) << "ts_Sub2Sub";
    ASSERT_STRCASEEQ("ECClassId", actualColNames[1].c_str()) << "ts_Sub2Sub";
    ASSERT_STRCASEEQ("P_Sub2sub", actualColNames[2].c_str()) << "ts_Sub2Sub";
    ASSERT_STRCASEEQ("P_Sub2subsub", actualColNames[3].c_str()) << "ts_Sub2Sub";

    Utf8String tsSub2SubDdl = RetrieveDdl(m_ecdb, "ts_Sub2Sub");
    ASSERT_TRUE(tsSub2SubDdl.ContainsI("[P_Sub2sub] INTEGER NOT NULL UNIQUE,")) << tsSub2SubDdl.c_str();
    ASSERT_TRUE(tsSub2SubDdl.ContainsI("[P_Sub2subsub] INTEGER UNIQUE,")) << tsSub2SubDdl.c_str();

    actualColNames.clear();
    ASSERT_TRUE(m_ecdb.GetColumns(actualColNames, "ts_Sub2Sub2"));
    ASSERT_EQ(4, actualColNames.size()) << "ts_Sub2Sub2";
    ASSERT_STRCASEEQ("BaseId", actualColNames[0].c_str()) << "ts_Sub2Sub2";
    ASSERT_STRCASEEQ("ECClassId", actualColNames[1].c_str()) << "ts_Sub2Sub2";
    ASSERT_STRCASEEQ("js1", actualColNames[2].c_str()) << "ts_Sub2Sub2";
    ASSERT_STRCASEEQ("js2", actualColNames[3].c_str()) << "ts_Sub2Sub2";

    Utf8String tsSub2Sub2Ddl = RetrieveDdl(m_ecdb, "ts_Sub2Sub2");
    ASSERT_TRUE(tsSub2Sub2Ddl.ContainsI("[js1] BLOB,")) << tsSub2Sub2Ddl.c_str();
    ASSERT_TRUE(tsSub2Sub2Ddl.ContainsI("[js2] BLOB,")) << tsSub2Sub2Ddl.c_str();

    actualColNames.clear();
    ASSERT_TRUE(m_ecdb.GetColumns(actualColNames, "ts_Sub2Sub2_Overflow"));
    ASSERT_EQ(3, actualColNames.size()) << "ts_Sub2Sub2_Overflow";
    ASSERT_STRCASEEQ("BaseId", actualColNames[0].c_str()) << "ts_Sub2Sub2_Overflow";
    ASSERT_STRCASEEQ("ECClassId", actualColNames[1].c_str()) << "ts_Sub2Sub2_Overflow";

    Utf8String tsSub2Sub2_OverflowDdl = RetrieveDdl(m_ecdb, "ts_Sub2Sub2_Overflow");
    ASSERT_TRUE(tsSub2Sub2_OverflowDdl.ContainsI("[os1] BLOB,")) << tsSub2Sub2_OverflowDdl.c_str();


    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, PropertyMapCAIsNullableAndIndexWhereClause)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("notnullableproptest1.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts1' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='B' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "               <Indexes>"
        "                   <DbIndex>"
        "                       <Name>ix_b_code</Name>"
        "                       <Properties>"
        "                           <string>Code</string>"
        "                       </Properties>"
        "                       <Where>IndexedColumnsAreNotNull</Where>"
        "                   </DbIndex>"
        "               </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='long' >"
        "           <ECCustomAttributes>"
        "            <PropertyMap xmlns='ECDbMap.02.00'>"
        "               <IsNullable>false</IsNullable>"
        "            </PropertyMap>"
        "           </ECCustomAttributes>"
        "        </ECProperty>"
        "        <ECProperty propertyName='Name' typeName='string' />"
        "    </ECEntityClass>"
        "</ECSchema>")));
    AssertIndex(m_ecdb, "ix_b_code", false, "ts1_B", {"Code"});

    ASSERT_EQ(SUCCESS, SetupECDb("notnullableproptest2.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts2' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='B' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "               <Indexes>"
        "                   <DbIndex>"
        "                       <Name>ix_b_code_name</Name>"
        "                       <Properties>"
        "                           <string>Code</string>"
        "                           <string>Name</string>"
        "                       </Properties>"
        "                       <Where>IndexedColumnsAreNotNull</Where>"
        "                   </DbIndex>"
        "               </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='long' >"
        "           <ECCustomAttributes>"
        "            <PropertyMap xmlns='ECDbMap.02.00'>"
        "               <IsNullable>false</IsNullable>"
        "            </PropertyMap>"
        "           </ECCustomAttributes>"
        "        </ECProperty>"
        "        <ECProperty propertyName='Name' typeName='string' />"
        "    </ECEntityClass>"
        "</ECSchema>")));
    AssertIndex(m_ecdb, "ix_b_code_name", false, "ts2_B", {"Code","Name"}, "([Name] IS NOT NULL)");


    ASSERT_EQ(SUCCESS, SetupECDb("notnullableproptest3.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts3' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='B' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <DbIndexList xmlns='ECDbMap.02.00'>"
        "               <Indexes>"
        "                   <DbIndex>"
        "                       <Name>ix_b_code_name</Name>"
        "                       <Properties>"
        "                           <string>Code</string>"
        "                           <string>Name</string>"
        "                       </Properties>"
        "                       <Where>IndexedColumnsAreNotNull</Where>"
        "                   </DbIndex>"
        "               </Indexes>"
        "            </DbIndexList>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='long' >"
        "           <ECCustomAttributes>"
        "            <PropertyMap xmlns='ECDbMap.02.00'>"
        "               <IsNullable>false</IsNullable>"
        "            </PropertyMap>"
        "           </ECCustomAttributes>"
        "        </ECProperty>"
        "        <ECProperty propertyName='Name' typeName='string'>"
        "           <ECCustomAttributes>"
        "            <PropertyMap xmlns='ECDbMap.02.00'>"
        "               <IsNullable>false</IsNullable>"
        "            </PropertyMap>"
        "           </ECCustomAttributes>"
        "        </ECProperty>"
        "    </ECEntityClass>"
        "</ECSchema>")));

    AssertIndex(m_ecdb, "ix_b_code_name", false, "ts3_B", {"Code", "Name"});
    }


//--------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         05/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, ClassHasCurrentTimeStampCA)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("classhascurrenttimestampCA.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECSchemaReference name="CoreCustomAttributes" version="01.00" alias="CoreCA" />
        <ECEntityClass typeName="Foo">
            <ECCustomAttributes>
                <ClassHasCurrentTimeStampProperty xmlns="CoreCustomAttributes.01.00">
                    <PropertyName>LastMod</PropertyName>
                </ClassHasCurrentTimeStampProperty>
            </ECCustomAttributes>
            <ECProperty propertyName="Code" typeName="int" />
            <ECProperty propertyName="LastMod" typeName="dateTime" />
        </ECEntityClass>
        </ECSchema>)xml")));

    ECInstanceKey key;
    {
    ECSqlStatement insertStatement;
    ASSERT_EQ(ECSqlStatus::Success, insertStatement.Prepare(m_ecdb, "INSERT INTO ts.Foo(Code) VALUES(12)"));
    ASSERT_EQ(BE_SQLITE_DONE, insertStatement.Step(key));
    }

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT LastMod FROM ts.Foo WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindId(1, key.GetInstanceId()));

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_FALSE(statement.IsValueNull(0));
    DateTime lastMod1 = statement.GetValueDateTime(0);
    statement.Reset();
    statement.ClearBindings();

    {
    BeThreadUtilities::BeSleep(200); // make sure the time is different by more than the resolution of the timestamp
    ECSqlStatement updateStatement;
    ASSERT_EQ(ECSqlStatus::Success, updateStatement.Prepare(m_ecdb, "UPDATE ts.Foo SET Code=23 WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, updateStatement.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, updateStatement.Step());
    }

    ASSERT_EQ(ECSqlStatus::Success, statement.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_FALSE(statement.IsValueNull(0));
    DateTime lastMod2 = statement.GetValueDateTime(0);

    int64_t lastMod1Msec, lastMod2Msec;
    ASSERT_EQ(SUCCESS, lastMod1.ToUnixMilliseconds(lastMod1Msec));
    ASSERT_EQ(SUCCESS, lastMod2.ToUnixMilliseconds(lastMod2Msec));
    ASSERT_TRUE(lastMod2Msec - lastMod1Msec > INT64_C(100)) << "LastMod should have been updated after the last UPDATE statement";
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle             03/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, ClassHasCurrentTimeStampCAOnMixin)
    {
    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECSchemaReference name="CoreCustomAttributes" version="01.00" alias="CoreCA" />
        <ECEntityClass typeName="Base" modifier="Abstract" />
        <ECEntityClass typeName="IHasLastMod" modifier="Abstract">
            <ECCustomAttributes>
                <IsMixin xmlns="CoreCustomAttributes.01.00">
                    <AppliesToEntityClass>Base</AppliesToEntityClass>
                </IsMixin>
                <ClassHasCurrentTimeStampProperty xmlns="CoreCustomAttributes.01.00">
                    <PropertyName>LastMod</PropertyName>
                </ClassHasCurrentTimeStampProperty>
            </ECCustomAttributes>
            <ECProperty propertyName="LastMod" typeName="dateTime" />
        </ECEntityClass>
        <ECEntityClass typeName="Foo" >
            <BaseClass>Base</BaseClass>
            <BaseClass>IHasLastMod</BaseClass>
            <ECProperty propertyName="Code" typeName="int" />
        </ECEntityClass>
        </ECSchema>)xml"))) << "ClassHasCurrentTimeStampProperty may not be assigned to Mixin";
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, DiamondProblem_Case0)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("diamond_problem.ecdb",
              SchemaItem("<?xml version='1.0' encoding='UTF-8'?>"
                         "<ECSchema schemaName='Foo' alias='Foo' version='01.00.00' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                         "  <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA'/>"
                         "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                         "  <ECEntityClass typeName='IBehaviour1' modifier='Abstract'>"
                         "    <ECCustomAttributes>"
                         "      <IsMixin xmlns='CoreCustomAttributes.01.00'>"
                         "        <AppliesToEntityClass>Object</AppliesToEntityClass>"
                         "      </IsMixin>"
                         "    </ECCustomAttributes>"
                         "    <ECProperty propertyName='IB1' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='IBehaviour2' modifier='Abstract'>"
                         "    <ECCustomAttributes>"
                         "      <IsMixin xmlns='CoreCustomAttributes.01.00'>"
                         "        <AppliesToEntityClass>Object</AppliesToEntityClass>"
                         "      </IsMixin>"
                         "    </ECCustomAttributes>"
                         "    <ECProperty propertyName='IB2' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='IBehaviour3' modifier='Abstract'>"
                         "    <ECCustomAttributes>"
                         "      <IsMixin xmlns='CoreCustomAttributes.01.00'>"
                         "        <AppliesToEntityClass>Object</AppliesToEntityClass>"
                         "      </IsMixin>"
                         "    </ECCustomAttributes>"
                         "    <ECProperty propertyName='IB3' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='Object' modifier='Abstract'>"
                         "    <ECCustomAttributes>"
                         "      <ClassMap xmlns='ECDbMap.02.00'>"
                         "        <MapStrategy>TablePerHierarchy</MapStrategy>"
                         "      </ClassMap>"
                         "      <ShareColumns xmlns='ECDbMap.02.00'>"
                         "        <MaxSharedColumnsBeforeOverflow>10</MaxSharedColumnsBeforeOverflow>"
                         "        <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>"
                         "      </ShareColumns>"
                         "    </ECCustomAttributes>"
                         "    <ECProperty propertyName='P0' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='SubObject1'>"
                         "    <BaseClass>Object</BaseClass>"
                         "    <BaseClass>IBehaviour1</BaseClass>"
                         "    <ECProperty propertyName='P1' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='SubObject2'>"
                         "    <BaseClass>Object</BaseClass>"
                         "    <BaseClass>IBehaviour2</BaseClass>"
                         "    <ECProperty propertyName='P2' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='SubObject3'>"
                         "    <BaseClass>Object</BaseClass>"
                         "    <BaseClass>IBehaviour3</BaseClass>"
                         "    <ECProperty propertyName='P3' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='SubObject123'>"
                         "    <BaseClass>Object</BaseClass>"
                         "    <BaseClass>IBehaviour1</BaseClass>"
                         "    <BaseClass>IBehaviour2</BaseClass>"
                         "    <BaseClass>IBehaviour3</BaseClass>"
                         "    <ECProperty propertyName='P123' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='SubObject11'>"
                         "    <BaseClass>Object</BaseClass>"
                         "    <BaseClass>IBehaviour1</BaseClass>"
                         "    <ECProperty propertyName='P11' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='SubObject12'>"
                         "    <BaseClass>SubObject11</BaseClass>"
                         "    <BaseClass>IBehaviour2</BaseClass>"
                         "    <ECProperty propertyName='P12' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='SubObject13'>"
                         "    <BaseClass>SubObject12</BaseClass>"
                         "    <BaseClass>IBehaviour3</BaseClass>"
                         "    <ECProperty propertyName='P13' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='SubObject21'>"
                         "    <BaseClass>Object</BaseClass>"
                         "    <BaseClass>IBehaviour1</BaseClass>"
                         "    <ECProperty propertyName='P21' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='SubObject22'>"
                         "    <BaseClass>SubObject21</BaseClass>"
                         "    <BaseClass>IBehaviour2</BaseClass>"
                         "    <ECProperty propertyName='P22' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='SubObject23'>"
                         "    <BaseClass>SubObject22</BaseClass>"
                         "    <BaseClass>IBehaviour3</BaseClass>"
                         "    <ECProperty propertyName='P23' typeName='string' />"
                         "  </ECEntityClass>"
                         "</ECSchema>" )));
    m_ecdb.Schemas().CreateClassViewsInDb();

    /*
    IBehaviour1(IB1)
    IBehaviour2(IB2)
    IBehaviour3(IB3)
    Object(P0)
    SubObject1(P0, IB1, P1)
    SubObject2(P0, IB2, P2)
    SubObject3(P0, IB3, P3)
    SubObject123(P0, IB1, IB2, IB3, P123)
    SubObject11(P0, IB1, P11)
    SubObject12(P0, IB1, P11, IB2, P12)
    SubObject13(P0, IB1, P11, IB2, P12, IB3, P13)
    SubObject21(P0, IB1, P21)
    SubObject22(P0, IB1, P21, IB2, P22)
    SubObject23(P0, IB1, P21, IB2, P22, IB3, P23)
    */
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO Foo.SubObject1    (P0, IB1, P1)                       VALUES ('P0-1', 'IB1-1', 'P1-1')"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO Foo.SubObject2    (P0, IB2, P2)                       VALUES ('P0-2', 'IB2-1', 'P2-1')"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO Foo.SubObject3    (P0, IB3, P3)                       VALUES ('P0-3', 'IB3-1', 'P3-1')"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO Foo.SubObject123  (P0, IB1, IB2, IB3, P123)           VALUES ('P0-4', 'IB1-2', 'IB2-2', 'IB3-2', 'P123-1')"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO Foo.SubObject11   (P0, IB1, P11)                      VALUES ('P0-5', 'IB1-3', 'P11-1')"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO Foo.SubObject12   (P0, IB1, P11, IB2, P12)            VALUES ('P0-6', 'IB1-4', 'P11-2', 'IB2-3', 'P12-1')"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO Foo.SubObject13   (P0, IB1, P11, IB2, P12, IB3, P13)  VALUES ('P0-7', 'IB1-5', 'P11-3', 'IB2-4', 'P12-2', 'IB3-3', 'P13-1')"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO Foo.SubObject21   (P0, IB1, P21)                      VALUES ('P0-8', 'IB1-6', 'P21-1')"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO Foo.SubObject22   (P0, IB1, P21, IB2, P22)            VALUES ('P0-9', 'IB1-7', 'P21-2', 'IB2-5', 'P22-1')"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO Foo.SubObject23   (P0, IB1, P21, IB2, P22, IB3, P23)  VALUES ('P0-0', 'IB1-0', 'P21-3', 'IB2-6', 'P22-2', 'IB3-4', 'P23-1')"));
    m_ecdb.SaveChanges();

    //====[Foo.Object]====================================================
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT P0 FROM Foo.Object ORDER BY ECInstanceId"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_STREQ("P0-1", stmt.GetValueText(0));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_STREQ("P0-2", stmt.GetValueText(0));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_STREQ("P0-3", stmt.GetValueText(0));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_STREQ("P0-4", stmt.GetValueText(0));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_STREQ("P0-5", stmt.GetValueText(0));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_STREQ("P0-6", stmt.GetValueText(0));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_STREQ("P0-7", stmt.GetValueText(0));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_STREQ("P0-8", stmt.GetValueText(0));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_STREQ("P0-9", stmt.GetValueText(0));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_STREQ("P0-0", stmt.GetValueText(0));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    //====[Foo.SubObject1]====================================================
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT P0, IB1, P1 FROM Foo.SubObject1 ORDER BY ECInstanceId"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_STREQ("P0-1", stmt.GetValueText(0));
    ASSERT_STREQ("IB1-1", stmt.GetValueText(1));
    ASSERT_STREQ("P1-1", stmt.GetValueText(2));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    //====[Foo.SubObject2]====================================================
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT P0, IB2, P2 FROM Foo.SubObject2 ORDER BY ECInstanceId"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_STREQ("P0-2", stmt.GetValueText(0));
    ASSERT_STREQ("IB2-1", stmt.GetValueText(1));
    ASSERT_STREQ("P2-1", stmt.GetValueText(2));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    //====[Foo.SubObject3]====================================================
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT P0, IB3, P3 FROM Foo.SubObject3 ORDER BY ECInstanceId"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_STREQ("P0-3", stmt.GetValueText(0));
    ASSERT_STREQ("IB3-1", stmt.GetValueText(1));
    ASSERT_STREQ("P3-1", stmt.GetValueText(2));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    //====[Foo.SubObject123]====================================================
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT P0, IB1, IB2, IB3, P123 FROM Foo.SubObject123 ORDER BY ECInstanceId"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_STREQ("P0-4", stmt.GetValueText(0));
    ASSERT_STREQ("IB1-2", stmt.GetValueText(1));
    ASSERT_STREQ("IB2-2", stmt.GetValueText(2));
    ASSERT_STREQ("IB3-2", stmt.GetValueText(3));
    ASSERT_STREQ("P123-1", stmt.GetValueText(4));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    //====[Foo.SubObject11]====================================================
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT P0, IB1, P11 FROM Foo.SubObject11 ORDER BY ECInstanceId"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_STREQ("P0-5", stmt.GetValueText(0)); ASSERT_STREQ("IB1-3", stmt.GetValueText(1)); ASSERT_STREQ("P11-1", stmt.GetValueText(2));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_STREQ("P0-6", stmt.GetValueText(0)); ASSERT_STREQ("IB1-4", stmt.GetValueText(1)); ASSERT_STREQ("P11-2", stmt.GetValueText(2));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_STREQ("P0-7", stmt.GetValueText(0)); ASSERT_STREQ("IB1-5", stmt.GetValueText(1)); ASSERT_STREQ("P11-3", stmt.GetValueText(2));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    //====[Foo.SubObject12]====================================================
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT P0, IB1, P11, IB2, P12 FROM Foo.SubObject12 ORDER BY ECInstanceId"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_STREQ("P0-6", stmt.GetValueText(0)); ASSERT_STREQ("IB1-4", stmt.GetValueText(1)); ASSERT_STREQ("P11-2", stmt.GetValueText(2)); ASSERT_STREQ("IB2-3", stmt.GetValueText(3)); ASSERT_STREQ("P12-1", stmt.GetValueText(4));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_STREQ("P0-7", stmt.GetValueText(0)); ASSERT_STREQ("IB1-5", stmt.GetValueText(1)); ASSERT_STREQ("P11-3", stmt.GetValueText(2)); ASSERT_STREQ("IB2-4", stmt.GetValueText(3)); ASSERT_STREQ("P12-2", stmt.GetValueText(4));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    //====[Foo.SubObject13]====================================================
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT P0, IB1, P11, IB2, P12, IB3, P13 FROM Foo.SubObject13 ORDER BY ECInstanceId"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_STREQ("P0-7", stmt.GetValueText(0)); ASSERT_STREQ("IB1-5", stmt.GetValueText(1)); ASSERT_STREQ("P11-3", stmt.GetValueText(2)); ASSERT_STREQ("IB2-4", stmt.GetValueText(3)); ASSERT_STREQ("P12-2", stmt.GetValueText(4)); ASSERT_STREQ("IB3-3", stmt.GetValueText(5)); ASSERT_STREQ("P13-1", stmt.GetValueText(6));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    //====[Foo.SubObject21]====================================================
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT P0, IB1, P21 FROM Foo.SubObject21 ORDER BY ECInstanceId"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_STREQ("P0-8", stmt.GetValueText(0)); ASSERT_STREQ("IB1-6", stmt.GetValueText(1)); ASSERT_STREQ("P21-1", stmt.GetValueText(2));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_STREQ("P0-9", stmt.GetValueText(0)); ASSERT_STREQ("IB1-7", stmt.GetValueText(1)); ASSERT_STREQ("P21-2", stmt.GetValueText(2));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_STREQ("P0-0", stmt.GetValueText(0)); ASSERT_STREQ("IB1-0", stmt.GetValueText(1)); ASSERT_STREQ("P21-3", stmt.GetValueText(2));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    //====[Foo.SubObject22]====================================================
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT P0, IB1, P21, IB2, P22 FROM Foo.SubObject22 ORDER BY ECInstanceId"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_STREQ("P0-9", stmt.GetValueText(0)); ASSERT_STREQ("IB1-7", stmt.GetValueText(1)); ASSERT_STREQ("P21-2", stmt.GetValueText(2)); ASSERT_STREQ("IB2-5", stmt.GetValueText(3)); ASSERT_STREQ("P22-1", stmt.GetValueText(4));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_STREQ("P0-0", stmt.GetValueText(0)); ASSERT_STREQ("IB1-0", stmt.GetValueText(1)); ASSERT_STREQ("P21-3", stmt.GetValueText(2)); ASSERT_STREQ("IB2-6", stmt.GetValueText(3)); ASSERT_STREQ("P22-2", stmt.GetValueText(4));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    //====[Foo.SubObject23]====================================================
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT P0, IB1, P21, IB2, P22, IB3, P23 FROM Foo.SubObject23 ORDER BY ECInstanceId"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_STREQ("P0-0", stmt.GetValueText(0)); ASSERT_STREQ("IB1-0", stmt.GetValueText(1)); ASSERT_STREQ("P21-3", stmt.GetValueText(2)); ASSERT_STREQ("IB2-6", stmt.GetValueText(3)); ASSERT_STREQ("P22-2", stmt.GetValueText(4)); ASSERT_STREQ("IB3-4", stmt.GetValueText(5)); ASSERT_STREQ("P23-1", stmt.GetValueText(6));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    //====[Foo.IBehaviour1]====================================================
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT IB1 FROM Foo.IBehaviour1 ORDER BY ECInstanceId"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_STREQ("IB1-1", stmt.GetValueText(0));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_STREQ("IB1-2", stmt.GetValueText(0));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_STREQ("IB1-3", stmt.GetValueText(0));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_STREQ("IB1-4", stmt.GetValueText(0));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_STREQ("IB1-5", stmt.GetValueText(0));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_STREQ("IB1-6", stmt.GetValueText(0));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_STREQ("IB1-7", stmt.GetValueText(0));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_STREQ("IB1-0", stmt.GetValueText(0));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    //====[Foo.IBehaviour2]====================================================
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT IB2 FROM Foo.IBehaviour2 ORDER BY ECInstanceId"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_STREQ("IB2-1", stmt.GetValueText(0));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_STREQ("IB2-2", stmt.GetValueText(0));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_STREQ("IB2-3", stmt.GetValueText(0));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_STREQ("IB2-4", stmt.GetValueText(0));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_STREQ("IB2-5", stmt.GetValueText(0));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_STREQ("IB2-6", stmt.GetValueText(0));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    //====[Foo.IBehaviour3]====================================================
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT IB3 FROM Foo.IBehaviour3 ORDER BY ECInstanceId"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_STREQ("IB3-1", stmt.GetValueText(0));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_STREQ("IB3-2", stmt.GetValueText(0));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_STREQ("IB3-3", stmt.GetValueText(0));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()); ASSERT_STREQ("IB3-4", stmt.GetValueText(0));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, DiamondProblem_Case1)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("diamond_problem.ecdb", SchemaItem(
                         "<ECSchema schemaName='TestSchema' alias='ts' version='01.00.00' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                         "  <ECSchemaReference name='ECDbMap' version='02.00.00' alias='ecdbmap' />"
                         "  <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA'/>"
                         "  <ECEntityClass typeName='BaseClass' modifier='Abstract' >"
                         "      <ECCustomAttributes>"
                         "          <ClassMap xmlns='ECDbMap.02.00'>"
                         "              <MapStrategy>TablePerHierarchy</MapStrategy>"
                         "          </ClassMap>"
                         "          <ShareColumns xmlns='ECDbMap.02.00'>"
                         "              <MaxSharedColumnsBeforeOverflow>7</MaxSharedColumnsBeforeOverflow>"
                         "              <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>"
                         "          </ShareColumns>"
                         "      </ECCustomAttributes>"
                         "      <ECProperty propertyName='P1' typeName='long' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='IXFace' modifier='Abstract'>"
                         "      <ECCustomAttributes>"
                         "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
                         "              <AppliesToEntityClass>BaseClass</AppliesToEntityClass>"
                         "          </IsMixin>"
                         "      </ECCustomAttributes>"
                         "      <ECProperty propertyName='P2' typeName='long' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='D_A'>" //(p1,p2,p3)
                         "      <BaseClass>BaseClass</BaseClass>"
                         "      <BaseClass>IXFace</BaseClass>"
                         "      <ECProperty propertyName='P3' typeName='long' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='D_B'>"//(p1,p4)
                         "      <BaseClass>BaseClass</BaseClass>"   //p1
                         "      <ECProperty propertyName='P4' typeName='long' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='DB_XFace'>"//(p1,p2, p4)
                         "      <BaseClass>D_B</BaseClass>"
                         "      <BaseClass>IXFace</BaseClass>"
                         "      <ECProperty propertyName='P5' typeName='long' />"
                         "  </ECEntityClass>"
                         "</ECSchema>"))) << "Diamond Problem";

    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO ts.D_A      (P1, P2, P3) VALUES (11, 21, 31)"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO ts.D_B      (P1, P4    ) VALUES (12, 42    )"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO ts.DB_XFace (P1, P2, P4) VALUES (12, 22, 43)"));
    m_ecdb.Schemas().CreateClassViewsInDb();
    m_ecdb.SaveChanges();
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT P1, P2, P3 FROM ts.D_A WHERE ECInstanceId = 1"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(11, stmt.GetValueInt64(0));
    ASSERT_EQ(21, stmt.GetValueInt64(1));
    ASSERT_EQ(31, stmt.GetValueInt64(2));

    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT P1, P4 FROM ts.D_B WHERE ECInstanceId = 2"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(12, stmt.GetValueInt64(0)) << stmt.GetNativeSql();
    ASSERT_EQ(42, stmt.GetValueInt64(1)) << stmt.GetNativeSql();

    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT P1, P2, P4 FROM ts.DB_XFace WHERE ECInstanceId = 3"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(12, stmt.GetValueInt64(0)) << stmt.GetNativeSql();
    ASSERT_EQ(22, stmt.GetValueInt64(1)) << stmt.GetNativeSql();
    ASSERT_EQ(43, stmt.GetValueInt64(2)) << stmt.GetNativeSql();

    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT P2 FROM ts.IXFace ORDER BY P2 "));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(21, stmt.GetValueInt64(0)) << stmt.GetNativeSql();
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(22, stmt.GetValueInt64(0)) << stmt.GetNativeSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, DiamondProblem_Case2)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("diamond_problem.ecdb",
              SchemaItem("<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                         "  <ECSchemaReference name='ECDbMap' version='02.00.00' alias='ecdbmap' />"
                         "  <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA'/>"
                         "  <ECEntityClass typeName='BaseClass' modifier='Abstract'>"
                         "      <ECCustomAttributes>"
                         "          <ClassMap xmlns='ECDbMap.02.00'>"
                         "              <MapStrategy>TablePerHierarchy</MapStrategy>"
                         "          </ClassMap>"
                         "          <ShareColumns xmlns='ECDbMap.02.00'>"
                         "              <MaxSharedColumnsBeforeOverflow>10</MaxSharedColumnsBeforeOverflow>"
                         "              <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>"
                         "          </ShareColumns>"
                         "      </ECCustomAttributes>"
                         "      <ECProperty propertyName='P1' typeName='long' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='IXFaceA' modifier='Abstract'>"
                         "      <ECCustomAttributes>"
                         "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
                         "              <AppliesToEntityClass>BaseClass</AppliesToEntityClass>"
                         "          </IsMixin>"
                         "      </ECCustomAttributes>"
                         "      <ECProperty propertyName='P2' typeName='long' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='IXFaceB' modifier='Abstract'>"
                         "      <ECCustomAttributes>"
                         "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
                         "              <AppliesToEntityClass>BaseClass</AppliesToEntityClass>"
                         "          </IsMixin>"
                         "      </ECCustomAttributes>"
                         "      <ECProperty propertyName='P3' typeName='long' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='D3_A'>" //(p1,p2,p4)
                         "      <BaseClass>BaseClass</BaseClass>"   //p1
                         "      <BaseClass>IXFaceA</BaseClass>"     //p2
                         "      <ECProperty propertyName='P4' typeName='long' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='D4_B'>"//(p1,p3,p5)
                         "      <BaseClass>BaseClass</BaseClass>"   //p1
                         "      <BaseClass>IXFaceB</BaseClass>"     //P3
                         "      <ECProperty propertyName='P5' typeName='long' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='D1_AB'>"//(p1,p2,p4,p3,p6)
                         "      <BaseClass>D3_A</BaseClass>"    //p1,p2,p4
                         "      <BaseClass>IXFaceB</BaseClass>" //p3
                         "      <ECProperty propertyName='P6' typeName='long' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='D2_AB'>"//(p1,p3,p5,p2,p7)
                         "      <BaseClass>D4_B</BaseClass>"    //p1,p3,p5
                         "      <BaseClass>IXFaceA</BaseClass>" //p2
                         "      <ECProperty propertyName='P7' typeName='long' />"
                         "  </ECEntityClass>"
                         "</ECSchema>")));

    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO ts.D1_AB (P1, P2, P3, P4, P6) VALUES (11, 21, 31, 41, 61)"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO ts.D2_AB (P1, P2, P3, P5, P7) VALUES (12, 22, 32, 52, 72)"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO ts.D3_A  (P1, P2, P4)     VALUES (13, 23, 43)"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO ts.D4_B  (P1, P3, P5)     VALUES (14, 34, 54)"));
    m_ecdb.Schemas().CreateClassViewsInDb();
    m_ecdb.SaveChanges();
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT P1, P2, P3, P4, P6 FROM ts.D1_AB WHERE ECInstanceId = 1"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(11, stmt.GetValueInt64(0));
    ASSERT_EQ(21, stmt.GetValueInt64(1));
    ASSERT_EQ(31, stmt.GetValueInt64(2));
    ASSERT_EQ(41, stmt.GetValueInt64(3));
    ASSERT_EQ(61, stmt.GetValueInt64(4));

    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT P1, P2, P3, P5, P7 FROM ts.D2_AB WHERE ECInstanceId = 2"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(12, stmt.GetValueInt64(0));
    ASSERT_EQ(22, stmt.GetValueInt64(1));
    ASSERT_EQ(32, stmt.GetValueInt64(2));
    ASSERT_EQ(52, stmt.GetValueInt64(3));
    ASSERT_EQ(72, stmt.GetValueInt64(4));

    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT P1, P2, P4 FROM ts.D3_A WHERE ECInstanceId = 3"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(13, stmt.GetValueInt64(0));
    ASSERT_EQ(23, stmt.GetValueInt64(1));
    ASSERT_EQ(43, stmt.GetValueInt64(2));

    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT P1, P3, P5 FROM ts.D4_B WHERE ECInstanceId = 4"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(14, stmt.GetValueInt64(0));
    ASSERT_EQ(34, stmt.GetValueInt64(1));
    ASSERT_EQ(54, stmt.GetValueInt64(2));

    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT P2 FROM ts.IXFaceA ORDER BY P2 "));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(21, stmt.GetValueInt64(0));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(22, stmt.GetValueInt64(0));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(23, stmt.GetValueInt64(0));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT P3 FROM ts.IXFaceB ORDER BY P3 "));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(31, stmt.GetValueInt64(0));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(32, stmt.GetValueInt64(0));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(34, stmt.GetValueInt64(0));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, DiamondProblem_Case3)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("diamond_problem3.ecdb",
              SchemaItem("<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                         "  <ECSchemaReference name='ECDbMap' version='02.00.00' alias='ecdbmap' />"
                         "  <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA'/>"
                         "  <ECEntityClass typeName='Base'>"
                         "      <ECCustomAttributes>"
                         "          <ClassMap xmlns='ECDbMap.02.00'>"
                         "              <MapStrategy>TablePerHierarchy</MapStrategy>"
                         "          </ClassMap>"
                         "          <ShareColumns xmlns='ECDbMap.02.00'>"
                         "              <MaxSharedColumnsBeforeOverflow>10</MaxSharedColumnsBeforeOverflow>"
                         "              <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>"
                         "          </ShareColumns>"
                         "      </ECCustomAttributes>"
                         "      <ECProperty propertyName='P1' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='ClassA'>"
                         "      <BaseClass>Base</BaseClass>"
                         "      <ECProperty propertyName='S1' typeName='string' />"
                         "      <ECProperty propertyName='Z1' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='IClassB' modifier='Abstract'>"
                         "      <ECCustomAttributes>"
                         "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
                         "              <AppliesToEntityClass>Base</AppliesToEntityClass>"
                         "          </IsMixin>"
                         "      </ECCustomAttributes>"
                         "      <ECProperty propertyName='P3' typeName='string' />"
                         "      <ECProperty propertyName='S1' typeName='string' />"
                         "      <ECProperty propertyName='Z1' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='ClassC'>"
                         "      <BaseClass>ClassA</BaseClass>"
                         "      <BaseClass>IClassB</BaseClass>"
                         "      <ECProperty propertyName='P4' typeName='string' />"
                         "  </ECEntityClass>"
                         "</ECSchema>")));
    m_ecdb.Schemas().CreateClassViewsInDb();
    m_ecdb.SaveChanges();

    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO ts.Base   (P1                ) VALUES ('P1-Base')"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO ts.ClassA (P1, S1, Z1        ) VALUES ('P1-ClassA', 'S1-ClassA', 'Z1-ClassA')"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO ts.ClassC (P1, P3, P4, S1, Z1) VALUES ('P1-ClassC', 'P3-ClassC', 'P4-ClassC', 'S1-ClassC', 'Z1-ClassC')"));


    m_ecdb.SaveChanges();
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "INSERT INTO ts.IClassB(P3,S1,Z1) VALUES ('P3-IClassB', 'S1-IClassB', 'Z1-IClassB')"));
    }

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT P1 FROM ts.ClassA ORDER BY P1"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_STREQ("P1-ClassA", stmt.GetValueText(0));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_STREQ("P1-ClassC", stmt.GetValueText(0));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT S1,Z1 FROM ts.IClassB"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_STREQ("S1-ClassC", stmt.GetValueText(0));
    ASSERT_STREQ("Z1-ClassC", stmt.GetValueText(1));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT S1 FROM ts.ClassC"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_STREQ("S1-ClassC", stmt.GetValueText(0));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    }



//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, VerifyPositionOfColumnsForNavigationProperty)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("useecinstanceidasfk3.ecdb",
              SchemaItem(R"xml(
    <ECSchema schemaName="TestSchema" alias="ts3" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
        <ECEntityClass typeName="Parent">
            <ECProperty propertyName="Name" typeName="string" />
        </ECEntityClass>
        <ECEntityClass typeName="Child" >
            <ECCustomAttributes>
                <ClassMap xmlns="ECDbMap.02.00">
                    <MapStrategy>TablePerHierarchy</MapStrategy>
                </ClassMap>
            </ECCustomAttributes>
            <ECProperty propertyName="ChildName" typeName="string" />
            <ECNavigationProperty propertyName="Parent" relationshipName="ParentHasChildrenBase" direction="Backward" >
                <ECCustomAttributes>
                <ForeignKeyConstraint xmlns="ECDbMap.02.00">
                    <OnDeleteAction>Cascade</OnDeleteAction>
                </ForeignKeyConstraint>
                </ECCustomAttributes>
            </ECNavigationProperty>
            <ECProperty propertyName="Phone" typeName="string" />
            <ECProperty propertyName="Address" typeName="string" />
        </ECEntityClass>
        <ECRelationshipClass typeName="ParentHasChildrenBase" strength="embedding" modifier="Abstract">
            <Source multiplicity="(1..1)" polymorphic="True" roleLabel="is parent of">
                <Class class="Parent" />
            </Source>
            <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is child of">
                <Class class="Child"/>
            </Target>
        </ECRelationshipClass>
    </ECSchema>)xml")));
    Statement stmt;
    stmt.Prepare(m_ecdb, "PRAGMA table_info(ts3_Child)");
    int indexOfParentId = -1;
    int indexOfParentRelECClassId = -1;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        if (strcmp(stmt.GetValueText(1), "ParentId") == 0)
            indexOfParentId = stmt.GetValueInt(0);
        else if (strcmp(stmt.GetValueText(1), "ParentRelECClassId") == 0)
            indexOfParentRelECClassId = stmt.GetValueInt(0);
        }

    ASSERT_EQ(3, indexOfParentId) << "ParentId must be at position 3 after ChildName column";
    ASSERT_EQ(4, indexOfParentRelECClassId) << "ParentRelECClassId must be next to ParentId column";
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, DiamondProblemInMixin)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("useecinstanceidasfk3.ecdb",
              SchemaItem(R"xml(
    <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
        <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA'/>
        <ECEntityClass typeName='MxBase' modifier='Abstract'>
            <ECCustomAttributes>"
                <IsMixin xmlns='CoreCustomAttributes.01.00'>
                    <AppliesToEntityClass>Base</AppliesToEntityClass>
                </IsMixin>
            </ECCustomAttributes>
            <ECProperty propertyName='MxBase_Prop' typeName='long' />
        </ECEntityClass>"
        <ECEntityClass typeName='MxA' modifier='Abstract'>
            <ECCustomAttributes>
                <IsMixin xmlns='CoreCustomAttributes.01.00'>
                    <AppliesToEntityClass>Base</AppliesToEntityClass>
                </IsMixin>
            </ECCustomAttributes>
            <BaseClass>MxBase</BaseClass>
            <ECProperty propertyName='MxA_Prop' typeName='long' />
        </ECEntityClass>
        <ECEntityClass typeName='MxB' modifier='Abstract'>
            <ECCustomAttributes>
                <IsMixin xmlns='CoreCustomAttributes.01.00'>
                    <AppliesToEntityClass>Base</AppliesToEntityClass>
                </IsMixin>
            </ECCustomAttributes>
            <BaseClass>MxA</BaseClass>
            <ECProperty propertyName='MxB_Prop' typeName='long' />
        </ECEntityClass>
        <ECEntityClass typeName="Base">
            <ECCustomAttributes>
                <ClassMap xmlns='ECDbMap.02.00'>
                    <MapStrategy>TablePerHierarchy</MapStrategy>
                </ClassMap>
                <ShareColumns xmlns='ECDbMap.02.00'>
                    <MaxSharedColumnsBeforeOverflow>10</MaxSharedColumnsBeforeOverflow>
                    <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>
                </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="Base_Prop" typeName="long" />
        </ECEntityClass>
        <ECEntityClass typeName="ChildA" >
            <BaseClass>Base</BaseClass>
            <ECProperty propertyName="Child_Prop" typeName="long" />
        </ECEntityClass>
        <ECEntityClass typeName="ChildB" >
            <BaseClass>ChildA</BaseClass>
            <BaseClass>MxA</BaseClass>
            <ECProperty propertyName="Child_Prop" typeName="long" />
        </ECEntityClass>
        <ECEntityClass typeName="ChildC" >
            <BaseClass>ChildB</BaseClass>
            <BaseClass>MxB</BaseClass>
            <ECProperty propertyName="Child_Prop" typeName="long" />
        </ECEntityClass>
    </ECSchema>)xml")));
    m_ecdb.Schemas().CreateClassViewsInDb();
    m_ecdb.SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         11/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, OverflowingStructColumns)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("OverflowPartiallyMapStructToOverFlow.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'> "
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Element' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <MaxSharedColumnsBeforeOverflow>8</MaxSharedColumnsBeforeOverflow>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='string' />"      //Code  
        "    </ECEntityClass>"
        "    <ECStructClass typeName='Matrix4x4' modifier='None'>"
        "        <ECProperty propertyName='M11' typeName='double'/>"        //sc1   [SharedColumn]
        "        <ECProperty propertyName='M12' typeName='double'/>"        //sc2   [SharedColumn]
        "        <ECProperty propertyName='M13' typeName='double'/>"        //sc3   [SharedColumn]
        "        <ECProperty propertyName='M14' typeName='double'/>"        //sc4   [SharedColumn]
        "        <ECProperty propertyName='M21' typeName='double'/>"        //sc5   [SharedColumn]
        "        <ECProperty propertyName='M22' typeName='double'/>"        //sc6   [SharedColumn]
        "        <ECProperty propertyName='M23' typeName='double'/>"        //sc7   [SharedColumn]
        "        <ECProperty propertyName='M24' typeName='double'/>"        //sc8   [Overflow]
        "        <ECProperty propertyName='M31' typeName='double'/>"        //sc9   [Overflow]
        "        <ECProperty propertyName='M32' typeName='double'/>"        //sc10  [Overflow]
        "        <ECProperty propertyName='M33' typeName='double'/>"        //sc11  [Overflow]
        "        <ECProperty propertyName='M34' typeName='double'/>"        //sc12  [Overflow]
        "        <ECProperty propertyName='M41' typeName='double'/>"        //sc13  [Overflow]
        "        <ECProperty propertyName='M42' typeName='double'/>"        //sc14  [Overflow]
        "        <ECProperty propertyName='M43' typeName='double'/>"        //sc15  [Overflow]
        "        <ECProperty propertyName='M44' typeName='double'/>"        //sc16  [Overflow]
        "    </ECStructClass>"
        "    <ECEntityClass typeName='TestElement' modifier='None'>"
        "        <BaseClass>Element</BaseClass>"
        "        <ECStructProperty propertyName='Mtx4x4' typeName='Matrix4x4'/>"
        "    </ECEntityClass>"
        "</ECSchema>")));

    Utf8CP codeA = "CodeA";
    Utf8CP codeB = "CodeB";
    std::vector<Utf8CP> mtx4x4Properties = {"M11","M12", "M13", "M14","M21","M22", "M23", "M24","M31","M32", "M33", "M34","M41","M42", "M43", "M44"};
    std::vector<double> mtx4x4ValuesA = {1.1, 1.2, 1.3, 1.4, 2.1, 2.2, 2.3, 2.4, 3.1, 3.2, 3.3, 3.4, 4.1, 4.2, 4.3, 4.4};
    std::vector<double> mtx4x4ValuesB = {1.1342, 1.2357, 1.3134, 1.4963, 2.1168, 2.2848, 2.6521, 2.4460, 3.1249, 3.2149, 3.3709, 3.4357, 4.1126, 4.2579, 4.3327, 4.4419};

    //INSERT a row was data
    {//===================================================================
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.TestElement (Code) VALUES (?)"));
    stmt.BindText(1, codeA, IECSqlBinder::MakeCopy::No);
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    m_ecdb.SaveChanges();
    }//===================================================================

     //UPDATE a row was data
    {//===================================================================
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE ts.TestElement SET Mtx4x4 = ? WHERE Code = ?"));
    IECSqlBinder& mtx = stmt.GetBinder(1);
    for (size_t i = 0; i < mtx4x4Properties.size(); i++)
        {
        ASSERT_EQ(ECSqlStatus::Success, mtx[mtx4x4Properties[i]].BindDouble(mtx4x4ValuesA[i]));
        }

    stmt.BindText(2, codeA, IECSqlBinder::MakeCopy::No);
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    m_ecdb.SaveChanges();
    }//===================================================================

     //INSERT a row was data
    {//===================================================================
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.TestElement (Code, Mtx4x4) VALUES (?,?)"));
    stmt.BindText(1, codeB, IECSqlBinder::MakeCopy::No);
    IECSqlBinder& mtx = stmt.GetBinder(2);
    for (size_t i = 0; i < mtx4x4Properties.size(); i++)
        {
        ASSERT_EQ(ECSqlStatus::Success, mtx[mtx4x4Properties[i]].BindDouble(mtx4x4ValuesB[i]));
        }

    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    m_ecdb.SaveChanges();
    }//===================================================================

     //Verify Row A
    {//===================================================================
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT Mtx4x4 FROM ts.TestElement WHERE Code = ?"));
    stmt.BindText(1, codeA, IECSqlBinder::MakeCopy::No);
    IECSqlValue const& mtx = stmt.GetValue(0);
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    for (IECSqlValue const& memberVal : mtx.GetStructIterable())
        {
        size_t memberIndex = 0;
        bool found = false;
        for (Utf8CP memberName : mtx4x4Properties)
            {
            if (memberVal.GetColumnInfo().GetProperty()->GetName().EqualsIAscii(memberName))
                {
                found = true;
                break;
                }

            memberIndex++;
            }

        ASSERT_TRUE(found);
        ASSERT_DOUBLE_EQ(mtx4x4ValuesA[memberIndex], memberVal.GetDouble());
        }
    }//===================================================================

     //Verify Row B
    {//===================================================================
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT Mtx4x4 FROM ts.TestElement WHERE Code = ?"));
    stmt.BindText(1, codeB, IECSqlBinder::MakeCopy::No);
    IECSqlValue const& mtx = stmt.GetValue(0);
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    for (IECSqlValue const& memberVal : mtx.GetStructIterable())
        {
        size_t memberIndex = 0;
        bool found = false;
        for (Utf8CP memberName : mtx4x4Properties)
            {
            if (memberVal.GetColumnInfo().GetProperty()->GetName().EqualsIAscii(memberName))
                {
                found = true;
                break;
                }

            memberIndex++;
            }

        ASSERT_TRUE(found);
        ASSERT_DOUBLE_EQ(mtx4x4ValuesB[memberIndex], memberVal.GetDouble());
        }
    }//===================================================================

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      05/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, ShareColumnsJoinedTableCACombinations)
    {
    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(R"xml(<ECSchema schemaName="TestSchema1" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Element" modifier="Abstract">
                        <ECCustomAttributes>
                            <ClassMap xlmns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <JoinedTablePerDirectSubclass xlmns="ECDbMap.02.00"/>
                            <ShareColumns xlmns="ECDbMap.02.00">
                                <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                       <ECProperty propertyName="Name" typeName="string" />
                       <ECProperty propertyName="Code" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="GeometricElement" modifier="None" >
                       <BaseClass>Element</BaseClass>
                       <ECProperty propertyName="GeomStream" typeName="Binary" />
                       <ECProperty propertyName="Type" typeName="int" />
                     </ECEntityClass>
                   </ECSchema>)xml"))) << "Column sharing starts before the joined table split";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(R"xml(<ECSchema schemaName="TestSchema2" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Element" modifier="Abstract">
                        <ECCustomAttributes>
                            <ClassMap xlmns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xlmns="ECDbMap.02.00">
                                <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                       <ECProperty propertyName="Name" typeName="string" />
                       <ECProperty propertyName="Code" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="GeometricElement" modifier="Abstract" >
                        <ECCustomAttributes>
                            <JoinedTablePerDirectSubclass xlmns="ECDbMap.02.00"/>
                        </ECCustomAttributes>
                       <BaseClass>Element</BaseClass>
                       <ECProperty propertyName="GeomStream" typeName="Binary" />
                       <ECProperty propertyName="Type" typeName="int" />
                     </ECEntityClass>
                    <ECEntityClass typeName="Geometric3dElement" modifier="None" >
                       <BaseClass>GeometricElement</BaseClass>
                       <ECProperty propertyName="Origin" typeName="Point3d" />
                     </ECEntityClass>
                   </ECSchema>)xml"))) << "Column sharing starts before the joined table split";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(R"xml(<ECSchema schemaName="TestSchema3" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Element" modifier="Abstract">
                        <ECCustomAttributes>
                            <ClassMap xlmns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xlmns="ECDbMap.02.00">
                                <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                       <ECProperty propertyName="Name" typeName="string" />
                       <ECProperty propertyName="Code" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="GeometricElement" modifier="Abstract" >
                        <ECCustomAttributes>
                            <JoinedTablePerDirectSubclass xlmns="ECDbMap.02.00"/>
                        </ECCustomAttributes>
                       <BaseClass>Element</BaseClass>
                       <ECProperty propertyName="GeomStream" typeName="Binary" />
                       <ECProperty propertyName="Type" typeName="int" />
                     </ECEntityClass>
                    <ECEntityClass typeName="Geometric3dElement" modifier="None" >
                       <BaseClass>GeometricElement</BaseClass>
                       <ECProperty propertyName="Origin" typeName="Point3d" />
                     </ECEntityClass>
                   </ECSchema>)xml"))) << "Column sharing starts in base class of class having the joined table CA";

    {
    ASSERT_EQ(SUCCESS, SetupECDb("OverflowAndJoinedTableCombinations10.ecdb",SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema10" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Element" >
                        <ECCustomAttributes>
                            <ClassMap xlmns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <JoinedTablePerDirectSubclass xlmns="ECDbMap.02.00"/>
                            <ShareColumns xlmns="ECDbMap.02.00">
                                <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                       <ECProperty propertyName="Name" typeName="string" />
                       <ECProperty propertyName="Code" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="GeometricElement" modifier="None" >
                       <BaseClass>Element</BaseClass>
                       <ECProperty propertyName="GeomStream" typeName="Binary" />
                       <ECStructProperty propertyName="Transform" typeName="Transform" />
                     </ECEntityClass>
                    <ECStructClass typeName="Transform" modifier="Sealed">
                       <ECProperty propertyName="Prop1" typeName="double" />
                       <ECProperty propertyName="Prop2" typeName="double" />
                       <ECProperty propertyName="Prop3" typeName="double" />
                       <ECProperty propertyName="Prop4" typeName="double" />
                       <ECProperty propertyName="Prop5" typeName="double" />
                       <ECProperty propertyName="Prop6" typeName="double" />
                     </ECStructClass>
                   </ECSchema>)xml")));

    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().CreateClassViewsInDb());
    AssertColumnNames(m_ecdb, "ts_Element", {"Id","ECClassId","Name","Code"});
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Element", "Name"), ColumnInfo("ts_Element", "Name"));
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Element", "Code"), ColumnInfo("ts_Element", "Code"));
    AssertColumnNames(m_ecdb, "ts_GeometricElement", {"ElementId","ECClassId","js1","js2","js3","js4","js5","js6","js7"});
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "GeometricElement", "GeomStream"), ColumnInfo("ts_GeometricElement", "js1"));
    ASSERT_PROPERTYMAPPING_MULTICOL(m_ecdb, PropertyAccessString("ts", "GeometricElement", "Transform"),
                                    ColumnInfo::List({
                                            {"Transform.Prop1","ts_GeometricElement","js2"},
                                            {"Transform.Prop2","ts_GeometricElement","js3"},
                                            {"Transform.Prop3","ts_GeometricElement","js4"},
                                            {"Transform.Prop4","ts_GeometricElement","js5"},
                                            {"Transform.Prop5","ts_GeometricElement","js6"},
                                            {"Transform.Prop6","ts_GeometricElement","js7"}}));

    ASSERT_FALSE(m_ecdb.TableExists("ts_GeometricElement_Overflow"));
    ASSERT_FALSE(m_ecdb.TableExists("ts_Element_Overflow"));
    }

    {
    //Struct that doesn't fit in joined table comes before primitive prop 
    //->prim prop expected in joined table
    //->struct expected in overflow table
    ASSERT_EQ(SUCCESS, SetupECDb("OverflowAndJoinedTableCombinations11.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema11" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Element" >
                        <ECCustomAttributes>
                            <ClassMap xlmns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <JoinedTablePerDirectSubclass xlmns="ECDbMap.02.00"/>
                            <ShareColumns xlmns="ECDbMap.02.00">
                                <MaxSharedColumnsBeforeOverflow>3</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                       <ECProperty propertyName="Name" typeName="string" />
                       <ECProperty propertyName="Code" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="GeometricElement" modifier="None" >
                       <BaseClass>Element</BaseClass>
                       <ECProperty propertyName="GeomStream" typeName="Binary" />
                       <ECStructProperty propertyName="Transform" typeName="Transform" />
                       <ECProperty propertyName="Type" typeName="string" />
                     </ECEntityClass>
                    <ECStructClass typeName="Transform" modifier="Sealed">
                       <ECProperty propertyName="Prop1" typeName="double" />
                       <ECProperty propertyName="Prop2" typeName="double" />
                       <ECProperty propertyName="Prop3" typeName="double" />
                       <ECProperty propertyName="Prop4" typeName="double" />
                       <ECProperty propertyName="Prop5" typeName="double" />
                       <ECProperty propertyName="Prop6" typeName="double" />
                     </ECStructClass>
                   </ECSchema>)xml")));
    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().CreateClassViewsInDb());

    AssertColumnNames(m_ecdb, "ts_Element", {"Id","ECClassId","Name","Code"});
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Element", "Name"), ColumnInfo("ts_Element","Name"));
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Element", "Code"), ColumnInfo("ts_Element","Code"));
    AssertColumnNames(m_ecdb, "ts_GeometricElement", {"ElementId","ECClassId","js1","js2"});
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "GeometricElement", "GeomStream"), ColumnInfo("ts_GeometricElement","js1"));
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "GeometricElement", "Type"), ColumnInfo("ts_GeometricElement","js2"));
    AssertColumnNames(m_ecdb, "ts_GeometricElement_Overflow", {"ElementId","ECClassId","os1","os2","os3","os4","os5","os6"});
    ASSERT_PROPERTYMAPPING_MULTICOL(m_ecdb, PropertyAccessString("ts", "GeometricElement", "Transform"),
                           ColumnInfo::List({
                            {"Transform.Prop1","ts_GeometricElement_Overflow","os1"},
                            {"Transform.Prop2","ts_GeometricElement_Overflow","os2"},
                            {"Transform.Prop3","ts_GeometricElement_Overflow","os3"},
                            {"Transform.Prop4","ts_GeometricElement_Overflow","os4"},
                            {"Transform.Prop5","ts_GeometricElement_Overflow","os5"},
                            {"Transform.Prop6","ts_GeometricElement_Overflow","os6"}}));
    }

    {
    //Struct that doesn't fit in joined table comes before primitive prop 
    //->prim prop expected in joined table
    //->struct expected in overflow table
    ASSERT_EQ(SUCCESS, SetupECDb("OverflowAndJoinedTableCombinations11.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema11" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Element" >
                        <ECCustomAttributes>
                            <ClassMap xlmns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <JoinedTablePerDirectSubclass xlmns="ECDbMap.02.00"/>
                            <ShareColumns xlmns="ECDbMap.02.00">
                                <MaxSharedColumnsBeforeOverflow>3</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                       <ECProperty propertyName="Name" typeName="string" />
                       <ECProperty propertyName="Code" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="GeometricElement" modifier="None" >
                       <BaseClass>Element</BaseClass>
                       <ECProperty propertyName="GeomStream" typeName="Binary" />
                       <ECStructProperty propertyName="Transform" typeName="Transform" />
                       <ECProperty propertyName="Origin" typeName="Point2d" />
                     </ECEntityClass>
                    <ECStructClass typeName="Transform" modifier="Sealed">
                       <ECProperty propertyName="Prop1" typeName="double" />
                       <ECProperty propertyName="Prop2" typeName="double" />
                       <ECProperty propertyName="Prop3" typeName="double" />
                       <ECProperty propertyName="Prop4" typeName="double" />
                       <ECProperty propertyName="Prop5" typeName="double" />
                       <ECProperty propertyName="Prop6" typeName="double" />
                     </ECStructClass>
                   </ECSchema>)xml")));
    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().CreateClassViewsInDb());

    AssertColumnNames(m_ecdb, "ts_Element", {"Id","ECClassId","Name","Code"});
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Element", "Name"), ColumnInfo("ts_Element","Name"));
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Element", "Code"), ColumnInfo("ts_Element","Code"));
    AssertColumnNames(m_ecdb, "ts_GeometricElement", {"ElementId","ECClassId","js1","js2","js3"});
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "GeometricElement", "GeomStream"), ColumnInfo("ts_GeometricElement","js1"));
    ASSERT_PROPERTYMAPPING_MULTICOL(m_ecdb, PropertyAccessString("ts", "GeometricElement", "Origin"),
                                    ColumnInfo::List({{"Origin.X","ts_GeometricElement","js2"},
                                                     {"Origin.Y","ts_GeometricElement","js3"}}));
    
    AssertColumnNames(m_ecdb, "ts_GeometricElement_Overflow", {"ElementId","ECClassId","os1","os2","os3","os4","os5","os6"});
    ASSERT_PROPERTYMAPPING_MULTICOL(m_ecdb, PropertyAccessString("ts", "GeometricElement", "Transform"),
                                ColumnInfo::List({
                                        {"Transform.Prop1","ts_GeometricElement_Overflow","os1"},
                                        {"Transform.Prop2","ts_GeometricElement_Overflow","os2"},
                                        {"Transform.Prop3","ts_GeometricElement_Overflow","os3"},
                                        {"Transform.Prop4","ts_GeometricElement_Overflow","os4"},
                                        {"Transform.Prop5","ts_GeometricElement_Overflow","os5"},
                                        {"Transform.Prop6","ts_GeometricElement_Overflow","os6"}}));
    }

    {
    ASSERT_EQ(SUCCESS, SetupECDb("OverflowAndJoinedTableCombinations12.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema12" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Element" >
                        <ECCustomAttributes>
                            <ClassMap xlmns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <JoinedTablePerDirectSubclass xlmns="ECDbMap.02.00"/>
                            <ShareColumns xlmns="ECDbMap.02.00">
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                       <ECProperty propertyName="Name" typeName="string" />
                     </ECEntityClass>
                   </ECSchema>)xml")));
    AssertColumnNames(m_ecdb, "ts_Element", {"Id","ECClassId","Name"});
    ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Element", "Name"), ColumnInfo("ts_Element","Name"));
    ASSERT_FALSE(m_ecdb.TableExists("ts_Element_Overflow"));
    }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      05/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, DefaultMaxSharedColumnCountBeforeOverflow)
    {
            {
            ASSERT_EQ(SUCCESS, SetupECDb("DefaultMaxSharedColumnCountBeforeOverflow1.ecdb", SchemaItem(
                R"xml(<ECSchema schemaName="TestSchema1" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                <ECEntityClass typeName="Element" modifier="Abstract">
                    <ECCustomAttributes>
                        <ClassMap xlmns="ECDbMap.02.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                        <ShareColumns xlmns="ECDbMap.02.00">
                            <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>
                        </ShareColumns>
                    </ECCustomAttributes>
                    <ECStructProperty propertyName="Prop1" typeName="TenColumnStruct" />
                    <ECStructProperty propertyName="Prop2" typeName="TenColumnStruct" />
                    <ECStructProperty propertyName="Prop3" typeName="TenColumnStruct" />
                    <ECStructProperty propertyName="Prop4" typeName="TenColumnStruct" />
                    <ECStructProperty propertyName="Prop5" typeName="TenColumnStruct" />
                    <ECStructProperty propertyName="Prop6" typeName="TenColumnStruct" />
                    <ECProperty propertyName="Prop7" typeName="string" />
                    <ECProperty propertyName="ExpectedToOverflow" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="SubElement">
                    <BaseClass>Element</BaseClass>
                    <ECProperty propertyName="ExpectedToBeInOverflowAsWell" typeName="double" />
                </ECEntityClass>
                <ECStructClass typeName="TenColumnStruct" modifier="Sealed">
                    <ECProperty propertyName="P1" typeName="Point2d" />
                    <ECProperty propertyName="P2" typeName="Point3d" />
                    <ECProperty propertyName="P3" typeName="Point2d" />
                    <ECProperty propertyName="P4" typeName="Point3d" />
                </ECStructClass>
                </ECSchema>)xml")));
            AssertColumnCount(m_ecdb, {{"ts_Element", 63}, {"ts_Element_Overflow", 4}});
            AssertColumnNames(m_ecdb, "ts_Element_Overflow", {"ElementId","ECClassId","os1", "os2"});
            ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Element", "Prop7"), ColumnInfo("ts_Element", "ps61"));
            ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "Element", "ExpectedToOverflow"), ColumnInfo("ts_Element_Overflow", "os1"));
            ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "SubElement", "ExpectedToOverflow"), ColumnInfo("ts_Element_Overflow", "os1"));
            ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "SubElement", "ExpectedToBeInOverflowAsWell"), ColumnInfo("ts_Element_Overflow", "os2"));
            }

        {
            ASSERT_EQ(SUCCESS, SetupECDb("DefaultMaxSharedColumnCountBeforeOverflow2.ecdb", SchemaItem(
            R"xml(<ECSchema schemaName="TestSchema2" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                <ECEntityClass typeName="Element" >
                    <ECCustomAttributes>
                        <ClassMap xlmns="ECDbMap.02.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                        <ShareColumns xlmns="ECDbMap.02.00">
                            <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                        </ShareColumns>
                    </ECCustomAttributes>
                    <ECProperty propertyName="BaseProp" typeName="int" />
                </ECEntityClass>
                <ECEntityClass typeName="SubElement" >
                    <BaseClass>Element</BaseClass>
                    <ECStructProperty propertyName="Prop1" typeName="TenColumnStruct" />
                    <ECStructProperty propertyName="Prop2" typeName="TenColumnStruct" />
                    <ECStructProperty propertyName="Prop3" typeName="TenColumnStruct" />
                    <ECStructProperty propertyName="Prop4" typeName="TenColumnStruct" />
                    <ECStructProperty propertyName="Prop5" typeName="TenColumnStruct" />
                    <ECStructProperty propertyName="Prop6" typeName="TenColumnStruct" />
                    <ECProperty propertyName="ExpectedToOverflow" typeName="string" />
                </ECEntityClass>
                <ECStructClass typeName="TenColumnStruct" modifier="Sealed">
                    <ECProperty propertyName="P1" typeName="Point2d" />
                    <ECProperty propertyName="P2" typeName="Point3d" />
                    <ECProperty propertyName="P3" typeName="Point2d" />
                    <ECProperty propertyName="P4" typeName="Point3d" />
                 </ECStructClass>
                </ECSchema>)xml")));
        AssertColumnCount(m_ecdb, {{"ts_Element", 63}, {"ts_Element_Overflow", 3}});
        AssertColumnNames(m_ecdb, "ts_Element_Overflow", {"ElementId","ECClassId","os1"});
        ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "SubElement", "BaseProp"), ColumnInfo("ts_Element","BaseProp"));
        ASSERT_PROPERTYMAPPING_MULTICOL(m_ecdb, PropertyAccessString("ts", "SubElement", "Prop1.P1"), ColumnInfo::List({{"Prop1.P1.X", "ts_Element","ps1"},
                                                                                           {"Prop1.P1.Y", "ts_Element","ps2"}}));
        ASSERT_PROPERTYMAPPING_MULTICOL(m_ecdb, PropertyAccessString("ts", "SubElement", "Prop3.P2"), ColumnInfo::List({{"Prop3.P2.X","ts_Element","ps23"},
                                                                    {"Prop3.P2.Y", "ts_Element","ps24"},
                                                                    {"Prop3.P2.Z", "ts_Element","ps25"}}));

        ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "SubElement", "ExpectedToOverflow"), ColumnInfo("ts_Element_Overflow","os1"));
        }

        {
        ASSERT_EQ(SUCCESS, SetupECDb("DefaultMaxSharedColumnCountBeforeOverflow3.ecdb", SchemaItem(
            R"xml(<ECSchema schemaName="TestSchema3" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                <ECEntityClass typeName="Element" >
                    <ECCustomAttributes>
                        <ClassMap xlmns="ECDbMap.02.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                        <JoinedTablePerDirectSubclass xlmns="ECDbMap.02.00"/>
                    </ECCustomAttributes>
                    <ECProperty propertyName="BaseProp1" typeName="int" />
                    <ECStructProperty propertyName="BaseProp2" typeName="TenColumnStruct" />
                </ECEntityClass>
                <ECEntityClass typeName="SubElement" >
                    <ECCustomAttributes>
                        <ShareColumns xlmns="ECDbMap.02.00">
                            <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>
                        </ShareColumns>
                    </ECCustomAttributes>
                    <BaseClass>Element</BaseClass>
                    <ECStructProperty propertyName="Prop1" typeName="TenColumnStruct" />
                    <ECStructProperty propertyName="Prop2" typeName="TenColumnStruct" />
                    <ECStructProperty propertyName="Prop3" typeName="TenColumnStruct" />
                    <ECStructProperty propertyName="Prop4" typeName="TenColumnStruct" />
                    <ECStructProperty propertyName="Prop5" typeName="TenColumnStruct" />
                    <ECStructProperty propertyName="Prop6" typeName="TenColumnStruct" />
                    <ECProperty propertyName="Prop7" typeName="string" />
                    <ECProperty propertyName="ExpectedToOverflow" typeName="string" />
                </ECEntityClass>
                <ECStructClass typeName="TenColumnStruct" modifier="Sealed">
                    <ECProperty propertyName="P1" typeName="Point2d" />
                    <ECProperty propertyName="P2" typeName="Point3d" />
                    <ECProperty propertyName="P3" typeName="Point2d" />
                    <ECProperty propertyName="P4" typeName="Point3d" />
                </ECStructClass>
                </ECSchema>)xml")));
        AssertColumnCount(m_ecdb, {{"ts_Element", 13}, {"ts_SubElement", 63}, {"ts_SubElement_Overflow", 3}});
        AssertColumnNames(m_ecdb, "ts_SubElement_Overflow", {"ElementId","ECClassId","os1"});
        ASSERT_PROPERTYMAPPING(m_ecdb, PropertyAccessString("ts", "SubElement", "ExpectedToOverflow"), ColumnInfo("ts_SubElement_Overflow","os1"));
        }
    }


//--------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         05/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, OrderOfPropertyIsPreservedInTableColumns)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("propertyOrderTest.ecdb", SchemaItem("<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                                                   "<ECSchema schemaName=\"OrderSchema\" alias=\"os\" version=\"1.0\" xmlns = \"http://www.bentley.com/schemas/Bentley.ECXML.3.1\">"
                                                   "  <ECStructClass typeName=\"OrderedStruct\">"
                                                   "   <ECProperty propertyName=\"a\" typeName=\"string\"/>"
                                                   "     <ECProperty propertyName=\"g\" typeName=\"int\"/>"
                                                   "     <ECProperty propertyName=\"c\" typeName=\"dateTime\"/>"
                                                   "   <ECProperty propertyName=\"z\" typeName=\"point3d\"/>"
                                                   "     <ECProperty propertyName=\"y\" typeName=\"point2d\"/>"
                                                   "     <ECProperty propertyName=\"t\" typeName=\"boolean\"/>"
                                                   "   <ECProperty propertyName=\"u\" typeName=\"double\"/>"
                                                   "     <ECProperty propertyName=\"k\" typeName=\"string\"/>"
                                                   "     <ECProperty propertyName=\"r\" typeName=\"string\"/>"
                                                   "  </ECStructClass>"
                                                   "  <ECEntityClass typeName=\"PropertyOrderTest\" >"
                                                   "   <ECProperty propertyName=\"x\" typeName=\"string\"/>"
                                                   "     <ECProperty propertyName=\"h\" typeName=\"int\"/>"
                                                   "     <ECProperty propertyName=\"i\" typeName=\"dateTime\"/>"
                                                   "   <ECProperty propertyName=\"d\" typeName=\"point3d\"/>"
                                                   "     <ECProperty propertyName=\"u\" typeName=\"point2d\"/>"
                                                   "     <ECProperty propertyName=\"f\" typeName=\"boolean\"/>"
                                                   "     <ECStructArrayProperty propertyName=\"sarray\" typeName=\"OrderedStruct\"/>"
                                                   "   <ECProperty propertyName=\"e\" typeName=\"double\"/>"
                                                   "     <ECProperty propertyName=\"p\" typeName=\"string\"/>"
                                                   "     <ECStructProperty propertyName=\"o\" typeName=\"OrderedStruct\"/>"
                                                   "     <ECProperty propertyName=\"z\" typeName=\"long\"/>"
                                                   "  </ECEntityClass>"
                                                   "</ECSchema>")));

    Statement statement;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, statement.Prepare(m_ecdb, "PRAGMA table_info('os_PropertyOrderTest')"));
    Utf8String order_PropertyOrderTest;
    while (statement.Step() == BE_SQLITE_ROW)
        {
        order_PropertyOrderTest.append(statement.GetValueText(1)).append(" ");
        }

    ASSERT_STREQ("Id x h i d_X d_Y d_Z u_X u_Y f sarray e p o_a o_g o_c o_z_X o_z_Y o_z_Z o_y_X o_y_Y o_t o_u o_k o_r z ", order_PropertyOrderTest.c_str());
    ASSERT_FALSE(m_ecdb.TableExists("os_OrderedStruct"));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         05/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, LoadECSchemas)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("StartupCompany.ecdb", BeFileName(L"StartupCompany.02.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::Readonly)));

    std::vector<Utf8CP> expectedSchemas;
    expectedSchemas.push_back("Bentley_Standard_CustomAttributes");
    expectedSchemas.push_back("CoreCustomAttributes");
    expectedSchemas.push_back("ECDbFileInfo");
    expectedSchemas.push_back("ECDbMap");
    expectedSchemas.push_back("ECDbMeta");
    expectedSchemas.push_back("ECDbSystem");
    expectedSchemas.push_back("EditorCustomAttributes");
    expectedSchemas.push_back("StartupCompany");

    // Validate the expected ECSchemas in the project
    Statement stmt;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT Name FROM ec_Schema ORDER BY Name"));
    int i = 0;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        ASSERT_STREQ(expectedSchemas[i], stmt.GetValueText(0));
        i++;
        }

    ASSERT_EQ(expectedSchemas.size(), i);
    }


//--------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         05/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, ImportECSchemaWithSameVersionAndSameContentTwice)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("StartupCompany.ecdb", BeFileName(L"StartupCompany.02.00.ecschema.xml")));
    ECSchemaReadContextPtr schemaContext = nullptr;
    ECSchemaPtr ecSchema = ReadECSchemaFromDisk(schemaContext, BeFileName(L"StartupCompany.02.00.ecschema.xml"));
    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().ImportSchemas(schemaContext->GetCache().GetSchemas()));
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   04/13
//+---------------+---------------+---------------+---------------+---------------+------
void AssertImportedSchema(ECDbCR ecdb, Utf8CP expectedSchemaName, Utf8CP expectedClassName, Utf8CP expectedPropertyName)
    {
    CachedStatementPtr findClassStmt = nullptr;
    ecdb.GetCachedStatement(findClassStmt, "SELECT NULL FROM ec_Class c, ec_Schema s WHERE c.SchemaId = s.Id AND s.Name = ? AND c.Name = ? LIMIT 1");
    findClassStmt->BindText(1, expectedSchemaName, Statement::MakeCopy::No);
    findClassStmt->BindText(2, expectedClassName, Statement::MakeCopy::No);
    EXPECT_EQ(BE_SQLITE_ROW, findClassStmt->Step()) << "ECClass " << expectedClassName << " of ECSchema " << expectedSchemaName << " is expected to be found in ec_Class table.";

    if (expectedPropertyName != nullptr)
        {
        CachedStatementPtr findPropertyStmt = nullptr;
        ecdb.GetCachedStatement(findPropertyStmt, "SELECT NULL FROM ec_Property p, ec_Class c, ec_Schema s WHERE p.ClassId = c.Id AND c.SchemaId = s.Id AND s.Name = ? AND c.Name = ? AND p.Name = ? LIMIT 1");
        findPropertyStmt->BindText(1, expectedSchemaName, Statement::MakeCopy::No);
        findPropertyStmt->BindText(2, expectedClassName, Statement::MakeCopy::No);
        findPropertyStmt->BindText(3, expectedPropertyName, Statement::MakeCopy::No);
        EXPECT_EQ(BE_SQLITE_ROW, findPropertyStmt->Step()) << "ECProperty " << expectedPropertyName << " in ECClass " << expectedClassName << " of ECSchema " << expectedSchemaName << " is expected to be found in ec_Property table.";;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   05/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, SchemaImportWithExistingTables)
    {
        {
        ASSERT_EQ(BE_SQLITE_OK, SetupECDb("schemaimport_existingtables.ecdb"));

        //create ec table bypassing ECDb API, but don't add it to the ec_ profile tables
        ASSERT_EQ(BE_SQLITE_OK, m_ecdb.ExecuteSql("CREATE TABLE t_Foo(Name TEXT)"));

        ASSERT_EQ(ERROR, ImportSchema(m_ecdb, SchemaItem(R"xml(<ECSchema schemaName="test" alias="t" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECEntityClass typeName="Foo" >
                    <ECProperty propertyName="Name" typeName="string"/>
                </ECEntityClass>
                <ECEntityClass typeName="Goo" >
                    <ECProperty propertyName="Price" typeName="double"/>
                    <ECNavigationProperty propertyName="Foo" relationshipName="FooHasGoo" direction="Backward"/>
                </ECEntityClass>
                <ECRelationshipClass typeName="FooHasGoo" modifier="Sealed" strength="Referencing" >
                    <Source multiplicity="(0..1)" polymorphic="true" roleLabel="references">
                        <Class class="Foo"/>
                    </Source>
                    <Target multiplicity="(0..*)" polymorphic="true" roleLabel="is referenced by">
                        <Class class="Goo"/>
                    </Target>
                    </ECRelationshipClass>
                </ECSchema>)xml")));
        m_ecdb.AbandonChanges();
        }

        {
        ASSERT_EQ(BE_SQLITE_OK, SetupECDb("schemaimport_existingtables.ecdb"));
        ASSERT_EQ(BE_SQLITE_OK, m_ecdb.ExecuteSql("CREATE TABLE t_Foo(Id INTEGER PRIMARY KEY, Name TEXT)"));

        ASSERT_EQ(ERROR, ImportSchema(m_ecdb, SchemaItem(R"xml(<ECSchema schemaName="test" alias="t" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECEntityClass typeName="Foo" >
                    <ECProperty propertyName="Name" typeName="string"/>
                </ECEntityClass>
                <ECEntityClass typeName="Goo" >
                    <ECProperty propertyName="Price" typeName="double"/>
                    <ECNavigationProperty propertyName="Foo" relationshipName="FooHasGoo" direction="Backward"/>
                </ECEntityClass>
                <ECRelationshipClass typeName="FooHasGoo" modifier="Sealed" strength="Referencing" >
                    <Source multiplicity="(0..1)" polymorphic="true" roleLabel="references">
                        <Class class="Foo"/>
                    </Source>
                    <Target multiplicity="(0..*)" polymorphic="true" roleLabel="is referenced by">
                        <Class class="Goo"/>
                    </Target>
                    </ECRelationshipClass>
                </ECSchema>)xml")));
        }

        {
        ASSERT_EQ(BE_SQLITE_OK, SetupECDb("schemaimport_existingtables.ecdb"));
        ASSERT_EQ(BE_SQLITE_OK, m_ecdb.ExecuteSql("CREATE TABLE t_Goo (Id INTEGER PRIMARY KEY, Price REAL, FooId INTEGER)"));

        ASSERT_EQ(ERROR, ImportSchema(m_ecdb, SchemaItem(R"xml(<ECSchema schemaName="test" alias="t" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECEntityClass typeName="Foo" >
                       <ECProperty propertyName="Name" typeName="string"/>
                    </ECEntityClass>
                    <ECEntityClass typeName="Goo" >
                       <ECProperty propertyName="Price" typeName="double"/>
                       <ECNavigationProperty propertyName="Foo" relationshipName="FooHasGoo" direction="Backward"/>
                    </ECEntityClass>
                    <ECRelationshipClass typeName="FooHasGoo" modifier="Sealed" strength="Referencing" >
                       <Source multiplicity="(0..1)" polymorphic="true" roleLabel="references">
                            <Class class="Foo"/>
                       </Source>
                       <Target multiplicity="(0..*)" polymorphic="true" roleLabel="is referenced by">
                            <Class class="Goo"/>
                       </Target>
                     </ECRelationshipClass>
                    <ECRelationshipClass typeName="FooHasGooLinkTable" modifier="Sealed" strength="Referencing" >
                       <Source multiplicity="(0..*)" polymorphic="true" roleLabel="references">
                            <Class class="Goo"/>
                       </Source>
                       <Target multiplicity="(0..*)" polymorphic="true" roleLabel="is referenced by">
                            <Class class="Goo"/>
                       </Target>
                     </ECRelationshipClass>
                   </ECSchema>)xml")));
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/12
//+---------------+---------------+---------------+---------------+---------------+------
void CreateCustomAttributeTestSchema(ECSchemaPtr& testSchema, ECSchemaCachePtr& testSchemaCache)
    {
    ECSchemaPtr schema = nullptr;
    ECObjectsStatus stat = ECSchema::CreateSchema(schema, "foo", "f", 1, 0, 0);
    ASSERT_EQ(ECObjectsStatus::Success, stat) << "Creating test schema failed";

    ECEntityClassP domainClass = nullptr;
    stat = schema->CreateEntityClass(domainClass, "domain1");
    ASSERT_EQ(ECObjectsStatus::Success, stat) << "Creating domain class 1 in schema failed";

    ECEntityClassP domainClass2 = nullptr;
    stat = schema->CreateEntityClass(domainClass2, "domain2");
    ASSERT_EQ(ECObjectsStatus::Success, stat) << "Creating domain class 2 in schema failed";

    ECCustomAttributeClassP caClass = nullptr;
    stat = schema->CreateCustomAttributeClass(caClass, "MyCA");
    ASSERT_EQ(ECObjectsStatus::Success, stat) << "Creating CA class in schema failed";

    PrimitiveECPropertyP dateProp = nullptr;
    caClass->CreatePrimitiveProperty(dateProp, "dateprop", PRIMITIVETYPE_DateTime);

    PrimitiveECPropertyP stringProp = nullptr;
    caClass->CreatePrimitiveProperty(stringProp, "stringprop", PRIMITIVETYPE_String);

    PrimitiveECPropertyP doubleProp = nullptr;
    caClass->CreatePrimitiveProperty(doubleProp, "doubleprop", PRIMITIVETYPE_Double);

    PrimitiveECPropertyP pointProp = nullptr;
    caClass->CreatePrimitiveProperty(pointProp, "pointprop", PRIMITIVETYPE_Point3d);

    ECSchemaCachePtr cache = ECSchemaCache::Create();
    cache->AddSchema(*schema);

    testSchema = schema;
    testSchemaCache = cache;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/12
//+---------------+---------------+---------------+---------------+---------------+------
void AssignCustomAttribute(IECInstancePtr& caInstance, ECSchemaPtr schema, Utf8CP containerClassName, Utf8CP caClassName, Utf8CP instanceId, bmap<Utf8String, ECValue> const& caPropValues)
    {
    ECClassP caClass = schema->GetClassP(caClassName);
    IECInstancePtr ca = caClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ASSERT_TRUE(ca.IsValid());

    ECObjectsStatus stat;
    if (instanceId != nullptr)
        {
        stat = ca->SetInstanceId(instanceId);
        ASSERT_EQ(ECObjectsStatus::Success, stat) << "Setting instance id in CA instance failed";
        }

    typedef bpair<Utf8String, ECValue> T_PropValuePair;

    for (T_PropValuePair const& pair : caPropValues)
        {
        stat = ca->SetValue(pair.first.c_str(), pair.second);
        ASSERT_EQ(ECObjectsStatus::Success, stat) << "Assigning property value to CA instance failed";
        }

    ECClassP containerClass = schema->GetClassP(containerClassName);
    stat = containerClass->SetCustomAttribute(*ca);
    ASSERT_EQ(ECObjectsStatus::Success, stat) << "Assigning CA instance to container class failed";

    caInstance = ca;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/12
//+---------------+---------------+---------------+---------------+---------------+------
IECInstancePtr CreateAndAssignRandomCAInstance(ECSchemaPtr testSchema)
    {
    //assign CA with instance id and all props populated
    bmap<Utf8String, ECValue> propValueMap;
    propValueMap[Utf8String("dateprop")] = ECValue(DateTime(DateTime::Kind::Unspecified, 1971, 4, 30, 21, 9, 0, 0));
    propValueMap[Utf8String("stringprop")] = ECValue("hello world", true);
    propValueMap[Utf8String("doubleprop")] = ECValue(3.14);
    DPoint3d point;
    point.x = 1.0;
    point.y = -2.0;
    point.z = 3.0;
    propValueMap[Utf8String("pointprop")] = ECValue(point);

    IECInstancePtr ca = nullptr;
    AssignCustomAttribute(ca, testSchema, "domain1", "MyCA", "bla bla", propValueMap);

    return ca;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, ReadCustomAttributesTest)
    {
    Utf8CP const CAClassName = "MyCA";

    ECSchemaPtr testSchema = nullptr;
    ECSchemaCachePtr testSchemaCache = nullptr;
    CreateCustomAttributeTestSchema(testSchema, testSchemaCache);

    //assign CA with instance id and all props populated
    IECInstancePtr expectedCAInstanceWithInstanceId = CreateAndAssignRandomCAInstance(testSchema);

    //assign CA without instance id and only a few props populated
    bmap<Utf8String, ECValue> propValueMap;
    propValueMap[Utf8String("doubleprop")] = ECValue(3.14);
    IECInstancePtr expectedCAInstanceWithoutInstanceId = nullptr;
    AssignCustomAttribute(expectedCAInstanceWithoutInstanceId, testSchema, "domain2", CAClassName, nullptr, propValueMap);

    //create test db and close it again
    Utf8String dbPath;
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("customattributestest.ecdb"));
    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().ImportSchemas(testSchemaCache->GetSchemas())) << "Could not import test schema into ECDb file";
    m_ecdb.SaveChanges();
    dbPath = m_ecdb.GetDbFileName();
    m_ecdb.CloseDb();
    }

    //reopen test ECDb file (to make sure that the stored schema is read correctly)
    DbResult stat = m_ecdb.OpenBeSQLiteDb(dbPath.c_str(), Db::OpenParams(Db::OpenMode::Readonly));
    ASSERT_EQ(BE_SQLITE_OK, stat) << "Could not open test ECDb file";

    ECSchemaCP readSchema = m_ecdb.Schemas().GetSchema(testSchema->GetName().c_str());
    ASSERT_TRUE(readSchema != nullptr) << "Could not read test schema from reopened ECDb file.";
    //*** assert custom attribute instance with instance id
    ECClassCP domainClass1 = readSchema->GetClassCP("domain1");
    ASSERT_TRUE(domainClass1 != nullptr) << "Could not retrieve domain class 1 from re-read test schema.";
    IECInstancePtr actualCAInstanceWithInstanceId = domainClass1->GetCustomAttribute(CAClassName);
    ASSERT_TRUE(actualCAInstanceWithInstanceId.IsValid()) << "Test custom attribute instance not found on domain class 1.";

    //compare instance ids
    ASSERT_STREQ(expectedCAInstanceWithInstanceId->GetInstanceId().c_str(), actualCAInstanceWithInstanceId->GetInstanceId().c_str()) << "Instance Ids of retrieved custom attribute instance doesn't match.";

    //compare rest of instance
    bool equal = ECDbTestUtility::CompareECInstances(*expectedCAInstanceWithInstanceId, *actualCAInstanceWithInstanceId);
    ASSERT_TRUE(equal) << "Read custom attribute instance with instance id differs from expected.";

    //*** assert custom attribute instance without instance id
    ECClassCP domainClass2 = readSchema->GetClassCP("domain2");
    ASSERT_TRUE(domainClass2 != nullptr) << "Could not retrieve domain class 2 from re-read test schema.";
    IECInstancePtr actualCAInstanceWithoutInstanceId = domainClass2->GetCustomAttribute(CAClassName);
    ASSERT_TRUE(actualCAInstanceWithoutInstanceId.IsValid()) << "Test custom attribute instance not found on domain class 2.";

    //compare instance ids
    ASSERT_STREQ(expectedCAInstanceWithoutInstanceId->GetInstanceId().c_str(), actualCAInstanceWithoutInstanceId->GetInstanceId().c_str()) << "Instance Ids of retrieved custom attribute instance doesn't match.";
    ASSERT_STREQ("", actualCAInstanceWithoutInstanceId->GetInstanceId().c_str()) << "Instance Ids of retrieved custom attribute instance is expected to be empty";

    //compare rest of instance
    equal = ECDbTestUtility::CompareECInstances(*expectedCAInstanceWithoutInstanceId, *actualCAInstanceWithoutInstanceId);
    ASSERT_TRUE(equal) << "Read custom attribute instance without instance id differs from expected.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, CheckCustomAttributesXmlFormatTest)
    {
    ECSchemaPtr testSchema = nullptr;
    ECSchemaCachePtr testSchemaCache = nullptr;
    CreateCustomAttributeTestSchema(testSchema, testSchemaCache);

    //assign CA with instance id
    CreateAndAssignRandomCAInstance(testSchema);

    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("customattributestest.ecdb"));
    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().ImportSchemas(testSchemaCache->GetSchemas()));
   //now retrieve the persisted CA XML from ECDb directly
    Statement stmt;
    DbResult stat = stmt.Prepare(m_ecdb, "SELECT Instance from ec_CustomAttribute ca, ec_Class c where ca.ClassId = c.Id AND c.Name = 'MyCA'");
    ASSERT_EQ(BE_SQLITE_OK, stat) << "Preparing the SQL statement to fetch the persisted CA XML string failed.";

    int rowCount = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        rowCount++;
        Utf8CP caXml = stmt.GetValueText(0);
        ASSERT_TRUE(caXml != nullptr) << "Retrieved custom attribute XML string is expected to be not null.";
        Utf8String caXmlString(caXml);
        EXPECT_LT(0, (int) caXmlString.length()) << "Retrieved custom attribute XML string is not expected to be empty.";

        //It is expected that the XML string doesn't contain the XML descriptor.
        EXPECT_TRUE(!caXmlString.ContainsI("<?xml")) << "The custom attribute XML string is expected to not contain the XML description tag.";

        //It is expected that the XML string does contain the instance id if the original CA was assigned one
        EXPECT_TRUE(caXmlString.ContainsI("instanceId=")) << "The custom attribute XML string is expected to contain the instance id for the given custom attribute instance.";
        }

    ASSERT_EQ(1, rowCount) << "Only one test custom attribute instance had been created.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                   11/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, ImportSupplementalSchemas)
    {
    Utf8CP dbFileName = "supplementalschematest.ecdb";
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb(dbFileName));

    ECSchemaPtr ecSchema = nullptr;
    ECSchemaReadContextPtr schemaContext = nullptr;

    ECSchemaPtr startup = ReadECSchemaFromDisk(schemaContext, BeFileName(L"StartupCompany.02.00.ecschema.xml"));
    ECSchemaPtr supple = ReadECSchemaFromDisk(schemaContext, BeFileName(L"StartupCompany_Supplemental_ECDbTest.01.00.ecschema.xml"));
    SchemaKey key("StartupCompany", 2, 0);

    bvector<ECSchemaP> supplementalSchemas;
    supplementalSchemas.push_back(supple.get());
    SupplementedSchemaBuilder builder;

    BentleyStatus schemaStatus = m_ecdb.Schemas().ImportSchemas(schemaContext->GetCache().GetSchemas());
    ASSERT_EQ(SUCCESS, schemaStatus);

    m_ecdb.SaveChanges();
    m_ecdb.CloseDb();

    BeFileName dbPath;
    BeTest::GetHost().GetOutputRoot(dbPath);
    dbPath.AppendToPath(BeFileName(dbFileName));

    ASSERT_EQ(DbResult::BE_SQLITE_OK, m_ecdb.OpenBeSQLiteDb(dbPath, Db::OpenParams(Db::OpenMode::Readonly)));
    ECSchemaCP startupCompanySchema = m_ecdb.Schemas().GetSchema("StartupCompany");
    ASSERT_TRUE(startupCompanySchema != nullptr);
    ECClassCP aaa2 = startupCompanySchema->GetClassCP("AAA");

    ECCustomAttributeInstanceIterable allCustomAttributes2 = aaa2->GetCustomAttributes(false);
    uint32_t allCustomAttributesCount2 = 0;
    for (IECInstancePtr attribute : allCustomAttributes2)
        {
        allCustomAttributesCount2++;
        }
    ASSERT_EQ(2, allCustomAttributesCount2);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                   11/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, ArrayPropertyTest)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("StartupCompany.ecdb", BeFileName(L"StartupCompany.02.00.ecschema.xml")));

    ECSchemaCP startupCompanySchema = m_ecdb.Schemas().GetSchema("StartupCompany", true);
    ASSERT_TRUE(startupCompanySchema != nullptr);

    ECClassCP arrayTestClass = startupCompanySchema->GetClassCP("ArrayTestclass");
    ASSERT_TRUE(arrayTestClass != nullptr);

    ArrayECPropertyCP p0_unbounded = arrayTestClass->GetPropertyP("p0_unbounded")->GetAsArrayProperty();
    ASSERT_TRUE(p0_unbounded != nullptr);
    ASSERT_EQ(p0_unbounded->GetMinOccurs(), 0);
    ASSERT_EQ(p0_unbounded->GetMaxOccurs(), UINT32_MAX);

    ArrayECPropertyCP p1_unbounded = arrayTestClass->GetPropertyP("p1_unbounded")->GetAsArrayProperty();
    ASSERT_TRUE(p1_unbounded != nullptr);
    ASSERT_EQ(p1_unbounded->GetMinOccurs(), 1);
    ASSERT_EQ(p1_unbounded->GetMaxOccurs(), UINT32_MAX);

    ArrayECPropertyCP p0_1 = arrayTestClass->GetPropertyP("p0_1")->GetAsArrayProperty();
    ASSERT_TRUE(p0_1 != nullptr);
    ASSERT_EQ(p0_1->GetMinOccurs(), 0);
    ASSERT_EQ(p0_1->GetMaxOccurs(), UINT32_MAX);

    ArrayECPropertyCP p1_1 = arrayTestClass->GetPropertyP("p1_1")->GetAsArrayProperty();
    ASSERT_TRUE(p1_1 != nullptr);
    ASSERT_EQ(p1_1->GetMinOccurs(), 1);
    ASSERT_EQ(p1_1->GetMaxOccurs(), UINT32_MAX);

    ArrayECPropertyCP p1_10000 = arrayTestClass->GetPropertyP("p1_10000")->GetAsArrayProperty();
    ASSERT_TRUE(p1_10000 != nullptr);
    ASSERT_EQ(p1_10000->GetMinOccurs(), 1);
    ASSERT_EQ(p1_10000->GetMaxOccurs(), UINT32_MAX);

    ArrayECPropertyCP p100_10000 = arrayTestClass->GetPropertyP("p100_10000")->GetAsArrayProperty();
    ASSERT_TRUE(p100_10000 != nullptr);
    ASSERT_EQ(p100_10000->GetMinOccurs(), 100);
    ASSERT_EQ(p100_10000->GetMaxOccurs(), UINT32_MAX);

    ArrayECPropertyCP p123_12345 = arrayTestClass->GetPropertyP("p123_12345")->GetAsArrayProperty();
    ASSERT_TRUE(p123_12345 != nullptr);
    ASSERT_EQ(p123_12345->GetMinOccurs(), 123);
    ASSERT_EQ(p123_12345->GetMaxOccurs(), UINT32_MAX);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         05/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, VerifyDatabaseSchemaAfterImport)
    {
    auto getColumnCount = [] (ECDbCR db, Utf8CP table)
        {
        Statement stmt;
        stmt.Prepare(db, SqlPrintfString("SELECT * FROM %s LIMIT 1", table));
        return stmt.GetColumnCount();
        };

    ASSERT_EQ(SUCCESS, SetupECDb("StartupCompany.ecdb", BeFileName(L"StartupCompany.02.00.ecschema.xml")));
    ECDbCR db = m_ecdb;
    //========================[sc_ClassWithPrimitiveProperties===================================
    Utf8CP tblClassWithPrimitiveProperties = "sc_ClassWithPrimitiveProperties";
    EXPECT_TRUE(db.TableExists(tblClassWithPrimitiveProperties));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveProperties, "Id"));
    EXPECT_EQ(13, getColumnCount(db, tblClassWithPrimitiveProperties));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveProperties, "intProp"));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveProperties, "longProp"));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveProperties, "doubleProp"));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveProperties, "stringProp"));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveProperties, "dateTimeProp"));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveProperties, "binaryProp"));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveProperties, "booleanProp"));
    //point2Prop is stored as x,y 2 columns
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveProperties, "point2dProp_X"));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveProperties, "point2dProp_Y"));
    //point3Prop is stored as x,y,z 3 columns
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveProperties, "point3dProp_X"));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveProperties, "point3dProp_Y"));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveProperties, "point3dProp_Z"));

    //========================[StructWithPrimitiveProperties==================================
    EXPECT_FALSE(db.TableExists("sc_StructWithPrimitiveProperties"));

    //========================[sc_ClassWithPrimitiveArrayProperties==============================
    Utf8CP tblClassWithPrimitiveArrayProperties = "sc_ClassWithPrimitiveArrayProperties";
    EXPECT_TRUE(db.TableExists(tblClassWithPrimitiveArrayProperties));
    EXPECT_EQ(10, getColumnCount(db, tblClassWithPrimitiveArrayProperties));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveArrayProperties, "Id"));

    //Verify columns
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveArrayProperties, "intArrayProp"));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveArrayProperties, "longArrayProp"));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveArrayProperties, "doubleArrayProp"));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveArrayProperties, "stringArrayProp"));// MapStrategy=Blob
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveArrayProperties, "dateTimeArrayProp"));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveArrayProperties, "binaryArrayProp"));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveArrayProperties, "booleanArrayProp"));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveArrayProperties, "point2dArrayProp"));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveArrayProperties, "point3dArrayProp")); // MapStrategy=Blob

                                                                                            //========================[StructWithPrimitiveArrayProperties=============================
    EXPECT_FALSE(db.TableExists("sc_StructWithPrimitiveArrayProperties"));

    //verify system array tables. They are created if  a primitive array property is encountered in schema
    //========================[sc_Asset]=========================================================
    //baseClass
    Utf8CP tblAsset = "sc_Asset";
    EXPECT_TRUE(db.TableExists(tblAsset));
    EXPECT_EQ(34, getColumnCount(db, tblAsset));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Id"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "ECClassId"));

    EXPECT_TRUE(db.ColumnExists(tblAsset, "AssetID"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "AssetOwner"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "BarCode"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "AssetUserID"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Cost"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Room"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "AssetRecordKey"));
    //Local properties of Furniture   
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Condition"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Material"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Weight"));
    // Properties of Chair which is derived from Furniture
    EXPECT_TRUE(db.ColumnExists(tblAsset, "ChairFootPrint"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Type"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Color"));

    // Properties of Desk which is derived from Furniture    
    EXPECT_TRUE(db.ColumnExists(tblAsset, "DeskFootPrint"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "NumberOfCabinets"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Size"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Type"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Breadth"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Length"));
    //relation keys
    EXPECT_TRUE(db.ColumnExists(tblAsset, "EmployeeId"));

    EXPECT_TRUE(db.ColumnExists(tblAsset, "HasWarranty"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "IsCompanyProperty"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Make"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Model"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "WarrantyExpiryDate"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Vendor"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Weight"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Type"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Size"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Type"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Vendor"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Weight"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Number"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Owner"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "User"));

    //========================[sc_Employee]======================================================
    //Related to Furniture. Employee can have one or more furniture
    Utf8CP tblEmployee = "sc_Employee";
    EXPECT_TRUE(db.TableExists(tblEmployee));
    EXPECT_EQ(32, getColumnCount(db, tblEmployee));
    EXPECT_TRUE(db.ColumnExists(tblEmployee, "Id"));

    EXPECT_TRUE(db.ColumnExists(tblEmployee, "EmployeeID"));
    EXPECT_TRUE(db.ColumnExists(tblEmployee, "FirstName"));
    EXPECT_TRUE(db.ColumnExists(tblEmployee, "JobTitle"));
    EXPECT_TRUE(db.ColumnExists(tblEmployee, "LastName"));
    EXPECT_TRUE(db.ColumnExists(tblEmployee, "ManagerID"));
    EXPECT_TRUE(db.ColumnExists(tblEmployee, "Room"));
    EXPECT_TRUE(db.ColumnExists(tblEmployee, "SSN"));
    EXPECT_TRUE(db.ColumnExists(tblEmployee, "Project"));
    EXPECT_TRUE(db.ColumnExists(tblEmployee, "FullName"));
    EXPECT_TRUE(db.ColumnExists(tblEmployee, "EmployeeType"));
    EXPECT_TRUE(db.ColumnExists(tblEmployee, "EmployeeRecordKey"));
    EXPECT_TRUE(db.ColumnExists(tblEmployee, "CompanyId"));
    EXPECT_TRUE(db.ColumnExists(tblEmployee, "Certifications"));

    //========================[sc_Company]=======================================================
    Utf8CP tblCompany = "sc_Company";
    EXPECT_TRUE(db.TableExists(tblCompany));
    EXPECT_EQ(15, getColumnCount(db, tblCompany));
    EXPECT_TRUE(db.ColumnExists(tblCompany, "Id"));

    EXPECT_TRUE(db.ColumnExists(tblCompany, "Name"));
    EXPECT_TRUE(db.ColumnExists(tblCompany, "NumberOfEmployees"));
    EXPECT_TRUE(db.ColumnExists(tblCompany, "ContactAddress"));
    EXPECT_TRUE(db.ColumnExists(tblCompany, "RecordKey"));

    //======================== EmployeeCertifications========================================
    EXPECT_FALSE(db.TableExists("sc_EmployeeCertification")) << "struct don't get a table";

    //========================[sc_Project]=======================================================
    Utf8CP tblProject = "sc_Project";
    EXPECT_TRUE(db.TableExists(tblProject));
    EXPECT_EQ(14, getColumnCount(db, tblProject));

    EXPECT_TRUE(db.ColumnExists(tblProject, "Id"));
    //It must not have ECClassId to differentiate each row to see which class it belong to.
    EXPECT_FALSE(db.ColumnExists(tblProject, "ECClassId"));

    EXPECT_TRUE(db.ColumnExists(tblProject, "CompletionDate"));
    EXPECT_TRUE(db.ColumnExists(tblProject, "EstimatedCost"));
    EXPECT_TRUE(db.ColumnExists(tblProject, "ProjectName"));
    EXPECT_TRUE(db.ColumnExists(tblProject, "ProjectDescription"));
    EXPECT_TRUE(db.ColumnExists(tblProject, "ProjectState"));
    EXPECT_TRUE(db.ColumnExists(tblProject, "StartDate"));
    EXPECT_TRUE(db.ColumnExists(tblProject, "InProgress"));
    EXPECT_TRUE(db.ColumnExists(tblProject, "TeamSize"));
    EXPECT_TRUE(db.ColumnExists(tblProject, "Logo"));
    EXPECT_TRUE(db.ColumnExists(tblProject, "Manager"));
    EXPECT_TRUE(db.ColumnExists(tblProject, "ProjectRecordKey"));
    //struct/arrays mapped to table
    EXPECT_TRUE(db.ColumnExists(tblProject, "TeamMemberList"));  //int array
                                                                 //relation
    EXPECT_TRUE(db.ColumnExists(tblProject, "CompanyId"));

    //========================[sc_Building]======================================================
    Utf8CP tblBuilding = "sc_Building";
    EXPECT_TRUE(db.TableExists(tblBuilding));
    EXPECT_EQ(14, getColumnCount(db, tblBuilding));

    EXPECT_TRUE(db.ColumnExists(tblBuilding, "Id"));
    //It must not have ECClassId to differentiate each row to see which class it belong to.
    EXPECT_FALSE(db.ColumnExists(tblBuilding, "ECClassId"));

    EXPECT_TRUE(db.ColumnExists(tblBuilding, "Number"));
    EXPECT_TRUE(db.ColumnExists(tblBuilding, "Name"));
    EXPECT_TRUE(db.ColumnExists(tblBuilding, "NumberOfFloors"));
    EXPECT_TRUE(db.ColumnExists(tblBuilding, "BuildingCode"));
    EXPECT_TRUE(db.ColumnExists(tblBuilding, "RecordKey"));
    //struct array
    EXPECT_FALSE(db.ColumnExists(tblBuilding, "Location"));

    //========================[Location]======================================================
    EXPECT_FALSE(db.TableExists("sc_Location")) << "no tables for structs";

    //========================[sc_BuildingFloor]=================================================
    Utf8CP tblBuildingFloor = "sc_BuildingFloor";
    EXPECT_TRUE(db.TableExists(tblBuildingFloor));
    EXPECT_EQ(7, getColumnCount(db, tblBuildingFloor));

    EXPECT_TRUE(db.ColumnExists(tblBuildingFloor, "Id"));
    //It must not have ECClassId to differentiate each row to see which class it belong to.
    EXPECT_FALSE(db.ColumnExists(tblBuildingFloor, "ECClassId"));

    EXPECT_TRUE(db.ColumnExists(tblBuildingFloor, "FloorNumber"));
    EXPECT_TRUE(db.ColumnExists(tblBuildingFloor, "BuildingCode"));
    EXPECT_TRUE(db.ColumnExists(tblBuildingFloor, "NumberOfOffices"));
    EXPECT_TRUE(db.ColumnExists(tblBuildingFloor, "Area"));
    EXPECT_TRUE(db.ColumnExists(tblBuildingFloor, "FloorCode"));
    EXPECT_TRUE(db.ColumnExists(tblBuildingFloor, "RecordKey"));

    //========================[sc_Cubicle]=================================================
    Utf8CP tblCubicle = "sc_Cubicle";
    EXPECT_TRUE(db.TableExists(tblCubicle));
    EXPECT_EQ(12, getColumnCount(db, tblCubicle));

    EXPECT_TRUE(db.ColumnExists(tblCubicle, "Id"));
    //It must not have ECClassId to differentiate each row to see which class it belong to.
    EXPECT_FALSE(db.ColumnExists(tblCubicle, "ECClassId"));

    EXPECT_TRUE(db.ColumnExists(tblCubicle, "Bay"));
    EXPECT_TRUE(db.ColumnExists(tblCubicle, "IsOccupied"));
    EXPECT_TRUE(db.ColumnExists(tblCubicle, "BuildingFloor"));
    EXPECT_TRUE(db.ColumnExists(tblCubicle, "Length"));
    EXPECT_TRUE(db.ColumnExists(tblCubicle, "Breadth"));
    EXPECT_TRUE(db.ColumnExists(tblCubicle, "NumberOfOccupants"));
    EXPECT_TRUE(db.ColumnExists(tblCubicle, "BuildingCode"));
    EXPECT_TRUE(db.ColumnExists(tblCubicle, "OfficeCode"));
    EXPECT_TRUE(db.ColumnExists(tblCubicle, "Area"));
    //array    
    EXPECT_TRUE(db.ColumnExists(tblCubicle, "OccupiedBy"));
    //relation
    EXPECT_TRUE(db.ColumnExists(tblCubicle, "FloorId"));

    //========================AnglesStruct======================================================
    EXPECT_FALSE(db.TableExists("sc_AnglesStruct")) << "structs are not mapped to any tables";

    //========================[sc_ABFoo]======================================================
    Utf8CP tblABFoo = "sc_ABFoo";
    EXPECT_TRUE(db.TableExists(tblABFoo));
    EXPECT_EQ(2, getColumnCount(db, tblABFoo));

    EXPECT_TRUE(db.ColumnExists(tblABFoo, "Id"));
    //It must not have ECClassId to differentiate each row to see which class it belong to.
    EXPECT_FALSE(db.ColumnExists(tblABFoo, "ECClassId"));

    EXPECT_TRUE(db.ColumnExists(tblABFoo, "stringABFoo"));

    //========================[sc_AAFoo]=========================================================
    Utf8CP tblAAFoo = "sc_AAFoo";
    EXPECT_TRUE(db.TableExists(tblAAFoo));
    EXPECT_EQ(24, getColumnCount(db, tblAAFoo));
    EXPECT_TRUE(db.ColumnExists(tblAAFoo, "Id"));
    //This a TablePerHieracrchy
    EXPECT_TRUE(db.ColumnExists(tblAAFoo, "ECClassId"));

    //Local properties
    EXPECT_TRUE(db.ColumnExists(tblAAFoo, "FooTag"));
    EXPECT_TRUE(db.ColumnExists(tblAAFoo, "intAAFoo"));
    EXPECT_TRUE(db.ColumnExists(tblAAFoo, "longAAFoo"));
    EXPECT_TRUE(db.ColumnExists(tblAAFoo, "stringAAFoo"));
    EXPECT_TRUE(db.ColumnExists(tblAAFoo, "doubleAAFoo"));
    EXPECT_TRUE(db.ColumnExists(tblAAFoo, "datetimeAAFoo"));
    EXPECT_TRUE(db.ColumnExists(tblAAFoo, "binaryAAFoo"));
    EXPECT_TRUE(db.ColumnExists(tblAAFoo, "booleanAAFoo"));
    EXPECT_TRUE(db.ColumnExists(tblAAFoo, "point2dAAFoo_X"));
    EXPECT_TRUE(db.ColumnExists(tblAAFoo, "point2dAAFoo_Y"));
    EXPECT_TRUE(db.ColumnExists(tblAAFoo, "point3dAAFoo_X"));
    EXPECT_TRUE(db.ColumnExists(tblAAFoo, "point3dAAFoo_Y"));
    EXPECT_TRUE(db.ColumnExists(tblAAFoo, "point3dAAFoo_Z"));
    EXPECT_TRUE(db.ColumnExists(tblAAFoo, "commonGeometryAAFoo"));
    EXPECT_TRUE(db.ColumnExists(tblAAFoo, "colorAAFoo"));

    // arrays
    EXPECT_TRUE(db.ColumnExists(tblAAFoo, "arrayOfIntsAAFoo"));
    EXPECT_TRUE(db.ColumnExists(tblAAFoo, "arrayOfpoint2dAAFoo"));
    EXPECT_TRUE(db.ColumnExists(tblAAFoo, "arrayOfpoint3dAAFoo"));

    //========================[sc_Bar]===========================================================
    Utf8CP tblBar = "sc_Bar";
    EXPECT_TRUE(db.TableExists(tblBar));
    EXPECT_EQ(2, getColumnCount(db, tblBar));
    EXPECT_TRUE(db.ColumnExists(tblBar, "Id"));
    //This a TablePerHieracrchy
    EXPECT_FALSE(db.ColumnExists(tblBar, "ECClassId"));
    //Local properties
    EXPECT_TRUE(db.ColumnExists(tblBar, "stringBar"));

    //========================[sc_Foo]===========================================================
    Utf8CP tblFoo = "sc_Foo";
    EXPECT_TRUE(db.TableExists(tblFoo));
    EXPECT_EQ(20, getColumnCount(db, tblFoo));

    EXPECT_TRUE(db.ColumnExists(tblFoo, "Id"));
    //This a TablePerHieracrchy
    EXPECT_TRUE(db.ColumnExists(tblFoo, "ECClassId"));

    //Local properties
    EXPECT_TRUE(db.ColumnExists(tblFoo, "intFoo"));
    EXPECT_TRUE(db.ColumnExists(tblFoo, "longFoo"));
    EXPECT_TRUE(db.ColumnExists(tblFoo, "stringFoo"));
    EXPECT_TRUE(db.ColumnExists(tblFoo, "doubleFoo"));
    EXPECT_TRUE(db.ColumnExists(tblFoo, "datetimeFoo"));
    EXPECT_TRUE(db.ColumnExists(tblFoo, "binaryFoo"));
    EXPECT_TRUE(db.ColumnExists(tblFoo, "booleanFoo"));
    EXPECT_TRUE(db.ColumnExists(tblFoo, "point2dFoo_X"));
    EXPECT_TRUE(db.ColumnExists(tblFoo, "point2dFoo_Y"));
    EXPECT_TRUE(db.ColumnExists(tblFoo, "point3dFoo_X"));
    EXPECT_TRUE(db.ColumnExists(tblFoo, "point3dFoo_Y"));
    EXPECT_TRUE(db.ColumnExists(tblFoo, "point3dFoo_Z"));
    EXPECT_TRUE(db.ColumnExists(tblFoo, "commonGeometryFoo"));

    // arrays/struct
    EXPECT_TRUE(db.ColumnExists(tblFoo, "arrayOfIntsFoo"));
    EXPECT_TRUE(db.ColumnExists(tblFoo, "arrayOfAnglesStructsFoo"));
    EXPECT_TRUE(db.ColumnExists(tblFoo, "anglesFoo_Alpha"));
    EXPECT_TRUE(db.ColumnExists(tblFoo, "anglesFoo_Beta"));
    EXPECT_TRUE(db.ColumnExists(tblFoo, "anglesFoo_Theta"));

    m_ecdb.CloseDb();
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Affan Khan                  04/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, SharedColumnConflictIssueWhenUsingMixinsAsRelationshipEndPoint)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("concept_station_mixin_issue.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="Diego" alias="diego" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="CoreCustomAttributes" version="01.00" alias="CoreCA"/>
            <ECSchemaReference name="Bentley_Standard_CustomAttributes" version="01.12" alias="bsca"/>
            <ECSchemaReference name="EditorCustomAttributes" version="01.03" alias="beca" />
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap"/>
            <!-- Subset of BisCore schema -->
            <ECEntityClass typeName="Element" modifier="Abstract" >
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
                <ECProperty propertyName="LastMod" typeName="dateTime" readOnly="True"  >
                    <ECCustomAttributes>
                        <DateTimeInfo xmlns="CoreCustomAttributes.01.00">
                            <DateTimeKind>Utc</DateTimeKind>
                        </DateTimeInfo>
                    </ECCustomAttributes>
                </ECProperty>
            </ECEntityClass>
            <ECEntityClass typeName="PhysicalElement" modifier="Abstract"  >
                <BaseClass>SpatialElement</BaseClass>
            </ECEntityClass>    
                <ECEntityClass typeName="SpatialElement" modifier="Abstract"  >
                <BaseClass>GeometricElement3d</BaseClass>
            </ECEntityClass>
            <ECEntityClass typeName="GeometricElement3d" modifier="Abstract"  >
                <BaseClass>GeometricElement</BaseClass>
                <ECCustomAttributes>
                    <ShareColumns xmlns="ECDbMap.02.00">
                        <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                        <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                    </ShareColumns>
                </ECCustomAttributes>
                <ECProperty propertyName="GeometryStream" typeName="binary" extendedTypeName="GeometryStream"  />
                <ECProperty propertyName="Origin" typeName="point3d" />
                <ECProperty propertyName="Yaw" typeName="double" />
                <ECProperty propertyName="Pitch" typeName="double" />
                <ECProperty propertyName="Roll" typeName="double" />
                <ECProperty propertyName="BBoxLow" typeName="point3d"  />
                <ECProperty propertyName="BBoxHigh" typeName="point3d"  />
            </ECEntityClass>    
            <ECEntityClass typeName="GeometricElement" modifier="Abstract"  >
                <BaseClass>Element</BaseClass>
                <ECCustomAttributes>
                    <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00"/>
                </ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="DefinitionElement" modifier="Abstract"  >
                <BaseClass>InformationContentElement</BaseClass>
                <ECCustomAttributes>
                    <ShareColumns xmlns="ECDbMap.02.00">
                        <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                        <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                    </ShareColumns>
                </ECCustomAttributes>
                <ECProperty propertyName="IsPrivate" typeName="boolean"  />
            </ECEntityClass>
            <ECEntityClass typeName="InformationContentElement" modifier="Abstract"  >
                <BaseClass>Element</BaseClass>
                <ECCustomAttributes>
                    <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00"/>
                </ECCustomAttributes>
            </ECEntityClass>
            <!-- Test classes causing the issue -->
            <ECEntityClass typeName="ILinearElement" modifier="Abstract">
                <ECCustomAttributes>
                    <IsMixin xmlns="CoreCustomAttributes.01.00">
                        <AppliesToEntityClass>Element</AppliesToEntityClass>
                    </IsMixin>
                </ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="ILinearlyLocated" modifier="Abstract">
                <ECCustomAttributes>
                    <IsMixin xmlns="CoreCustomAttributes.01.00">
                        <AppliesToEntityClass>Element</AppliesToEntityClass>
                    </IsMixin>
                </ECCustomAttributes>
                <ECNavigationProperty propertyName="ILinearElement" relationshipName="ILinearlyLocatedAlongILinearElement" direction="forward"/>
            </ECEntityClass>
            <ECEntityClass typeName="ILinearlyLocatedElement" modifier="Abstract">
                <BaseClass>ILinearlyLocated</BaseClass>
                <ECCustomAttributes>
                    <IsMixin xmlns="CoreCustomAttributes.01.00">
                        <AppliesToEntityClass>GeometricElement</AppliesToEntityClass>
                    </IsMixin>
                </ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="Tunnel" modifier="Sealed"  >
                <BaseClass>PhysicalElement</BaseClass>
                <BaseClass>ILinearElement</BaseClass>
            </ECEntityClass>
            <ECEntityClass typeName="FurnitureElement" modifier="Abstract" >
                <BaseClass>PhysicalElement</BaseClass>
                <ECNavigationProperty propertyName="FurnitureDefinition" relationshipName="FurnitureRefersToDefinition" direction="Forward"/>
            </ECEntityClass>
            <ECEntityClass typeName="LinearlyLocatedFurnitureElement" modifier="Abstract" >
                <BaseClass>FurnitureElement</BaseClass>
                <BaseClass>ILinearlyLocatedElement</BaseClass>
            </ECEntityClass>    
            <ECEntityClass typeName="GeometryDefinitionElement" modifier="Abstract">
                <BaseClass>DefinitionElement</BaseClass>
            </ECEntityClass>
            <ECEntityClass typeName="TemplateGeometryDefinitionElement" modifier="Abstract">
                <BaseClass>GeometryDefinitionElement</BaseClass>
            </ECEntityClass>
            <ECEntityClass typeName="GenericTemplateGeometryDefinition" modifier="Sealed">
                <BaseClass>TemplateGeometryDefinitionElement</BaseClass>
            </ECEntityClass>
            <ECEntityClass typeName="FurnitureDefinitionElement" modifier="Abstract">
                <BaseClass>DefinitionElement</BaseClass>
                <ECProperty propertyName="Thumbnail" typeName="binary" />
                <ECProperty propertyName="LightsJson" typeName="string" />
                <ECNavigationProperty propertyName="ElementGeometryDefinition" relationshipName="FurnitureDefinitionRefersToGeometryDefinition" direction="Forward"/>
            </ECEntityClass>
            <ECRelationshipClass typeName="ILinearlyLocatedAlongILinearElement" strength="referencing" modifier="None">
                <Source multiplicity="(0..*)" polymorphic="true" roleLabel="is the axis for N linearly-located entities">
                    <Class class="ILinearlyLocated"/>
                </Source>
                <Target multiplicity="(0..1)" polymorphic="true" roleLabel="N entities linearly located along 1 linear-element">
                    <Class class="ILinearElement"/>
                </Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="FurnitureRefersToDefinition" strength="referencing" strengthDirection="Backward" modifier="Abstract">
                <Source multiplicity="(0..*)" polymorphic="true" roleLabel="is referred by N furniture elements">
                    <Class class="FurnitureElement"/>
                </Source>
                <Target multiplicity="(0..1)" polymorphic="true" roleLabel="refers to 1 furniture definition">
                    <Class class="FurnitureDefinitionElement"/>
                </Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="FurnitureDefinitionRefersToGeometryDefinition" strength="referencing" modifier="Abstract">
                <Source multiplicity="(0..*)" polymorphic="true" roleLabel="refers to 1 Geometry definition">
                    <Class class="FurnitureDefinitionElement"/>
                </Source>
                <Target multiplicity="(0..1)" polymorphic="true" roleLabel="is referred by zero or more furniture definitions">
                    <Class class="GeometryDefinitionElement"/>
                </Target>
            </ECRelationshipClass>
        </ECSchema>)xml")));
    m_ecdb.Schemas().CreateClassViewsInDb();

    ECSqlStatement stmt;
    stmt.Prepare(m_ecdb, "SELECT * FROM diego.ILinearlyLocated");
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Affan Khan                  04/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, NullViewForMixIn)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("nullview.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="Diego" alias="diego" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="CoreCustomAttributes" version="01.00" alias="CoreCA"/>
            <ECSchemaReference name="Bentley_Standard_CustomAttributes" version="01.12" alias="bsca"/>
            <ECSchemaReference name="EditorCustomAttributes" version="01.03" alias="beca" />
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap"/>
            <!-- Subset of BisCore schema -->
            <ECEntityClass typeName="Element" modifier="Abstract" >
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
                <ECProperty propertyName="LastMod" typeName="dateTime" readOnly="True"  >
                    <ECCustomAttributes>
                        <DateTimeInfo xmlns="CoreCustomAttributes.01.00">
                            <DateTimeKind>Utc</DateTimeKind>
                        </DateTimeInfo>
                    </ECCustomAttributes>
                </ECProperty>
            </ECEntityClass>
            <ECEntityClass typeName="PhysicalElement" modifier="Abstract"  >
                <BaseClass>SpatialElement</BaseClass>
            </ECEntityClass>    
                <ECEntityClass typeName="SpatialElement" modifier="Abstract"  >
                <BaseClass>GeometricElement3d</BaseClass>
            </ECEntityClass>
            <ECEntityClass typeName="GeometricElement3d" modifier="Abstract"  >
                <BaseClass>GeometricElement</BaseClass>
                <ECCustomAttributes>
                    <ShareColumns xmlns="ECDbMap.02.00">
                        <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                        <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                    </ShareColumns>
                </ECCustomAttributes>
                <ECProperty propertyName="GeometryStream" typeName="binary" extendedTypeName="GeometryStream"  />
                <ECProperty propertyName="Origin" typeName="point3d" />
                <ECProperty propertyName="Yaw" typeName="double" />
                <ECProperty propertyName="Pitch" typeName="double" />
                <ECProperty propertyName="Roll" typeName="double" />
                <ECProperty propertyName="BBoxLow" typeName="point3d"  />
                <ECProperty propertyName="BBoxHigh" typeName="point3d"  />
            </ECEntityClass>    
            <ECEntityClass typeName="GeometricElement" modifier="Abstract"  >
                <BaseClass>Element</BaseClass>
                <ECCustomAttributes>
                    <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00"/>
                </ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="DefinitionElement" modifier="Abstract"  >
                <BaseClass>InformationContentElement</BaseClass>
                <ECCustomAttributes>
                    <ShareColumns xmlns="ECDbMap.02.00">
                        <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                        <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                    </ShareColumns>
                </ECCustomAttributes>
                <ECProperty propertyName="IsPrivate" typeName="boolean"  />
            </ECEntityClass>
            <ECEntityClass typeName="InformationContentElement" modifier="Abstract"  >
                <BaseClass>Element</BaseClass>
                <ECCustomAttributes>
                    <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00"/>
                </ECCustomAttributes>
            </ECEntityClass>
            <!-- Test classes causing the issue -->
            <ECEntityClass typeName="ILinearElement" modifier="Abstract">
                <ECCustomAttributes>
                    <IsMixin xmlns="CoreCustomAttributes.01.00">
                        <AppliesToEntityClass>Element</AppliesToEntityClass>
                    </IsMixin>
                </ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="ILinearlyLocated" modifier="Abstract">
                <ECCustomAttributes>
                    <IsMixin xmlns="CoreCustomAttributes.01.00">
                        <AppliesToEntityClass>Element</AppliesToEntityClass>
                    </IsMixin>
                </ECCustomAttributes>
                <ECNavigationProperty propertyName="ILinearElement" relationshipName="ILinearlyLocatedAlongILinearElement" direction="forward"/>
            </ECEntityClass>
            <ECEntityClass typeName="ILinearlyLocatedElement" modifier="Abstract">
                <BaseClass>ILinearlyLocated</BaseClass>
                <ECCustomAttributes>
                    <IsMixin xmlns="CoreCustomAttributes.01.00">
                        <AppliesToEntityClass>GeometricElement</AppliesToEntityClass>
                    </IsMixin>
                </ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="Tunnel" modifier="Sealed"  >
                <BaseClass>PhysicalElement</BaseClass>
                <BaseClass>ILinearElement</BaseClass>
            </ECEntityClass>
            <ECEntityClass typeName="FurnitureElement" modifier="Abstract" >
                <BaseClass>PhysicalElement</BaseClass>
                <ECNavigationProperty propertyName="FurnitureDefinition" relationshipName="FurnitureRefersToDefinition" direction="Forward"/>
            </ECEntityClass>
  
            <ECEntityClass typeName="GeometryDefinitionElement" modifier="Abstract">
                <BaseClass>DefinitionElement</BaseClass>
            </ECEntityClass>
            <ECEntityClass typeName="TemplateGeometryDefinitionElement" modifier="Abstract">
                <BaseClass>GeometryDefinitionElement</BaseClass>
            </ECEntityClass>
            <ECEntityClass typeName="GenericTemplateGeometryDefinition" modifier="Sealed">
                <BaseClass>TemplateGeometryDefinitionElement</BaseClass>
            </ECEntityClass>
            <ECEntityClass typeName="FurnitureDefinitionElement" modifier="Abstract">
                <BaseClass>DefinitionElement</BaseClass>
                <ECProperty propertyName="Thumbnail" typeName="binary" />
                <ECProperty propertyName="LightsJson" typeName="string" />
                <ECNavigationProperty propertyName="ElementGeometryDefinition" relationshipName="FurnitureDefinitionRefersToGeometryDefinition" direction="Forward"/>
            </ECEntityClass>
            <ECRelationshipClass typeName="ILinearlyLocatedAlongILinearElement" strength="referencing" modifier="None">
                <Source multiplicity="(0..*)" polymorphic="true" roleLabel="is the axis for N linearly-located entities">
                    <Class class="ILinearlyLocated"/>
                </Source>
                <Target multiplicity="(0..1)" polymorphic="true" roleLabel="N entities linearly located along 1 linear-element">
                    <Class class="ILinearElement"/>
                </Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="FurnitureRefersToDefinition" strength="referencing" strengthDirection="Backward" modifier="Abstract">
                <Source multiplicity="(0..*)" polymorphic="true" roleLabel="is referred by N furniture elements">
                    <Class class="FurnitureElement"/>
                </Source>
                <Target multiplicity="(0..1)" polymorphic="true" roleLabel="refers to 1 furniture definition">
                    <Class class="FurnitureDefinitionElement"/>
                </Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="FurnitureDefinitionRefersToGeometryDefinition" strength="referencing" modifier="Abstract">
                <Source multiplicity="(0..*)" polymorphic="true" roleLabel="refers to 1 Geometry definition">
                    <Class class="FurnitureDefinitionElement"/>
                </Source>
                <Target multiplicity="(0..1)" polymorphic="true" roleLabel="is referred by zero or more furniture definitions">
                    <Class class="GeometryDefinitionElement"/>
                </Target>
            </ECRelationshipClass>
        </ECSchema>)xml")));

    m_ecdb.Schemas().CreateClassViewsInDb();
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Affan Khan                  04/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, OverflowTableTest)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("OverflowTableTest.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="Diego" alias="diego" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap"/>
            <!-- Subset of BisCore schema -->
            <ECEntityClass typeName="B1" modifier="None" >
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00">
                        <MaxSharedColumnsBeforeOverflow>20</MaxSharedColumnsBeforeOverflow>
                        <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                    </ShareColumns>
                </ECCustomAttributes>
                <ECProperty propertyName="P01" typeName="point3d" />
                <ECProperty propertyName="P02" typeName="point3d" />
                <ECProperty propertyName="P03" typeName="point3d" />
                <ECProperty propertyName="P04" typeName="point3d" />
            </ECEntityClass>
            <ECEntityClass typeName="B2" modifier="None" >
                <BaseClass>B1</BaseClass>
                <ECProperty propertyName="P11" typeName="point3d" />
                <ECProperty propertyName="P12" typeName="point3d" />
                <ECProperty propertyName="P13" typeName="point3d" />
                <ECProperty propertyName="P14" typeName="point3d" />
            </ECEntityClass>
            <ECEntityClass typeName="B3" modifier="None" >
                <BaseClass>B2</BaseClass>
                <ECProperty propertyName="P21" typeName="point3d" />
                <ECProperty propertyName="P22" typeName="point3d" />
                <ECProperty propertyName="P23" typeName="point3d" />
                <ECProperty propertyName="P24" typeName="point3d" />
            </ECEntityClass>
            <ECEntityClass typeName="B4" modifier="None" >
                <BaseClass>B3</BaseClass>
                <ECProperty propertyName="P31" typeName="point3d" />
                <ECProperty propertyName="P32" typeName="point3d" />
                <ECProperty propertyName="P33" typeName="point3d" />
                <ECProperty propertyName="P34" typeName="point3d" />
            </ECEntityClass>
            <ECEntityClass typeName="B5" modifier="None" >
                <BaseClass>B4</BaseClass>
                <ECProperty propertyName="P41" typeName="point3d" />
                <ECProperty propertyName="P42" typeName="point3d" />
                <ECProperty propertyName="P43" typeName="point3d" />
                <ECProperty propertyName="P44" typeName="point3d" />
            </ECEntityClass>
            <ECEntityClass typeName="B6" modifier="None" >
                <BaseClass>B5</BaseClass>
                <ECProperty propertyName="P51" typeName="point3d" />
                <ECProperty propertyName="P52" typeName="point3d" />
                <ECProperty propertyName="P53" typeName="point3d" />
                <ECProperty propertyName="P54" typeName="point3d" />
            </ECEntityClass>
            <ECEntityClass typeName="B7" modifier="None" >
                <BaseClass>B6</BaseClass>
                <ECProperty propertyName="P61" typeName="point3d" />
                <ECProperty propertyName="P62" typeName="point3d" />
                <ECProperty propertyName="P63" typeName="point3d" />
                <ECProperty propertyName="P64" typeName="point3d" />
            </ECEntityClass>
            <ECEntityClass typeName="B8" modifier="None" >
                <BaseClass>B7</BaseClass>
                <ECProperty propertyName="P71" typeName="point3d" />
                <ECProperty propertyName="P72" typeName="point3d" />
                <ECProperty propertyName="P73" typeName="point3d" />
                <ECProperty propertyName="P74" typeName="point3d" />
            </ECEntityClass>
        </ECSchema>)xml")));
    m_ecdb.Schemas().CreateClassViewsInDb();
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Affan Khan                  04/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DbMappingTestFixture, OverflowTableJoinedTest)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("OverflowTableTest.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="Diego" alias="diego" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap"/>
            <!-- Subset of BisCore schema -->
            <ECEntityClass typeName="B1" modifier="None" >
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00">
                        <MaxSharedColumnsBeforeOverflow>20</MaxSharedColumnsBeforeOverflow>
                        <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                    </ShareColumns>
                    <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                </ECCustomAttributes>
                <ECProperty propertyName="P01" typeName="point3d" />
                <ECProperty propertyName="P02" typeName="point3d" />
                <ECProperty propertyName="P03" typeName="point3d" />
                <ECProperty propertyName="P04" typeName="point3d" />
            </ECEntityClass>
            <ECEntityClass typeName="B2" modifier="None" >
                <BaseClass>B1</BaseClass>
                <ECProperty propertyName="P11" typeName="point3d" />
                <ECProperty propertyName="P12" typeName="point3d" />
                <ECProperty propertyName="P13" typeName="point3d" />
                <ECProperty propertyName="P14" typeName="point3d" />
            </ECEntityClass>
            <ECEntityClass typeName="B3" modifier="None" >
                <BaseClass>B2</BaseClass>
                <ECProperty propertyName="P21" typeName="point3d" />
                <ECProperty propertyName="P22" typeName="point3d" />
                <ECProperty propertyName="P23" typeName="point3d" />
                <ECProperty propertyName="P24" typeName="point3d" />
            </ECEntityClass>
            <ECEntityClass typeName="B4" modifier="None" >
                <BaseClass>B3</BaseClass>
                <ECProperty propertyName="P31" typeName="point3d" />
                <ECProperty propertyName="P32" typeName="point3d" />
                <ECProperty propertyName="P33" typeName="point3d" />
                <ECProperty propertyName="P34" typeName="point3d" />
            </ECEntityClass>
            <ECEntityClass typeName="B5" modifier="None" >
                <BaseClass>B4</BaseClass>
                <ECProperty propertyName="P41" typeName="point3d" />
                <ECProperty propertyName="P42" typeName="point3d" />
                <ECProperty propertyName="P43" typeName="point3d" />
                <ECProperty propertyName="P44" typeName="point3d" />
            </ECEntityClass>
            <ECEntityClass typeName="B6" modifier="None" >
                <BaseClass>B5</BaseClass>
                <ECProperty propertyName="P51" typeName="point3d" />
                <ECProperty propertyName="P52" typeName="point3d" />
                <ECProperty propertyName="P53" typeName="point3d" />
                <ECProperty propertyName="P54" typeName="point3d" />
            </ECEntityClass>
            <ECEntityClass typeName="B7" modifier="None" >
                <BaseClass>B6</BaseClass>
                <ECProperty propertyName="P61" typeName="point3d" />
                <ECProperty propertyName="P62" typeName="point3d" />
                <ECProperty propertyName="P63" typeName="point3d" />
                <ECProperty propertyName="P64" typeName="point3d" />
            </ECEntityClass>
            <ECEntityClass typeName="B8" modifier="None" >
                <BaseClass>B7</BaseClass>
                <ECProperty propertyName="P71" typeName="point3d" />
                <ECProperty propertyName="P72" typeName="point3d" />
                <ECProperty propertyName="P73" typeName="point3d" />
                <ECProperty propertyName="P74" typeName="point3d" />
            </ECEntityClass>
        </ECSchema>)xml")));
    ASSERT_EQ(SUCCESS, Populate(m_ecdb, 3));

    ECSqlStatement b1, b2, b3;
    b3.Prepare(m_ecdb, "SELECT * FROM diego.B3");  //Access B1, b2, b2_overflow
    b1.Prepare(m_ecdb, "SELECT * FROM diego.B1"); //Access B1
    b2.Prepare(m_ecdb, "SELECT * FROM diego.B2"); //Access B1, B2
    }
END_ECDBUNITTESTS_NAMESPACE
 