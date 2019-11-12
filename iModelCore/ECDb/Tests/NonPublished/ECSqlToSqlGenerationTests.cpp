/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

struct ECSqlToSqlGenerationTests : ECDbTestFixture {};

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlToSqlGenerationTests, CastForSharedColumns)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("PolymorphicSqlExpr.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
          <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA"/>
          <ECEntityClass typeName="BaseClass" modifier="Abstract" >
              <ECCustomAttributes>
                  <ClassMap xmlns="ECDbMap.02.00">
                      <MapStrategy>TablePerHierarchy</MapStrategy>
                  </ClassMap>
                  <ShareColumns xmlns="ECDbMap.02.00">
                      <MaxSharedColumnsBeforeOverflow>7</MaxSharedColumnsBeforeOverflow>
                      <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>
                  </ShareColumns>
              </ECCustomAttributes>
              <ECProperty propertyName="P1" typeName="long" />
          </ECEntityClass>
          <ECEntityClass typeName="IXFace" modifier="Abstract">
              <ECCustomAttributes>
                  <IsMixin xmlns="CoreCustomAttributes.01.00">
                      <AppliesToEntityClass>BaseClass</AppliesToEntityClass>
                  </IsMixin>
              </ECCustomAttributes>
              <ECProperty propertyName="P2" typeName="long" />
          </ECEntityClass>
          <ECEntityClass typeName="D_A">
              <BaseClass>BaseClass</BaseClass>
              <BaseClass>IXFace</BaseClass>
              <ECProperty propertyName="P3" typeName="long" />
          </ECEntityClass>
          <ECEntityClass typeName="D_B">
              <BaseClass>BaseClass</BaseClass>  
              <ECProperty propertyName="P4" typeName="long" />
          </ECEntityClass>
          <ECEntityClass typeName="DB_XFace">
              <BaseClass>D_B</BaseClass>
              <BaseClass>IXFace</BaseClass>
              <ECProperty propertyName="P5" typeName="long" />
          </ECEntityClass>
        </ECSchema>)xml"))) << "Diamond Problem";

    ECClassId daId = m_ecdb.Schemas().GetClassId("TestSchema", "D_A");
    ASSERT_TRUE(daId.IsValid());

    EXPECT_STREQ("SELECT [BaseClass].[ECInstanceId],[BaseClass].[ECClassId],[BaseClass].[ps1] FROM (SELECT [ts_BaseClass].[Id] ECInstanceId,[ts_BaseClass].[ECClassId],CAST([ts_BaseClass].[ps1] AS INTEGER) [ps1] FROM [main].[ts_BaseClass]) [BaseClass]", 
                 GetHelper().ECSqlToSql("SELECT * FROM ts.BaseClass").c_str());

    EXPECT_FALSE(GetHelper().ECSqlToSql("SELECT * FROM ts.D_A").Contains("INNER JOIN [main].ec_cache_ClassHierarchy [CHC_ts_BaseClass] ON [CHC_ts_BaseClass].[ClassId]=[ts_BaseClass].ECClassId AND [CHC_ts_BaseClass].[BaseClassId]"));

    EXPECT_STREQ(Utf8PrintfString("SELECT [D_A].[ECInstanceId],[D_A].[ECClassId],[D_A].[ps1],[D_A].[ps2],[D_A].[ps3] FROM (SELECT [Id] ECInstanceId,[ECClassId],CAST([ps1] AS INTEGER) [ps1],CAST([ps2] AS INTEGER) [ps2],CAST([ps3] AS INTEGER) [ps3] FROM [main].[ts_BaseClass] WHERE [ts_BaseClass].ECClassId=%s) [D_A]", daId.ToString().c_str()).c_str(),
                 GetHelper().ECSqlToSql("SELECT * FROM ONLY ts.D_A").c_str()) << "with ONLY keyword";

    EXPECT_TRUE(GetHelper().ECSqlToSql("SELECT * FROM ts.D_A ECSQLOPTIONS NoECClassIdFilter").Contains("FROM [main].[ts_BaseClass]) [D_A]")) << "NoECClassIdFilter";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                     10/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlToSqlGenerationTests, SharedColumnCastingForBinaryAndGeometryProps)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("SharedColumnCastingForBinaryAndGeometryProps.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
          <ECEntityClass typeName="Foo" modifier="none">
              <ECCustomAttributes>
                  <ClassMap xmlns="ECDbMap.02.00">
                      <MapStrategy>TablePerHierarchy</MapStrategy>
                  </ClassMap>
                  <ShareColumns xmlns="ECDbMap.02.00">
                      <MaxSharedColumnsBeforeOverflow>10</MaxSharedColumnsBeforeOverflow>
                  </ShareColumns>
              </ECCustomAttributes>
              <ECProperty propertyName="MyNumber" typeName="int" />
              <ECProperty propertyName="MyBlob" typeName="Binary" />
              <ECProperty propertyName="MyGeom" typeName="Bentley.Geometry.Common.IGeometry" />
          </ECEntityClass>
        </ECSchema>)xml")));

    ECClassId fooClassId = m_ecdb.Schemas().GetClassId("TestSchema", "Foo");
    ASSERT_TRUE(fooClassId.IsValid());

    EXPECT_STREQ(Utf8PrintfString("INSERT INTO [ts_Foo] ([Id],[ps1],[ps2],[ps3],ECClassId) VALUES (:_ecdb_ecsqlparam_id_col1,:_ecdb_ecsqlparam_ix1_col1,:_ecdb_ecsqlparam_ix2_col1,:_ecdb_ecsqlparam_ix3_col1,%s)", fooClassId.ToString().c_str()).c_str(),
                 GetHelper().ECSqlToSql("INSERT INTO ts.Foo(MyNumber,MyBlob,MyGeom) VALUES(?,?,?)").c_str()) << "For INSERT casting never happens";

    EXPECT_STREQ("SELECT [Foo].[ps1],[Foo].[ps2],[Foo].[ps3] FROM (SELECT [Id] ECInstanceId,[ECClassId],CAST([ps1] AS INTEGER) [ps1],[ps2],[ps3] FROM [main].[ts_Foo]) [Foo] WHERE [Foo].[ps1] IS NOT NULL AND [Foo].[ps2] IS NOT NULL AND [Foo].[ps3] IS NOT NULL AND [Foo].[ps1] BETWEEN :_ecdb_sqlparam_ix1_col1 AND :_ecdb_sqlparam_ix2_col1 AND [Foo].[ps2]=:_ecdb_sqlparam_ix3_col1 AND [Foo].[ps3]=:_ecdb_sqlparam_ix4_col1",
                 GetHelper().ECSqlToSql("SELECT MyNumber,MyBlob,MyGeom FROM ts.Foo WHERE MyNumber IS NOT NULL AND MyBlob IS NOT NULL AND MyGeom IS NOT NULL AND MyNumber BETWEEN ? AND ? AND MyBlob=? AND MyGeom=?").c_str()) << "For select only the select view has casts, not the outer select";

    EXPECT_STREQ("UPDATE [ts_Foo] SET [ps1]=:_ecdb_ecsqlparam_ix1_col1,[ps2]=:_ecdb_ecsqlparam_ix2_col1,[ps3]=:_ecdb_ecsqlparam_ix3_col1 WHERE CAST([ps1] AS INTEGER) IS NOT NULL AND [ps2] IS NOT NULL AND [ps3] IS NOT NULL AND "
                 "CAST([ps1] AS INTEGER) BETWEEN :_ecdb_ecsqlparam_ix4_col1 AND :_ecdb_ecsqlparam_ix5_col1 AND [ps2]=:_ecdb_ecsqlparam_ix6_col1 AND [ps3]=:_ecdb_ecsqlparam_ix7_col1",
                 GetHelper().ECSqlToSql("UPDATE ts.Foo SET MyNumber=?,MyBlob=?,MyGeom=? WHERE MyNumber IS NOT NULL AND MyBlob IS NOT NULL AND MyGeom IS NOT NULL AND MyNumber BETWEEN ? AND ? AND MyBlob=? AND MyGeom=?").c_str()) << "For updates binary and geom props are not cast as they are mapped to a blob column anyways";

}


