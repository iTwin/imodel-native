/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

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
            ECSqlStatus stat = stmt.Prepare(m_ecdb, ecsql);
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

    ASSERT_TRUE(m_ecdb.IsDbOpen());

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
            {//Logical FK
            //EC3.1 schema
            ASSERT_EQ(SUCCESS, SetupECDb("ecsqlnavpropsupport.ecdb",
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
                                                    "</ECSchema>")));

            ASSERT_FALSE(m_ecdb.ColumnExists("ts_Element", "ModelRelECClassId"));
            ASSERT_TRUE(m_ecdb.ColumnExists("ts_Element", "ParentRelECClassId"));
            ASSERT_FALSE(GetHelper().IndexExists("ix_ts_Element_ModelRelECClassId")) << "rel class id is virtual -> no index";
            ASSERT_FALSE(GetHelper().IndexExists("ix_ts_Element_ParentRelECClassId")) << "logical FK -> no index";
            }

            {//Physical FK
            //EC3.1 schema
            ASSERT_EQ(SUCCESS, SetupECDb("ecsqlnavpropsupport.ecdb",
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
                                                    "        <ECNavigationProperty propertyName='Model' relationshipName='ModelHasElement' direction='Backward' >"
                                                    "          <ECCustomAttributes>"
                                                    "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                                    "          </ECCustomAttributes>"
                                                    "        </ECNavigationProperty>"
                                                    "        <ECNavigationProperty propertyName='Parent' relationshipName='ElementOwnsChildElement' direction='Backward' >"
                                                    "          <ECCustomAttributes>"
                                                    "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                                    "          </ECCustomAttributes>"
                                                    "        </ECNavigationProperty>"
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
                                                    "</ECSchema>")));

            ASSERT_FALSE(m_ecdb.ColumnExists("ts_Element", "ModelRelECClassId"));
            ASSERT_TRUE(m_ecdb.ColumnExists("ts_Element", "ParentRelECClassId"));
            ASSERT_FALSE(GetHelper().IndexExists("ix_ts_Element_ModelRelECClassId"));
            Utf8CP indexName = "ix_ts_Element_ParentRelECClassId";
            ASSERT_STRCASEEQ(IndexInfo(indexName, false, "ts_Element", "ParentRelECClassId", IndexInfo::WhereClause(true, {"ParentRelECClassId"})).ToDdl().c_str(),
                             GetHelper().GetIndexDdl(indexName).c_str()) << indexName;
            }

            {
            //EC3.0 schema
            ASSERT_EQ(SUCCESS, SetupECDb("ecsqlnavpropsupport.ecdb",
                 SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                <ECSchema schemaName="Workspace" nameSpacePrefix="WS" version="02.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.0">
                    <ECSchemaReference name="ECDbMap" version="02.00" prefix="ecdbmap"/>
                    <ECEntityClass typeName="Repository" modifier="Sealed">
                        <ECCustomAttributes>
                            <DbIndexList xmlns="ECDbMap.02.00">
                                <Indexes>
                                    <DbIndex>
                                        <Name>Repository_Alias_Index</Name>
                                        <IsUnique>True</IsUnique>
                                        <Properties>
                                            <string>Alias</string>
                                        </Properties>
                                    </DbIndex>
                                </Indexes>
                            </DbIndexList>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Type" typeName="int"/>
                        <ECProperty propertyName="Alias" typeName="string"/>
                        <ECProperty propertyName="Label" typeName="string"/>
                    </ECEntityClass>
                    <ECEntityClass typeName="View" modifier="Sealed">
                        <ECCustomAttributes>
                            <DbIndexList xmlns="ECDbMap.02.00">
                                <Indexes>
                                    <DbIndex>
                                        <Name>View_Alias_Index</Name>
                                        <IsUnique>True</IsUnique>
                                        <Properties>
                                            <string>Alias</string>
                                        </Properties>
                                    </DbIndex>
                                </Indexes>
                            </DbIndexList>
                        </ECCustomAttributes>
                        <ECNavigationProperty propertyName="Repository" relationshipName="RepositoryToView" direction="backward" />
                        <ECProperty propertyName="Label" typeName="string"/>
                        <ECProperty propertyName="Alias" typeName="string" />
                    </ECEntityClass>
                    <ECRelationshipClass typeName="RepositoryToView" strength="embedding" modifier="Sealed">
                        <Source cardinality="(1,1)" polymorphic="true">
                            <Class class="Repository"/>
                        </Source>
                        <Target cardinality="(0,N)" polymorphic="true">
                            <Class class="View"/>
                        </Target>
                    </ECRelationshipClass>
                </ECSchema>)xml")));

            ASSERT_EQ(ExpectedColumns({ExpectedColumn("WS_View","RepositoryId"), ExpectedColumn("WS_View","RepositoryRelECClassId", Virtual::Yes)}),
                                       GetHelper().GetPropertyMapColumns(AccessString("WS", "View", "Repository")));
            }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlNavigationPropertyTestFixture, LogicalForeignKeyWithNotNullAndUniqueConstraintsAndSharedCols)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ForeignKeyWithNotNullAndUniqueConstraintsAndSharedCols.ecdb",
                                 SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                        <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                            <ECEntityClass typeName="Model">
                                <ECProperty propertyName="Name" typeName="string" />
                            </ECEntityClass>
                            <ECEntityClass typeName="Tag">
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
                                <ECNavigationProperty propertyName="Tag" relationshipName="ElementHasTag" direction="Backward" />
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
                            <ECRelationshipClass typeName="ElementHasTag" strength="Embedding"  modifier="None">
                                <Source multiplicity="(1..1)" polymorphic="True" roleLabel="Tag">
                                    <Class class ="Tag" />
                              </Source>
                              <Target multiplicity="(0..1)" polymorphic="True" roleLabel="Element">
                                  <Class class ="Element" />
                              </Target>
                           </ECRelationshipClass>
                          </ECSchema>)xml")));

    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().CreateClassViewsInDb());

    ASSERT_EQ(std::vector<Utf8String>({{"ix_ts_Element_ecclassid"}}), GetHelper().GetIndexNamesForTable("ts_Element")) << "Logical FK relationships never create indexes to enforce uniqueness";

    Column modelIdCol = GetHelper().GetPropertyMapColumn(AccessString("ts", "Element", "Model.Id"));
    ASSERT_TRUE(modelIdCol.Exists()) << "PropertyMap column for Element.Model.Id";
    ASSERT_EQ(ExpectedColumn("ts_Element", "ps2"), modelIdCol) << "PropertyMap column for Element.Model.Id";
    ASSERT_FALSE(modelIdCol.GetNotNullConstraint()) << "PropertyMap column for Element.Model.Id: Cardinality implies a NOT NULL, which is not enforced because of shared column";
    ASSERT_FALSE(modelIdCol.GetUniqueConstraint()) << "PropertyMap column for Element.Model.Id: Cardinality is never enforced by a UNIQUE constraint. If needed, a unique index is created";
    ASSERT_FALSE(GetHelper().IsForeignKeyColumn("ts_Element", "ps2")) << "Logical FK must not create FK constraint";

    ASSERT_EQ(ExpectedColumn("ts_Element", "ModelRelECClassId", Virtual::Yes), GetHelper().GetPropertyMapColumn(AccessString("ts", "Element", "Model.RelECClassId")));

    Column parentIdCol = GetHelper().GetPropertyMapColumn(AccessString("ts", "Element", "Parent.Id"));
    ASSERT_TRUE(parentIdCol.Exists()) << "PropertyMap column for Element.Parent.Id";
    ASSERT_EQ(ExpectedColumn("ts_Element", "ps3"), parentIdCol) << "PropertyMap column for Element.Parent.Id";
    ASSERT_FALSE(parentIdCol.GetNotNullConstraint()) << "PropertyMap column for Element.Parent.Id: Cardinality does not imply NOT NULL";
    ASSERT_FALSE(parentIdCol.GetUniqueConstraint()) << "PropertyMap column for Element.Parent.Id: Cardinality is never enforced by a UNIQUE constraint. If needed, a unique index is created";

    ASSERT_FALSE(GetHelper().IsForeignKeyColumn("ts_Element", "ps3")) << "Logical FK must not create FK constraint";

    Column parentRelClassIdCol = GetHelper().GetPropertyMapColumn(AccessString("ts", "Element", "Parent.RelECClassId"));
    ASSERT_TRUE(parentRelClassIdCol.Exists()) << "PropertyMap column for Element.Parent.RelECClassId";
    ASSERT_EQ(ExpectedColumn("ts_Element", "ps4"), parentRelClassIdCol) << "PropertyMap column for Element.Parent.RelECClassId";
    ASSERT_FALSE(parentRelClassIdCol.GetNotNullConstraint()) << "PropertyMap column for Element.Parent.RelECClassId. No constraints expected because of shared col";
    ASSERT_FALSE(parentRelClassIdCol.GetUniqueConstraint()) << "PropertyMap column for Element.Parent.RelECClassId. No constraints expected because of shared col";

    Column tagIdCol = GetHelper().GetPropertyMapColumn(AccessString("ts", "Element", "Tag.Id"));
    ASSERT_TRUE(tagIdCol.Exists()) << "PropertyMap column for Element.Tag.Id";
    ASSERT_EQ(ExpectedColumn("ts_Element", "ps5"), tagIdCol) << "PropertyMap column for Element.Tag.Id";
    ASSERT_FALSE(tagIdCol.GetNotNullConstraint()) << "PropertyMap column for Element.Tag.Id: Cardinality implies a NOT NULL, which is not enforced because of shared column";
    ASSERT_FALSE(tagIdCol.GetUniqueConstraint()) << "PropertyMap column for Element.Tag.Id: Cardinality is never enforced by a UNIQUE constraint. If needed, a unique index is created";
    ASSERT_FALSE(GetHelper().IsForeignKeyColumn("ts_Element", "ps5")) << "Logical FK must not create FK constraint";

    Column tagRelClassIdCol = GetHelper().GetPropertyMapColumn(AccessString("ts", "Element", "Tag.RelECClassId"));
    ASSERT_TRUE(tagRelClassIdCol.Exists()) << "PropertyMap column for Element.Tag.RelECClassId";
    ASSERT_EQ(ExpectedColumn("ts_Element", "ps6"), tagRelClassIdCol) << "PropertyMap column for Element.Tag.RelECClassId";
    ASSERT_FALSE(tagRelClassIdCol.GetNotNullConstraint()) << "PropertyMap column for Element.Tag.RelECClassId. No constraints expected because of shared col";
    ASSERT_FALSE(tagRelClassIdCol.GetUniqueConstraint()) << "PropertyMap column for Element.Tag.RelECClassId. No constraints expected because of shared col";


    ECInstanceKey modelKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(modelKey, "INSERT INTO ts.Model(Name) VALUES('Main')"));

    ECInstanceKey tag1Key, tag2Key;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(tag1Key, "INSERT INTO ts.Tag(Name) VALUES('Tag1')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(tag2Key, "INSERT INTO ts.Tag(Name) VALUES('Tag2')"));

    ECInstanceKey rootElementKey, fooElementKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.SubElement(Code,Model,Parent,Tag) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Root", IECSqlBinder::MakeCopy::No)) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(2, modelKey.GetInstanceId())) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(4, tag1Key.GetInstanceId(), m_ecdb.Schemas().GetClassId("TestSchema", "ElementHasTag"))) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(rootElementKey)) << stmt.GetECSql();
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Foo", IECSqlBinder::MakeCopy::No)) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(2, modelKey.GetInstanceId())) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(3, rootElementKey.GetInstanceId(), m_ecdb.Schemas().GetClassId("TestSchema", "ElementOwnsChildElement"))) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(4, tag2Key.GetInstanceId(), m_ecdb.Schemas().GetClassId("TestSchema", "ElementHasTag"))) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(fooElementKey)) << stmt.GetECSql();
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Foo3", IECSqlBinder::MakeCopy::No)) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(2, modelKey.GetInstanceId())) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(3, rootElementKey.GetInstanceId(), m_ecdb.Schemas().GetClassId("TestSchema", "ElementOwnsChildElement"))) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Tag is mandatory, but not enforced because of Logical FK" << stmt.GetECSql();
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Foo2", IECSqlBinder::MakeCopy::No)) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(2, modelKey.GetInstanceId())) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(3, rootElementKey.GetInstanceId(), m_ecdb.Schemas().GetClassId("TestSchema", "ElementOwnsChildElement"))) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(4, tag2Key.GetInstanceId(), m_ecdb.Schemas().GetClassId("TestSchema", "ElementHasTag"))) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Two elements for tag1 violates the 1:1 cardinality of the 'element has tag' relationship. This is not enforced though because of the logical FK." << stmt.GetECSql();
    }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlNavigationPropertyTestFixture, LogicalForeignKeyWithNotNullAndUniqueConstraintsAndUnsharedCols)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("LogicalForeignKeyWithNotNullAndUniqueConstraintsAndUnsharedCols.ecdb",
                                 SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                        <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                            <ECEntityClass typeName="Model">
                                <ECProperty propertyName="Name" typeName="string" />
                            </ECEntityClass>
                            <ECEntityClass typeName="Tag">
                                <ECProperty propertyName="Name" typeName="string" />
                            </ECEntityClass>
                            <ECEntityClass typeName="Element" modifier="Abstract">
                                <ECCustomAttributes>
                                    <ClassMap xmlns="ECDbMap.02.00">
                                        <MapStrategy>TablePerHierarchy</MapStrategy>
                                    </ClassMap>
                                </ECCustomAttributes>
                                <ECProperty propertyName="Code" typeName="string" />
                                <ECNavigationProperty propertyName="Model" relationshipName="ModelHasElement" direction="Backward" />
                                <ECNavigationProperty propertyName="Parent" relationshipName="ElementOwnsChildElement" direction="Backward" />
                                <ECNavigationProperty propertyName="Tag" relationshipName="ElementHasTag" direction="Backward" />
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
                            <ECRelationshipClass typeName="ElementHasTag" strength="Embedding"  modifier="None">
                                <Source multiplicity="(1..1)" polymorphic="True" roleLabel="Tag">
                                    <Class class ="Tag" />
                              </Source>
                              <Target multiplicity="(0..1)" polymorphic="True" roleLabel="Element">
                                  <Class class ="Element" />
                              </Target>
                           </ECRelationshipClass>
                          </ECSchema>)xml")));

    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().CreateClassViewsInDb());

    EXPECT_EQ(std::vector<Utf8String>({{"ix_ts_Element_ecclassid"}}), GetHelper().GetIndexNamesForTable("ts_Element")) << "Logical FK relationships never create indexes to enforce uniqueness";

    Column modelIdCol = GetHelper().GetPropertyMapColumn(AccessString("ts", "Element", "Model.Id"));
    ASSERT_TRUE(modelIdCol.Exists()) << "PropertyMap column for Element.Model.Id";
    EXPECT_EQ(ExpectedColumn("ts_Element", "ModelId"), modelIdCol) << "PropertyMap column for Element.Model.Id";
    EXPECT_FALSE(modelIdCol.GetNotNullConstraint()) << "PropertyMap column for Element.Model.Id: Cardinality implies a NOT NULL, which is not enforced for logical FKs";
    EXPECT_FALSE(modelIdCol.GetUniqueConstraint()) << "PropertyMap column for Element.Model.Id: Cardinality is never enforced by a UNIQUE constraint, but with a unique index if necessary";
    EXPECT_FALSE(GetHelper().IsForeignKeyColumn("ts_Element", "ModelId")) << "Logical FK must not create FK constraint";

    EXPECT_EQ(ExpectedColumn("ts_Element", "ModelRelECClassId", Virtual::Yes), GetHelper().GetPropertyMapColumn(AccessString("ts", "Element", "Model.RelECClassId")));

    Column parentIdCol = GetHelper().GetPropertyMapColumn(AccessString("ts", "Element", "Parent.Id"));
    ASSERT_TRUE(parentIdCol.Exists()) << "PropertyMap column for Element.Parent.Id";
    EXPECT_EQ(ExpectedColumn("ts_Element", "ParentId"), parentIdCol) << "PropertyMap column for Element.Parent.Id";
    EXPECT_FALSE(parentIdCol.GetNotNullConstraint()) << "PropertyMap column for Element.Parent.Id: Cardinality does not imply NOT NULL";
    EXPECT_FALSE(parentIdCol.GetUniqueConstraint()) << "PropertyMap column for Element.Parent.Id: Cardinality is never enforced by a UNIQUE constraint, but with a unique index if necessary";
    EXPECT_FALSE(GetHelper().IsForeignKeyColumn("ts_Element", "ParentId")) << "Logical FK must not create FK constraint";

    Column parentRelClassIdCol = GetHelper().GetPropertyMapColumn(AccessString("ts", "Element", "Parent.RelECClassId"));
    ASSERT_TRUE(parentRelClassIdCol.Exists()) << "PropertyMap column for Element.Parent.RelECClassId";
    EXPECT_EQ(ExpectedColumn("ts_Element", "ParentRelECClassId"), parentRelClassIdCol) << "PropertyMap column for Element.Parent.RelECClassId";
    EXPECT_FALSE(parentRelClassIdCol.GetNotNullConstraint()) << "PropertyMap column for Element.Parent.RelECClassId";
    EXPECT_FALSE(parentRelClassIdCol.GetUniqueConstraint()) << "PropertyMap column for Element.Parent.RelECClassId";

    Column tagIdCol = GetHelper().GetPropertyMapColumn(AccessString("ts", "Element", "Tag.Id"));
    ASSERT_TRUE(tagIdCol.Exists()) << "PropertyMap column for Element.Tag.Id";
    EXPECT_EQ(ExpectedColumn("ts_Element", "TagId"), tagIdCol) << "PropertyMap column for Element.Tag.Id";
    EXPECT_FALSE(tagIdCol.GetNotNullConstraint()) << "PropertyMap column for Element.Tag.Id: Cardinality implies a NOT NULL which is not enforced though for logical FKs";
    EXPECT_FALSE(tagIdCol.GetUniqueConstraint()) << "PropertyMap column for Element.Tag.Id: Cardinality is never enforced by a UNIQUE constraint, but with a unique index if necessary";
    EXPECT_FALSE(GetHelper().IsForeignKeyColumn("ts_Element", "TagId")) << "Logical FK must not create FK constraint";

    Column tagRelClassIdCol = GetHelper().GetPropertyMapColumn(AccessString("ts", "Element", "Tag.RelECClassId"));
    ASSERT_TRUE(tagRelClassIdCol.Exists()) << "PropertyMap column for Element.Tag.RelECClassId";
    EXPECT_EQ(ExpectedColumn("ts_Element", "TagRelECClassId"), tagRelClassIdCol) << "PropertyMap column for Element.Tag.RelECClassId";
    EXPECT_FALSE(tagRelClassIdCol.GetNotNullConstraint()) << "PropertyMap column for Element.Tag.RelECClassId. Cardinality implies a NOT NULL which is not enforced for logical FKs though";
    EXPECT_FALSE(tagRelClassIdCol.GetUniqueConstraint()) << "PropertyMap column for Element.Tag.RelECClassId";


    ECInstanceKey modelKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(modelKey, "INSERT INTO ts.Model(Name) VALUES('Main')"));

    ECInstanceKey tag1Key, tag2Key;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(tag1Key, "INSERT INTO ts.Tag(Name) VALUES('Tag1')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(tag2Key, "INSERT INTO ts.Tag(Name) VALUES('Tag2')"));

    ECInstanceKey rootElementKey, fooElementKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.SubElement(Code,Model,Parent,Tag) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Root", IECSqlBinder::MakeCopy::No)) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(2, modelKey.GetInstanceId())) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(4, tag1Key.GetInstanceId(), m_ecdb.Schemas().GetClassId("TestSchema", "ElementHasTag"))) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(rootElementKey)) << stmt.GetECSql();
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Foo", IECSqlBinder::MakeCopy::No)) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(2, modelKey.GetInstanceId())) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(3, rootElementKey.GetInstanceId(), m_ecdb.Schemas().GetClassId("TestSchema", "ElementOwnsChildElement"))) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(4, tag2Key.GetInstanceId(), m_ecdb.Schemas().GetClassId("TestSchema", "ElementHasTag"))) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(fooElementKey)) << stmt.GetECSql();
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Foo3", IECSqlBinder::MakeCopy::No)) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(2, modelKey.GetInstanceId())) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(3, rootElementKey.GetInstanceId(), m_ecdb.Schemas().GetClassId("TestSchema", "ElementOwnsChildElement"))) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Tag is mandatory, but not enforced because of Logical FK" << stmt.GetECSql();
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Foo2", IECSqlBinder::MakeCopy::No)) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(2, modelKey.GetInstanceId())) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(3, rootElementKey.GetInstanceId(), m_ecdb.Schemas().GetClassId("TestSchema", "ElementOwnsChildElement"))) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(4, tag2Key.GetInstanceId(), m_ecdb.Schemas().GetClassId("TestSchema", "ElementHasTag"))) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Two elements for tag1 violates the 1:1 cardinality of the 'element has tag' relationship. This is not enforced though because of the logical FK." << stmt.GetECSql();
    }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 07/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlNavigationPropertyTestFixture, PhysicalForeignKeyWithNotNullAndUniqueConstraintsAndUnsharedCols)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("PhysicalForeignKeyWithNotNullAndUniqueConstraintsAndUnsharedCols.ecdb",
                                 SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                        <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                            <ECEntityClass typeName="Model">
                                <ECProperty propertyName="Name" typeName="string" />
                            </ECEntityClass>
                            <ECEntityClass typeName="Tag">
                                <ECProperty propertyName="Name" typeName="string" />
                            </ECEntityClass>
                            <ECEntityClass typeName="Element" modifier="Abstract">
                                <ECCustomAttributes>
                                    <ClassMap xmlns="ECDbMap.02.00">
                                        <MapStrategy>TablePerHierarchy</MapStrategy>
                                    </ClassMap>
                                </ECCustomAttributes>
                                <ECProperty propertyName="Code" typeName="string" />
                                <ECNavigationProperty propertyName="Model" relationshipName="ModelHasElement" direction="Backward">
                                    <ECCustomAttributes>
                                        <ForeignkeyConstraint xmlns="ECDbMap.02.00"/>
                                    </ECCustomAttributes>
                                </ECNavigationProperty>
                                <ECNavigationProperty propertyName="Parent" relationshipName="ElementOwnsChildElement" direction="Backward" >
                                    <ECCustomAttributes>
                                        <ForeignkeyConstraint xmlns="ECDbMap.02.00"/>
                                    </ECCustomAttributes>
                                </ECNavigationProperty>
                                <ECNavigationProperty propertyName="Tag" relationshipName="ElementHasTag" direction="Backward" >
                                    <ECCustomAttributes>
                                        <ForeignkeyConstraint xmlns="ECDbMap.02.00"/>
                                    </ECCustomAttributes>
                                </ECNavigationProperty>
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
                            <ECRelationshipClass typeName="ElementHasTag" strength="Embedding"  modifier="None">
                                <Source multiplicity="(1..1)" polymorphic="True" roleLabel="Tag">
                                    <Class class ="Tag" />
                              </Source>
                              <Target multiplicity="(0..1)" polymorphic="True" roleLabel="Element">
                                  <Class class ="Element" />
                              </Target>
                           </ECRelationshipClass>
                          </ECSchema>)xml")));

    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().CreateClassViewsInDb());

    EXPECT_EQ(std::vector<Utf8String>({{"ix_ts_Element_ecclassid"},
    {"ix_ts_Element_fk_ts_ElementOwnsChildElement_target"},
    {"ix_ts_Element_fk_ts_ModelHasElement_target"},
    {"ix_ts_Element_ParentRelECClassId"},
    {"ix_ts_Element_TagRelECClassId"},
    {"uix_ts_Element_fk_ts_ElementHasTag_target"}}), GetHelper().GetIndexNamesForTable("ts_Element")) << "Physical FK relationships create indexes on FK columns and to enforce uniqueness";

    Column modelIdCol = GetHelper().GetPropertyMapColumn(AccessString("ts", "Element", "Model.Id"));
    ASSERT_TRUE(modelIdCol.Exists()) << "PropertyMap column for Element.Model.Id";
    EXPECT_EQ(ExpectedColumn("ts_Element", "ModelId"), modelIdCol) << "PropertyMap column for Element.Model.Id";
    EXPECT_TRUE(modelIdCol.GetNotNullConstraint()) << "PropertyMap column for Element.Model.Id: Cardinality implies a NOT NULL";
    EXPECT_FALSE(modelIdCol.GetUniqueConstraint()) << "PropertyMap column for Element.Model.Id: Cardinality is never enforced by a UNIQUE constraint, but with a unique index if necessary";
    EXPECT_TRUE(GetHelper().IsForeignKeyColumn("ts_Element", "ModelId")) << "Physical FK must create FK constraint";

    EXPECT_EQ(ExpectedColumn("ts_Element", "ModelRelECClassId", Virtual::Yes), GetHelper().GetPropertyMapColumn(AccessString("ts", "Element", "Model.RelECClassId")));

    Column parentIdCol = GetHelper().GetPropertyMapColumn(AccessString("ts", "Element", "Parent.Id"));
    ASSERT_TRUE(parentIdCol.Exists()) << "PropertyMap column for Element.Parent.Id";
    EXPECT_EQ(ExpectedColumn("ts_Element", "ParentId"), parentIdCol) << "PropertyMap column for Element.Parent.Id";
    EXPECT_FALSE(parentIdCol.GetNotNullConstraint()) << "PropertyMap column for Element.Parent.Id: Cardinality does not imply NOT NULL";
    EXPECT_FALSE(parentIdCol.GetUniqueConstraint()) << "PropertyMap column for Element.Parent.Id: Cardinality is never enforced by a UNIQUE constraint, but with a unique index if necessary";
    EXPECT_TRUE(GetHelper().IsForeignKeyColumn("ts_Element", "ParentId")) << "Physical FK must create FK constraint";

    Column parentRelClassIdCol = GetHelper().GetPropertyMapColumn(AccessString("ts", "Element", "Parent.RelECClassId"));
    ASSERT_TRUE(parentRelClassIdCol.Exists()) << "PropertyMap column for Element.Parent.RelECClassId";
    EXPECT_EQ(ExpectedColumn("ts_Element", "ParentRelECClassId"), parentRelClassIdCol) << "PropertyMap column for Element.Parent.RelECClassId";
    EXPECT_FALSE(parentRelClassIdCol.GetNotNullConstraint()) << "PropertyMap column for Element.Parent.RelECClassId";
    EXPECT_FALSE(parentRelClassIdCol.GetUniqueConstraint()) << "PropertyMap column for Element.Parent.RelECClassId";

    Column tagIdCol = GetHelper().GetPropertyMapColumn(AccessString("ts", "Element", "Tag.Id"));
    ASSERT_TRUE(tagIdCol.Exists()) << "PropertyMap column for Element.Tag.Id";
    EXPECT_EQ(ExpectedColumn("ts_Element", "TagId"), tagIdCol) << "PropertyMap column for Element.Tag.Id";
    EXPECT_TRUE(tagIdCol.GetNotNullConstraint()) << "PropertyMap column for Element.Tag.Id: Cardinality implies a NOT NULL";
    EXPECT_FALSE(tagIdCol.GetUniqueConstraint()) << "PropertyMap column for Element.Tag.Id: Cardinality is never enforced by a UNIQUE constraint, but with a unique index if necessary";
    EXPECT_TRUE(GetHelper().IsForeignKeyColumn("ts_Element", "TagId")) << "Physical FK must create FK constraint";

    Column tagRelClassIdCol = GetHelper().GetPropertyMapColumn(AccessString("ts", "Element", "Tag.RelECClassId"));
    ASSERT_TRUE(tagRelClassIdCol.Exists()) << "PropertyMap column for Element.Tag.RelECClassId";
    EXPECT_EQ(ExpectedColumn("ts_Element", "TagRelECClassId"), tagRelClassIdCol) << "PropertyMap column for Element.Tag.RelECClassId";
    EXPECT_TRUE(tagRelClassIdCol.GetNotNullConstraint()) << "PropertyMap column for Element.Tag.RelECClassId.";
    EXPECT_FALSE(tagRelClassIdCol.GetUniqueConstraint()) << "PropertyMap column for Element.Tag.RelECClassId";


    ECInstanceKey modelKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(modelKey, "INSERT INTO ts.Model(Name) VALUES('Main')"));

    ECInstanceKey tag1Key, tag2Key;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(tag1Key, "INSERT INTO ts.Tag(Name) VALUES('Tag1')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(tag2Key, "INSERT INTO ts.Tag(Name) VALUES('Tag2')"));

    ECInstanceKey rootElementKey, fooElementKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.SubElement(Code,Model,Parent,Tag) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Root", IECSqlBinder::MakeCopy::No)) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(2, modelKey.GetInstanceId())) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(4, tag1Key.GetInstanceId(), m_ecdb.Schemas().GetClassId("TestSchema", "ElementHasTag"))) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(rootElementKey)) << stmt.GetECSql();
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Foo", IECSqlBinder::MakeCopy::No)) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(2, modelKey.GetInstanceId())) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(3, rootElementKey.GetInstanceId(), m_ecdb.Schemas().GetClassId("TestSchema", "ElementOwnsChildElement"))) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(4, tag2Key.GetInstanceId(), m_ecdb.Schemas().GetClassId("TestSchema", "ElementHasTag"))) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(fooElementKey)) << stmt.GetECSql();
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Foo3", IECSqlBinder::MakeCopy::No)) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(2, modelKey.GetInstanceId())) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(3, rootElementKey.GetInstanceId(), m_ecdb.Schemas().GetClassId("TestSchema", "ElementOwnsChildElement"))) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_CONSTRAINT_NOTNULL, stmt.Step()) << "Tag is mandatory" << stmt.GetECSql();
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Foo2", IECSqlBinder::MakeCopy::No)) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(2, modelKey.GetInstanceId())) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(3, rootElementKey.GetInstanceId(), m_ecdb.Schemas().GetClassId("TestSchema", "ElementOwnsChildElement"))) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(4, tag2Key.GetInstanceId(), m_ecdb.Schemas().GetClassId("TestSchema", "ElementHasTag"))) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_CONSTRAINT_UNIQUE, stmt.Step()) << "Two elements for tag1 violates the 1:1 cardinality of the element has tag relationship. " << stmt.GetECSql();
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  05/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlNavigationPropertyTestFixture, Overriding)
    {
    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema1" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
               <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
               <ECEntityClass typeName="Parent" modifier="Sealed">
                   <ECProperty propertyName="Name" typeName="string" />
               </ECEntityClass>
               <ECEntityClass typeName="Child" modifier="Abstract">
                 <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.2.0">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                 </ECCustomAttributes>
                   <ECProperty propertyName="Name" typeName="string" />
                   <ECNavigationProperty propertyName="Parent" relationshipName="ParentHasChild" direction="Backward" />
               </ECEntityClass>
               <ECEntityClass typeName="SubChild" modifier="Abstract">
                   <BaseClass>Child</BaseClass>
                   <ECProperty propertyName="SubProp" typeName="string" />
                   <ECNavigationProperty propertyName="Parent" relationshipName="ParentHasChild" direction="Backward" >
                        <ECCustomAttributes>
                            <ForeignKeyConstraint xmlns="ECDbMap.2.0"/>
                        </ECCustomAttributes>
                   </ECNavigationProperty>
               </ECEntityClass>
              <ECRelationshipClass typeName="ParentHasChild" strength="Referencing"  modifier="None">
                <Source multiplicity="(1..1)" polymorphic="True" roleLabel="Parent Element">
                    <Class class ="Parent" />
                </Source>
                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="Child Element">
                    <Class class ="Child" />
                </Target>
             </ECRelationshipClass>
            </ECSchema>)xml"))) << "Overriding nav prop adds ForeignKeyConstraint CA causes duplicate nav prop error";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema2" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
               <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
               <ECEntityClass typeName="Parent" modifier="Sealed">
                   <ECProperty propertyName="Name" typeName="string" />
               </ECEntityClass>
               <ECEntityClass typeName="Child" modifier="Abstract">
                 <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.2.0">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                 </ECCustomAttributes>
                   <ECProperty propertyName="Name" typeName="string" />
                   <ECNavigationProperty propertyName="Parent" relationshipName="ParentHasChild" direction="Backward" />
               </ECEntityClass>
               <ECEntityClass typeName="SubChild" modifier="Abstract">
                   <BaseClass>Child</BaseClass>
                   <ECProperty propertyName="SubProp" typeName="string" />
                   <ECNavigationProperty propertyName="Parent" relationshipName="ParentHasSubChild" direction="Backward" />
               </ECEntityClass>
              <ECRelationshipClass typeName="ParentHasChild" strength="Referencing" modifier="None">
                <Source multiplicity="(1..1)" polymorphic="True" roleLabel="Parent Element">
                    <Class class ="Parent" />
                </Source>
                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="Child Element">
                    <Class class ="Child" />
                </Target>
             </ECRelationshipClass>
              <ECRelationshipClass typeName="ParentHasSubChild" strength="Referencing"  modifier="None">
                <BaseClass>ParentHasChild</BaseClass>
                <Source multiplicity="(1..1)" polymorphic="True" roleLabel="Parent Element">
                    <Class class ="Parent" />
                </Source>
                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="Child Element">
                    <Class class ="SubChild" />
                </Target>
             </ECRelationshipClass>
            </ECSchema>)xml"))) << "Overriding nav prop changes relationship which is not supported";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema3" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
               <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
               <ECEntityClass typeName="Element" modifier="Abstract">
                 <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.2.0">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                 </ECCustomAttributes>
                   <ECProperty propertyName="Name" typeName="string" />
                   <ECNavigationProperty propertyName="Parent" relationshipName="ElementHasPartnerElement" direction="Backward" />
               </ECEntityClass>
                <ECEntityClass typeName="SubElement" modifier="Abstract">
                   <BaseClass>Element</BaseClass>
                   <ECProperty propertyName="SubProp" typeName="string" />
                   <ECNavigationProperty propertyName="Parent" relationshipName="ElementHasPartnerElement" direction="Forward" />
               </ECEntityClass>
               <ECRelationshipClass typeName="ElementHasPartnerElement" strength="Referencing"  modifier="None">
                <Source multiplicity="(0..1)" polymorphic="True" roleLabel="Parent Element">
                    <Class class ="Element" />
                </Source>
                <Target multiplicity="(0..1)" polymorphic="True" roleLabel="Child Element">
                    <Class class ="Element" />
                </Target>
             </ECRelationshipClass>
            </ECSchema>)xml"))) << "Overriding nav prop changes direction which is not supported";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  05/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlNavigationPropertyTestFixture, ColumnOrder)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("navpropcolumnorder.ecdb", SchemaItem(
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
                   <ECNavigationProperty propertyName="CodeSpec" relationshipName="CodeSpecHasFoo" direction="Backward" >
                        <ECCustomAttributes>
                            <ForeignKeyConstraint xmlns="ECDbMap.2.0"/>
                        </ECCustomAttributes>
                   </ECNavigationProperty>
                   <ECNavigationProperty propertyName="CodeScope" relationshipName="CodeScopeHasFoo" direction="Backward" >
                        <ECCustomAttributes>
                            <ForeignKeyConstraint xmlns="ECDbMap.2.0"/>
                        </ECCustomAttributes>
                   </ECNavigationProperty>
                   <ECProperty propertyName="LastMod" typeName="DateTime" />
               </ECEntityClass>
               <ECEntityClass typeName="HasNavPropsWithoutRelECClassId" modifier="Abstract">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.2.0">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
                   <ECProperty propertyName="Name" typeName="string" />
                   <ECNavigationProperty propertyName="CodeSpec" relationshipName="CodeSpecHasFoo_Sealed" direction="Backward" >
                        <ECCustomAttributes>
                            <ForeignKeyConstraint xmlns="ECDbMap.2.0"/>
                        </ECCustomAttributes>
                   </ECNavigationProperty>
                   <ECNavigationProperty propertyName="CodeScope" relationshipName="CodeScopeHasFoo_Sealed" direction="Backward" >
                        <ECCustomAttributes>
                            <ForeignKeyConstraint xmlns="ECDbMap.2.0"/>
                        </ECCustomAttributes>
                   </ECNavigationProperty>
                   <ECProperty propertyName="LastMod" typeName="DateTime" />
               </ECEntityClass>
             <ECRelationshipClass typeName="CodeSpecHasFoo" strength="Referencing"  modifier="None">
                <Source multiplicity="(1..1)" polymorphic="True" roleLabel="Parent Element">
                    <Class class ="CodeSpec" />
                </Source>
                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="Child Element">
                    <Class class ="HasNavPropsWithRelECClassId" />
                </Target>
             </ECRelationshipClass>
             <ECRelationshipClass typeName="CodeScopeHasFoo" strength="Referencing"  modifier="None">
                <Source multiplicity="(1..1)" polymorphic="True" roleLabel="Parent Element">
                    <Class class ="CodeScope" />
                </Source>
                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="Child Element">
                    <Class class ="HasNavPropsWithRelECClassId" />
                </Target>
             </ECRelationshipClass>
             <ECRelationshipClass typeName="CodeSpecHasFoo_Sealed" strength="Referencing"  modifier="Sealed">
                <Source multiplicity="(1..1)" polymorphic="True" roleLabel="Parent Element">
                    <Class class ="CodeSpec" />
                </Source>
                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="Child Element">
                    <Class class ="HasNavPropsWithoutRelECClassId" />
                </Target>
             </ECRelationshipClass>
             <ECRelationshipClass typeName="CodeScopeHasFoo_Sealed" strength="Referencing"  modifier="Sealed">
                <Source multiplicity="(1..1)" polymorphic="True" roleLabel="Parent Element">
                    <Class class ="CodeScope" />
                </Source>
                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="Child Element">
                    <Class class ="HasNavPropsWithoutRelECClassId" />
                </Target>
             </ECRelationshipClass>
              </ECSchema>)xml")));

    std::vector<Utf8String> v({"Id","ECClassId","Name","CodeSpecId","CodeSpecRelECClassId","CodeScopeId","CodeScopeRelECClassId","LastMod"});
    ASSERT_EQ(v,
              GetHelper().GetColumnNames("ts_HasNavPropsWithRelECClassId"));

    ASSERT_EQ(std::vector<Utf8String>({"Id", "ECClassId", "Name", "CodeSpecId", "CodeScopeId", "LastMod"}), GetHelper().GetColumnNames("ts_HasNavPropsWithoutRelECClassId"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlNavigationPropertyTestFixture, ClearBindings)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("navpropclearbindings.ecdb",
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
                          </ECSchema>)xml")));

    ECClassId elementOwnsElementRelClassId = m_ecdb.Schemas().GetClassId("TestSchema", "ElementOwnsChildElement");
    ASSERT_TRUE(elementOwnsElementRelClassId.IsValid());

    ECInstanceKey rootElementKey, element1Key, element2Key;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.SubElement(Code,Parent) VALUES(?,?)"));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT Code, Parent FROM ts.SubElement"));
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
    ASSERT_EQ(SUCCESS, SetupECDb("NavPropForVirtualRelClassId.ecdb",SchemaItem(
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
        "</ECSchema>")));

    ECSqlStatement stmt;
    ECInstanceKey modelKey;

    ECClassId parentHasChildrenRelClassId = m_ecdb.Schemas().GetClassId("TestSchema", "ParentHasChildren");
    ASSERT_TRUE(parentHasChildrenRelClassId.IsValid());

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.DgnModel(Name) VALUES('TestVal-1')"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, stmt.Step(modelKey));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.DgnElement(Model.Id, Model.RelECClassId,Code) VALUES(?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, modelKey.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, parentHasChildrenRelClassId));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(3, "TestVal-2", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * FROM ts.DgnElement WHERE Model IS NULL"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, stmt.Step());
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 11/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlNavigationPropertyTestFixture, BindingWithOptionalRelClassId)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlnavpropsupport.ecdb",
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
                                       "        <ECNavigationProperty propertyName='Model' relationshipName='ModelHasElements' direction='Backward' >"
                                       "          <ECCustomAttributes>"
                                       "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                       "          </ECCustomAttributes>"
                                       "        </ECNavigationProperty>"
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
                                       "</ECSchema>")));

    ECClassId modelHasElementsClassId = m_ecdb.Schemas().GetClassId("TestSchema", "ModelHasElements");
    ASSERT_TRUE(modelHasElementsClassId.IsValid());

    ECInstanceKey modelKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Model(Name) VALUES('MainModel')"));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.InfoElement(Code,Model) VALUES(?,?)"));

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Info-1", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(2, modelKey.GetInstanceId(), modelHasElementsClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(info1Key));
    stmt.Reset();
    stmt.ClearBindings();

    bool insertWasValid = false;
    validateInsert(insertWasValid, m_ecdb, info1Key.GetInstanceId(), modelKey.GetInstanceId(), modelHasElementsClassId);
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

    validateInsert(insertWasValid, m_ecdb, newKey.GetInstanceId(), modelKey.GetInstanceId(), modelHasElementsClassId);
    ASSERT_TRUE(insertWasValid);

    //now with omitting optional rel class id
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Info-3", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(2, modelKey.GetInstanceId(), ECClassId())) << "RelECClassId is virtual for ModelHasElements, therefore passing it to BindNavigationValue is optional";
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey));
    stmt.Reset();
    stmt.ClearBindings();

    validateInsert(insertWasValid, m_ecdb, newKey.GetInstanceId(), modelKey.GetInstanceId(), modelHasElementsClassId);
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
    ASSERT_EQ(BE_SQLITE_CONSTRAINT_FOREIGNKEY, stmt.Step(newKey)) << m_ecdb.GetLastError().c_str();
    stmt.Reset();
    stmt.ClearBindings();

    //invalid rel class model id -> ignored
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Info-6", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(2, modelKey.GetInstanceId(), ECClassId(modelHasElementsClassId.GetValue() + 1)));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey)) << m_ecdb.GetLastError().c_str();
    stmt.Reset();
    stmt.ClearBindings();
    //wrong rel class id is just ignored. SELECT will return correct one
    validateInsert(insertWasValid, m_ecdb, newKey.GetInstanceId(), modelKey.GetInstanceId(), modelHasElementsClassId);
    ASSERT_TRUE(insertWasValid);
    }

    //expanding ECSQL syntax
    {
    ECInstanceKey newKey;

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.InfoElement(Code,Model.Id,Model.RelECClassId) VALUES(?,?,?)"));

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Info-2-1", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, modelKey.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, modelHasElementsClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey)) << m_ecdb.GetLastError().c_str();
    stmt.Reset();
    stmt.ClearBindings();

    bool insertWasValid = false;
    validateInsert(insertWasValid, m_ecdb, newKey.GetInstanceId(), modelKey.GetInstanceId(), modelHasElementsClassId);
    ASSERT_TRUE(insertWasValid);

    //omit optional rel class id
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Info-2-2", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, modelKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey)) << m_ecdb.GetLastError().c_str();
    stmt.Reset();
    stmt.ClearBindings();

    validateInsert(insertWasValid, m_ecdb, newKey.GetInstanceId(), modelKey.GetInstanceId(), modelHasElementsClassId);
    ASSERT_TRUE(insertWasValid);

    //omit non-optional model id
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Info-2-3", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, modelHasElementsClassId));
    ASSERT_EQ(BE_SQLITE_CONSTRAINT_NOTNULL, stmt.Step(newKey)) << m_ecdb.GetLastError().c_str();
    stmt.Reset();
    stmt.ClearBindings();

    //invalid model id -> FK violation
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Info-2-4", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(2, modelKey.GetInstanceId().GetValue() + 1));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, modelHasElementsClassId));
    ASSERT_EQ(BE_SQLITE_CONSTRAINT_FOREIGNKEY, stmt.Step(newKey)) << m_ecdb.GetLastError().c_str();
    stmt.Reset();
    stmt.ClearBindings();

    //invalid rel class model id -> ignored
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Info-2-5", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, modelKey.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(3, modelHasElementsClassId.GetValue() + 1));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey)) << m_ecdb.GetLastError().c_str();
    stmt.Reset();
    stmt.ClearBindings();
    //wrong rel class id is just ignored. SELECT will return correct one
    validateInsert(insertWasValid, m_ecdb, newKey.GetInstanceId(), modelKey.GetInstanceId(), modelHasElementsClassId);
    ASSERT_TRUE(insertWasValid);
    }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 11/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlNavigationPropertyTestFixture, BindingWithMandatoryRelClassId)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlnavpropsupport.ecdb",
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
                                       "        <ECNavigationProperty propertyName='Parent' relationshipName='ElementOwnsChildElements' direction='Backward' >"
                                       "          <ECCustomAttributes>"
                                       "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                       "          </ECCustomAttributes>"
                                       "        </ECNavigationProperty>"
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
                                       "</ECSchema>")));

    ECClassId elementOwnsPhysicalElementsClassId = m_ecdb.Schemas().GetClassId("TestSchema", "ElementOwnsPhysicalElements");
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.InfoElement(Code) VALUES('Info-1')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(info1Key));
    }

    {
    ECInstanceKey newKey;
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.PhysicalElement(Code,Parent) VALUES(?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Physical-1", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(2, info1Key.GetInstanceId(), elementOwnsPhysicalElementsClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey));

    stmt.Reset();
    stmt.ClearBindings();

    bool insertWasValid = false;
    validateInsert(insertWasValid, m_ecdb, newKey.GetInstanceId(), info1Key.GetInstanceId(), elementOwnsPhysicalElementsClassId);
    ASSERT_TRUE(insertWasValid);

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Physical-2", IECSqlBinder::MakeCopy::No));
    IECSqlBinder& navPropBinder = stmt.GetBinder(2);
    ASSERT_EQ(ECSqlStatus::Success, navPropBinder["Id"].BindId(info1Key.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, navPropBinder["RelECClassId"].BindId(elementOwnsPhysicalElementsClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey));

    stmt.Reset();
    stmt.ClearBindings();

    validateInsert(insertWasValid, m_ecdb, newKey.GetInstanceId(), info1Key.GetInstanceId(), elementOwnsPhysicalElementsClassId);
    ASSERT_TRUE(insertWasValid);

    //now with omitting input
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Physical-3", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(2, info1Key.GetInstanceId(), ECClassId())) << stmt.GetECSql();
    //ECDb cannot enforce the rel class id to be set. It is the responsibility of the caller to do that
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey)) << stmt.GetECSql();
    stmt.Reset();
    stmt.ClearBindings();
    validateInsert(insertWasValid, m_ecdb, newKey.GetInstanceId(), info1Key.GetInstanceId(), ECClassId());
    ASSERT_TRUE(insertWasValid);

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Physical-4", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(2, ECInstanceId(), ECClassId())) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey)) << stmt.GetECSql();
    stmt.Reset();
    stmt.ClearBindings();
    validateInsert(insertWasValid, m_ecdb, newKey.GetInstanceId(), ECInstanceId(), ECClassId());
    ASSERT_TRUE(insertWasValid);

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Physical-5", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey)) << "Not calling BindNavigationValue at all";
    stmt.Reset();
    stmt.ClearBindings();
    validateInsert(insertWasValid, m_ecdb, newKey.GetInstanceId(), ECInstanceId(), ECClassId());

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

    validateInsert(insertWasValid, m_ecdb, newKey.GetInstanceId(), info1Key.GetInstanceId(), ECClassId(elementOwnsPhysicalElementsClassId.GetValue() + 10000));
    ASSERT_TRUE(insertWasValid);
    }

    //expanding ECSQL syntax
    {
    ECInstanceKey newKey;
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.PhysicalElement(Code,Parent.Id,Parent.RelECClassId) VALUES(?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Physical-2-1", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, info1Key.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, elementOwnsPhysicalElementsClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey));

    stmt.Reset();
    stmt.ClearBindings();

    bool insertWasValid = false;
    validateInsert(insertWasValid, m_ecdb, newKey.GetInstanceId(), info1Key.GetInstanceId(), elementOwnsPhysicalElementsClassId);
    ASSERT_TRUE(insertWasValid);

    //now with omitting input
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Physical-2-2", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, info1Key.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, ECClassId()));
    //ECDb cannot enforce the rel class id to be set. It is the responsibility of the caller to do that
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey)) << stmt.GetECSql();
    stmt.Reset();
    stmt.ClearBindings();
    validateInsert(insertWasValid, m_ecdb, newKey.GetInstanceId(), info1Key.GetInstanceId(), ECClassId());
    ASSERT_TRUE(insertWasValid);

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Physical-2-3", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, info1Key.GetInstanceId()));
    //ECDb cannot enforce the rel class id to be set. It is the responsibility of the caller to do that
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey)) << stmt.GetECSql();
    stmt.Reset();
    stmt.ClearBindings();
    validateInsert(insertWasValid, m_ecdb, newKey.GetInstanceId(), info1Key.GetInstanceId(), ECClassId());
    ASSERT_TRUE(insertWasValid);

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Physical-2-4", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey)) << stmt.GetECSql();
    stmt.Reset();
    stmt.ClearBindings();
    validateInsert(insertWasValid, m_ecdb, newKey.GetInstanceId(), ECInstanceId(), ECClassId());
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

    validateInsert(insertWasValid, m_ecdb, newKey.GetInstanceId(), info1Key.GetInstanceId(), ECClassId(elementOwnsPhysicalElementsClassId.GetValue() + 10000));
    ASSERT_TRUE(insertWasValid);
    }

    //expanding ECSQL syntax with literals
    {
    ECInstanceKey newKey;

    Utf8String ecsql;
    ecsql.Sprintf("INSERT INTO ts.PhysicalElement(Code,Parent.Id,Parent.RelECClassId) VALUES('Physical-3-1','%s','%s')",
                  info1Key.GetInstanceId().ToString().c_str(), elementOwnsPhysicalElementsClassId.ToString().c_str());

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey)) << stmt.GetECSql();
    stmt.Finalize();

    bool insertWasValid = false;
    validateInsert(insertWasValid, m_ecdb, newKey.GetInstanceId(), info1Key.GetInstanceId(), elementOwnsPhysicalElementsClassId);
    ASSERT_TRUE(insertWasValid);

    //now with omitting input
    ecsql.Sprintf("INSERT INTO ts.PhysicalElement(Code,Parent.Id) VALUES('Physical-3-2','%s')",
                  info1Key.GetInstanceId().ToString().c_str());
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str();
    //ECDb cannot enforce the rel class id to be set. It is the responsibility of the caller to do that
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey)) << stmt.GetECSql();
    stmt.Finalize();
    validateInsert(insertWasValid, m_ecdb, newKey.GetInstanceId(), info1Key.GetInstanceId(), ECClassId());
    ASSERT_TRUE(insertWasValid);

    ecsql.Sprintf("INSERT INTO ts.PhysicalElement(Code,Parent.Id,Parent.RelECClassId) VALUES('Physical-3-3','%s',NULL)",
                  info1Key.GetInstanceId().ToString().c_str());
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str();
    //ECDb cannot enforce the rel class id to be set. It is the responsibility of the caller to do that
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey)) << stmt.GetECSql();
    stmt.Finalize();
    validateInsert(insertWasValid, m_ecdb, newKey.GetInstanceId(), info1Key.GetInstanceId(), ECClassId());
    ASSERT_TRUE(insertWasValid);

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.PhysicalElement(Code) VALUES('Physical-3-4')"));
    //Parent is not mandatory
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey)) << stmt.GetECSql();
    stmt.Finalize();
    validateInsert(insertWasValid, m_ecdb, newKey.GetInstanceId(), ECInstanceId(), ECClassId());
    ASSERT_TRUE(insertWasValid);

    ecsql.Sprintf("INSERT INTO ts.PhysicalElement(Code,Parent.RelECClassId) VALUES('Physical-3-5','%s')",
                  elementOwnsPhysicalElementsClassId.ToString().c_str());
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str();
    //ECDb cannot check whether rel class id is set without actual id
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey)) << stmt.GetECSql();
    stmt.Finalize();
    validateInsert(insertWasValid, m_ecdb, newKey.GetInstanceId(), ECInstanceId(), ECClassId());
    ASSERT_TRUE(insertWasValid);

    ecsql.Sprintf("INSERT INTO ts.PhysicalElement(Code,Parent.Id, Parent.RelECClassId) VALUES('Physical-3-6',NULL,'%s')",
                  elementOwnsPhysicalElementsClassId.ToString().c_str());
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str();
    //ECDb cannot check whether rel class id is set without actual id
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey)) << stmt.GetECSql();
    stmt.Finalize();
    validateInsert(insertWasValid, m_ecdb, newKey.GetInstanceId(), ECInstanceId(), ECClassId());
    ASSERT_TRUE(insertWasValid);

    //wrong nav id
    ecsql.Sprintf("INSERT INTO ts.PhysicalElement(Code,Parent.Id,Parent.RelECClassId) VALUES('Physical-3-7','%s','%s')",
                  ECInstanceId(info1Key.GetInstanceId().GetValue() + 1000).ToString().c_str(),
                  elementOwnsPhysicalElementsClassId.ToString().c_str());
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str();
    ASSERT_EQ(BE_SQLITE_CONSTRAINT_FOREIGNKEY, stmt.Step(newKey)) << stmt.GetECSql();
    stmt.Finalize();

    //wrong rel class id -> This doesn't fail, nor is there any validation happening -> apps must ensure this
    ecsql.Sprintf("INSERT INTO ts.PhysicalElement(Code,Parent.Id,Parent.RelECClassId) VALUES('Physical-3-8','%s','%s')",
                  info1Key.GetInstanceId().ToString().c_str(),
                  ECClassId(elementOwnsPhysicalElementsClassId.GetValue() + 10000).ToString().c_str());
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey)) << stmt.GetECSql();
    stmt.Finalize();
    validateInsert(insertWasValid, m_ecdb, newKey.GetInstanceId(), info1Key.GetInstanceId(), ECClassId(elementOwnsPhysicalElementsClassId.GetValue() + 10000));
    ASSERT_TRUE(insertWasValid);
    }

    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 11/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlNavigationPropertyTestFixture, GetValueWithOptionalRelClassId)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlnavpropsupport.ecdb",
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
                                       "</ECSchema>")));

    ECClassId modelHasElementsClassId = m_ecdb.Schemas().GetClassId("TestSchema", "ModelHasElements");
    ASSERT_TRUE(modelHasElementsClassId.IsValid());

    ECInstanceKey modelKey;
    ECInstanceKey elementKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Model(Name) VALUES('MainModel')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(modelKey));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.InfoElement(Code,Model) VALUES('Info-1',?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(1, modelKey.GetInstanceId(), modelHasElementsClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(elementKey));
    }

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT Model FROM ts.InfoElement WHERE ECInstanceId=?"));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ts.InfoElement WHERE Model.Id=?"));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT Model.Id, Model.RelECClassId FROM ts.InfoElement WHERE ECInstanceId=?"));
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
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlnavpropsupport.ecdb",
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
                                       "</ECSchema>")));

    ECClassId elementOwnsPhysicalElementsClassId = m_ecdb.Schemas().GetClassId("TestSchema", "ElementOwnsPhysicalElements");
    ASSERT_TRUE(elementOwnsPhysicalElementsClassId.IsValid());

    ECInstanceKey parentKey;
    ECInstanceKey elementWithParentKey, elementWithoutParentKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.InfoElement(Code) VALUES('Info-1')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(parentKey));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.PhysicalElement(Code,Parent) VALUES(?,?)"));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT Parent FROM ts.Element WHERE ECInstanceId=?"));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ts.Element WHERE Parent.Id=?"));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT Parent.Id, Parent.RelECClassId FROM ts.Element WHERE ECInstanceId=?"));
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
// @bsiclass                                     Affan.Khan                 11/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlNavigationPropertyTestFixture, NavigationPropertyFromOverflowTableIsDetectedAsDuplicate)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlnavpropsupport_ec.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                            <ECEntityClass typeName="_From">
                                <ECProperty propertyName="Code" typeName="string" />
                            </ECEntityClass>
                            <ECEntityClass typeName="_To" modifier="Abstract">
                                <ECCustomAttributes>
                                    <ClassMap xmlns="ECDbMap.02.00">
                                        <MapStrategy>TablePerHierarchy</MapStrategy>
                                    </ClassMap>
                                    <ShareColumns xmlns="ECDbMap.02.00">
                                        <MaxSharedColumnsBeforeOverflow>2</MaxSharedColumnsBeforeOverflow>
                                        <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                                    </ShareColumns>
                                </ECCustomAttributes>
                                <ECProperty propertyName="Code" typeName="string" />
                            </ECEntityClass>
                            <ECEntityClass typeName="ClassB1">
                                <BaseClass>_To</BaseClass>
                                <ECProperty propertyName="B1" typeName="string" />
                                <ECNavigationProperty propertyName="_R1" relationshipName="_R1" direction="Backward" />
                            </ECEntityClass>
                            <ECEntityClass typeName="ClassB2">
                                <BaseClass>_To</BaseClass>
                                <ECProperty propertyName="B2" typeName="string" />
                                <ECNavigationProperty propertyName="_R1" relationshipName="_R1" direction="Backward" />
                            </ECEntityClass>
                            <ECRelationshipClass typeName="_R1" strength="referencing"  modifier="None"  strengthDirection="backward" >
                                <Source multiplicity="(0..1)" polymorphic="True" roleLabel="_From">
                                    <Class class ="_From" />
                                </Source>
                                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="_To" abstractConstraint="_To">
                                    <Class class ="ClassB1" />
                                    <Class class ="ClassB2" />
                                </Target>
                            </ECRelationshipClass>
                        </ECSchema>)xml")));
    }
