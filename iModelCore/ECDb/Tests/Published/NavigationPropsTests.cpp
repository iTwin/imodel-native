/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/NavigationPropsTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "SchemaImportTestFixture.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 01/16
//+---------------+---------------+---------------+---------------+---------------+------
struct ECSqlNavigationPropertyTestFixture : DbMappingTestFixture
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
                         "        <ECNavigationProperty propertyName='A' relationshipName='AHasB' direction='Backward' />"
                         "    </ECEntityClass>"
                         "    <ECEntityClass typeName='C'>"
                         "        <ECProperty propertyName='PC' typeName='int' />"
                         "    </ECEntityClass>"
                         "    <ECEntityClass typeName='D'>"
                         "        <ECProperty propertyName='PD' typeName='int' />"
                         "    </ECEntityClass>"
                         "   <ECRelationshipClass typeName='AHasB' strength='Embedding'  modifier='Sealed'>"
                         "      <Source cardinality='(0,1)' polymorphic='False'>"
                         "          <Class class ='A' />"
                         "      </Source>"
                         "      <Target cardinality='(0,N)' polymorphic='False'>"
                         "          <Class class ='B' />"
                         "      </Target>"
                         "   </ECRelationshipClass>"
                         "   <ECRelationshipClass typeName='CHasDLinkTable' strength='Referencing'  modifier='Sealed'>"
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
    AssertPrepare("SELECT PB, A FROM ts.B WHERE ECInstanceId=?", true, "");
    AssertPrepare("SELECT PB, A.Id, A.RelECClassId FROM ts.B WHERE ECInstanceId=?", true, "");
    AssertPrepare("SELECT * FROM ts.B WHERE ECInstanceId=?", true, "");

    AssertPrepare("SELECT PD FROM ts.D WHERE ECInstanceId=?", true, "");

    AssertPrepare("INSERT INTO ts.B (PB,A.Id) VALUES(123,?)", true, "NavProp with single related instance is expected to be supported.");
    AssertPrepare("INSERT INTO ts.D (PD) VALUES(123)", true, "");

    AssertPrepare("UPDATE ONLY ts.B SET A.Id=?", true, "Updating NavProp with end table rel is supported.");
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlNavigationPropertyTestFixture, RelECClassId)
    {
    ECDbCR ecdb = SetupECDb("ecsqlnavpropsupport.ecdb",
              SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                         "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                         "<ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                         "    <ECEntityClass typeName='Model'>"
                         "        <ECProperty propertyName='Name' typeName='string' />"
                         "    </ECEntityClass>"
                         "    <ECEntityClass typeName='Element' modifier='Abstract'>"
                         "        <ECCustomAttributes>"
                         "         <ClassMap xmlns='ECDbMap.02.00'>"
                         "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                         "            </ClassMap>"
                         "        </ECCustomAttributes>"
                         "        <ECProperty propertyName='Code' typeName='string' />"
                         "        <ECNavigationProperty propertyName='Model' relationshipName='ModelHasElement' direction='Backward' />"
                         "        <ECNavigationProperty propertyName='Parent' relationshipName='ElementOwnsChildElement' direction='Backward' />"
                         "    </ECEntityClass>"
                         "    <ECEntityClass typeName='SubElement'>"
                         "        <BaseClass>Element</BaseClass>"
                         "        <ECProperty propertyName='SubProp1' typeName='int' />"
                         "    </ECEntityClass>"
                         "   <ECRelationshipClass typeName='ModelHasElement' strength='Embedding'  modifier='Sealed'>"
                         "      <Source multiplicity='(1..1)' polymorphic='False' roleLabel='Model'>"
                         "          <Class class ='Model' />"
                         "      </Source>"
                         "      <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Element'>"
                         "          <Class class ='Element' />"
                         "      </Target>"
                         "   </ECRelationshipClass>"
                         "   <ECRelationshipClass typeName='ElementOwnsChildElement' strength='Embedding'  modifier='Abstract'>"
                         "      <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Parent Element'>"
                         "          <Class class ='Element' />"
                         "      </Source>"
                         "      <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Child Element'>"
                         "          <Class class ='Element' />"
                         "      </Target>"
                         "   </ECRelationshipClass>"
                         "   <ECRelationshipClass typeName='ElementOwnsSubElement' strength='Embedding'  modifier='Sealed'>"
                         "        <BaseClass>ElementOwnsChildElement</BaseClass>"
                         "      <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Owner Element'>"
                         "          <Class class ='Element' />"
                         "      </Source>"
                         "      <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Owned SubElement'>"
                         "          <Class class ='SubElement' />"
                         "      </Target>"
                         "   </ECRelationshipClass>"
                         "</ECSchema>"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    ASSERT_FALSE(ecdb.ColumnExists("ts_Element", "ModelRelECClassId"));
    ASSERT_TRUE(ecdb.ColumnExists("ts_Element", "ParentRelECClassId"));
    AssertIndexExists(ecdb, "ix_ts_Element_ModelRelECClassId", false);
    AssertIndex(ecdb, "ix_ts_Element_ParentRelECClassId", false, "ts_Element", {"ParentRelECClassId"}, "([ParentRelECClassId] IS NOT NULL)");
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlNavigationPropertyTestFixture, RelECClassIdAndSharedColumns)
    {
    ECDbCR ecdb = SetupECDb("relecclassidandsharedcolumns.ecdb",
                            SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                        <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                            <ECEntityClass typeName="Model">
                                <ECProperty propertyName="Name" typeName="string" />
                            </ECEntityClass>
                            <ECEntityClass typeName="Element" modifier="Abstract">
                                <ECCustomAttributes>
                                    <ClassMap xmlns="ECDbMap.02.00">
                                        <MapStrategy>TablePerHierarchy</MapStrategy>
                                    </ClassMap>
                                    <ShareColumns xmlns="ECDbMap.02.00">
                                        <MaxSharedColumnsBeforeOverflow>10</MaxSharedColumnsBeforeOverflow>
                                    </ShareColumns>
                                </ECCustomAttributes>
                                <ECProperty propertyName="Code" typeName="string" />
                                <ECNavigationProperty propertyName="Model" relationshipName="ModelHasElement" direction="Backward" />
                                <ECNavigationProperty propertyName="Parent" relationshipName="ElementOwnsChildElement" direction="Backward" />
                            </ECEntityClass>
                            <ECEntityClass typeName="SubElement">
                                <BaseClass>Element</BaseClass>
                                <ECProperty propertyName="SubProp1" typeName="int" />
                            </ECEntityClass>
                            <ECRelationshipClass typeName="ModelHasElement" strength="Embedding"  modifier="Sealed">
                                <Source multiplicity="(1..1)" polymorphic="False" roleLabel="Model">
                                    <Class class ="Model" />
                                </Source>
                                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="Element">
                                    <Class class ="Element" />
                                </Target>
                            </ECRelationshipClass>
                            <ECRelationshipClass typeName="ElementOwnsChildElement" strength="Embedding"  modifier="None">
                                <Source multiplicity="(0..1)" polymorphic="True" roleLabel="Parent Element">
                                    <Class class ="Element" />
                              </Source>
                              <Target multiplicity="(0..*)" polymorphic="True" roleLabel="Child Element">
                                  <Class class ="Element" />
                              </Target>
                           </ECRelationshipClass>
                          </ECSchema>)xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    ASSERT_EQ(SUCCESS, ecdb.Schemas().CreateClassViewsInDb());

    ECClassId modelHasElementRelClassId = ecdb.Schemas().GetClassId("TestSchema", "ModelHasElement");
    ASSERT_TRUE(modelHasElementRelClassId.IsValid());
    ECClassId elementOwnsElementRelClassId = ecdb.Schemas().GetClassId("TestSchema", "ElementOwnsChildElement");
    ASSERT_TRUE(elementOwnsElementRelClassId.IsValid());

    ECInstanceKey modelKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.Model(Name) VALUES('Main')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(modelKey)) << stmt.GetECSql();
    }

    ECInstanceKey rootElementKey, fooElementKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.SubElement(Code,Model,Parent) VALUES(?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Root", IECSqlBinder::MakeCopy::No)) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(2, modelKey.GetInstanceId())) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(rootElementKey)) << stmt.GetECSql();
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Foo", IECSqlBinder::MakeCopy::No)) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(2, modelKey.GetInstanceId())) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(3, rootElementKey.GetInstanceId(), elementOwnsElementRelClassId)) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(fooElementKey)) << stmt.GetECSql();
    }

    AssertPropertyMapping(ecdb, PropertyAccessString("ts", "Element", "Model"), 
        std::map<Utf8String, ColumnInfo>{{"Model.Id", ColumnInfo("ts_Element","ps5")}, 
         {"Model.RelECClassId", ColumnInfo("ts_Element","ModelRelECClassId",true)}});
    AssertPropertyMapping(ecdb, PropertyAccessString("ts", "Element", "Parent"),
                          std::map<Utf8String, ColumnInfo>{{"Parent.Id", ColumnInfo("ts_Element","ps3")},
                                    {"Parent.RelECClassId", ColumnInfo("ts_Element","ps4")}});
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  05/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlNavigationPropertyTestFixture, ColumnOrder)
    {
    ECDbCR ecdb = SetupECDb("navpropcolumnorder.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
               <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
               <ECEntityClass typeName="CodeSpec" modifier="Sealed">
                   <ECProperty propertyName="Name" typeName="string" />
               </ECEntityClass>
               <ECEntityClass typeName="CodeScope" modifier="Sealed">
                   <ECProperty propertyName="Name" typeName="string" />
               </ECEntityClass>
               <ECEntityClass typeName="HasNavPropsWithRelECClassId" modifier="Abstract">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.2.0">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
                   <ECProperty propertyName="Name" typeName="string" />
                   <ECNavigationProperty propertyName="CodeSpec" relationshipName="CodeSpecHasFoo" direction="Backward" />
                   <ECNavigationProperty propertyName="CodeScope" relationshipName="CodeScopeHasFoo" direction="Backward" />
                   <ECProperty propertyName="LastMod" typeName="DateTime" />
               </ECEntityClass>
               <ECEntityClass typeName="HasNavPropsWithoutRelECClassId" modifier="Abstract">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.2.0">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
                   <ECProperty propertyName="Name" typeName="string" />
                   <ECNavigationProperty propertyName="CodeSpec" relationshipName="CodeSpecHasFoo_Sealed" direction="Backward" />
                   <ECNavigationProperty propertyName="CodeScope" relationshipName="CodeScopeHasFoo_Sealed" direction="Backward" />
                   <ECProperty propertyName="LastMod" typeName="DateTime" />
               </ECEntityClass>
             <ECRelationshipClass typeName="CodeSpecHasFoo" strength="Referencing"  modifier="None">
                <ECCustomAttributes>
                    <ForeignKeyConstraint xmlns="ECDbMap.2.0"/>
                </ECCustomAttributes>
                <Source multiplicity="(1..1)" polymorphic="True" roleLabel="Parent Element">
                    <Class class ="CodeSpec" />
                </Source>
                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="Child Element">
                    <Class class ="HasNavPropsWithRelECClassId" />
                </Target>
             </ECRelationshipClass>
             <ECRelationshipClass typeName="CodeScopeHasFoo" strength="Referencing"  modifier="None">
                <ECCustomAttributes>
                    <ForeignKeyConstraint xmlns="ECDbMap.2.0"/>
                </ECCustomAttributes>
                <Source multiplicity="(1..1)" polymorphic="True" roleLabel="Parent Element">
                    <Class class ="CodeScope" />
                </Source>
                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="Child Element">
                    <Class class ="HasNavPropsWithRelECClassId" />
                </Target>
             </ECRelationshipClass>
             <ECRelationshipClass typeName="CodeSpecHasFoo_Sealed" strength="Referencing"  modifier="Sealed">
                <ECCustomAttributes>
                    <ForeignKeyConstraint xmlns="ECDbMap.2.0"/>
                </ECCustomAttributes>
                <Source multiplicity="(1..1)" polymorphic="True" roleLabel="Parent Element">
                    <Class class ="CodeSpec" />
                </Source>
                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="Child Element">
                    <Class class ="HasNavPropsWithoutRelECClassId" />
                </Target>
             </ECRelationshipClass>
             <ECRelationshipClass typeName="CodeScopeHasFoo_Sealed" strength="Referencing"  modifier="Sealed">
                <ECCustomAttributes>
                    <ForeignKeyConstraint xmlns="ECDbMap.2.0"/>
                </ECCustomAttributes>
                <Source multiplicity="(1..1)" polymorphic="True" roleLabel="Parent Element">
                    <Class class ="CodeScope" />
                </Source>
                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="Child Element">
                    <Class class ="HasNavPropsWithoutRelECClassId" />
                </Target>
             </ECRelationshipClass>
              </ECSchema>)xml"));

    ASSERT_TRUE(ecdb.IsDbOpen());
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlNavigationPropertyTestFixture, ClearBindings)
    {
    ECDbCR ecdb = SetupECDb("navpropclearbindings.ecdb",
                            SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                        <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                            <ECEntityClass typeName="Element" modifier="Abstract">
                                <ECCustomAttributes>
                                    <ClassMap xmlns="ECDbMap.02.00">
                                        <MapStrategy>TablePerHierarchy</MapStrategy>
                                    </ClassMap>
                                </ECCustomAttributes>
                                <ECProperty propertyName="Code" typeName="string" />
                                <ECNavigationProperty propertyName="Parent" relationshipName="ElementOwnsChildElement" direction="Backward" />
                            </ECEntityClass>
                            <ECEntityClass typeName="SubElement">
                                <BaseClass>Element</BaseClass>
                                <ECProperty propertyName="SubProp1" typeName="int" />
                            </ECEntityClass>
                            <ECRelationshipClass typeName="ElementOwnsChildElement" strength="Embedding"  modifier="None">
                                <Source multiplicity="(0..1)" polymorphic="True" roleLabel="Parent Element">
                                    <Class class ="Element" />
                              </Source>
                              <Target multiplicity="(0..*)" polymorphic="True" roleLabel="Child Element">
                                  <Class class ="Element" />
                              </Target>
                           </ECRelationshipClass>
                          </ECSchema>)xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    ECClassId elementOwnsElementRelClassId = ecdb.Schemas().GetClassId("TestSchema", "ElementOwnsChildElement");
    ASSERT_TRUE(elementOwnsElementRelClassId.IsValid());

    ECInstanceKey rootElementKey, element1Key, element2Key;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.SubElement(Code,Parent) VALUES(?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Root", IECSqlBinder::MakeCopy::No)) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(rootElementKey)) << stmt.GetECSql();
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Foo1", IECSqlBinder::MakeCopy::No)) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(2, rootElementKey.GetInstanceId(), elementOwnsElementRelClassId)) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(element1Key)) << stmt.GetECSql();
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Foo2", IECSqlBinder::MakeCopy::No)) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(element2Key)) << stmt.GetECSql();
    }

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT Code, Parent FROM ts.SubElement"));
    while (BE_SQLITE_ROW == stmt.Step())
        {
        Utf8CP code = stmt.GetValueText(0);
        if (BeStringUtilities::StricmpAscii(code, "Foo1") == 0)
            {
            ASSERT_FALSE(stmt.IsValueNull(1)) << "Foo1.Parent";
            ECClassId actualRelClassId;
            ECInstanceId actualParentId = stmt.GetValueNavigation<ECInstanceId>(1, &actualRelClassId);
            ASSERT_EQ(rootElementKey.GetInstanceId().GetValue(), actualParentId.GetValue()) << "Foo1.Parent";
            ASSERT_EQ(elementOwnsElementRelClassId.GetValue(), actualRelClassId.GetValue()) << "Foo1.Parent";
            continue;
            }

        ASSERT_TRUE(stmt.IsValueNull(1)) << code << ".Parent";
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     12/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlNavigationPropertyTestFixture, NavPropForVirtualRelClassId)
    {

    SchemaItem testItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "<ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "    <ECEntityClass typeName='DgnModel'>"
        "        <ECProperty propertyName='Name' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='DgnElement'>"
        "        <ECProperty propertyName='Code' typeName='string' />"
        "        <ECNavigationProperty propertyName='Model' relationshipName='ParentHasChildren' direction='Backward' />"
        "    </ECEntityClass>"
        "   <ECRelationshipClass typeName='ParentHasChildren' strength='Referencing'  modifier='Sealed'>"
        "      <Source multiplicity='(0..1)' polymorphic='False' roleLabel='Model'>"
        "          <Class class ='DgnModel' />"
        "      </Source>"
        "      <Target multiplicity='(0..*)' polymorphic='False' roleLabel='Element'>"
        "          <Class class ='DgnElement' />"
        "      </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>", true, "");

    ECDb db;
    bool asserted = false;
    AssertSchemaImport(db, asserted, testItem, "NavPropForVirtualRelClassId.ecdb");
    ASSERT_FALSE(asserted);

    ECSqlStatement stmt;
    ECInstanceKey modelKey;

    ECClassId parentHasChildrenRelClassId = db.Schemas().GetClassId("TestSchema", "ParentHasChildren");
    ASSERT_TRUE(parentHasChildrenRelClassId.IsValid());

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "INSERT INTO ts.DgnModel(Name) VALUES('TestVal-1')"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, stmt.Step(modelKey));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "INSERT INTO ts.DgnElement(Model.Id, Model.RelECClassId,Code) VALUES(?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, modelKey.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, parentHasChildrenRelClassId));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(3, "TestVal-2", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "SELECT * FROM ts.DgnElement WHERE Model IS NULL"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, stmt.Step());
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 11/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlNavigationPropertyTestFixture, BindingWithOptionalRelClassId)
    {
    ECDbCR ecdb = SetupECDb("ecsqlnavpropsupport.ecdb",
                            SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                       "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                       "<ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                                       "    <ECEntityClass typeName='Model'>"
                                       "        <ECProperty propertyName='Name' typeName='string' />"
                                       "    </ECEntityClass>"
                                       "    <ECEntityClass typeName='Element' modifier='Abstract'>"
                                       "        <ECCustomAttributes>"
                                       "            <ClassMap xmlns='ECDbMap.02.00'>"
                                       "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                       "            </ClassMap>"
                                       "        </ECCustomAttributes>"
                                       "        <ECProperty propertyName='Code' typeName='string' />"
                                       "        <ECNavigationProperty propertyName='Model' relationshipName='ModelHasElements' direction='Backward' />"
                                       "    </ECEntityClass>"
                                       "    <ECEntityClass typeName='InfoElement'>"
                                       "        <BaseClass>Element</BaseClass>"
                                       "        <ECProperty propertyName='InfoTag' typeName='string' />"
                                       "    </ECEntityClass>"
                                       "    <ECEntityClass typeName='PhysicalElement'>"
                                       "        <BaseClass>Element</BaseClass>"
                                       "        <ECProperty propertyName='Geometry' typeName='string' />"
                                       "    </ECEntityClass>"
                                       "   <ECRelationshipClass typeName='ModelHasElements' strength='Embedding'  modifier='Sealed'>"
                                        "        <ECCustomAttributes>"
                                        "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                        "        </ECCustomAttributes>"
                                       "      <Source multiplicity='(1..1)' polymorphic='False' roleLabel='Model'>"
                                       "          <Class class ='Model' />"
                                       "      </Source>"
                                       "      <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Element'>"
                                       "          <Class class ='Element' />"
                                       "      </Target>"
                                       "   </ECRelationshipClass>"
                                       "</ECSchema>"));
    ASSERT_TRUE(ecdb.IsDbOpen());
    ECClassId modelHasElementsClassId = ecdb.Schemas().GetClassId("TestSchema", "ModelHasElements");
    ASSERT_TRUE(modelHasElementsClassId.IsValid());

    ECInstanceKey modelKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.Model(Name) VALUES('MainModel')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(modelKey));
    stmt.Finalize();
    }

    auto validateInsert = [] (bool& isValid, ECDbCR ecdb, ECInstanceId elementId, ECInstanceId expectedModelId, ECClassId expectedRelClassId)
        {
        isValid = false;
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT Model.Id, Model.RelECClassId FROM ts.Element WHERE ECInstanceId=?"));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, elementId));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(expectedModelId.GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue()) << "Model.Id";
        ASSERT_EQ(expectedRelClassId.GetValue(), stmt.GetValueId<ECClassId>(1).GetValue()) << "Model.RelECClassId";
        isValid = true;
        };

    //*** Bind via Nav prop with virtual rel class id
    ECInstanceKey info1Key;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.InfoElement(Code,Model) VALUES(?,?)"));

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Info-1", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(2, modelKey.GetInstanceId(), modelHasElementsClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(info1Key));
    stmt.Reset();
    stmt.ClearBindings();

    bool insertWasValid = false;
    validateInsert(insertWasValid, ecdb, info1Key.GetInstanceId(), modelKey.GetInstanceId(), modelHasElementsClassId);
    ASSERT_TRUE(insertWasValid);

    //now use alternative API via struct binder
    ECInstanceKey newKey;
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Info-2", IECSqlBinder::MakeCopy::No));
    IECSqlBinder& navPropBinder = stmt.GetBinder(2);
    ASSERT_EQ(ECSqlStatus::Success, navPropBinder["Id"].BindId(modelKey.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, navPropBinder["RelECClassId"].BindId(modelHasElementsClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey));
    stmt.Reset();
    stmt.ClearBindings();

    validateInsert(insertWasValid, ecdb, newKey.GetInstanceId(), modelKey.GetInstanceId(), modelHasElementsClassId);
    ASSERT_TRUE(insertWasValid);

    //now with omitting optional rel class id
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Info-3", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(2, modelKey.GetInstanceId(), ECClassId())) << "RelECClassId is virtual for ModelHasElements, therefore passing it to BindNavigationValue is optional";
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey));
    stmt.Reset();
    stmt.ClearBindings();

    validateInsert(insertWasValid, ecdb, newKey.GetInstanceId(), modelKey.GetInstanceId(), modelHasElementsClassId);
    ASSERT_TRUE(insertWasValid);

    //now with omitting navigation id.
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Info-4", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(2, ECInstanceId(), ECClassId())) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_CONSTRAINT_NOTNULL, stmt.Step(newKey)) << "NavigationProp.Id not bound to " << stmt.GetECSql();
    stmt.Reset();
    stmt.ClearBindings();

    //invalid model id -> FK violation
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Info-5", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(2, ECInstanceId(modelKey.GetInstanceId().GetValue() + 1), ECClassId()));
    ASSERT_EQ(BE_SQLITE_CONSTRAINT_FOREIGNKEY, stmt.Step(newKey)) << ecdb.GetLastError().c_str();
    stmt.Reset();
    stmt.ClearBindings();

    //invalid rel class model id -> ignored
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Info-6", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(2, modelKey.GetInstanceId(), ECClassId(modelHasElementsClassId.GetValue() + 1)));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey)) << ecdb.GetLastError().c_str();
    stmt.Reset();
    stmt.ClearBindings();
    //wrong rel class id is just ignored. SELECT will return correct one
    validateInsert(insertWasValid, ecdb, newKey.GetInstanceId(), modelKey.GetInstanceId(), modelHasElementsClassId);
    ASSERT_TRUE(insertWasValid);
    }

    //expanding ECSQL syntax
    {
    ECInstanceKey newKey;

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.InfoElement(Code,Model.Id,Model.RelECClassId) VALUES(?,?,?)"));

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Info-2-1", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, modelKey.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, modelHasElementsClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey)) << ecdb.GetLastError().c_str();
    stmt.Reset();
    stmt.ClearBindings();

    bool insertWasValid = false;
    validateInsert(insertWasValid, ecdb, newKey.GetInstanceId(), modelKey.GetInstanceId(), modelHasElementsClassId);
    ASSERT_TRUE(insertWasValid);

    //omit optional rel class id
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Info-2-2", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, modelKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey)) << ecdb.GetLastError().c_str();
    stmt.Reset();
    stmt.ClearBindings();

    validateInsert(insertWasValid, ecdb, newKey.GetInstanceId(), modelKey.GetInstanceId(), modelHasElementsClassId);
    ASSERT_TRUE(insertWasValid);

    //omit non-optional model id
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Info-2-3", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, modelHasElementsClassId));
    ASSERT_EQ(BE_SQLITE_CONSTRAINT_NOTNULL, stmt.Step(newKey)) << ecdb.GetLastError().c_str();
    stmt.Reset();
    stmt.ClearBindings();

    //invalid model id -> FK violation
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Info-2-4", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(2, modelKey.GetInstanceId().GetValue() + 1));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, modelHasElementsClassId));
    ASSERT_EQ(BE_SQLITE_CONSTRAINT_FOREIGNKEY, stmt.Step(newKey)) << ecdb.GetLastError().c_str();
    stmt.Reset();
    stmt.ClearBindings();

    //invalid rel class model id -> ignored
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Info-2-5", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, modelKey.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(3, modelHasElementsClassId.GetValue() + 1));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey)) << ecdb.GetLastError().c_str();
    stmt.Reset();
    stmt.ClearBindings();
    //wrong rel class id is just ignored. SELECT will return correct one
    validateInsert(insertWasValid, ecdb, newKey.GetInstanceId(), modelKey.GetInstanceId(), modelHasElementsClassId);
    ASSERT_TRUE(insertWasValid);
    }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 11/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlNavigationPropertyTestFixture, BindingWithMandatoryRelClassId)
    {
    ECDbCR ecdb = SetupECDb("ecsqlnavpropsupport.ecdb",
                            SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                       "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                       "<ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                                       "    <ECEntityClass typeName='Element' modifier='Abstract'>"
                                       "        <ECCustomAttributes>"
                                       "            <ClassMap xmlns='ECDbMap.02.00'>"
                                       "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                       "            </ClassMap>"
                                       "        </ECCustomAttributes>"
                                       "        <ECProperty propertyName='Code' typeName='string' />"
                                       "        <ECNavigationProperty propertyName='Parent' relationshipName='ElementOwnsChildElements' direction='Backward' />"
                                       "    </ECEntityClass>"
                                       "    <ECEntityClass typeName='InfoElement'>"
                                       "        <BaseClass>Element</BaseClass>"
                                       "        <ECProperty propertyName='InfoTag' typeName='string' />"
                                       "    </ECEntityClass>"
                                       "    <ECEntityClass typeName='PhysicalElement'>"
                                       "        <BaseClass>Element</BaseClass>"
                                       "        <ECProperty propertyName='Geometry' typeName='string' />"
                                       "    </ECEntityClass>"
                                       "   <ECRelationshipClass typeName='ElementOwnsChildElements' strength='Embedding'  modifier='Abstract'>"
                                        "        <ECCustomAttributes>"
                                        "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                        "        </ECCustomAttributes>"
                                       "      <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Parent Element'>"
                                       "          <Class class ='Element' />"
                                       "      </Source>"
                                       "      <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Child Element'>"
                                       "          <Class class ='Element' />"
                                       "      </Target>"
                                       "   </ECRelationshipClass>"
                                       "   <ECRelationshipClass typeName='ElementOwnsPhysicalElements' strength='Embedding'  modifier='Sealed'>"
                                       "        <BaseClass>ElementOwnsChildElements</BaseClass>"
                                       "      <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Owner Element'>"
                                       "          <Class class ='Element' />"
                                       "      </Source>"
                                       "      <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Owned PhysicalElement'>"
                                       "          <Class class ='PhysicalElement' />"
                                       "      </Target>"
                                       "   </ECRelationshipClass>"
                                       "</ECSchema>"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    ECClassId elementOwnsPhysicalElementsClassId = ecdb.Schemas().GetClassId("TestSchema", "ElementOwnsPhysicalElements");
    ASSERT_TRUE(elementOwnsPhysicalElementsClassId.IsValid());

    auto validateInsert = [] (bool& isValid, ECDbCR ecdb, ECInstanceId elementId, ECInstanceId expectedParentId, ECClassId expectedRelClassId)
        {
        isValid = false;
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT Parent.Id, Parent.RelECClassId FROM ts.Element WHERE ECInstanceId=?"));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, elementId));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        if (expectedParentId.IsValid())
            ASSERT_EQ(expectedParentId.GetValueUnchecked(), stmt.GetValueId<ECInstanceId>(0).GetValueUnchecked()) << "Parent.Id";
        else
            ASSERT_TRUE(stmt.IsValueNull(0)) << "Parent.Id";

        if (expectedRelClassId.IsValid())
            ASSERT_EQ(expectedRelClassId.GetValueUnchecked(), stmt.GetValueId<ECClassId>(1).GetValueUnchecked()) << "Parent.RelECClassId";
        else
            ASSERT_TRUE(stmt.IsValueNull(1)) << "Parent.RelECClassId";

        isValid = true;
        };

    ECInstanceKey info1Key;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.InfoElement(Code) VALUES('Info-1')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(info1Key));
    }

    {
    ECInstanceKey newKey;
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.PhysicalElement(Code,Parent) VALUES(?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Physical-1", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(2, info1Key.GetInstanceId(), elementOwnsPhysicalElementsClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey));

    stmt.Reset();
    stmt.ClearBindings();

    bool insertWasValid = false;
    validateInsert(insertWasValid, ecdb, newKey.GetInstanceId(), info1Key.GetInstanceId(), elementOwnsPhysicalElementsClassId);
    ASSERT_TRUE(insertWasValid);

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Physical-2", IECSqlBinder::MakeCopy::No));
    IECSqlBinder& navPropBinder = stmt.GetBinder(2);
    ASSERT_EQ(ECSqlStatus::Success, navPropBinder["Id"].BindId(info1Key.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, navPropBinder["RelECClassId"].BindId(elementOwnsPhysicalElementsClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey));

    stmt.Reset();
    stmt.ClearBindings();

    validateInsert(insertWasValid, ecdb, newKey.GetInstanceId(), info1Key.GetInstanceId(), elementOwnsPhysicalElementsClassId);
    ASSERT_TRUE(insertWasValid);

    //now with omitting input
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Physical-3", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(2, info1Key.GetInstanceId(), ECClassId())) << stmt.GetECSql();
    //ECDb cannot enforce the rel class id to be set. It is the responsibility of the caller to do that
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey)) << stmt.GetECSql();
    stmt.Reset();
    stmt.ClearBindings();
    validateInsert(insertWasValid, ecdb, newKey.GetInstanceId(), info1Key.GetInstanceId(), ECClassId());
    ASSERT_TRUE(insertWasValid);

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Physical-4", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(2, ECInstanceId(), ECClassId())) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey)) << stmt.GetECSql();
    stmt.Reset();
    stmt.ClearBindings();
    validateInsert(insertWasValid, ecdb, newKey.GetInstanceId(), ECInstanceId(), ECClassId());
    ASSERT_TRUE(insertWasValid);

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Physical-5", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey)) << "Not calling BindNavigationValue at all";
    stmt.Reset();
    stmt.ClearBindings();
    validateInsert(insertWasValid, ecdb, newKey.GetInstanceId(), ECInstanceId(), ECClassId());

    //wrong nav id
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Physical-6", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(2, ECInstanceId(info1Key.GetInstanceId().GetValue() + 1000), elementOwnsPhysicalElementsClassId)) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_CONSTRAINT_FOREIGNKEY, stmt.Step(newKey)) << stmt.GetECSql();
    stmt.Reset();
    stmt.ClearBindings();

    //wrong rel class id
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Physical-7", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(2, info1Key.GetInstanceId(), ECClassId(elementOwnsPhysicalElementsClassId.GetValue() + 10000))) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey)) << stmt.GetECSql();
    stmt.Reset();
    stmt.ClearBindings();

    validateInsert(insertWasValid, ecdb, newKey.GetInstanceId(), info1Key.GetInstanceId(), ECClassId(elementOwnsPhysicalElementsClassId.GetValue() + 10000));
    ASSERT_TRUE(insertWasValid);
    }

    //expanding ECSQL syntax
    {
    ECInstanceKey newKey;
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.PhysicalElement(Code,Parent.Id,Parent.RelECClassId) VALUES(?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Physical-2-1", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, info1Key.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, elementOwnsPhysicalElementsClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey));

    stmt.Reset();
    stmt.ClearBindings();

    bool insertWasValid = false;
    validateInsert(insertWasValid, ecdb, newKey.GetInstanceId(), info1Key.GetInstanceId(), elementOwnsPhysicalElementsClassId);
    ASSERT_TRUE(insertWasValid);

    //now with omitting input
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Physical-2-2", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, info1Key.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, ECClassId()));
    //ECDb cannot enforce the rel class id to be set. It is the responsibility of the caller to do that
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey)) << stmt.GetECSql();
    stmt.Reset();
    stmt.ClearBindings();
    validateInsert(insertWasValid, ecdb, newKey.GetInstanceId(), info1Key.GetInstanceId(), ECClassId());
    ASSERT_TRUE(insertWasValid);

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Physical-2-3", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, info1Key.GetInstanceId()));
    //ECDb cannot enforce the rel class id to be set. It is the responsibility of the caller to do that
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey)) << stmt.GetECSql();
    stmt.Reset();
    stmt.ClearBindings();
    validateInsert(insertWasValid, ecdb, newKey.GetInstanceId(), info1Key.GetInstanceId(), ECClassId());
    ASSERT_TRUE(insertWasValid);

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Physical-2-4", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey)) << stmt.GetECSql();
    stmt.Reset();
    stmt.ClearBindings();
    validateInsert(insertWasValid, ecdb, newKey.GetInstanceId(), ECInstanceId(), ECClassId());
    ASSERT_TRUE(insertWasValid);

    //wrong nav id
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Physical-2-5", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, ECInstanceId(info1Key.GetInstanceId().GetValue() + 1000)));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, elementOwnsPhysicalElementsClassId));
    ASSERT_EQ(BE_SQLITE_CONSTRAINT_FOREIGNKEY, stmt.Step(newKey)) << stmt.GetECSql();
    stmt.Reset();
    stmt.ClearBindings();

    //wrong rel class id -> This doesn't fail, nor is there any validation happening -> apps must ensure this
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Physical-2-6", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, info1Key.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, ECClassId(elementOwnsPhysicalElementsClassId.GetValue() + 10000)));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey)) << stmt.GetECSql();
    stmt.Reset();
    stmt.ClearBindings();

    validateInsert(insertWasValid, ecdb, newKey.GetInstanceId(), info1Key.GetInstanceId(), ECClassId(elementOwnsPhysicalElementsClassId.GetValue() + 10000));
    ASSERT_TRUE(insertWasValid);
    }

    //expanding ECSQL syntax with literals
    {
    ECInstanceKey newKey;

    Utf8String ecsql;
    ecsql.Sprintf("INSERT INTO ts.PhysicalElement(Code,Parent.Id,Parent.RelECClassId) VALUES('Physical-3-1','%s','%s')",
                  info1Key.GetInstanceId().ToString().c_str(), elementOwnsPhysicalElementsClassId.ToString().c_str());

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str())) << ecsql.c_str();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey)) << stmt.GetECSql();
    stmt.Finalize();

    bool insertWasValid = false;
    validateInsert(insertWasValid, ecdb, newKey.GetInstanceId(), info1Key.GetInstanceId(), elementOwnsPhysicalElementsClassId);
    ASSERT_TRUE(insertWasValid);

    //now with omitting input
    ecsql.Sprintf("INSERT INTO ts.PhysicalElement(Code,Parent.Id) VALUES('Physical-3-2','%s')",
                  info1Key.GetInstanceId().ToString().c_str());
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str())) << ecsql.c_str();
    //ECDb cannot enforce the rel class id to be set. It is the responsibility of the caller to do that
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey)) << stmt.GetECSql();
    stmt.Finalize();
    validateInsert(insertWasValid, ecdb, newKey.GetInstanceId(), info1Key.GetInstanceId(), ECClassId());
    ASSERT_TRUE(insertWasValid);

    ecsql.Sprintf("INSERT INTO ts.PhysicalElement(Code,Parent.Id,Parent.RelECClassId) VALUES('Physical-3-3','%s',NULL)",
                  info1Key.GetInstanceId().ToString().c_str());
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str())) << ecsql.c_str();
    //ECDb cannot enforce the rel class id to be set. It is the responsibility of the caller to do that
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey)) << stmt.GetECSql();
    stmt.Finalize();
    validateInsert(insertWasValid, ecdb, newKey.GetInstanceId(), info1Key.GetInstanceId(), ECClassId());
    ASSERT_TRUE(insertWasValid);

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.PhysicalElement(Code) VALUES('Physical-3-4')"));
    //Parent is not mandatory
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey)) << stmt.GetECSql();
    stmt.Finalize();
    validateInsert(insertWasValid, ecdb, newKey.GetInstanceId(), ECInstanceId(), ECClassId());
    ASSERT_TRUE(insertWasValid);

    ecsql.Sprintf("INSERT INTO ts.PhysicalElement(Code,Parent.RelECClassId) VALUES('Physical-3-5','%s')",
                  elementOwnsPhysicalElementsClassId.ToString().c_str());
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str())) << ecsql.c_str();
    //ECDb cannot check whether rel class id is set without actual id
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey)) << stmt.GetECSql();
    stmt.Finalize();
    validateInsert(insertWasValid, ecdb, newKey.GetInstanceId(), ECInstanceId(), ECClassId());
    ASSERT_TRUE(insertWasValid);

    ecsql.Sprintf("INSERT INTO ts.PhysicalElement(Code,Parent.Id, Parent.RelECClassId) VALUES('Physical-3-6',NULL,'%s')",
                  elementOwnsPhysicalElementsClassId.ToString().c_str());
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str())) << ecsql.c_str();
    //ECDb cannot check whether rel class id is set without actual id
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey)) << stmt.GetECSql();
    stmt.Finalize();
    validateInsert(insertWasValid, ecdb, newKey.GetInstanceId(), ECInstanceId(), ECClassId());
    ASSERT_TRUE(insertWasValid);

    //wrong nav id
    ecsql.Sprintf("INSERT INTO ts.PhysicalElement(Code,Parent.Id,Parent.RelECClassId) VALUES('Physical-3-7','%s','%s')",
                  ECInstanceId(info1Key.GetInstanceId().GetValue() + 1000).ToString().c_str(),
                  elementOwnsPhysicalElementsClassId.ToString().c_str());
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str())) << ecsql.c_str();
    ASSERT_EQ(BE_SQLITE_CONSTRAINT_FOREIGNKEY, stmt.Step(newKey)) << stmt.GetECSql();
    stmt.Finalize();

    //wrong rel class id -> This doesn't fail, nor is there any validation happening -> apps must ensure this
    ecsql.Sprintf("INSERT INTO ts.PhysicalElement(Code,Parent.Id,Parent.RelECClassId) VALUES('Physical-3-8','%s','%s')",
                  info1Key.GetInstanceId().ToString().c_str(),
                  ECClassId(elementOwnsPhysicalElementsClassId.GetValue() + 10000).ToString().c_str());
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str())) << ecsql.c_str();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey)) << stmt.GetECSql();
    stmt.Finalize();
    validateInsert(insertWasValid, ecdb, newKey.GetInstanceId(), info1Key.GetInstanceId(), ECClassId(elementOwnsPhysicalElementsClassId.GetValue() + 10000));
    ASSERT_TRUE(insertWasValid);
    }

    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 11/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlNavigationPropertyTestFixture, GetValueWithOptionalRelClassId)
    {
    ECDbCR ecdb = SetupECDb("ecsqlnavpropsupport.ecdb",
                            SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                       "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                       "<ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                                       "    <ECEntityClass typeName='Model'>"
                                       "        <ECProperty propertyName='Name' typeName='string' />"
                                       "    </ECEntityClass>"
                                       "    <ECEntityClass typeName='Element' modifier='Abstract'>"
                                       "        <ECCustomAttributes>"
                                       "            <ClassMap xmlns='ECDbMap.02.00'>"
                                       "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                       "            </ClassMap>"
                                       "        </ECCustomAttributes>"
                                       "        <ECProperty propertyName='Code' typeName='string' />"
                                       "        <ECNavigationProperty propertyName='Model' relationshipName='ModelHasElements' direction='Backward' />"
                                       "    </ECEntityClass>"
                                       "    <ECEntityClass typeName='InfoElement'>"
                                       "        <BaseClass>Element</BaseClass>"
                                       "        <ECProperty propertyName='InfoTag' typeName='string' />"
                                       "    </ECEntityClass>"
                                       "    <ECEntityClass typeName='PhysicalElement'>"
                                       "        <BaseClass>Element</BaseClass>"
                                       "        <ECProperty propertyName='Geometry' typeName='string' />"
                                       "    </ECEntityClass>"
                                       "   <ECRelationshipClass typeName='ModelHasElements' strength='Embedding'  modifier='Sealed'>"
                                       "      <Source multiplicity='(1..1)' polymorphic='False' roleLabel='Model'>"
                                       "          <Class class ='Model' />"
                                       "      </Source>"
                                       "      <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Element'>"
                                       "          <Class class ='Element' />"
                                       "      </Target>"
                                       "   </ECRelationshipClass>"
                                       "</ECSchema>"));
    ASSERT_TRUE(ecdb.IsDbOpen());
    ECClassId modelHasElementsClassId = ecdb.Schemas().GetClassId("TestSchema", "ModelHasElements");
    ASSERT_TRUE(modelHasElementsClassId.IsValid());

    ECInstanceKey modelKey;
    ECInstanceKey elementKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.Model(Name) VALUES('MainModel')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(modelKey));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.InfoElement(Code,Model) VALUES('Info-1',?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(1, modelKey.GetInstanceId(), modelHasElementsClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(elementKey));
    }

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT Model FROM ts.InfoElement WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, elementKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    ASSERT_FALSE(stmt.IsValueNull(0)) << stmt.GetECSql();
    ASSERT_TRUE(stmt.GetColumnInfo(0).GetDataType().IsNavigation());
    ASSERT_TRUE(stmt.GetColumnInfo(0).GetProperty() != nullptr && stmt.GetColumnInfo(0).GetProperty()->GetIsNavigation());
    
    ECClassId actualRelClassId;
    ECInstanceId actualModelId = stmt.GetValueNavigation<ECInstanceId>(0, &actualRelClassId);
    ASSERT_EQ(modelKey.GetInstanceId().GetValue(), actualModelId.GetValueUnchecked()) << stmt.GetECSql();
    ASSERT_EQ(modelHasElementsClassId.GetValue(), actualRelClassId.GetValueUnchecked()) << stmt.GetECSql();

    //make use of default parameter in GetValueNavigation
    actualModelId = stmt.GetValueNavigation<ECInstanceId>(0);
    ASSERT_EQ(modelKey.GetInstanceId().GetValue(), actualModelId.GetValueUnchecked()) << stmt.GetECSql();

    //alternative API via struct value
    IECSqlValue const& navValue = stmt.GetValue(0);
    ASSERT_STREQ("Model.Id", navValue["Id"].GetColumnInfo().GetPropertyPath().ToString().c_str()) << stmt.GetECSql();
    ASSERT_EQ(modelKey.GetInstanceId().GetValue(), navValue["Id"].GetId<ECInstanceId>().GetValueUnchecked()) << stmt.GetECSql();
    ASSERT_STREQ("Model.RelECClassId", navValue["RelECClassId"].GetColumnInfo().GetPropertyPath().ToString().c_str()) << stmt.GetECSql();
    ASSERT_EQ(modelHasElementsClassId.GetValue(), navValue["RelECClassId"].GetId<ECClassId>().GetValueUnchecked()) << stmt.GetECSql();
    stmt.Finalize();

    //Nav prop in where clause
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECInstanceId FROM ts.InfoElement WHERE Model.Id=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, modelKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    ASSERT_EQ(elementKey.GetInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValueUnchecked()) << stmt.GetECSql();
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, ECInstanceId(modelKey.GetInstanceId().GetValue() + 1000))) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNull(1)) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();

    stmt.Finalize();

    //expanding syntax
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT Model.Id, Model.RelECClassId FROM ts.InfoElement WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, elementKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    ASSERT_FALSE(stmt.IsValueNull(0)) << stmt.GetECSql();
    ASSERT_FALSE(stmt.IsValueNull(1)) << stmt.GetECSql();
    ASSERT_EQ(modelKey.GetInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue()) << stmt.GetECSql();
    ASSERT_EQ(modelHasElementsClassId.GetValue(), stmt.GetValueId<ECClassId>(1).GetValue()) << stmt.GetECSql();
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 11/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlNavigationPropertyTestFixture, GetValueWithMandatoryRelClassId)
    {
    ECDbCR ecdb = SetupECDb("ecsqlnavpropsupport.ecdb",
                            SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                       "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                       "<ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                                       "    <ECEntityClass typeName='Element' modifier='Abstract'>"
                                       "        <ECCustomAttributes>"
                                       "            <ClassMap xmlns='ECDbMap.02.00'>"
                                       "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                       "            </ClassMap>"
                                       "        </ECCustomAttributes>"
                                       "        <ECProperty propertyName='Code' typeName='string' />"
                                       "        <ECNavigationProperty propertyName='Parent' relationshipName='ElementOwnsChildElements' direction='Backward' />"
                                       "    </ECEntityClass>"
                                       "    <ECEntityClass typeName='InfoElement'>"
                                       "        <BaseClass>Element</BaseClass>"
                                       "        <ECProperty propertyName='InfoTag' typeName='string' />"
                                       "    </ECEntityClass>"
                                       "    <ECEntityClass typeName='PhysicalElement'>"
                                       "        <BaseClass>Element</BaseClass>"
                                       "        <ECProperty propertyName='Geometry' typeName='string' />"
                                       "    </ECEntityClass>"
                                       "   <ECRelationshipClass typeName='ElementOwnsChildElements' strength='Embedding'  modifier='Abstract'>"
                                       "      <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Parent Element'>"
                                       "          <Class class ='Element' />"
                                       "      </Source>"
                                       "      <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Child Element'>"
                                       "          <Class class ='Element' />"
                                       "      </Target>"
                                       "   </ECRelationshipClass>"
                                       "   <ECRelationshipClass typeName='ElementOwnsPhysicalElements' strength='Embedding'  modifier='Sealed'>"
                                       "        <BaseClass>ElementOwnsChildElements</BaseClass>"
                                       "      <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Owner Element'>"
                                       "          <Class class ='Element' />"
                                       "      </Source>"
                                       "      <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Owned PhysicalElement'>"
                                       "          <Class class ='PhysicalElement' />"
                                       "      </Target>"
                                       "   </ECRelationshipClass>"
                                       "</ECSchema>"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    ECClassId elementOwnsPhysicalElementsClassId = ecdb.Schemas().GetClassId("TestSchema", "ElementOwnsPhysicalElements");
    ASSERT_TRUE(elementOwnsPhysicalElementsClassId.IsValid());

    ECInstanceKey parentKey;
    ECInstanceKey elementWithParentKey, elementWithoutParentKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.InfoElement(Code) VALUES('Info-1')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(parentKey));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.PhysicalElement(Code,Parent) VALUES(?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Physical-1", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(2, parentKey.GetInstanceId(), elementOwnsPhysicalElementsClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(elementWithParentKey));
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Physical-2", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNull(2));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(elementWithoutParentKey));
    }

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT Parent FROM ts.Element WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, elementWithParentKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    ASSERT_FALSE(stmt.IsValueNull(0)) << stmt.GetECSql();

    ASSERT_TRUE(stmt.GetValue(0).GetColumnInfo().GetDataType().IsNavigation());
    ASSERT_TRUE(stmt.GetValue(0).GetColumnInfo().GetProperty()->GetIsNavigation());

    ECClassId actualRelClassId;
    ECInstanceId actualParentId = stmt.GetValueNavigation<ECInstanceId>(0, &actualRelClassId);
    ASSERT_EQ(parentKey.GetInstanceId().GetValue(), actualParentId.GetValueUnchecked()) << stmt.GetECSql();
    ASSERT_EQ(elementOwnsPhysicalElementsClassId.GetValue(), actualRelClassId.GetValueUnchecked()) << stmt.GetECSql();

    //alternative API via struct value
    IECSqlValue const& navValue = stmt.GetValue(0);
    ASSERT_STREQ("Parent.Id", navValue["Id"].GetColumnInfo().GetPropertyPath().ToString().c_str()) << stmt.GetECSql();
    ASSERT_EQ(parentKey.GetInstanceId().GetValue(), navValue["Id"].GetId<ECInstanceId>().GetValueUnchecked()) << stmt.GetECSql();
    ASSERT_STREQ("Parent.RelECClassId", navValue["RelECClassId"].GetColumnInfo().GetPropertyPath().ToString().c_str()) << stmt.GetECSql();
    ASSERT_EQ(elementOwnsPhysicalElementsClassId.GetValue(), navValue["RelECClassId"].GetId<ECClassId>().GetValueUnchecked()) << stmt.GetECSql();

    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, elementWithoutParentKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    ASSERT_TRUE(stmt.IsValueNull(0)) << stmt.GetECSql();
    actualParentId = stmt.GetValueNavigation<ECInstanceId>(0, &actualRelClassId);
    ASSERT_FALSE(actualParentId.IsValid());
    ASSERT_FALSE(actualRelClassId.IsValid());
    stmt.Finalize();

    //Nav prop in where clause
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECInstanceId FROM ts.Element WHERE Parent.Id=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, parentKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    ASSERT_EQ(elementWithParentKey.GetInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValueUnchecked()) << stmt.GetECSql();
    //only one row expected
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, ECInstanceId(parentKey.GetInstanceId().GetValue() + 1000))) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNull(1)) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    stmt.Finalize();

    //expanding syntax
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT Parent.Id, Parent.RelECClassId FROM ts.Element WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, elementWithParentKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    ASSERT_FALSE(stmt.IsValueNull(0)) << stmt.GetECSql();
    ASSERT_FALSE(stmt.IsValueNull(1)) << stmt.GetECSql();
    ASSERT_EQ(parentKey.GetInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue()) << stmt.GetECSql();
    ASSERT_EQ(elementOwnsPhysicalElementsClassId.GetValue(), stmt.GetValueId<ECClassId>(1).GetValue()) << stmt.GetECSql();

    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, elementWithoutParentKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    ASSERT_TRUE(stmt.IsValueNull(0)) << stmt.GetECSql();
    ASSERT_TRUE(stmt.IsValueNull(1)) << stmt.GetECSql();
    ASSERT_FALSE(stmt.GetValueId<ECInstanceId>(0).IsValid());
    ASSERT_FALSE(stmt.GetValueId<ECClassId>(1).IsValid());
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 11/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlNavigationPropertyTestFixture, Null)
    {
    ECDbR ecdb = SetupECDb("ecsqlnavpropsupport.ecdb",
                            SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                       "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                       "<ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                                       "    <ECEntityClass typeName='Model'>"
                                       "        <ECProperty propertyName='Name' typeName='string' />"
                                       "    </ECEntityClass>"
                                       "    <ECEntityClass typeName='Element' modifier='Abstract'>"
                                       "        <ECCustomAttributes>"
                                       "         <ClassMap xmlns='ECDbMap.02.00'>"
                                       "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                       "            </ClassMap>"
                                       "        </ECCustomAttributes>"
                                       "        <ECProperty propertyName='Code' typeName='string' />"
                                       "        <ECNavigationProperty propertyName='Model' relationshipName='ModelHasElements' direction='Backward' />"
                                       "        <ECNavigationProperty propertyName='Parent' relationshipName='ElementOwnsChildElements' direction='Backward' />"
                                       "    </ECEntityClass>"
                                       "    <ECEntityClass typeName='SubElement'>"
                                       "        <BaseClass>Element</BaseClass>"
                                       "        <ECProperty propertyName='SubProp1' typeName='int' />"
                                       "    </ECEntityClass>"
                                       "   <ECRelationshipClass typeName='ModelHasElements' strength='Embedding'  modifier='Sealed'>"
                                        "        <ECCustomAttributes>"
                                        "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                        "        </ECCustomAttributes>"
                                       "      <Source multiplicity='(0..1)' polymorphic='False' roleLabel='Model'>"
                                       "          <Class class ='Model' />"
                                       "      </Source>"
                                       "      <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Element'>"
                                       "          <Class class ='Element' />"
                                       "      </Target>"
                                       "   </ECRelationshipClass>"
                                       "   <ECRelationshipClass typeName='ElementOwnsChildElements' strength='Embedding'  modifier='Abstract'>"
                                        "        <ECCustomAttributes>"
                                        "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                        "        </ECCustomAttributes>"
                                       "      <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Parent Element'>"
                                       "          <Class class ='Element' />"
                                       "      </Source>"
                                       "      <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Child Element'>"
                                       "          <Class class ='Element' />"
                                       "      </Target>"
                                       "   </ECRelationshipClass>"
                                       "   <ECRelationshipClass typeName='ElementOwnsSubElements' strength='Embedding'  modifier='Sealed'>"

                                       "        <BaseClass>ElementOwnsChildElements</BaseClass>"
                                       "      <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Owner Element'>"
                                       "          <Class class ='Element' />"
                                       "      </Source>"
                                       "      <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Owned SubElement'>"
                                       "          <Class class ='SubElement' />"
                                       "      </Target>"
                                       "   </ECRelationshipClass>"
                                       "</ECSchema>"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    ECClassId modelHasElementslementRelClassId = ecdb.Schemas().GetClassId("TestSchema", "ModelHasElements");
    ASSERT_TRUE(modelHasElementslementRelClassId.IsValid());

    ECClassId elementOwnsSubElementRelClassId = ecdb.Schemas().GetClassId("TestSchema", "ElementOwnsSubElements");
    ASSERT_TRUE(elementOwnsSubElementRelClassId.IsValid());

    ECInstanceKey modelKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.Model(Name) VALUES('Main')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(modelKey));
    stmt.Finalize();
    }

    ECInstanceKey element1Key, element2Key, element3Key;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.SubElement(Code, Model, Parent) VALUES(?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "1", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(element1Key));
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "2", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(2, modelKey.GetInstanceId(), ECClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(element2Key));
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "3", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(3, element1Key.GetInstanceId(), elementOwnsSubElementRelClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(element3Key));
    }

    ecdb.SaveChanges();

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT count(*) FROM ts.SubElement WHERE Model IS NULL"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    ASSERT_EQ(2, stmt.GetValueInt(0)) << stmt.GetECSql();
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT count(*) FROM ts.SubElement WHERE Parent IS NULL"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    ASSERT_EQ(2, stmt.GetValueInt(0)) << stmt.GetECSql();
    stmt.Finalize();


    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT Model, Model.Id ModelId, Model.RelECClassId ModelRelECClassId, Parent, Parent.Id ParentId, Parent.RelECClassId ParentRelECClassId FROM ts.SubElement WHERE ECInstanceId=?"));

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, element1Key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << "Element1";

    ASSERT_TRUE(stmt.IsValueNull(0)) << stmt.GetECSql();
    ECClassId relClassId;
    ASSERT_FALSE(stmt.GetValueNavigation<ECInstanceId>(0, &relClassId).IsValid()) << "Select clause item 0 in: " << stmt.GetECSql();
    ASSERT_FALSE(relClassId.IsValid()) << "Select clause item 0 in: " << stmt.GetECSql();

    ASSERT_TRUE(stmt.IsValueNull(1)) << stmt.GetECSql();
    ASSERT_TRUE(stmt.IsValueNull(2)) << stmt.GetECSql();
    ASSERT_TRUE(stmt.IsValueNull(3)) << stmt.GetECSql();

    relClassId.Invalidate();
    ASSERT_FALSE(stmt.GetValueNavigation<ECInstanceId>(3, &relClassId).IsValid()) << "Select clause item 3 in: " << stmt.GetECSql();
    ASSERT_FALSE(relClassId.IsValid()) << "Select clause item 3 in: " << stmt.GetECSql();

    ASSERT_TRUE(stmt.IsValueNull(4)) << stmt.GetECSql();
    ASSERT_TRUE(stmt.IsValueNull(5)) << stmt.GetECSql();


    
    stmt.ClearBindings();
    stmt.Reset();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, element2Key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << "Element2";

    relClassId.Invalidate();
    ASSERT_EQ(modelKey.GetInstanceId(), stmt.GetValueNavigation<ECInstanceId>(0, &relClassId)) << stmt.GetECSql();
    ASSERT_EQ(modelHasElementslementRelClassId, relClassId) << stmt.GetECSql();
    ASSERT_EQ(modelKey.GetInstanceId(), stmt.GetValueId<ECInstanceId>(1)) << stmt.GetECSql();
    ASSERT_EQ(modelHasElementslementRelClassId, stmt.GetValueId<ECClassId>(2)) << stmt.GetECSql();

    ASSERT_TRUE(stmt.IsValueNull(3)) << stmt.GetECSql();
    ASSERT_TRUE(stmt.IsValueNull(4)) << stmt.GetECSql();
    ASSERT_TRUE(stmt.IsValueNull(5)) << stmt.GetECSql();
    ASSERT_FALSE(stmt.GetValueNavigation<ECInstanceId>(3, &relClassId).IsValid()) << "Select clause item 3 in: " << stmt.GetECSql();
    ASSERT_FALSE(relClassId.IsValid()) << "Select clause item 3 in: " << stmt.GetECSql();

    stmt.ClearBindings();
    stmt.Reset();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, element3Key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << "Element3";

    relClassId.Invalidate();
    ASSERT_TRUE(stmt.IsValueNull(0)) << stmt.GetECSql();
    ASSERT_FALSE(stmt.GetValueNavigation<ECInstanceId>(0, &relClassId).IsValid()) << "Select clause item 0 in: " << stmt.GetECSql();
    ASSERT_FALSE(relClassId.IsValid()) << "Select clause item 0 in: " << stmt.GetECSql();

    ASSERT_TRUE(stmt.IsValueNull(1)) << stmt.GetECSql();
    ASSERT_TRUE(stmt.IsValueNull(2)) << stmt.GetECSql();

    relClassId.Invalidate();
    ASSERT_EQ(element1Key.GetInstanceId(), stmt.GetValueNavigation<ECInstanceId>(3, &relClassId)) << stmt.GetECSql();
    ASSERT_EQ(elementOwnsSubElementRelClassId, relClassId) << stmt.GetECSql();
    ASSERT_EQ(element1Key.GetInstanceId(), stmt.GetValueId<ECInstanceId>(4)) << stmt.GetECSql();
    ASSERT_EQ(elementOwnsSubElementRelClassId, stmt.GetValueId<ECClassId>(5)) << stmt.GetECSql();
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "UPDATE ts.SubElement SET Model=? WHERE Model IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(1, modelKey.GetInstanceId(), ECClassId())) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    stmt.Finalize();
    ASSERT_EQ(2, ecdb.GetModifiedRowCount());
    {
    ECSqlStatement verifyStmt;
    ASSERT_EQ(ECSqlStatus::Success, verifyStmt.Prepare(ecdb, "SELECT NULL FROM ts.SubElement WHERE Model IS NULL"));
    ASSERT_EQ(BE_SQLITE_DONE, verifyStmt.Step()) << verifyStmt.GetECSql();
    }

    ASSERT_EQ(BE_SQLITE_OK, ecdb.AbandonChanges());

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "DELETE FROM ts.SubElement WHERE Model IS NULL"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    stmt.Finalize();
    ASSERT_EQ(1, ecdb.GetModifiedRowCount()) << "Actually 2 are deleted, but one per FK constraint action which Db::GetModifiedRowCount does not return.";

    {
    ECSqlStatement verifyStmt;
    ASSERT_EQ(ECSqlStatus::Success, verifyStmt.Prepare(ecdb, "SELECT NULL FROM ts.SubElement WHERE Model IS NULL"));
    ASSERT_EQ(BE_SQLITE_DONE, verifyStmt.Step()) << verifyStmt.GetECSql();
    }

    ASSERT_EQ(BE_SQLITE_OK, ecdb.AbandonChanges());


    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "UPDATE ts.SubElement SET Parent=? WHERE Parent IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(1, element1Key.GetInstanceId(), elementOwnsSubElementRelClassId)) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    stmt.Finalize();
    ASSERT_EQ(2, ecdb.GetModifiedRowCount());
    {
    ECSqlStatement verifyStmt;
    ASSERT_EQ(ECSqlStatus::Success, verifyStmt.Prepare(ecdb, "SELECT NULL FROM ts.SubElement WHERE Parent IS NULL"));
    ASSERT_EQ(BE_SQLITE_DONE, verifyStmt.Step()) << verifyStmt.GetECSql();
    }

    ASSERT_EQ(BE_SQLITE_OK, ecdb.AbandonChanges());

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "DELETE FROM ts.SubElement WHERE Parent IS NULL"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    stmt.Finalize();
    ASSERT_EQ(2, ecdb.GetModifiedRowCount());

    {
    ECSqlStatement verifyStmt;
    ASSERT_EQ(ECSqlStatus::Success, verifyStmt.Prepare(ecdb, "SELECT NULL FROM ts.SubElement WHERE Parent IS NULL"));
    ASSERT_EQ(BE_SQLITE_DONE, verifyStmt.Step()) << verifyStmt.GetECSql();
    }

    ASSERT_EQ(BE_SQLITE_OK, ecdb.AbandonChanges());
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlNavigationPropertyTestFixture, CRUD)
    {
    const int rowCount = 3;
    ECDbR ecdb = SetupECDb("ecsqlnavpropsupport.ecdb",
              SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                         "<ECSchema schemaName='TestSchema' alias='np' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                         "<ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                         "    <ECEntityClass typeName='DgnModel'>"
                         "        <ECProperty propertyName='Name' typeName='string' />"
                         "    </ECEntityClass>"
                         "    <ECEntityClass typeName='DgnElement'>"
                         "        <ECProperty propertyName='Code' typeName='string' />"
                         "        <ECNavigationProperty propertyName='Model' relationshipName='ParentHasChildren' direction='Backward' />"
                         "    </ECEntityClass>"
                         "   <ECRelationshipClass typeName='ParentHasChildren' strength='Referencing'  modifier='Sealed'>"
                         "      <Source multiplicity='(0..1)' polymorphic='False' roleLabel='Model'>"
                         "          <Class class ='DgnModel' />"
                         "      </Source>"
                         "      <Target multiplicity='(0..*)' polymorphic='False' roleLabel='Element'>"
                         "          <Class class ='DgnElement' />"
                         "      </Target>"
                         "   </ECRelationshipClass>"
                         "</ECSchema>"), rowCount);

    ASSERT_TRUE(ecdb.IsDbOpen());

    ECClassId parentHasChildrenRelClassId = ecdb.Schemas().GetClassId("TestSchema", "ParentHasChildren");
    ASSERT_TRUE(parentHasChildrenRelClassId.IsValid());

    ECInstanceKey modelKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECClassId, ECInstanceId FROM np.DgnModel LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    modelKey = ECInstanceKey(stmt.GetValueId<ECClassId>(0), stmt.GetValueId<ECInstanceId>(1));
    ASSERT_TRUE(modelKey.IsValid());
    }

    //INSERT with nav props (various ways)
    ECInstanceKey elementKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO np.DgnElement(Model.Id,Code) VALUES(?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, modelKey.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(2, "TestCode-1", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(elementKey));

    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO np.DgnElement(Model.Id, Model.RelECClassId,Code) VALUES(?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, modelKey.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, parentHasChildrenRelClassId));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(3, "TestCode-2", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();
    //with unbound rel class id
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, modelKey.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(3, "TestCode-3", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    ecdb.SaveChanges();
    }

    //verify relationship was inserted
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT SourceECInstanceId,SourceECClassId,ECClassId FROM np.ParentHasChildren WHERE TargetECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, elementKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(modelKey.GetInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue());
    ASSERT_EQ(modelKey.GetClassId().GetValue(), stmt.GetValueId<ECClassId>(1).GetValue());
    ASSERT_EQ(parentHasChildrenRelClassId.GetValue(), stmt.GetValueId<ECClassId>(2).GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT TargetECInstanceId,TargetECClassId,ECClassId FROM np.ParentHasChildren WHERE SourceECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, modelKey.GetInstanceId()));
    int actualRowCount = 0;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        ASSERT_EQ(parentHasChildrenRelClassId.GetValue(), stmt.GetValueId<ECClassId>(2).GetValue());
        actualRowCount++;
        }
    ASSERT_EQ(3, actualRowCount);
    }

    //select from class with navprops (various ways)
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT Code FROM np.DgnElement WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, elementKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT Code, Model.Id, Model.RelECClassId FROM np.DgnElement WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, elementKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(modelKey.GetInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(1).GetValue());
    ASSERT_EQ(parentHasChildrenRelClassId.GetValue(), stmt.GetValueId<ECClassId>(2).GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT Code, Model FROM np.DgnElement WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, elementKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    IECSqlValue const& modelVal = stmt.GetValue(1);
    ASSERT_TRUE(modelVal.GetColumnInfo().GetDataType().IsNavigation());
    ASSERT_TRUE(modelVal.GetColumnInfo().GetProperty()->GetIsNavigation());

    IECSqlValue const& modelIdVal = modelVal["Id"];
    ASSERT_STRCASEEQ("Model.Id", modelIdVal.GetColumnInfo().GetPropertyPath().ToString().c_str());
    ASSERT_TRUE(modelIdVal.GetColumnInfo().GetDataType().IsPrimitive());
    ASSERT_EQ(PRIMITIVETYPE_Long, modelIdVal.GetColumnInfo().GetDataType().GetPrimitiveType());
    ASSERT_EQ(modelKey.GetInstanceId().GetValue(), modelIdVal.GetId<ECInstanceId>().GetValue());

    IECSqlValue const& modelRelClassIdVal = modelVal["RelECClassId"];
    ASSERT_STRCASEEQ("Model.RelECClassId", modelRelClassIdVal.GetColumnInfo().GetPropertyPath().ToString().c_str());
    ASSERT_TRUE(modelRelClassIdVal.GetColumnInfo().GetDataType().IsPrimitive());
    ASSERT_EQ(PRIMITIVETYPE_Long, modelRelClassIdVal.GetColumnInfo().GetDataType().GetPrimitiveType());
    ASSERT_EQ(parentHasChildrenRelClassId.GetValue(), modelRelClassIdVal.GetId<ECClassId>().GetValue());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT * FROM np.DgnElement WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, elementKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    int navPropIx = -1;
    for (int i = 0; i < stmt.GetColumnCount(); i++)
        {
        if (stmt.GetColumnInfo(i).GetDataType().IsNavigation())
            {
            navPropIx = i;
            break;
            }
        }
    ASSERT_TRUE(navPropIx >= 0);
    ASSERT_EQ(modelKey.GetInstanceId().GetValue(), stmt.GetValue(navPropIx)["Id"].GetId<ECInstanceId>().GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    //Nav prop in where clause
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECInstanceId FROM np.DgnElement WHERE Model.Id=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, modelKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(elementKey.GetInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue());
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(elementKey.GetInstanceId().GetValue() + 1, stmt.GetValueId<ECInstanceId>(0).GetValue());
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(elementKey.GetInstanceId().GetValue() + 2, stmt.GetValueId<ECInstanceId>(0).GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    //with literal values
    Utf8String ecsql;
    ecsql.Sprintf("SELECT ECInstanceId FROM np.DgnElement WHERE Model.Id=%s", modelKey.GetInstanceId().ToString().c_str());
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(elementKey.GetInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue());
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(elementKey.GetInstanceId().GetValue() + 1, stmt.GetValueId<ECInstanceId>(0).GetValue());
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(elementKey.GetInstanceId().GetValue() + 2, stmt.GetValueId<ECInstanceId>(0).GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    //Nav prop in order by clause
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT * FROM np.DgnElement ORDER BY Model.Id"));

    int actualRowCount = 0;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        actualRowCount++;
        }
    //one element was inserted after the setup, therefore rowCount+1 is the expected value
    ASSERT_EQ(rowCount+3, actualRowCount) << stmt.GetECSql();
    stmt.Finalize();

    //Nav prop in order by clause
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT count(*), Model.Id FROM np.DgnElement GROUP BY Model.Id"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    stmt.Finalize();
    }
    }


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlNavigationPropertyTestFixture, ECInstanceAdapter)
    {
    const int rowCount = 3;
    ECDbR ecdb = SetupECDb("ecsqlnavpropsupport.ecdb",
                           SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                      "<ECSchema schemaName='np' alias='np' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                      "<ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                                      "    <ECEntityClass typeName='DgnModel'>"
                                      "        <ECProperty propertyName='Name' typeName='string' />"
                                      "    </ECEntityClass>"
                                      "    <ECEntityClass typeName='DgnElement'>"
                                      "        <ECProperty propertyName='Code' typeName='string' />"
                                      "        <ECNavigationProperty propertyName='Model' relationshipName='ParentHasChildren' direction='Backward' />"
                                      "    </ECEntityClass>"
                                      "   <ECRelationshipClass typeName='ParentHasChildren' strength='Referencing'  modifier='Sealed'>"
                                      "      <Source multiplicity='(0..1)' polymorphic='False' roleLabel='Model'>"
                                      "          <Class class ='DgnModel' />"
                                      "      </Source>"
                                      "      <Target multiplicity='(0..*)' polymorphic='False' roleLabel='Element'>"
                                      "          <Class class ='DgnElement' />"
                                      "      </Target>"
                                      "   </ECRelationshipClass>"
                                      "</ECSchema>"), rowCount);

    ASSERT_TRUE(ecdb.IsDbOpen());

    ECClassCP relClassGen = ecdb.Schemas().GetClass("np", "ParentHasChildren");
    ASSERT_TRUE(relClassGen != nullptr && relClassGen->IsRelationshipClass());
    ECRelationshipClassCR relClass = *relClassGen->GetRelationshipClassCP();

    ECInstanceKey modelKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECClassId, ECInstanceId FROM np.DgnModel LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    modelKey = ECInstanceKey(stmt.GetValueId<ECClassId>(0), stmt.GetValueId<ECInstanceId>(1));
    ASSERT_TRUE(modelKey.IsValid());
    }

    ECClassCP elementClass = ecdb.Schemas().GetClass("np", "DgnElement");
    ASSERT_TRUE(elementClass != nullptr);

    ECInstanceKey elementKey;
    {
    ECInstanceInserter elementInserter(ecdb, *elementClass, nullptr);
    ASSERT_TRUE(elementInserter.IsValid());

    IECInstancePtr elementInst = elementClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ASSERT_EQ(ECObjectsStatus::Success, elementInst->SetValue("Code", ECValue("TestCode-1", true)));
    ASSERT_EQ(ECObjectsStatus::Success, elementInst->SetValue("Model", ECValue(modelKey.GetInstanceId(), &relClass)));

    ASSERT_EQ(BE_SQLITE_OK, elementInserter.Insert(elementKey, *elementInst));
    ecdb.SaveChanges();
    }

    //verify relationship was inserted
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT SourceECInstanceId,SourceECClassId FROM np.ParentHasChildren WHERE TargetECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, elementKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(modelKey.GetInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue());
    ASSERT_EQ(modelKey.GetClassId().GetValue(), stmt.GetValueId<ECClassId>(1).GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT TargetECInstanceId,TargetECClassId FROM np.ParentHasChildren WHERE SourceECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, modelKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(elementKey.GetInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue());
    ASSERT_EQ(elementKey.GetClassId().GetValue(), stmt.GetValueId<ECClassId>(1).GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    ECSqlStatement selStmt;
    ASSERT_EQ(ECSqlStatus::Success, selStmt.Prepare(ecdb, "SELECT ECInstanceId, ECClassId, Code, Model FROM np.DgnElement"));

    ECInstanceECSqlSelectAdapter selAdapter(selStmt);
    bool verifiedElementWithSetNavProp = false;
    while (selStmt.Step() == BE_SQLITE_ROW)
        {
        IECInstancePtr inst = selAdapter.GetInstance();
        ASSERT_TRUE(inst != nullptr);
        ECInstanceId id;
        ASSERT_EQ(SUCCESS, ECInstanceId::FromString(id, inst->GetInstanceId().c_str()));
        if (elementKey.GetInstanceId() == id)
            {
            verifiedElementWithSetNavProp = true;

            ECClassId actualRelClassId;
            ASSERT_EQ(modelKey.GetInstanceId().GetValue(), selStmt.GetValueNavigation<ECInstanceId>(3, &actualRelClassId).GetValue()) << "Model via plain ECSQL";
            ASSERT_EQ(relClass.GetId().GetValue(), actualRelClassId.GetValue()) << "Model via plain ECSQL";

            ECValue v;
            ASSERT_EQ(ECObjectsStatus::Success, inst->GetValue(v, "Model"));
            ASSERT_FALSE(v.IsNull()) << "Model is not expected to be null in the read ECInstance";
            ECValue::NavigationInfo const& navInfo = v.GetNavigationInfo();
            ASSERT_EQ(modelKey.GetInstanceId(), navInfo.GetId<ECInstanceId>()) << "Model via ECInstance";
            ASSERT_EQ(relClass.GetId().GetValue(), navInfo.GetRelationshipClassId().GetValue()) << "Model via ECInstance";
            }
        else
            {
            ASSERT_TRUE(selStmt.IsValueNull(3)) << "Model via plain ECSQL";

            ECValue v;
            ASSERT_EQ(ECObjectsStatus::Success, inst->GetValue(v, "Model"));
            ASSERT_TRUE(v.IsNull());
            }
        }

    ASSERT_TRUE(verifiedElementWithSetNavProp);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlNavigationPropertyTestFixture, JsonAdapter)
    {
    const int rowCount = 3;
    ECDbR ecdb = SetupECDb("ecsqlnavpropsupport.ecdb",
                            SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                        "<ECSchema schemaName='np' alias='np' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                        "<ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                                        "    <ECEntityClass typeName='DgnModel'>"
                                        "        <ECProperty propertyName='Name' typeName='string' />"
                                        "    </ECEntityClass>"
                                        "    <ECEntityClass typeName='DgnElement'>"
                                        "        <ECProperty propertyName='Code' typeName='string' />"
                                        "        <ECNavigationProperty propertyName='Model' relationshipName='ParentHasChildren' direction='Backward' />"
                                        "    </ECEntityClass>"
                                        "   <ECRelationshipClass typeName='ParentHasChildren' strength='Referencing'  modifier='Sealed'>"
                                        "      <Source multiplicity='(0..1)' polymorphic='False' roleLabel='Model'>"
                                        "          <Class class ='DgnModel' />"
                                        "      </Source>"
                                        "      <Target multiplicity='(0..*)' polymorphic='False' roleLabel='Element'>"
                                        "          <Class class ='DgnElement' />"
                                        "      </Target>"
                                        "   </ECRelationshipClass>"
                                        "</ECSchema>"), rowCount);

    ASSERT_TRUE(ecdb.IsDbOpen());

    ECInstanceKey modelKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECClassId, ECInstanceId FROM np.DgnModel LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    modelKey = ECInstanceKey(stmt.GetValueId<ECClassId>(0), stmt.GetValueId<ECInstanceId>(1));
    ASSERT_TRUE(modelKey.IsValid());
    }

    ECClassCP elementClass = ecdb.Schemas().GetClass("np", "DgnElement");
    ASSERT_TRUE(elementClass != nullptr);

    ECInstanceKey elementKey;
    {
    JsonInserter elementInserter(ecdb, *elementClass, nullptr);
    ASSERT_TRUE(elementInserter.IsValid());

    Utf8String newElementJsonStr;
    newElementJsonStr.Sprintf("{\"Code\":\"TestCode-1\","
                              " \"Model\":{\"id\":\"%s\"}}", modelKey.GetInstanceId().ToString().c_str());

    rapidjson::Document newElementJson;
    ASSERT_FALSE(newElementJson.Parse<0>(newElementJsonStr.c_str()).HasParseError());

    ASSERT_EQ(BE_SQLITE_OK, elementInserter.Insert(elementKey, newElementJson));
    ecdb.SaveChanges();
    }
    //verify relationship was inserted
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT SourceECInstanceId,SourceECClassId FROM np.ParentHasChildren WHERE TargetECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, elementKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(modelKey.GetInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue());
    ASSERT_EQ(modelKey.GetClassId().GetValue(), stmt.GetValueId<ECClassId>(1).GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT TargetECInstanceId,TargetECClassId FROM np.ParentHasChildren WHERE SourceECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, modelKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(elementKey.GetInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue());
    ASSERT_EQ(elementKey.GetClassId().GetValue(), stmt.GetValueId<ECClassId>(1).GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    ECSqlStatement selStmt;
    ASSERT_EQ(ECSqlStatus::Success, selStmt.Prepare(ecdb, "SELECT ECInstanceId, Code, Model FROM np.DgnElement"));

    JsonECSqlSelectAdapter selAdapter(selStmt);
    bool verifiedElementWithSetNavProp = false;
    while (selStmt.Step() == BE_SQLITE_ROW)
        {
        Json::Value json;
        ASSERT_TRUE(selAdapter.GetRowInstance(json));

        Utf8CP idStr = json["$ECInstanceId"].asCString();
        ECInstanceId id;
        ASSERT_EQ(SUCCESS, ECInstanceId::FromString(id, idStr));
        if (elementKey.GetInstanceId() == id)
            {
            verifiedElementWithSetNavProp = true;

            ASSERT_EQ(modelKey.GetInstanceId(), selStmt.GetValueNavigation<ECInstanceId>(2)) << "Model via plain ECSQL";

            Json::Value const& modelJson = json["Model"];
            ASSERT_FALSE(modelJson.isNull()) << "Model is not expected to be null in the read ECInstance";
            Json::Value const& modelIdJson = modelJson["Id"];
            ASSERT_FALSE(modelIdJson.isNull()) << "Model.Id is not expected to be null in the read ECInstance";
            ASSERT_EQ(modelKey.GetInstanceId().GetValue(), BeJsonUtilities::Int64FromValue(modelIdJson));
            }
        else
            {
            ASSERT_TRUE(selStmt.IsValueNull(2)) << "Model via plain ECSQL";
            ASSERT_TRUE(json["Model"].isNull()) << "Model is not expected to be null in the read ECInstance";
            }
        }

    ASSERT_TRUE(verifiedElementWithSetNavProp);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlNavigationPropertyTestFixture, JoinedTable)
    {
    ECDbR ecdb = SetupECDb("ecsqlnavpropsupport_joinedtable.ecdb",
                           SchemaItem(
                               R"xml(<?xml version='1.0' encoding='utf-8'?>
                                <ECSchema schemaName='TestSchema' alias='np' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                                    <ECSchemaReference name='CoreCustomAttributes' version='01.00' alias='CoreCA' />
                                    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />
                                    <ECEntityClass typeName='DgnCategory'>
                                        <ECProperty propertyName='Name' typeName='string' />
                                    </ECEntityClass>
                                    <ECEntityClass typeName='IGeometrySource' modifier='Abstract'>
                                        <ECCustomAttributes>
                                            <IsMixin xmlns='CoreCustomAttributes.01.00'>
                                                <AppliesToEntityClass>Element</AppliesToEntityClass>
                                            </IsMixin>
                                        </ECCustomAttributes>
                                        <ECProperty propertyName='Geometry' typeName='binary' />
                                        <ECNavigationProperty propertyName='Category' relationshipName='GeometryIsInsCategory' direction='Forward' />
                                    </ECEntityClass>
                                    <ECEntityClass typeName='IGeometrySource3d' modifier='Abstract'>
                                        <ECCustomAttributes>
                                            <IsMixin xmlns='CoreCustomAttributes.01.00'>
                                                <AppliesToEntityClass>Element</AppliesToEntityClass>
                                            </IsMixin>
                                        </ECCustomAttributes>
                                        <BaseClass>IGeometrySource</BaseClass>
                                    </ECEntityClass>
                                    <ECEntityClass typeName='Element' modifier='Abstract'>
                                        <ECCustomAttributes>
                                            <ClassMap xmlns='ECDbMap.02.00'>
                                                <MapStrategy>TablePerHierarchy</MapStrategy>
                                            </ClassMap>
                                            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>
                                        </ECCustomAttributes>
                                        <ECProperty propertyName='Code' typeName='string' />
                                    </ECEntityClass>
                                    <ECEntityClass typeName='SpatialElement' modifier='Abstract'>
                                        <BaseClass>Element</BaseClass>
                                        <BaseClass>IGeometrySource3d</BaseClass>
                                    </ECEntityClass>
                                    <ECEntityClass typeName='AnnotationElement'>
                                        <BaseClass>Element</BaseClass>
                                        <BaseClass>IGeometrySource3d</BaseClass>
                                        <ECProperty propertyName='Text' typeName='string' />
                                    </ECEntityClass>
                                    <ECEntityClass typeName='PhysicalElement'>
                                        <ECCustomAttributes>
                                            <ShareColumns xmlns='ECDbMap.02.00'>
                                                <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>
                                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                                            </ShareColumns>
                                        </ECCustomAttributes>
                                        <BaseClass>SpatialElement</BaseClass>
                                    </ECEntityClass>
                                    <ECEntityClass typeName='FooElement'>
                                        <BaseClass>PhysicalElement</BaseClass>
                                        <ECProperty propertyName='Diameter' typeName='double' />
                                    </ECEntityClass>
                                    <ECEntityClass typeName='SystemElement' modifier='Abstract'>
                                        <BaseClass>Element</BaseClass>
                                    </ECEntityClass>
                                    <ECEntityClass typeName='DictionaryElement'>
                                        <BaseClass>Element</BaseClass>
                                    </ECEntityClass>
                                    <ECRelationshipClass typeName='GeometryIsInsCategory' strength='Referencing' modifier='Sealed'>
                                        <Source multiplicity='(0..*)' polymorphic='True' roleLabel='GeometrySource'>
                                            <Class class ='IGeometrySource' />
                                        </Source>
                                        <Target multiplicity='(1..1)' polymorphic='False' roleLabel='Category'>
                                            <Class class ='DgnCategory' />
                                        </Target>
                                    </ECRelationshipClass>
                                </ECSchema>)xml"), 0);
    ASSERT_TRUE(ecdb.IsDbOpen());
    ECInstanceKey catKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO np.DgnCategory(Name) VALUES('Main')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(catKey));
    ASSERT_TRUE(catKey.IsValid());
    }

    ECInstanceKey fooKey;
    double fooDiameter = 1.1;
    Utf8CP fooCode = "Foo-1";

    //INSERT
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO np.FooElement(Diameter, Code, Category.Id) VALUES(?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(1, fooDiameter));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(2, fooCode, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, catKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(fooKey));
    }

    //Verify via SELECT
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECInstanceId, Code, Category.Id, Diameter FROM np.FooElement"));
    int rowCount = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        rowCount++;

        ECInstanceId currentId = stmt.GetValueId<ECInstanceId>(0);
        if (currentId == fooKey.GetInstanceId())
            {
            ASSERT_STREQ(fooCode, stmt.GetValueText(1));
            ASSERT_EQ(catKey.GetInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(2).GetValue());
            ASSERT_EQ(fooDiameter, stmt.GetValueDouble(3));
            }
        else
            ASSERT_TRUE(stmt.IsValueNull(2));
        }

    ASSERT_GT(rowCount, 0);
    stmt.Finalize();

    //select via base class
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECInstanceId, ECClassId, Category.Id FROM np.IGeometrySource"));
    rowCount = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        rowCount++;

        ECInstanceId currentId = stmt.GetValueId<ECInstanceId>(0);
        if (currentId == fooKey.GetInstanceId())
            {
            ASSERT_EQ(fooKey.GetClassId().GetValue(), stmt.GetValueId<ECClassId>(1).GetValue());
            ASSERT_EQ(catKey.GetInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(2).GetValue());
            }
        else
            ASSERT_TRUE(stmt.IsValueNull(2));
        }

    ASSERT_GT(rowCount, 0);
    }
    ecdb.SaveChanges();
    
    //UPDATE Category.Id
    {
    auto verifyCategoryId = [] (ECDbCR ecdb, ECInstanceId expectedCatId)
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT Category.Id FROM np.FooElement"));
        int rowCount = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
            {
            rowCount++;
            ASSERT_FALSE(stmt.IsValueNull(0));
            ASSERT_EQ(expectedCatId.GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue());
            }

        ASSERT_GT(rowCount, 0);
        stmt.Finalize();

        //select via base class
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT Category.Id FROM np.IGeometrySource"));
        rowCount = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
            {
            rowCount++;
            ASSERT_FALSE(stmt.IsValueNull(0));
            ASSERT_EQ(expectedCatId.GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue());
            }

        ASSERT_GT(rowCount, 0);
        stmt.Finalize();
        };

    int modifiedRowCountBefore = ecdb.GetTotalModifiedRowCount();
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "UPDATE ONLY np.IGeometrySource SET Category.Id=? WHERE Category.Id IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, catKey.GetInstanceId())) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    ASSERT_EQ(0, ecdb.GetTotalModifiedRowCount() - modifiedRowCountBefore) << "Non-polymorphic update against mixin should be no-op";
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "UPDATE np.IGeometrySource SET Category.Id=? WHERE Category.Id IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, catKey.GetInstanceId())) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    stmt.Finalize();
    verifyCategoryId(ecdb, catKey.GetInstanceId());


    //UPDATE via classes that is mapped to a single joined table, is expected to work
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "UPDATE np.SpatialElement SET Category.Id=? WHERE Category.Id IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, catKey.GetInstanceId())) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    stmt.Finalize();
    verifyCategoryId(ecdb, catKey.GetInstanceId());

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "UPDATE np.AnnotationElement SET Category.Id=? WHERE Category.Id IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, catKey.GetInstanceId())) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    stmt.Finalize();
    verifyCategoryId(ecdb, catKey.GetInstanceId());

    auto getCount = [] (ECDbCR ecdb, Utf8CP className, Utf8CP whereClause = nullptr)
        {
        Utf8String ecsql("SELECT count(*) FROM ");
        ecsql.append(className);
        if (whereClause != nullptr)
            ecsql.append(" WHERE ").append(whereClause);

        ECSqlStatement stmt;
        if (ECSqlStatus::Success != stmt.Prepare(ecdb, ecsql.c_str()) || BE_SQLITE_ROW != stmt.Step())
            return -1;

        return stmt.GetValueInt(0);
        };

    int rowCountBefore = getCount(ecdb, "np.IGeometrySource");
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "DELETE FROM ONLY np.IGeometrySource"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    stmt.Finalize();
    ASSERT_EQ(0, rowCountBefore - getCount(ecdb, "np.IGeometrySource")) << "Non-polymorphic delete against mixin should be no-op";

    rowCountBefore = getCount(ecdb, "np.IGeometrySource");
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "DELETE FROM ONLY np.IGeometrySource WHERE Category.Id IS NOT NULL"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    stmt.Finalize();
    ASSERT_EQ(0, rowCountBefore - getCount(ecdb, "np.IGeometrySource")) << "Non-polymorphic delete against mixin should be no-op";

    rowCountBefore = getCount(ecdb, "np.IGeometrySource", "Category.Id IS NOT NULL");
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "DELETE FROM np.IGeometrySource WHERE Category.Id IS NOT NULL"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    stmt.Finalize();
    ASSERT_EQ(1, rowCountBefore - getCount(ecdb, "np.IGeometrySource", "Category.Id IS NOT NULL"));

    ecdb.AbandonChanges();
    }
    }