//---------------------------------------------------------------------------------------
// @bsimethod                                  Affan.Khan                     1/19
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlToSqlGenerationTests, IndexOnSharedColumnIsUsedBySQLite)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("IndexOnSharedColumnIsUsedBySQLite.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
              <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
                  <ECEntityClass typeName="Parent" modifier="None">
                    <ECCustomAttributes>
                      <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                      </ClassMap>
                      <ShareColumns xmlns="ECDbMap.02.00">
                        <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>
                        <MaxSharedColumnsBeforeOverflow>100</MaxSharedColumnsBeforeOverflow>
                      </ShareColumns>
                      <DbIndexList xmlns="ECDbMap.02.00">
                        <Indexes>
                          <DbIndex>
                            <IsUnique>False</IsUnique>
                            <Name>ix_parent_ps1ps2</Name>
                            <Properties>
                              <string>PS1</string>
                              <string>PS2</string>
                            </Properties>
                          </DbIndex>
                          <DbIndex>
                            <IsUnique>False</IsUnique>
                            <Name>ix_parent_ps3</Name>
                            <Properties>
                              <string>PS3</string>
                            </Properties>
                          </DbIndex>
                        </Indexes>
                      </DbIndexList>
                    </ECCustomAttributes>
                    <ECProperty propertyName="PS1" typeName="string"/>
                    <ECProperty propertyName="PS2" typeName="string"/>
                    <ECProperty propertyName="PS3" typeName="string"/>
                  </ECEntityClass>
            </ECSchema>)xml")));
    
    const int detailColumn = 3;
    m_ecdb.SaveChanges();
    {
    ECSqlStatement ecsqlStmt;
    ASSERT_EQ(ECSqlStatus::Success, ecsqlStmt.Prepare(m_ecdb, "select null from ts.parent where ps1=?"));
    Statement sqlStmt;
    ASSERT_EQ(BE_SQLITE_OK, sqlStmt.Prepare(m_ecdb, SqlPrintfString("EXPLAIN QUERY PLAN %s", ecsqlStmt.GetNativeSql())));
    ASSERT_EQ(BE_SQLITE_ROW, sqlStmt.Step());    
    ASSERT_STREQ("SEARCH TABLE ts_Parent USING INDEX ix_parent_ps1ps2 (ps1=?)", sqlStmt.GetValueText(detailColumn));
    ASSERT_EQ(BE_SQLITE_DONE, sqlStmt.Step());
    }

    {
    ECSqlStatement ecsqlStmt;
    ASSERT_EQ(ECSqlStatus::Success, ecsqlStmt.Prepare(m_ecdb, "select null from ts.parent where ps2=?"));
    Statement sqlStmt;
    ASSERT_EQ(BE_SQLITE_OK, sqlStmt.Prepare(m_ecdb, SqlPrintfString("EXPLAIN QUERY PLAN %s", ecsqlStmt.GetNativeSql())));
    ASSERT_EQ(BE_SQLITE_ROW, sqlStmt.Step());
    ASSERT_STREQ("SCAN TABLE ts_Parent", sqlStmt.GetValueText(detailColumn));
    ASSERT_EQ(BE_SQLITE_DONE, sqlStmt.Step());
    }

    {
    ECSqlStatement ecsqlStmt;
    ASSERT_EQ(ECSqlStatus::Success, ecsqlStmt.Prepare(m_ecdb, "select null from ts.parent where ps3=?"));
    Statement sqlStmt;
    ASSERT_EQ(BE_SQLITE_OK, sqlStmt.Prepare(m_ecdb, SqlPrintfString("EXPLAIN QUERY PLAN %s", ecsqlStmt.GetNativeSql())));
    ASSERT_EQ(BE_SQLITE_ROW, sqlStmt.Step());
    ASSERT_STREQ("SEARCH TABLE ts_Parent USING INDEX ix_parent_ps3 (ps3=?)", sqlStmt.GetValueText(detailColumn));
    ASSERT_EQ(BE_SQLITE_DONE, sqlStmt.Step());
    }

    {
    ECSqlStatement ecsqlStmt;
    ASSERT_EQ(ECSqlStatus::Success, ecsqlStmt.Prepare(m_ecdb, "select null from ts.parent where ps1=? and ps2=?"));
    Statement sqlStmt;
    ASSERT_EQ(BE_SQLITE_OK, sqlStmt.Prepare(m_ecdb, SqlPrintfString("EXPLAIN QUERY PLAN %s", ecsqlStmt.GetNativeSql())));
    ASSERT_EQ(BE_SQLITE_ROW, sqlStmt.Step());
    ASSERT_STREQ("SEARCH TABLE ts_Parent USING INDEX ix_parent_ps1ps2 (ps1=? AND ps2=?)", sqlStmt.GetValueText(detailColumn));
    ASSERT_EQ(BE_SQLITE_DONE, sqlStmt.Step());
    }

    {
    ECSqlStatement ecsqlStmt;
    ASSERT_EQ(ECSqlStatus::Success, ecsqlStmt.Prepare(m_ecdb, "select null from ts.parent where ps2=? and ps3=?"));
    Statement sqlStmt;
    ASSERT_EQ(BE_SQLITE_OK, sqlStmt.Prepare(m_ecdb, SqlPrintfString("EXPLAIN QUERY PLAN %s", ecsqlStmt.GetNativeSql())));
    ASSERT_EQ(BE_SQLITE_ROW, sqlStmt.Step());
    ASSERT_STREQ("SEARCH TABLE ts_Parent USING INDEX ix_parent_ps3 (ps3=?)", sqlStmt.GetValueText(detailColumn));
    ASSERT_EQ(BE_SQLITE_DONE, sqlStmt.Step());
    }

    {
    ECSqlStatement ecsqlStmt;
    ASSERT_EQ(ECSqlStatus::Success, ecsqlStmt.Prepare(m_ecdb, "select null from ts.parent where ps1=? and ps3=?"));
    Statement sqlStmt;
    ASSERT_EQ(BE_SQLITE_OK, sqlStmt.Prepare(m_ecdb, SqlPrintfString("EXPLAIN QUERY PLAN %s", ecsqlStmt.GetNativeSql())));
    ASSERT_EQ(BE_SQLITE_ROW, sqlStmt.Step());
    ASSERT_STREQ("SEARCH TABLE ts_Parent USING INDEX ix_parent_ps3 (ps3=?)", sqlStmt.GetValueText(detailColumn));
    ASSERT_EQ(BE_SQLITE_DONE, sqlStmt.Step());
    }

    {
    ECSqlStatement ecsqlStmt;
    ASSERT_EQ(ECSqlStatus::Success, ecsqlStmt.Prepare(m_ecdb, "select null from ts.parent where ps1=? and ps2=? and ps3=?"));
    Statement sqlStmt;
    ASSERT_EQ(BE_SQLITE_OK, sqlStmt.Prepare(m_ecdb, SqlPrintfString("EXPLAIN QUERY PLAN %s", ecsqlStmt.GetNativeSql())));
    ASSERT_EQ(BE_SQLITE_ROW, sqlStmt.Step());
    ASSERT_STREQ("SEARCH TABLE ts_Parent USING INDEX ix_parent_ps3 (ps3=?)", sqlStmt.GetValueText(detailColumn));
    ASSERT_EQ(BE_SQLITE_DONE, sqlStmt.Step());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                     10/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlToSqlGenerationTests, NavPropSharedColumnCasting)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("NavPropSharedColumnCasting.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
          <ECEntityClass typeName="Parent" modifier="none">
              <ECProperty propertyName="Name" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="Child" modifier="none">
              <ECCustomAttributes>
                  <ClassMap xmlns="ECDbMap.02.00">
                      <MapStrategy>TablePerHierarchy</MapStrategy>
                  </ClassMap>
                  <ShareColumns xmlns="ECDbMap.02.00">
                      <MaxSharedColumnsBeforeOverflow>10</MaxSharedColumnsBeforeOverflow>
                  </ShareColumns>
              </ECCustomAttributes>
              <ECProperty propertyName="D" typeName="double" />
              <ECProperty propertyName="S" typeName="string" />
              <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward"/>
          </ECEntityClass>
          <ECRelationshipClass typeName="Rel" strength="referencing" strengthDirection="Forward" modifier="None">
              <Source multiplicity="(0..1)" polymorphic="True" roleLabel="references">
                 <Class class="Parent" />
             </Source>
              <Target multiplicity="(0..*)" polymorphic="True" roleLabel="referenced by">
                <Class class="Child" />
             </Target>
          </ECRelationshipClass>
        </ECSchema>)xml")));

    ECClassId parentClassId = m_ecdb.Schemas().GetClassId("TestSchema", "Parent");
    ASSERT_TRUE(parentClassId.IsValid());

    ECClassId childClassId = m_ecdb.Schemas().GetClassId("TestSchema", "Child");
    ASSERT_TRUE(childClassId.IsValid());

    ECClassId relClassId = m_ecdb.Schemas().GetClassId("TestSchema", "Rel");
    ASSERT_TRUE(relClassId.IsValid());

    EXPECT_STREQ(Utf8PrintfString("INSERT INTO [ts_Child] ([Id],[ps1],[ps2],[ps3],[ps4],ECClassId) VALUES (:_ecdb_ecsqlparam_id_col1,:_ecdb_ecsqlparam_ix1_col1,:_ecdb_ecsqlparam_ix2_col1,:_ecdb_ecsqlparam_ix3_col1,:_ecdb_ecsqlparam_ix4_col1,%s)", childClassId.ToString().c_str()).c_str(),
                 GetHelper().ECSqlToSql("INSERT INTO ts.Child(D,S,Parent.Id,Parent.RelECClassId) VALUES(?,?,?,?)").c_str());

    EXPECT_STREQ("SELECT [Child].[ps1],[Child].[ps2],[Child].[ps3],[Child].[ps4] FROM (SELECT [Id] ECInstanceId,[ECClassId],CAST([ps1] AS REAL) [ps1],CAST([ps2] AS TEXT) [ps2],CAST([ps3] AS INTEGER) [ps3],CAST((CASE WHEN [ps3] IS NULL THEN NULL ELSE [ps4] END) AS INTEGER) [ps4] FROM [main].[ts_Child]) [Child]",
                 GetHelper().ECSqlToSql("SELECT D,S,Parent.Id,Parent.RelECClassId FROM ts.Child").c_str());

    EXPECT_STREQ("SELECT [Child].[ps1],[Child].[ps2],[Child].[ps3],[Child].[ps4] FROM (SELECT [Id] ECInstanceId,[ECClassId],CAST([ps1] AS REAL) [ps1],CAST([ps2] AS TEXT) [ps2],CAST([ps3] AS INTEGER) [ps3],CAST((CASE WHEN [ps3] IS NULL THEN NULL ELSE [ps4] END) AS INTEGER) [ps4] FROM [main].[ts_Child]) [Child]",
                 GetHelper().ECSqlToSql("SELECT D,S,Parent.Id,Parent.RelECClassId FROM ts.Child").c_str());

    EXPECT_STREQ(Utf8PrintfString("SELECT [Rel].[SourceECInstanceId],[Rel].[SourceECClassId],[Rel].[TargetECInstanceId],[Rel].[TargetECClassId] FROM (SELECT [ts_Child].[Id] ECInstanceId,CAST([ts_Child].[ps4] AS INTEGER) ECClassId,CAST([ts_Child].[ps3] AS INTEGER) SourceECInstanceId,%s SourceECClassId,[ts_Child].[Id] TargetECInstanceId,[ts_Child].[ECClassId] TargetECClassId FROM [main].[ts_Child] WHERE [ts_Child].[ps3] IS NOT NULL AND CAST([ts_Child].[ps4] AS INTEGER)=%s) [Rel]", parentClassId.ToString().c_str(), relClassId.ToString().c_str()).c_str(),
                 GetHelper().ECSqlToSql("SELECT SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId FROM ts.Rel").c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                     10/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlToSqlGenerationTests, LinkTableSharedColumnCasting)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("NavPropSharedColumnCasting.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
          <ECEntityClass typeName="Parent" modifier="none">
              <ECProperty propertyName="Name" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="Child" modifier="none">
              <ECProperty propertyName="Size" typeName="double" />
          </ECEntityClass>
          <ECRelationshipClass typeName="Rel" strength="referencing" strengthDirection="Forward" modifier="None">
              <ECCustomAttributes>
                  <ClassMap xmlns="ECDbMap.02.00">
                      <MapStrategy>TablePerHierarchy</MapStrategy>
                  </ClassMap>
                  <ShareColumns xmlns="ECDbMap.02.00">
                      <MaxSharedColumnsBeforeOverflow>10</MaxSharedColumnsBeforeOverflow>
                  </ShareColumns>
              </ECCustomAttributes>
              <Source multiplicity="(0..*)" polymorphic="True" roleLabel="references">
                 <Class class="Parent" />
             </Source>
              <Target multiplicity="(0..*)" polymorphic="True" roleLabel="referenced by">
                <Class class="Child" />
             </Target>
              <ECProperty propertyName="Name" typeName="string" />
          </ECRelationshipClass>
        </ECSchema>)xml")));

    ECClassId relClassId = m_ecdb.Schemas().GetClassId("TestSchema", "Rel");
    ASSERT_TRUE(relClassId.IsValid());
    ECClassId parentClassId = m_ecdb.Schemas().GetClassId("TestSchema", "Parent");
    ASSERT_TRUE(parentClassId.IsValid());
    ECClassId childClassId = m_ecdb.Schemas().GetClassId("TestSchema", "Child");
    ASSERT_TRUE(childClassId.IsValid());


    EXPECT_STREQ(Utf8PrintfString("INSERT INTO [ts_Rel] ([Id],[SourceId],[TargetId],[ps1],ECClassId) VALUES (:_ecdb_ecsqlparam_id_col1,:_ecdb_ecsqlparam_ix1_col1,:_ecdb_ecsqlparam_ix2_col1,:_ecdb_ecsqlparam_ix3_col1,%s)", relClassId.ToString().c_str()).c_str(),
                 GetHelper().ECSqlToSql("INSERT INTO ts.Rel(SourceECInstanceId,TargetECInstanceId,Name) VALUES(?,?,?)").c_str());

    EXPECT_STREQ(Utf8PrintfString("INSERT INTO [ts_Rel] ([Id],[SourceId],[TargetId],[ps1],ECClassId) VALUES (:_ecdb_ecsqlparam_id_col1,:_ecdb_ecsqlparam_ix1_col1,:_ecdb_ecsqlparam_ix3_col1,:_ecdb_ecsqlparam_ix5_col1,%s)", relClassId.ToString().c_str()).c_str(),
                 GetHelper().ECSqlToSql("INSERT INTO ts.Rel(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId,Name) VALUES(?,?,?,?,?)").c_str());

    EXPECT_STREQ(Utf8PrintfString("SELECT [Rel].[SourceECInstanceId],[Rel].[SourceECClassId],[Rel].[TargetECInstanceId],[Rel].[TargetECClassId],[Rel].[ps1] FROM (SELECT [ts_Rel].[Id] [ECInstanceId],[ts_Rel].[ECClassId],[ts_Rel].[SourceId] [SourceECInstanceId],%s [SourceECClassId],[ts_Rel].[TargetId] [TargetECInstanceId],%s [TargetECClassId],CAST([ps1] AS TEXT) [ps1] FROM [main].[ts_Rel]) [Rel]", parentClassId.ToString().c_str(), childClassId.ToString().c_str()).c_str(),
                 GetHelper().ECSqlToSql("SELECT SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId,Name FROM ts.Rel").c_str());
    }

END_ECDBUNITTESTS_NAMESPACE