//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 11/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlNavigationPropertyTestFixture, Null)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlnavpropsupport.ecdb",
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
                                       "        <ECNavigationProperty propertyName='Model' relationshipName='ModelHasElements' direction='Backward'>"
                                       "          <ECCustomAttributes>"
                                       "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                       "          </ECCustomAttributes>"
                                       "        </ECNavigationProperty>"
                                       "        <ECNavigationProperty propertyName='Parent' relationshipName='ElementOwnsChildElements' direction='Backward' >"
                                       "          <ECCustomAttributes>"
                                       "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                       "          </ECCustomAttributes>"
                                       "        </ECNavigationProperty>"
                                       "    </ECEntityClass>"
                                       "    <ECEntityClass typeName='SubElement'>"
                                       "        <BaseClass>Element</BaseClass>"
                                       "        <ECProperty propertyName='SubProp1' typeName='int' />"
                                       "    </ECEntityClass>"
                                       "   <ECRelationshipClass typeName='ModelHasElements' strength='Embedding'  modifier='Sealed'>"
                                       "      <Source multiplicity='(0..1)' polymorphic='False' roleLabel='Model'>"
                                       "          <Class class ='Model' />"
                                       "      </Source>"
                                       "      <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Element'>"
                                       "          <Class class ='Element' />"
                                       "      </Target>"
                                       "   </ECRelationshipClass>"
                                       "   <ECRelationshipClass typeName='ElementOwnsChildElements' strength='Embedding'  modifier='Abstract'>"
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
                                       "</ECSchema>")));

    ECClassId modelHasElementslementRelClassId = m_ecdb.Schemas().GetClassId("TestSchema", "ModelHasElements");
    ASSERT_TRUE(modelHasElementslementRelClassId.IsValid());

    ECClassId elementOwnsSubElementRelClassId = m_ecdb.Schemas().GetClassId("TestSchema", "ElementOwnsSubElements");
    ASSERT_TRUE(elementOwnsSubElementRelClassId.IsValid());

    ECInstanceKey modelKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Model(Name) VALUES('Main')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(modelKey));
    stmt.Finalize();
    }

    ECInstanceKey element1Key, element2Key, element3Key;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.SubElement(Code, Model, Parent) VALUES(?,?,?)"));
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

    m_ecdb.SaveChanges();

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT count(*) FROM ts.SubElement WHERE Model IS NULL"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    ASSERT_EQ(2, stmt.GetValueInt(0)) << stmt.GetECSql();
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT count(*) FROM ts.SubElement WHERE Parent IS NULL"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    ASSERT_EQ(2, stmt.GetValueInt(0)) << stmt.GetECSql();
    stmt.Finalize();


    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT Model, Model.Id ModelId, Model.RelECClassId ModelRelECClassId, Parent, Parent.Id ParentId, Parent.RelECClassId ParentRelECClassId FROM ts.SubElement WHERE ECInstanceId=?"));

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

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE ts.SubElement SET Model=? WHERE Model IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(1, modelKey.GetInstanceId(), ECClassId())) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    stmt.Finalize();
    ASSERT_EQ(2, m_ecdb.GetModifiedRowCount());
    {
    ECSqlStatement verifyStmt;
    ASSERT_EQ(ECSqlStatus::Success, verifyStmt.Prepare(m_ecdb, "SELECT NULL FROM ts.SubElement WHERE Model IS NULL"));
    ASSERT_EQ(BE_SQLITE_DONE, verifyStmt.Step()) << verifyStmt.GetECSql();
    }

    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.AbandonChanges());

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "DELETE FROM ts.SubElement WHERE Model IS NULL"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    stmt.Finalize();
    ASSERT_EQ(1, m_ecdb.GetModifiedRowCount()) << "Actually 2 are deleted, but one per FK constraint action which Db::GetModifiedRowCount does not return.";

    {
    ECSqlStatement verifyStmt;
    ASSERT_EQ(ECSqlStatus::Success, verifyStmt.Prepare(m_ecdb, "SELECT NULL FROM ts.SubElement WHERE Model IS NULL"));
    ASSERT_EQ(BE_SQLITE_DONE, verifyStmt.Step()) << verifyStmt.GetECSql();
    }

    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.AbandonChanges());


    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE ts.SubElement SET Parent=? WHERE Parent IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(1, element1Key.GetInstanceId(), elementOwnsSubElementRelClassId)) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    stmt.Finalize();
    ASSERT_EQ(2, m_ecdb.GetModifiedRowCount());
    {
    ECSqlStatement verifyStmt;
    ASSERT_EQ(ECSqlStatus::Success, verifyStmt.Prepare(m_ecdb, "SELECT NULL FROM ts.SubElement WHERE Parent IS NULL"));
    ASSERT_EQ(BE_SQLITE_DONE, verifyStmt.Step()) << verifyStmt.GetECSql();
    }

    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.AbandonChanges());

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "DELETE FROM ts.SubElement WHERE Parent IS NULL"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    stmt.Finalize();
    ASSERT_EQ(2, m_ecdb.GetModifiedRowCount());

    {
    ECSqlStatement verifyStmt;
    ASSERT_EQ(ECSqlStatus::Success, verifyStmt.Prepare(m_ecdb, "SELECT NULL FROM ts.SubElement WHERE Parent IS NULL"));
    ASSERT_EQ(BE_SQLITE_DONE, verifyStmt.Step()) << verifyStmt.GetECSql();
    }

    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.AbandonChanges());
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlNavigationPropertyTestFixture, CRUD)
    {
    const int rowCount = 3;
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlnavpropsupport.ecdb",
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
                         "</ECSchema>")));
    ASSERT_EQ(SUCCESS, PopulateECDb( rowCount));
    m_ecdb.SaveChanges();

    ECClassId parentHasChildrenRelClassId = m_ecdb.Schemas().GetClassId("TestSchema", "ParentHasChildren");
    ASSERT_TRUE(parentHasChildrenRelClassId.IsValid());

    ECInstanceKey modelKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECClassId, ECInstanceId FROM np.DgnModel LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    modelKey = ECInstanceKey(stmt.GetValueId<ECClassId>(0), stmt.GetValueId<ECInstanceId>(1));
    ASSERT_TRUE(modelKey.IsValid());
    }

    //INSERT with nav props (various ways)
    ECInstanceKey elementKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO np.DgnElement(Model.Id,Code) VALUES(?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, modelKey.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(2, "TestCode-1", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(elementKey));

    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO np.DgnElement(Model.Id, Model.RelECClassId,Code) VALUES(?,?,?)"));
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
    m_ecdb.SaveChanges();
    }

    //verify relationship was inserted
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT SourceECInstanceId,SourceECClassId,ECClassId FROM np.ParentHasChildren WHERE TargetECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, elementKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(modelKey.GetInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue());
    ASSERT_EQ(modelKey.GetClassId().GetValue(), stmt.GetValueId<ECClassId>(1).GetValue());
    ASSERT_EQ(parentHasChildrenRelClassId.GetValue(), stmt.GetValueId<ECClassId>(2).GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT TargetECInstanceId,TargetECClassId,ECClassId FROM np.ParentHasChildren WHERE SourceECInstanceId=?"));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT Code FROM np.DgnElement WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, elementKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT Code, Model.Id, Model.RelECClassId FROM np.DgnElement WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, elementKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(modelKey.GetInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(1).GetValue());
    ASSERT_EQ(parentHasChildrenRelClassId.GetValue(), stmt.GetValueId<ECClassId>(2).GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT Code, Model FROM np.DgnElement WHERE ECInstanceId=?"));
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

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * FROM np.DgnElement WHERE ECInstanceId=?"));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM np.DgnElement WHERE Model.Id=?"));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(elementKey.GetInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue());
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(elementKey.GetInstanceId().GetValue() + 1, stmt.GetValueId<ECInstanceId>(0).GetValue());
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(elementKey.GetInstanceId().GetValue() + 2, stmt.GetValueId<ECInstanceId>(0).GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    //Nav prop in order by clause
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * FROM np.DgnElement ORDER BY Model.Id"));

    int actualRowCount = 0;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        actualRowCount++;
        }
    //one element was inserted after the setup, therefore rowCount+1 is the expected value
    ASSERT_EQ(rowCount+3, actualRowCount) << stmt.GetECSql();
    stmt.Finalize();

    //Nav prop in order by clause
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT count(*), Model.Id FROM np.DgnElement GROUP BY Model.Id"));
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
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlnavpropsupport.ecdb",
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
                                      "</ECSchema>")));

    ASSERT_EQ(SUCCESS, PopulateECDb( rowCount));

    ECClassCP relClassGen = m_ecdb.Schemas().GetClass("np", "ParentHasChildren");
    ASSERT_TRUE(relClassGen != nullptr && relClassGen->IsRelationshipClass());
    ECRelationshipClassCR relClass = *relClassGen->GetRelationshipClassCP();

    ECInstanceKey modelKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECClassId, ECInstanceId FROM np.DgnModel LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    modelKey = ECInstanceKey(stmt.GetValueId<ECClassId>(0), stmt.GetValueId<ECInstanceId>(1));
    ASSERT_TRUE(modelKey.IsValid());
    }

    ECClassCP elementClass = m_ecdb.Schemas().GetClass("np", "DgnElement");
    ASSERT_TRUE(elementClass != nullptr);

    ECInstanceKey elementKey;
    {
    ECInstanceInserter elementInserter(m_ecdb, *elementClass, nullptr);
    ASSERT_TRUE(elementInserter.IsValid());

    IECInstancePtr elementInst = elementClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ASSERT_EQ(ECObjectsStatus::Success, elementInst->SetValue("Code", ECValue("TestCode-1", true)));
    ASSERT_EQ(ECObjectsStatus::Success, elementInst->SetValue("Model", ECValue(modelKey.GetInstanceId(), &relClass)));

    ASSERT_EQ(BE_SQLITE_OK, elementInserter.Insert(elementKey, *elementInst));
    m_ecdb.SaveChanges();
    }

    //verify relationship was inserted
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT SourceECInstanceId,SourceECClassId FROM np.ParentHasChildren WHERE TargetECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, elementKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(modelKey.GetInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue());
    ASSERT_EQ(modelKey.GetClassId().GetValue(), stmt.GetValueId<ECClassId>(1).GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT TargetECInstanceId,TargetECClassId FROM np.ParentHasChildren WHERE SourceECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, modelKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(elementKey.GetInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue());
    ASSERT_EQ(elementKey.GetClassId().GetValue(), stmt.GetValueId<ECClassId>(1).GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    ECSqlStatement selStmt;
    ASSERT_EQ(ECSqlStatus::Success, selStmt.Prepare(m_ecdb, "SELECT ECInstanceId, ECClassId, Code, Model FROM np.DgnElement"));

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
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlnavpropsupport.ecdb",
                                 SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                            "<ECSchema schemaName='np' alias='np' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                            "<ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                                            "    <ECEntityClass typeName='DgnModel'>"
                                            "        <ECProperty propertyName='Name' typeName='string' />"
                                            "    </ECEntityClass>"
                                            "    <ECEntityClass typeName='DgnElement'>"
                                            "        <ECProperty propertyName='Code' typeName='string' />"
                                            "        <ECNavigationProperty propertyName='Model1' relationshipName='ParentHasChildren1' direction='Backward' />"
                                            "        <ECNavigationProperty propertyName='Model2' relationshipName='ParentHasChildren2' direction='Backward' />"
                                            "    </ECEntityClass>"
                                            "   <ECRelationshipClass typeName='ParentHasChildren1' strength='Referencing'  modifier='Sealed'>"
                                            "      <Source multiplicity='(0..1)' polymorphic='False' roleLabel='Model'>"
                                            "          <Class class ='DgnModel' />"
                                            "      </Source>"
                                            "      <Target multiplicity='(0..*)' polymorphic='False' roleLabel='Element'>"
                                            "          <Class class ='DgnElement' />"
                                            "      </Target>"
                                            "   </ECRelationshipClass>"
                                            "   <ECRelationshipClass typeName='ParentHasChildren2' strength='Referencing'  modifier='None'>"
                                            "      <Source multiplicity='(0..1)' polymorphic='False' roleLabel='Model'>"
                                            "          <Class class ='DgnModel' />"
                                            "      </Source>"
                                            "      <Target multiplicity='(0..*)' polymorphic='False' roleLabel='Element'>"
                                            "          <Class class ='DgnElement' />"
                                            "      </Target>"
                                            "   </ECRelationshipClass>"
                                            "</ECSchema>")));

    ECInstanceKey modelKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO np.DgnModel(Name) VALUES('Main')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(modelKey));
    }

    ECClassCP elementClass = m_ecdb.Schemas().GetClass("np", "DgnElement");
    ASSERT_TRUE(elementClass != nullptr);

    ECInstanceKey elementKey;
    {
    JsonInserter elementInserter(m_ecdb, *elementClass, nullptr);
    ASSERT_TRUE(elementInserter.IsValid());

    Utf8String newElementJsonStr;
    newElementJsonStr.Sprintf("{\"Code\":\"TestCode-1\","
                              " \"Model1\":{\"id\": \"%s\"},"
                              " \"Model2\":{\"id\": \"%s\", \"relClassName\": \"np.ParentHasChildren2\"}}",
                              modelKey.GetInstanceId().ToString().c_str(),
                              modelKey.GetInstanceId().ToString().c_str());

    rapidjson::Document newElementJson;
    ASSERT_FALSE(newElementJson.Parse<0>(newElementJsonStr.c_str()).HasParseError()) << newElementJsonStr.c_str();

    ASSERT_EQ(BE_SQLITE_OK, elementInserter.Insert(elementKey, newElementJson));
    m_ecdb.SaveChanges();
    }
    //verify relationship1 was inserted
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT SourceECInstanceId,SourceECClassId FROM np.ParentHasChildren1 WHERE TargetECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, elementKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(modelKey.GetInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue());
    ASSERT_EQ(modelKey.GetClassId().GetValue(), stmt.GetValueId<ECClassId>(1).GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT TargetECInstanceId,TargetECClassId FROM np.ParentHasChildren1 WHERE SourceECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, modelKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(elementKey.GetInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue());
    ASSERT_EQ(elementKey.GetClassId().GetValue(), stmt.GetValueId<ECClassId>(1).GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    //verify relationship2 was inserted
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT SourceECInstanceId,SourceECClassId FROM np.ParentHasChildren2 WHERE TargetECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, elementKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(modelKey.GetInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue());
    ASSERT_EQ(modelKey.GetClassId().GetValue(), stmt.GetValueId<ECClassId>(1).GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT TargetECInstanceId,TargetECClassId FROM np.ParentHasChildren2 WHERE SourceECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, modelKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(elementKey.GetInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue());
    ASSERT_EQ(elementKey.GetClassId().GetValue(), stmt.GetValueId<ECClassId>(1).GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    ECSqlStatement selStmt;
    ASSERT_EQ(ECSqlStatus::Success, selStmt.Prepare(m_ecdb, "SELECT ECInstanceId, Code, Model1, Model2 FROM np.DgnElement"));

    JsonECSqlSelectAdapter selAdapter(selStmt);
    ASSERT_EQ(BE_SQLITE_ROW, selStmt.Step());
    Json::Value json;
    ASSERT_EQ(SUCCESS, selAdapter.GetRow(json));

    ECInstanceId id = ECJsonUtilities::JsonToId<ECInstanceId>(json[ECJsonUtilities::json_id()]);
    ASSERT_EQ(elementKey.GetInstanceId(), id);

    {
    //Model1
    ASSERT_EQ(modelKey.GetInstanceId(), selStmt.GetValueNavigation<ECInstanceId>(2)) << "Model1 via plain ECSQL";

    Json::Value const& modelJson = json["Model1"];
    ASSERT_FALSE(modelJson.isNull()) << "Model1 is not expected to be null in the read ECInstance";
    Json::Value const& modelIdJson = modelJson[ECJsonUtilities::json_navId()];
    ASSERT_FALSE(modelIdJson.isNull()) << "Model1.Id is not expected to be null in the read ECInstance";
    ASSERT_STRCASEEQ(modelKey.GetInstanceId().ToHexStr().c_str(), modelIdJson.asCString());

    Json::Value const& modelRelClassNameJson = modelJson[ECJsonUtilities::json_navRelClassName()];
    ASSERT_FALSE(modelRelClassNameJson.isNull()) << "Model1.RelECClassId is not expected to be null in the read ECInstance";
    ASSERT_STREQ("np.ParentHasChildren1", modelRelClassNameJson.asCString());
    }

    {
    //Model2
    ECClassId relClassId;
    ASSERT_EQ(modelKey.GetInstanceId(), selStmt.GetValueNavigation<ECInstanceId>(3, &relClassId)) << "Model2.Id via plain ECSQL";
    ASSERT_EQ(m_ecdb.Schemas().GetClassId("np", "ParentHasChildren2"), relClassId) << "Model2.RelECClassId via plain ECSQL";

    Json::Value const& modelJson = json["Model2"];
    ASSERT_FALSE(modelJson.isNull()) << "Model2 is not expected to be null in the read ECInstance";
    Json::Value const& modelIdJson = modelJson[ECJsonUtilities::json_navId()];
    ASSERT_FALSE(modelIdJson.isNull()) << "Model2.Id is not expected to be null in the read ECInstance";
    ASSERT_STRCASEEQ(modelKey.GetInstanceId().ToHexStr().c_str(), modelIdJson.asCString());

    Json::Value const& modelRelClassNameJson = modelJson[ECJsonUtilities::json_navRelClassName()];
    ASSERT_FALSE(modelRelClassNameJson.isNull()) << "Model2.RelECClassId is not expected to be null in the read ECInstance";
    ASSERT_STREQ("np.ParentHasChildren2", modelRelClassNameJson.asCString());
    }

    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlNavigationPropertyTestFixture, JoinedTable)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlnavpropsupport_joinedtable.ecdb",
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
                                </ECSchema>)xml")));
    ECInstanceKey catKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO np.DgnCategory(Name) VALUES('Main')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(catKey));
    ASSERT_TRUE(catKey.IsValid());
    }

    ECInstanceKey fooKey;
    double fooDiameter = 1.1;
    Utf8CP fooCode = "Foo-1";

    //INSERT
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO np.FooElement(Diameter, Code, Category.Id) VALUES(?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(1, fooDiameter));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(2, fooCode, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, catKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(fooKey));
    }

    //Verify via SELECT
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId, Code, Category.Id, Diameter FROM np.FooElement"));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId, ECClassId, Category.Id FROM np.IGeometrySource"));
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
    m_ecdb.SaveChanges();
    
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

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "UPDATE ONLY np.IGeometrySource SET Category.Id=? WHERE Category.Id IS NULL"));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "UPDATE np.IGeometrySource SET Category.Id=? WHERE Category.Id IS NULL"));
    stmt.Finalize();

    //UPDATE via classes that is mapped to a single joined table, is expected to work
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE np.SpatialElement SET Category.Id=? WHERE Category.Id IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, catKey.GetInstanceId())) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    stmt.Finalize();
    verifyCategoryId(m_ecdb, catKey.GetInstanceId());

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE np.AnnotationElement SET Category.Id=? WHERE Category.Id IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, catKey.GetInstanceId())) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    stmt.Finalize();
    verifyCategoryId(m_ecdb, catKey.GetInstanceId());

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

    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "DELETE FROM ONLY np.IGeometrySource"));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "DELETE FROM ONLY np.IGeometrySource WHERE Category.Id IS NOT NULL"));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "DELETE FROM np.IGeometrySource WHERE Category.Id IS NOT NULL"));
    stmt.Finalize();

    m_ecdb.AbandonChanges();
    }
    }