//---------------------------------------------------------------------------------------
// @bsiclass                                     Affan.Khan                 07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlNavigationPropertyTestFixture, EndTablePolymorphicRelationshipTest)
    {
    ECDbCR ecdb = SetupECDb("ecsqlnavpropsupport.ecdb",
                            SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                        "<ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                                        "    <ECEntityClass typeName='Model'>"
                                        "        <ECProperty propertyName='Name' typeName='string' />"
                                        "    </ECEntityClass>"
                                        "    <ECEntityClass typeName='Element' modifier='Abstract'>"
                                        "        <ECCustomAttributes>"
                                        "         <ClassMap xmlns='ECDbMap.02.00'>"
                                        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                        "            </ClassMap>"
                                        "        </ECCustomAttributes>"
                                        "        <ECProperty propertyName='Code' typeName='string' />"
                                        "        <ECNavigationProperty propertyName='Model' relationshipName='ModelHasElement' direction='Backward' />"
                                        "        <ECNavigationProperty propertyName='Parent' relationshipName='ElementOwnsChildElement' direction='Backward' />"
                                        "    </ECEntityClass>"
                                        "    <ECEntityClass typeName='SubElementA'>"
                                        "        <BaseClass>Element</BaseClass>"
                                        "        <ECProperty propertyName='SubProp1' typeName='int' />"
                                        "    </ECEntityClass>"
                                        "    <ECEntityClass typeName='SubElementB'>"
                                        "        <BaseClass>Element</BaseClass>"
                                        "        <ECProperty propertyName='SubProp2' typeName='int' />"
                                        "    </ECEntityClass>"
                                        "   <ECRelationshipClass typeName='ModelHasElement' strength='Embedding'  modifier='Sealed'>"
                                        "      <Source multiplicity='(1..1)' polymorphic='False' roleLabel='Model'>"
                                        "          <Class class ='Model' />"
                                        "      </Source>"
                                        "      <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Element'>"
                                        "          <Class class ='Element' />"
                                        "      </Target>"
                                        "   </ECRelationshipClass>"
                                        "   <ECRelationshipClass typeName='ElementOwnsChildElement' strength='Embedding'  modifier='Abstract'>"
                                        "      <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Parent Element'>"
                                        "          <Class class ='Element' />"
                                        "      </Source>"
                                        "      <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Child Element'>"
                                        "          <Class class ='Element' />"
                                        "      </Target>"
                                        "   </ECRelationshipClass>"
                                        "   <ECRelationshipClass typeName='ElementOwnsSubElementB' strength='Embedding'  modifier='Abstract'>"
                                        "        <BaseClass>ElementOwnsChildElement</BaseClass>"
                                        "      <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Owner Element'>"
                                        "          <Class class ='Element' />"
                                        "      </Source>"
                                        "      <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Owned SubElement'>"
                                        "          <Class class ='SubElementB' />"
                                        "      </Target>"
                                        "   </ECRelationshipClass>"
                                        "   <ECRelationshipClass typeName='SubElementAOwnsSubElementB' strength='Embedding'  modifier='Sealed'>"
                                        "        <BaseClass>ElementOwnsChildElement</BaseClass>"
                                        "      <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Owner Element'>"
                                        "          <Class class ='SubElementA' />"
                                        "      </Source>"
                                        "      <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Owned SubElement'>"
                                        "          <Class class ='SubElementB' />"
                                        "      </Target>"
                                        "   </ECRelationshipClass>"
                                        "</ECSchema>"));
    ASSERT_TRUE(ecdb.IsDbOpen());        
    ECSqlStatementCache cache(20); 
    CachedECSqlStatementPtr stmt;
    ECClassId elementOwnsSubElementB_ECClassId = ecdb.Schemas().GetClass("TestSchema", "ElementOwnsSubElementB")->GetId();
    ECClassId subElementAOwnsSubElementB_ECClassId = ecdb.Schemas().GetClass("TestSchema", "SubElementAOwnsSubElementB")->GetId();
    ECClassId modelHasElement_ECClassId = ecdb.Schemas().GetClass("TestSchema", "ModelHasElement")->GetId();

    //Add model
    ECInstanceKey modelKey1, modelKey2;
    stmt = cache.GetPreparedStatement(ecdb, "INSERT INTO ts.Model(Name) VALUES (?)");
    ASSERT_EQ (ECSqlStatus::Success, stmt->BindText(1, "MODEL-1", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ (BE_SQLITE_DONE, stmt->Step(modelKey1));

    stmt = cache.GetPreparedStatement(ecdb, "INSERT INTO ts.Model(Name) VALUES (?)");
    ASSERT_EQ(ECSqlStatus::Success, stmt->BindText(1, "MODEL-2", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, stmt->Step(modelKey2));

    ECInstanceKey subElementAKey1, subElementAKey2;
    stmt = cache.GetPreparedStatement(ecdb, "INSERT INTO ts.SubElementA(Code,Model,Parent) VALUES (?,?,?)");
    ASSERT_EQ(ECSqlStatus::Success, stmt->BindText(1, "CODE-A1-M1", IECSqlBinder::MakeCopy::No));   
    ASSERT_EQ(ECSqlStatus::Success, stmt->BindNavigationValue(2, modelKey1.GetInstanceId(), modelHasElement_ECClassId));
    ASSERT_EQ(ECSqlStatus::Success, stmt->BindNull(3));
    ASSERT_EQ(BE_SQLITE_DONE, stmt->Step(subElementAKey1));

    stmt = cache.GetPreparedStatement(ecdb, "INSERT INTO ts.SubElementA(Code,Model,Parent) VALUES (?,?,?)");
    ASSERT_EQ(ECSqlStatus::Success, stmt->BindText(1, "CODE-A2-M2", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt->BindNavigationValue(2, modelKey2.GetInstanceId(), modelHasElement_ECClassId));
    ASSERT_EQ(ECSqlStatus::Success, stmt->BindNull(3));
    ASSERT_EQ(BE_SQLITE_DONE, stmt->Step(subElementAKey2));

    ECInstanceKey subElementBKey1, subElementBKey2, subElementBKey3, subElementBKey4;
    stmt = cache.GetPreparedStatement(ecdb, "INSERT INTO ts.SubElementB(Code,Model,Parent) VALUES (?,?,?)");
    ASSERT_EQ(ECSqlStatus::Success, stmt->BindText(1, "CODE-B1-A1-M1", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt->BindNavigationValue(2, modelKey1.GetInstanceId(), modelHasElement_ECClassId));
    ASSERT_EQ(ECSqlStatus::Success, stmt->BindNavigationValue(3, subElementAKey1.GetInstanceId(), elementOwnsSubElementB_ECClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt->Step(subElementBKey1));

    stmt = cache.GetPreparedStatement(ecdb, "INSERT INTO ts.SubElementB(Code,Model,Parent) VALUES (?,?,?)");
    ASSERT_EQ(ECSqlStatus::Success, stmt->BindText(1, "CODE-B2-A2-M2", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt->BindNavigationValue(2, modelKey2.GetInstanceId(), modelHasElement_ECClassId));
    ASSERT_EQ(ECSqlStatus::Success, stmt->BindNavigationValue(3, subElementAKey2.GetInstanceId(), subElementAOwnsSubElementB_ECClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt->Step(subElementBKey2));

    stmt = cache.GetPreparedStatement(ecdb, "INSERT INTO ts.SubElementB(Code,Model,Parent) VALUES (?,?,?)");
    ASSERT_EQ(ECSqlStatus::Success, stmt->BindText(1, "CODE-B3-A1-M1", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt->BindNavigationValue(2, modelKey1.GetInstanceId(), modelHasElement_ECClassId));
    ASSERT_EQ(ECSqlStatus::Success, stmt->BindNavigationValue(3, subElementAKey1.GetInstanceId(), elementOwnsSubElementB_ECClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt->Step(subElementBKey3));

    stmt = cache.GetPreparedStatement(ecdb, "INSERT INTO ts.SubElementB(Code,Model,Parent) VALUES (?,?,?)");
    ASSERT_EQ(ECSqlStatus::Success, stmt->BindText(1, "CODE-B4-A2-M2", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt->BindNavigationValue(2, modelKey2.GetInstanceId(), modelHasElement_ECClassId));
    ASSERT_EQ(ECSqlStatus::Success, stmt->BindNavigationValue(3, subElementAKey2.GetInstanceId(), subElementAOwnsSubElementB_ECClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt->Step(subElementBKey4));

    stmt = cache.GetPreparedStatement(ecdb, "SELECT COUNT(*) FROM ts.Model");
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_EQ(2, stmt->GetValueInt(0));

    stmt = cache.GetPreparedStatement(ecdb, "SELECT COUNT(*) FROM ts.Element");
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_EQ(6, stmt->GetValueInt(0));

    stmt = cache.GetPreparedStatement(ecdb, "SELECT COUNT(*) FROM ts.Element WHERE Parent.RelECClassId = ?");
    ASSERT_EQ(ECSqlStatus::Success, stmt->BindId(1, elementOwnsSubElementB_ECClassId));
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_EQ(2, stmt->GetValueInt(0));

    stmt = cache.GetPreparedStatement(ecdb, "SELECT COUNT(*) FROM ts.Element WHERE Parent.RelECClassId = ?");
    ASSERT_EQ(ECSqlStatus::Success, stmt->BindId(1, subElementAOwnsSubElementB_ECClassId));
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_EQ(2, stmt->GetValueInt(0));

    stmt = cache.GetPreparedStatement(ecdb, "SELECT COUNT(*) FROM ts.ElementOwnsChildElement");
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_EQ(4, stmt->GetValueInt(0));

    stmt = cache.GetPreparedStatement(ecdb, "SELECT COUNT(*) FROM ONLY ts.ElementOwnsChildElement");
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_EQ(0, stmt->GetValueInt(0));

    stmt = cache.GetPreparedStatement(ecdb, "SELECT COUNT(*) FROM ts.ElementOwnsSubElementB");
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_EQ(2, stmt->GetValueInt(0));

    stmt = cache.GetPreparedStatement(ecdb, "SELECT COUNT(*) FROM ONLY ts.ElementOwnsSubElementB");
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_EQ(2, stmt->GetValueInt(0));

    stmt = cache.GetPreparedStatement(ecdb, "SELECT COUNT(*) FROM ts.SubElementAOwnsSubElementB");
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_EQ(2, stmt->GetValueInt(0));

    stmt = cache.GetPreparedStatement(ecdb, "SELECT COUNT(*) FROM ONLY ts.SubElementAOwnsSubElementB");
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_EQ(2, stmt->GetValueInt(0));

    const_cast<ECDbR>(ecdb).SaveChanges();
    }
END_ECDBUNITTESTS_NAMESPACE