//---------------------------------------------------------------------------------------
// @bsiclass                                     Affan.Khan                 07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlNavigationPropertyTestFixture, EndTablePolymorphicRelationshipTest)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlnavpropsupport.ecdb",
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
                                        "</ECSchema>")));

    ECSqlStatementCache cache(20); 
    CachedECSqlStatementPtr stmt;
    ECClassId elementOwnsSubElementB_ECClassId = m_ecdb.Schemas().GetClass("TestSchema", "ElementOwnsSubElementB")->GetId();
    ECClassId subElementAOwnsSubElementB_ECClassId = m_ecdb.Schemas().GetClass("TestSchema", "SubElementAOwnsSubElementB")->GetId();
    ECClassId modelHasElement_ECClassId = m_ecdb.Schemas().GetClass("TestSchema", "ModelHasElement")->GetId();

    //Add model
    ECInstanceKey modelKey1, modelKey2;
    stmt = cache.GetPreparedStatement(m_ecdb, "INSERT INTO ts.Model(Name) VALUES (?)");
    ASSERT_EQ (ECSqlStatus::Success, stmt->BindText(1, "MODEL-1", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ (BE_SQLITE_DONE, stmt->Step(modelKey1));

    stmt = cache.GetPreparedStatement(m_ecdb, "INSERT INTO ts.Model(Name) VALUES (?)");
    ASSERT_EQ(ECSqlStatus::Success, stmt->BindText(1, "MODEL-2", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, stmt->Step(modelKey2));

    ECInstanceKey subElementAKey1, subElementAKey2;
    stmt = cache.GetPreparedStatement(m_ecdb, "INSERT INTO ts.SubElementA(Code,Model,Parent) VALUES (?,?,?)");
    ASSERT_EQ(ECSqlStatus::Success, stmt->BindText(1, "CODE-A1-M1", IECSqlBinder::MakeCopy::No));   
    ASSERT_EQ(ECSqlStatus::Success, stmt->BindNavigationValue(2, modelKey1.GetInstanceId(), modelHasElement_ECClassId));
    ASSERT_EQ(ECSqlStatus::Success, stmt->BindNull(3));
    ASSERT_EQ(BE_SQLITE_DONE, stmt->Step(subElementAKey1));

    stmt = cache.GetPreparedStatement(m_ecdb, "INSERT INTO ts.SubElementA(Code,Model,Parent) VALUES (?,?,?)");
    ASSERT_EQ(ECSqlStatus::Success, stmt->BindText(1, "CODE-A2-M2", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt->BindNavigationValue(2, modelKey2.GetInstanceId(), modelHasElement_ECClassId));
    ASSERT_EQ(ECSqlStatus::Success, stmt->BindNull(3));
    ASSERT_EQ(BE_SQLITE_DONE, stmt->Step(subElementAKey2));

    ECInstanceKey subElementBKey1, subElementBKey2, subElementBKey3, subElementBKey4;
    stmt = cache.GetPreparedStatement(m_ecdb, "INSERT INTO ts.SubElementB(Code,Model,Parent) VALUES (?,?,?)");
    ASSERT_EQ(ECSqlStatus::Success, stmt->BindText(1, "CODE-B1-A1-M1", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt->BindNavigationValue(2, modelKey1.GetInstanceId(), modelHasElement_ECClassId));
    ASSERT_EQ(ECSqlStatus::Success, stmt->BindNavigationValue(3, subElementAKey1.GetInstanceId(), elementOwnsSubElementB_ECClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt->Step(subElementBKey1));

    stmt = cache.GetPreparedStatement(m_ecdb, "INSERT INTO ts.SubElementB(Code,Model,Parent) VALUES (?,?,?)");
    ASSERT_EQ(ECSqlStatus::Success, stmt->BindText(1, "CODE-B2-A2-M2", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt->BindNavigationValue(2, modelKey2.GetInstanceId(), modelHasElement_ECClassId));
    ASSERT_EQ(ECSqlStatus::Success, stmt->BindNavigationValue(3, subElementAKey2.GetInstanceId(), subElementAOwnsSubElementB_ECClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt->Step(subElementBKey2));

    stmt = cache.GetPreparedStatement(m_ecdb, "INSERT INTO ts.SubElementB(Code,Model,Parent) VALUES (?,?,?)");
    ASSERT_EQ(ECSqlStatus::Success, stmt->BindText(1, "CODE-B3-A1-M1", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt->BindNavigationValue(2, modelKey1.GetInstanceId(), modelHasElement_ECClassId));
    ASSERT_EQ(ECSqlStatus::Success, stmt->BindNavigationValue(3, subElementAKey1.GetInstanceId(), elementOwnsSubElementB_ECClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt->Step(subElementBKey3));

    stmt = cache.GetPreparedStatement(m_ecdb, "INSERT INTO ts.SubElementB(Code,Model,Parent) VALUES (?,?,?)");
    ASSERT_EQ(ECSqlStatus::Success, stmt->BindText(1, "CODE-B4-A2-M2", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt->BindNavigationValue(2, modelKey2.GetInstanceId(), modelHasElement_ECClassId));
    ASSERT_EQ(ECSqlStatus::Success, stmt->BindNavigationValue(3, subElementAKey2.GetInstanceId(), subElementAOwnsSubElementB_ECClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt->Step(subElementBKey4));

    stmt = cache.GetPreparedStatement(m_ecdb, "SELECT COUNT(*) FROM ts.Model");
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_EQ(2, stmt->GetValueInt(0));

    stmt = cache.GetPreparedStatement(m_ecdb, "SELECT COUNT(*) FROM ts.Element");
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_EQ(6, stmt->GetValueInt(0));

    stmt = cache.GetPreparedStatement(m_ecdb, "SELECT COUNT(*) FROM ts.Element WHERE Parent.RelECClassId = ?");
    ASSERT_EQ(ECSqlStatus::Success, stmt->BindId(1, elementOwnsSubElementB_ECClassId));
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_EQ(2, stmt->GetValueInt(0));

    stmt = cache.GetPreparedStatement(m_ecdb, "SELECT COUNT(*) FROM ts.Element WHERE Parent.RelECClassId = ?");
    ASSERT_EQ(ECSqlStatus::Success, stmt->BindId(1, subElementAOwnsSubElementB_ECClassId));
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_EQ(2, stmt->GetValueInt(0));

    stmt = cache.GetPreparedStatement(m_ecdb, "SELECT COUNT(*) FROM ts.ElementOwnsChildElement");
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_EQ(4, stmt->GetValueInt(0));

    stmt = cache.GetPreparedStatement(m_ecdb, "SELECT COUNT(*) FROM ONLY ts.ElementOwnsChildElement");
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_EQ(0, stmt->GetValueInt(0));

    stmt = cache.GetPreparedStatement(m_ecdb, "SELECT COUNT(*) FROM ts.ElementOwnsSubElementB");
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_EQ(2, stmt->GetValueInt(0));

    stmt = cache.GetPreparedStatement(m_ecdb, "SELECT COUNT(*) FROM ONLY ts.ElementOwnsSubElementB");
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_EQ(2, stmt->GetValueInt(0));

    stmt = cache.GetPreparedStatement(m_ecdb, "SELECT COUNT(*) FROM ts.SubElementAOwnsSubElementB");
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_EQ(2, stmt->GetValueInt(0));

    stmt = cache.GetPreparedStatement(m_ecdb, "SELECT COUNT(*) FROM ONLY ts.SubElementAOwnsSubElementB");
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_EQ(2, stmt->GetValueInt(0));

    m_ecdb.SaveChanges();
    }

END_ECDBUNITTESTS_NAMESPACE


