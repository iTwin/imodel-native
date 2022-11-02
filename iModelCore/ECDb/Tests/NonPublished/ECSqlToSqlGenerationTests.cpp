/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

struct ECSqlToSqlGenerationTests : ECDbTestFixture
{
void AssertFrequencyCount(Utf8CP ecsql, std::vector<std::pair<Utf8CP, int>> substringCountPairs, bool removeUnnecessaryJoinForClassIds = true)
    {
    m_ecdb.GetECSqlConfig().SetOptimizationOption(OptimizationOptions::OptimizeJoinForClassIds, removeUnnecessaryJoinForClassIds);
    Utf8String sql = GetHelper().ECSqlToSql(ecsql);
    for (auto substringCountPair : substringCountPairs)
        {
        EXPECT_EQ(substringCountPair.second, GetHelper().GetFrequencyCount(sql, substringCountPair.first)) << substringCountPair.first << " count should be " << substringCountPair.second
            << " in converted query with removeUnnecessaryJoinForClassIds flag set to " << (removeUnnecessaryJoinForClassIds ? "true" : "false")
            << "\nECSql: " << ecsql << "\nSql:   " << sql.c_str();
        }
    }
};

//---------------------------------------------------------------------------------------
// @bsimethod
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
    ASSERT_STREQ("SEARCH main.ts_Parent USING COVERING INDEX ix_parent_ps1ps2 (ps1=?)", sqlStmt.GetValueText(detailColumn));
    ASSERT_EQ(BE_SQLITE_DONE, sqlStmt.Step());
    }

    {
    ECSqlStatement ecsqlStmt;
    ASSERT_EQ(ECSqlStatus::Success, ecsqlStmt.Prepare(m_ecdb, "select null from ts.parent where ps2=?"));
    Statement sqlStmt;
    ASSERT_EQ(BE_SQLITE_OK, sqlStmt.Prepare(m_ecdb, SqlPrintfString("EXPLAIN QUERY PLAN %s", ecsqlStmt.GetNativeSql())));
    ASSERT_EQ(BE_SQLITE_ROW, sqlStmt.Step());
    ASSERT_STREQ("SCAN main.ts_Parent USING COVERING INDEX ix_parent_ps1ps2", sqlStmt.GetValueText(detailColumn));
    ASSERT_EQ(BE_SQLITE_DONE, sqlStmt.Step());
    }

    {
    ECSqlStatement ecsqlStmt;
    ASSERT_EQ(ECSqlStatus::Success, ecsqlStmt.Prepare(m_ecdb, "select null from ts.parent where ps3=?"));
    Statement sqlStmt;
    ASSERT_EQ(BE_SQLITE_OK, sqlStmt.Prepare(m_ecdb, SqlPrintfString("EXPLAIN QUERY PLAN %s", ecsqlStmt.GetNativeSql())));
    ASSERT_EQ(BE_SQLITE_ROW, sqlStmt.Step());
    ASSERT_STREQ("SEARCH main.ts_Parent USING COVERING INDEX ix_parent_ps3 (ps3=?)", sqlStmt.GetValueText(detailColumn));
    ASSERT_EQ(BE_SQLITE_DONE, sqlStmt.Step());
    }

    {
    ECSqlStatement ecsqlStmt;
    ASSERT_EQ(ECSqlStatus::Success, ecsqlStmt.Prepare(m_ecdb, "select null from ts.parent where ps1=? and ps2=?"));
    Statement sqlStmt;
    ASSERT_EQ(BE_SQLITE_OK, sqlStmt.Prepare(m_ecdb, SqlPrintfString("EXPLAIN QUERY PLAN %s", ecsqlStmt.GetNativeSql())));
    ASSERT_EQ(BE_SQLITE_ROW, sqlStmt.Step());
    ASSERT_STREQ("SEARCH main.ts_Parent USING COVERING INDEX ix_parent_ps1ps2 (ps1=? AND ps2=?)", sqlStmt.GetValueText(detailColumn));
    ASSERT_EQ(BE_SQLITE_DONE, sqlStmt.Step());
    }

    {
    ECSqlStatement ecsqlStmt;
    ASSERT_EQ(ECSqlStatus::Success, ecsqlStmt.Prepare(m_ecdb, "select null from ts.parent where ps2=? and ps3=?"));
    Statement sqlStmt;
    ASSERT_EQ(BE_SQLITE_OK, sqlStmt.Prepare(m_ecdb, SqlPrintfString("EXPLAIN QUERY PLAN %s", ecsqlStmt.GetNativeSql())));
    ASSERT_EQ(BE_SQLITE_ROW, sqlStmt.Step());
    ASSERT_STREQ("SEARCH main.ts_Parent USING INDEX ix_parent_ps3 (ps3=?)", sqlStmt.GetValueText(detailColumn));
    ASSERT_EQ(BE_SQLITE_DONE, sqlStmt.Step());
    }

    {
    ECSqlStatement ecsqlStmt;
    ASSERT_EQ(ECSqlStatus::Success, ecsqlStmt.Prepare(m_ecdb, "select null from ts.parent where ps1=? and ps3=?"));
    Statement sqlStmt;
    ASSERT_EQ(BE_SQLITE_OK, sqlStmt.Prepare(m_ecdb, SqlPrintfString("EXPLAIN QUERY PLAN %s", ecsqlStmt.GetNativeSql())));
    ASSERT_EQ(BE_SQLITE_ROW, sqlStmt.Step());
    ASSERT_STREQ("SEARCH main.ts_Parent USING INDEX ix_parent_ps3 (ps3=?)", sqlStmt.GetValueText(detailColumn));
    ASSERT_EQ(BE_SQLITE_DONE, sqlStmt.Step());
    }

    {
    ECSqlStatement ecsqlStmt;
    ASSERT_EQ(ECSqlStatus::Success, ecsqlStmt.Prepare(m_ecdb, "select null from ts.parent where ps1=? and ps2=? and ps3=?"));
    Statement sqlStmt;
    ASSERT_EQ(BE_SQLITE_OK, sqlStmt.Prepare(m_ecdb, SqlPrintfString("EXPLAIN QUERY PLAN %s", ecsqlStmt.GetNativeSql())));
    ASSERT_EQ(BE_SQLITE_ROW, sqlStmt.Step());
    ASSERT_STREQ("SEARCH main.ts_Parent USING INDEX ix_parent_ps3 (ps3=?)", sqlStmt.GetValueText(detailColumn));
    ASSERT_EQ(BE_SQLITE_DONE, sqlStmt.Step());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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

    EXPECT_STREQ("SELECT [Child].[ps1],[Child].[ps2],[Child].[ps3],[Child].[ps4] FROM (SELECT [Id] ECInstanceId,[ECClassId],[ps1],[ps2],[ps3],(CASE WHEN [ps3] IS NULL THEN NULL ELSE [ps4] END) [ps4] FROM [main].[ts_Child]) [Child]",
                 GetHelper().ECSqlToSql("SELECT D,S,Parent.Id,Parent.RelECClassId FROM ts.Child").c_str());

    EXPECT_STREQ("SELECT [Child].[ps1],[Child].[ps2],[Child].[ps3],[Child].[ps4] FROM (SELECT [Id] ECInstanceId,[ECClassId],[ps1],[ps2],[ps3],(CASE WHEN [ps3] IS NULL THEN NULL ELSE [ps4] END) [ps4] FROM [main].[ts_Child]) [Child]",
                 GetHelper().ECSqlToSql("SELECT D,S,Parent.Id,Parent.RelECClassId FROM ts.Child").c_str());

    EXPECT_STREQ(Utf8PrintfString("SELECT [Rel].[SourceECInstanceId],[Rel].[SourceECClassId],[Rel].[TargetECInstanceId],[Rel].[TargetECClassId] FROM (SELECT [ts_Child].[Id] ECInstanceId,[ts_Child].[ps4] ECClassId,[ts_Child].[ps3] SourceECInstanceId,%s SourceECClassId,[ts_Child].[Id] TargetECInstanceId,[ts_Child].[ECClassId] TargetECClassId FROM [main].[ts_Child] WHERE [ts_Child].[ps3] IS NOT NULL AND [ts_Child].[ps4]=%s) [Rel]", parentClassId.ToString().c_str(), relClassId.ToString().c_str()).c_str(),
                 GetHelper().ECSqlToSql("SELECT SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId FROM ts.Rel").c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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

    EXPECT_STREQ(Utf8PrintfString("SELECT [Rel].[SourceECInstanceId],[Rel].[SourceECClassId],[Rel].[TargetECInstanceId],[Rel].[TargetECClassId],[Rel].[ps1] FROM (SELECT [ts_Rel].[Id] [ECInstanceId],[ts_Rel].[ECClassId],[ts_Rel].[SourceId] [SourceECInstanceId],%s [SourceECClassId],[ts_Rel].[TargetId] [TargetECInstanceId],%s [TargetECClassId],[ps1] FROM [main].[ts_Rel]) [Rel]", parentClassId.ToString().c_str(), childClassId.ToString().c_str()).c_str(),
                 GetHelper().ECSqlToSql("SELECT SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId,Name FROM ts.Rel").c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlToSqlGenerationTests, OptimisedJoins)
    {
    SchemaItem schemaItem(R"schema(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="RoadRailPhysical" alias="rrphys" version="03.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA"/>
        <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="Element" modifier="Abstract">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.2.0.0">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
                <ECNavigationProperty propertyName="Model" relationshipName="ModelContainsElements" direction="backward" readOnly="true">
                    <ECCustomAttributes>
                        <ForeignKeyConstraint xmlns="ECDbMap.2.0.0">
                            <OnDeleteAction>NoAction</OnDeleteAction>
                        </ForeignKeyConstraint>
                    </ECCustomAttributes>
                </ECNavigationProperty>
                <ECProperty propertyName="LastMod" typeName="dateTime" readOnly="true">
                    <ECCustomAttributes>
                        <DateTimeInfo xmlns="CoreCustomAttributes.1.0.0">
                            <DateTimeKind>Utc</DateTimeKind>
                        </DateTimeInfo>
                        <HiddenProperty xmlns="CoreCustomAttributes.1.0.0" />
                    </ECCustomAttributes>
                </ECProperty>
                <ECProperty propertyName="CodeValue" typeName="string">
                    <ECCustomAttributes>
                        <PropertyMap xmlns="ECDbMap.2.0.0">
                            <IsNullable>True</IsNullable>
                            <Collation>NoCase</Collation>
                        </PropertyMap>
                    </ECCustomAttributes>
                </ECProperty>
                <ECProperty propertyName="UserLabel" typeName="string">
                    <ECCustomAttributes>
                        <PropertyMap xmlns="ECDbMap.2.0.0">
                            <IsNullable>True</IsNullable>
                            <Collation>NoCase</Collation>
                            <IsUnique>False</IsUnique>
                        </PropertyMap>
                    </ECCustomAttributes>
                </ECProperty>
                <ECProperty propertyName="FederationGuid" typeName="binary" extendedTypeName="BeGuid">
                    <ECCustomAttributes>
                        <PropertyMap xmlns="ECDbMap.2.0.0">
                            <IsNullable>True</IsNullable>
                            <IsUnique>True</IsUnique>
                        </PropertyMap>
                        <HiddenProperty xmlns="CoreCustomAttributes.1.0.0" />
                    </ECCustomAttributes>
                </ECProperty>
                <ECProperty propertyName="JsonProperties" typeName="string" extendedTypeName="Json">
                    <ECCustomAttributes>
                        <HiddenProperty xmlns="CoreCustomAttributes.1.0.0" />
                    </ECCustomAttributes>
                </ECProperty>
            </ECEntityClass>
            <ECRelationshipClass typeName="ModelContainsElements" strength="embedding" modifier="Sealed">
                <Source multiplicity="(1..1)" roleLabel="contains" polymorphic="true">
                    <Class class="Model"/>
                </Source>
                <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
                    <Class class="Element" />
                </Target>
            </ECRelationshipClass>
            <ECEntityClass typeName="Model" modifier="Abstract">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.2.0.0">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <ShareColumns xmlns="ECDbMap.2.0.0">
                        <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                        <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                    </ShareColumns>
                </ECCustomAttributes>
                <ECProperty propertyName="IsPrivate" typeName="boolean"/>
                <ECProperty propertyName="IsTemplate" typeName="boolean"/>
                <ECProperty propertyName="JsonProperties" typeName="string" extendedTypeName="Json"/>
                <ECProperty propertyName="LastMod" typeName="dateTime" />
            </ECEntityClass>
            <ECEntityClass typeName="GeometricElement" modifier="Abstract">
                <BaseClass>Element</BaseClass>
                <ECCustomAttributes>
                    <JoinedTablePerDirectSubclass xmlns="ECDbMap.2.0.0" />
                    <NotSubclassableInReferencingSchemas xmlns="CoreCustomAttributes.1.0.0" />
                </ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="GeometricElement3d" modifier="Abstract">
                <BaseClass>GeometricElement</BaseClass>
                <ECCustomAttributes>
                    <ShareColumns xmlns="ECDbMap.2.0.0">
                        <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                        <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                    </ShareColumns>
                </ECCustomAttributes>
                <ECProperty propertyName="InSpatialIndex" typeName="boolean">
                </ECProperty>
                <ECProperty propertyName="Origin" typeName="point3d">
                </ECProperty>
                <ECProperty propertyName="Yaw" typeName="double">
                </ECProperty>
                <ECProperty propertyName="Pitch" typeName="double">
                </ECProperty>
                <ECProperty propertyName="Roll" typeName="double">
                </ECProperty>
                <ECProperty propertyName="BBoxLow" typeName="point3d">
                </ECProperty>
                <ECProperty propertyName="BBoxHigh" typeName="point3d">
                </ECProperty>
                <ECProperty propertyName="GeometryStream" typeName="binary" extendedTypeName="GeometryStream">
                </ECProperty>
            </ECEntityClass>
        </ECSchema>)schema");

    ASSERT_EQ(SUCCESS, SetupECDb("OptimisedJoins.ecdb", schemaItem));
    Utf8String sql = "";

    m_ecdb.GetECSqlConfig().SetOptimizationOption(OptimizationOptions::OptimizeJoinForNestedSelectQuery, false);
    sql = GetHelper().ECSqlToSql(R"statement(
        SELECT
            *
        FROM
            (
                SELECT
                    Model.Id
                FROM
                    RoadRailPhysical.GeometricElement3d
                UNION
                SELECT
                    ECClassId
                FROM
                    RoadRailPhysical.GeometricElement3d
            );
        )statement");

    EXPECT_EQ(2, GetHelper().GetFrequencyCount(sql, "JOIN"));
    EXPECT_EQ(3, GetHelper().GetFrequencyCount(sql, "ModelId"));

    m_ecdb.GetECSqlConfig().SetOptimizationOption(OptimizationOptions::OptimizeJoinForNestedSelectQuery, true);
    sql = GetHelper().ECSqlToSql(R"statement(
        SELECT
            *
        FROM
            (
                SELECT
                    Model.Id
                FROM
                    RoadRailPhysical.GeometricElement3d
                UNION
                SELECT
                    ECClassId
                FROM
                    RoadRailPhysical.GeometricElement3d
            );
        )statement");

    EXPECT_EQ(1, GetHelper().GetFrequencyCount(sql, "JOIN"));
    EXPECT_EQ(2, GetHelper().GetFrequencyCount(sql, "ModelId"));

    m_ecdb.CloseDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlToSqlGenerationTests, LinkTableJoinGeneration_TablePerHierarchy)
    {
    SchemaItem schemaItem(R"schema(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA"/>
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            
            <ECEntityClass typeName="Element" modifier="Abstract">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.2.0.0">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
                <ECProperty propertyName="ElementProperty" typeName="string" />
            </ECEntityClass>

            <ECEntityClass typeName="MyClass" modifier="Abstract">
                <BaseClass>Element</BaseClass>
            </ECEntityClass>

            <ECRelationshipClass typeName="ElementRefersToElements" strength="referencing" modifier="Abstract">
                <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                    <Class class="Element"/>
                </Source>
                <Target multiplicity="(0..*)" roleLabel="is referenced by" polymorphic="true">
                    <Class class="Element"/>
                </Target>
                <ECProperty propertyName="ElRefElProperty" typeName="string" />
            </ECRelationshipClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("LinkTableJoinGeneration_TablePerHierarchy.ecdb", schemaItem));

    ECClassCP linkTableRelClass = m_ecdb.Schemas().GetClass("TestSchema", "ElementRefersToElements");
    ASSERT_TRUE(linkTableRelClass != nullptr);

    AssertFrequencyCount("SELECT SourceECInstanceId FROM ts.ElementRefersToElements",
        {{ "JOIN", 2 }, { "SourceECClassId", 1 }, { "TargetECClassId", 1 }}, false);
    AssertFrequencyCount("SELECT SourceECInstanceId FROM ts.ElementRefersToElements",
        {{ "JOIN", 0 }, { "SourceECClassId", 0 }, { "TargetECClassId", 0 }}, true);

    AssertFrequencyCount("SELECT SourceECInstanceId, SourceECClassId FROM ts.ElementRefersToElements",
        {{ "JOIN", 2 }, { "SourceECClassId", 2 }, { "TargetECClassId", 1 }}, false);
    AssertFrequencyCount("SELECT SourceECInstanceId, SourceECClassId FROM ts.ElementRefersToElements",
        {{ "JOIN", 1 }, { "SourceECClassId", 2 }, { "TargetECClassId", 0 }}, true);

    AssertFrequencyCount("SELECT SourceECInstanceId, SourceECClassId, TargetECClassId FROM ts.ElementRefersToElements",
        {{ "JOIN", 2 }, { "SourceECClassId", 2 }, { "TargetECClassId", 2 }}, false);
    AssertFrequencyCount("SELECT SourceECInstanceId, SourceECClassId, TargetECClassId FROM ts.ElementRefersToElements",
        {{ "JOIN", 2 }, { "SourceECClassId", 2 }, { "TargetECClassId", 2 }}, true);

    AssertFrequencyCount("SELECT e.ElementProperty, r.ElRefElProperty FROM ts.ElementRefersToElements r JOIN ts.Element e ON e.ECInstanceId=r.SourceECInstanceId",
        {{ "JOIN", 3 }, { "SourceECClassId", 1 }, { "TargetECClassId", 1 }}, false);
    AssertFrequencyCount("SELECT e.ElementProperty, r.ElRefElProperty FROM ts.ElementRefersToElements r JOIN ts.Element e ON e.ECInstanceId=r.SourceECInstanceId",
        {{ "JOIN", 1 }, { "SourceECClassId", 0 }, { "TargetECClassId", 0 }}, true);

    AssertFrequencyCount("SELECT e.ElementProperty FROM ts.ElementRefersToElements r JOIN ts.Element e ON e.ECInstanceId=r.SourceECInstanceId",
        {{ "JOIN", 3 }, { "SourceECClassId", 1 }, { "TargetECClassId", 1 }}, false);
    AssertFrequencyCount("SELECT e.ElementProperty FROM ts.ElementRefersToElements r JOIN ts.Element e ON e.ECInstanceId=r.SourceECInstanceId",
        {{ "JOIN", 1 }, { "SourceECClassId", 0 }, { "TargetECClassId", 0 }}, true);

    AssertFrequencyCount("SELECT ElRefElProperty FROM ts.ElementRefersToElements",
        {{ "JOIN", 2 }, { "SourceECClassId", 1 }, { "TargetECClassId", 1 }}, false);
    AssertFrequencyCount("SELECT ElRefElProperty FROM ts.ElementRefersToElements",
        {{ "JOIN", 0 }, { "SourceECClassId", 0 }, { "TargetECClassId", 0 }}, true);

    AssertFrequencyCount("SELECT ElRefElProperty FROM ts.ElementRefersToElements WHERE SourceECClassId = 123",
        {{ "JOIN", 2 }, { "SourceECClassId", 2 }, { "TargetECClassId", 1 }}, false);
    AssertFrequencyCount("SELECT ElRefElProperty FROM ts.ElementRefersToElements WHERE SourceECClassId = 123",
        {{ "JOIN", 1 }, { "SourceECClassId", 2 }, { "TargetECClassId", 0 }}, true);

    AssertFrequencyCount("SELECT ElRefElProperty FROM ts.ElementRefersToElements WHERE TargetECClassId = 123",
        {{ "JOIN", 2 }, { "SourceECClassId", 1 }, { "TargetECClassId", 2 }}, false);
    AssertFrequencyCount("SELECT ElRefElProperty FROM ts.ElementRefersToElements WHERE TargetECClassId = 123",
        {{ "JOIN", 1 }, { "SourceECClassId", 0 }, { "TargetECClassId", 2 }}, true);

    m_ecdb.CloseDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlToSqlGenerationTests, LinkTableJoinGeneration_OwnTable)
    {
    SchemaItem schemaItem(R"schema(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA"/>
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          
            <ECEntityClass typeName="Element" modifier="None">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.2.0.0">
                        <MapStrategy>OwnTable</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
                <ECProperty propertyName="ElementProperty" typeName="string" />
            </ECEntityClass>

            <ECRelationshipClass typeName="ElementRefersToElements" strength="referencing" modifier="None">
                <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="false">
                    <Class class="Element"/>
                </Source>
                <Target multiplicity="(0..*)" roleLabel="is referenced by" polymorphic="false">
                    <Class class="Element"/>
                </Target>
                <ECProperty propertyName="ElRefElProperty" typeName="string" />
            </ECRelationshipClass>            
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("LinkTableJoinGeneration_OwnTable.ecdb", schemaItem));

    ECClassCP linkTableRelClass = m_ecdb.Schemas().GetClass("TestSchema", "ElementRefersToElements");
    ASSERT_TRUE(linkTableRelClass != nullptr);

    AssertFrequencyCount("SELECT SourceECInstanceId FROM ts.ElementRefersToElements",
        {{ "JOIN", 0 }, { "SourceECClassId", 1 }, { "TargetECClassId", 1 }}, false);
    AssertFrequencyCount("SELECT SourceECInstanceId FROM ts.ElementRefersToElements",
        {{ "JOIN", 0 }, { "SourceECClassId", 0 }, { "TargetECClassId", 0 }}, true);

    AssertFrequencyCount("SELECT SourceECInstanceId, SourceECClassId FROM ts.ElementRefersToElements",
        {{ "JOIN", 0 }, { "SourceECClassId", 2 }, { "TargetECClassId", 1 }}, false);
    AssertFrequencyCount("SELECT SourceECInstanceId, SourceECClassId FROM ts.ElementRefersToElements",
        {{ "JOIN", 0 }, { "SourceECClassId", 2 }, { "TargetECClassId", 0 }}, true);

    AssertFrequencyCount("SELECT SourceECInstanceId, SourceECClassId, TargetECClassId FROM ts.ElementRefersToElements",
        {{ "JOIN", 0 }, { "SourceECClassId", 2 }, { "TargetECClassId", 2 }}, false);
    AssertFrequencyCount("SELECT SourceECInstanceId, SourceECClassId, TargetECClassId FROM ts.ElementRefersToElements",
        {{ "JOIN", 0 }, { "SourceECClassId", 2 }, { "TargetECClassId", 2 }}, true);

    AssertFrequencyCount("SELECT e.ElementProperty, r.ElRefElProperty FROM ts.ElementRefersToElements r JOIN ts.Element e ON e.ECInstanceId=r.SourceECInstanceId",
        {{ "JOIN", 1 }, { "SourceECClassId", 1 }, { "TargetECClassId", 1 }}, false);
    AssertFrequencyCount("SELECT e.ElementProperty, r.ElRefElProperty FROM ts.ElementRefersToElements r JOIN ts.Element e ON e.ECInstanceId=r.SourceECInstanceId",
        {{ "JOIN", 1 }, { "SourceECClassId", 0 }, { "TargetECClassId", 0 }}, true);

    AssertFrequencyCount("SELECT e.ElementProperty FROM ts.ElementRefersToElements r JOIN ts.Element e ON e.ECInstanceId=r.SourceECInstanceId",
        {{ "JOIN", 1 }, { "SourceECClassId", 1 }, { "TargetECClassId", 1 }}, false);
    AssertFrequencyCount("SELECT e.ElementProperty FROM ts.ElementRefersToElements r JOIN ts.Element e ON e.ECInstanceId=r.SourceECInstanceId",
        {{ "JOIN", 1 }, { "SourceECClassId", 0 }, { "TargetECClassId", 0 }}, true);

    AssertFrequencyCount("SELECT ElRefElProperty FROM ts.ElementRefersToElements",
        {{ "JOIN", 0 }, { "SourceECClassId", 1 }, { "TargetECClassId", 1 }}, false);
    AssertFrequencyCount("SELECT ElRefElProperty FROM ts.ElementRefersToElements",
        {{ "JOIN", 0 }, { "SourceECClassId", 0 }, { "TargetECClassId", 0 }}, true);

    AssertFrequencyCount("SELECT ElRefElProperty FROM ts.ElementRefersToElements WHERE SourceECClassId = 123",
        {{ "JOIN", 0 }, { "SourceECClassId", 2 }, { "TargetECClassId", 1 }}, false);
    AssertFrequencyCount("SELECT ElRefElProperty FROM ts.ElementRefersToElements WHERE SourceECClassId = 123",
        {{ "JOIN", 0 }, { "SourceECClassId", 2 }, { "TargetECClassId", 0 }}, true);

    AssertFrequencyCount("SELECT ElRefElProperty FROM ts.ElementRefersToElements WHERE TargetECClassId = 123",
        {{ "JOIN", 0 }, { "SourceECClassId", 1 }, { "TargetECClassId", 2 }}, false);
    AssertFrequencyCount("SELECT ElRefElProperty FROM ts.ElementRefersToElements WHERE TargetECClassId = 123",
        {{ "JOIN", 0 }, { "SourceECClassId", 0 }, { "TargetECClassId", 2 }}, true);

    m_ecdb.CloseDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlToSqlGenerationTests, LinkTableJoinGeneration_MixinConstraintAndJoinedTablePerSubClass)
    {
    SchemaItem schemaItem(R"schema(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA"/>
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          
            <ECEntityClass typeName="Element" modifier="None" />

            <ECEntityClass typeName="Fruit" modifier="None">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.2.0.0">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <JoinedTablePerDirectSubclass xmlns="ECDbMap.2.0.0" />
                </ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="IPeelable" modifier="Abstract" >
                <ECCustomAttributes>
                    <IsMixin xmlns="CoreCustomAttributes.1.0.0">
                        <AppliesToEntityClass>Fruit</AppliesToEntityClass>
                    </IsMixin>
                </ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="Banana" modifier="None">
                <BaseClass>Fruit</BaseClass>
                <BaseClass>IPeelable</BaseClass>
            </ECEntityClass>
            <ECEntityClass typeName="Orange" modifier="None">
                <BaseClass>Fruit</BaseClass>
                <BaseClass>IPeelable</BaseClass>
            </ECEntityClass>
            <ECRelationshipClass typeName="ElementHasPeelableFruit" strength="referencing" modifier="None">
                <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                    <Class class="Element"/>
                </Source>
                <Target multiplicity="(0..*)" roleLabel="is referenced by" polymorphic="true">
                    <Class class="IPeelable"/>
                </Target>
            </ECRelationshipClass>            
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("LinkTableJoinGeneration_OwnTable.ecdb", schemaItem));

    AssertFrequencyCount("SELECT SourceECClassId, TargetECClassId FROM ts.ElementHasPeelableFruit",
        {{ "JOIN", 0 }, { "SourceECClassId", 2 }, { "TargetECClassId", 2 }}, false);

    AssertFrequencyCount("SELECT SourceECClassId, TargetECClassId FROM ts.ElementHasPeelableFruit",
        {{ "JOIN", 0 }, { "SourceECClassId", 2 }, { "TargetECClassId", 2 }}, true);

    AssertFrequencyCount("SELECT SourceECInstanceId, TargetECInstanceId FROM ts.ElementHasPeelableFruit",
        {{ "JOIN", 0 }, { "SourceECClassId", 1 }, { "TargetECClassId", 1 }}, false);

    AssertFrequencyCount("SELECT SourceECInstanceId, TargetECInstanceId FROM ts.ElementHasPeelableFruit",
        {{ "JOIN", 0 }, { "SourceECClassId", 0 }, { "TargetECClassId", 0 }}, true);

    m_ecdb.CloseDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlToSqlGenerationTests, RoadRailPhysicalManyJoins)
    {
    SchemaItem schemaItem(R"schema(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="RoadRailPhysical" alias="rrphys" version="03.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA"/>
        <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
        <ECEntityClass typeName="Element" modifier="Abstract">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.2.0.0">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
                <ECNavigationProperty propertyName="Model" relationshipName="ModelContainsElements" direction="backward" readOnly="true">
                    <ECCustomAttributes>
                        <ForeignKeyConstraint xmlns="ECDbMap.2.0.0">
                            <OnDeleteAction>NoAction</OnDeleteAction>
                        </ForeignKeyConstraint>
                    </ECCustomAttributes>
                </ECNavigationProperty>
                <ECProperty propertyName="LastMod" typeName="dateTime" readOnly="true">
                    <ECCustomAttributes>
                        <DateTimeInfo xmlns="CoreCustomAttributes.1.0.0">
                            <DateTimeKind>Utc</DateTimeKind>
                        </DateTimeInfo>
                        <HiddenProperty xmlns="CoreCustomAttributes.1.0.0" />
                    </ECCustomAttributes>
                </ECProperty>
                <ECProperty propertyName="CodeValue" typeName="string">
                    <ECCustomAttributes>
                        <PropertyMap xmlns="ECDbMap.2.0.0">
                            <IsNullable>True</IsNullable>
                            <Collation>NoCase</Collation>
                        </PropertyMap>
                    </ECCustomAttributes>
                </ECProperty>
                <ECProperty propertyName="UserLabel" typeName="string">
                    <ECCustomAttributes>
                        <PropertyMap xmlns="ECDbMap.2.0.0">
                            <IsNullable>True</IsNullable>
                            <Collation>NoCase</Collation>
                            <IsUnique>False</IsUnique>
                        </PropertyMap>
                    </ECCustomAttributes>
                </ECProperty>
                <ECNavigationProperty propertyName="Parent" relationshipName="ElementOwnsChildElements" direction="backward" >
                    <ECCustomAttributes>
                        <ForeignKeyConstraint xmlns="ECDbMap.2.0.0">
                            <OnDeleteAction>NoAction</OnDeleteAction>
                        </ForeignKeyConstraint>
                    </ECCustomAttributes>
                </ECNavigationProperty>
                <ECProperty propertyName="FederationGuid" typeName="binary" extendedTypeName="BeGuid">
                    <ECCustomAttributes>
                        <PropertyMap xmlns="ECDbMap.2.0.0">
                            <IsNullable>True</IsNullable>
                            <IsUnique>True</IsUnique>
                        </PropertyMap>
                        <HiddenProperty xmlns="CoreCustomAttributes.1.0.0" />
                    </ECCustomAttributes>
                </ECProperty>
                <ECProperty propertyName="JsonProperties" typeName="string" extendedTypeName="Json">
                    <ECCustomAttributes>
                        <HiddenProperty xmlns="CoreCustomAttributes.1.0.0" />
                    </ECCustomAttributes>
                </ECProperty>
            </ECEntityClass>
            <ECRelationshipClass typeName="ElementOwnsChildElements" strength="embedding" modifier="None">
                <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
                    <Class class="Element"/>
                </Source>
                <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
                    <Class class="Element"/>
                </Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="ModelContainsElements" strength="embedding" modifier="Sealed">
                <Source multiplicity="(1..1)" roleLabel="contains" polymorphic="true">
                    <Class class="Model"/>
                </Source>
                <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
                    <Class class="Element" />
                </Target>
            </ECRelationshipClass>
            <ECEntityClass typeName="Model" modifier="Abstract">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.2.0.0">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <ShareColumns xmlns="ECDbMap.2.0.0">
                        <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                        <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                    </ShareColumns>
                </ECCustomAttributes>
                <ECProperty propertyName="IsPrivate" typeName="boolean"/>
                <ECProperty propertyName="IsTemplate" typeName="boolean"/>
                <ECProperty propertyName="JsonProperties" typeName="string" extendedTypeName="Json"/>
                <ECProperty propertyName="LastMod" typeName="dateTime" />
            </ECEntityClass>
        <ECEntityClass typeName="GeometricElement" modifier="Abstract">
                <BaseClass>Element</BaseClass>
                <ECCustomAttributes>
                    <JoinedTablePerDirectSubclass xmlns="ECDbMap.2.0.0" />
                    <NotSubclassableInReferencingSchemas xmlns="CoreCustomAttributes.1.0.0" />
                </ECCustomAttributes>
            </ECEntityClass>
        <ECEntityClass typeName="GeometricElement3d" modifier="Abstract">
                <BaseClass>GeometricElement</BaseClass>
                <ECCustomAttributes>
                    <ShareColumns xmlns="ECDbMap.2.0.0">
                        <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                        <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                    </ShareColumns>
                </ECCustomAttributes>
                <ECProperty propertyName="InSpatialIndex" typeName="boolean">
                </ECProperty>
                <ECProperty propertyName="Origin" typeName="point3d">
                </ECProperty>
                <ECProperty propertyName="Yaw" typeName="double">
                </ECProperty>
                <ECProperty propertyName="Pitch" typeName="double">
                </ECProperty>
                <ECProperty propertyName="Roll" typeName="double">
                </ECProperty>
                <ECProperty propertyName="BBoxLow" typeName="point3d">
                </ECProperty>
                <ECProperty propertyName="BBoxHigh" typeName="point3d">
                </ECProperty>
                <ECProperty propertyName="GeometryStream" typeName="binary" extendedTypeName="GeometryStream">
                </ECProperty>
            </ECEntityClass>
        <ECEntityClass typeName="SpatialElement" modifier="Abstract">
                <BaseClass>GeometricElement3d</BaseClass>
            </ECEntityClass>
        <ECEntityClass typeName="PhysicalElement" modifier="Abstract">
                <BaseClass>SpatialElement</BaseClass>
            </ECEntityClass>
        <ECEntityClass typeName="ILinearlyLocated" modifier="Abstract">
            <ECCustomAttributes>
            <IsMixin xmlns='CoreCustomAttributes.01.00.00'>
                <AppliesToEntityClass>Element</AppliesToEntityClass>
            </IsMixin>
            </ECCustomAttributes>
        </ECEntityClass>
        <ECEntityClass typeName="ILinearLocationElement" modifier="Abstract">
            <BaseClass>ILinearlyLocated</BaseClass>
            <ECCustomAttributes>
            <IsMixin xmlns='CoreCustomAttributes.01.00.00'>
                <AppliesToEntityClass>Element</AppliesToEntityClass>
            </IsMixin>
            </ECCustomAttributes>
        </ECEntityClass>
        <ECEntityClass typeName="LinearPhysicalElement" modifier="Abstract">
                <BaseClass>PhysicalElement</BaseClass>
                <BaseClass>ILinearLocationElement</BaseClass>
            </ECEntityClass>
        <ECEntityClass typeName="ISubModeledElement" modifier="Abstract">
                <ECCustomAttributes>
                    <IsMixin xmlns="CoreCustomAttributes.1.0.0">
                        <!-- Only subclasses of bis:Element can implement the ISubModeledElement interface -->
                        <AppliesToEntityClass>Element</AppliesToEntityClass>
                    </IsMixin>
                </ECCustomAttributes>
        </ECEntityClass>
        <ECEntityClass typeName="ILinearElementSource" modifier="Abstract">
            <ECCustomAttributes>
            <IsMixin xmlns="CoreCustomAttributes.1.0.0">
                <!-- Only subclasses of bis:Element can implement the IParentElement interface -->
                <AppliesToEntityClass>Element</AppliesToEntityClass>
            </IsMixin>
            </ECCustomAttributes>
        </ECEntityClass>

        <ECEntityClass typeName="Corridor" modifier="Sealed">
                <BaseClass>LinearPhysicalElement</BaseClass>
                <BaseClass>ISubModeledElement</BaseClass>
                <BaseClass>ILinearElementSource</BaseClass>
            </ECEntityClass>
        <ECEntityClass typeName="TransportationSystem" modifier="Sealed">
                <BaseClass>PhysicalElement</BaseClass>
                <BaseClass>ISubModeledElement</BaseClass>
            </ECEntityClass>
        <ECEntityClass typeName="IParentElement" modifier="Abstract">
                <ECCustomAttributes>
                    <IsMixin xmlns="CoreCustomAttributes.1.0.0">
                        <!-- Only subclasses of bis:Element can implement the IParentElement interface -->
                        <AppliesToEntityClass>Element</AppliesToEntityClass>
                    </IsMixin>
                </ECCustomAttributes>
            </ECEntityClass>
        <ECEntityClass typeName="CorridorPortionElement" modifier="Abstract">
                <BaseClass>PhysicalElement</BaseClass>
                <BaseClass>IParentElement</BaseClass>
                <BaseClass>ILinearElementSource</BaseClass>
            </ECEntityClass>
        <ECRelationshipClass typeName="ElementRefersToElements" strength="referencing" modifier="Abstract">
                <ECCustomAttributes>
                    <LinkTableRelationshipMap xmlns="ECDbMap.2.0.0">
                        <CreateForeignKeyConstraints>False</CreateForeignKeyConstraints>
                    </LinkTableRelationshipMap>
                    <ClassMap xmlns="ECDbMap.2.0.0">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <ShareColumns xmlns="ECDbMap.2.0.0">
                        <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                        <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                    </ShareColumns>
                </ECCustomAttributes>
                <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                    <Class class="Element" />
                </Source>
                <Target multiplicity="(0..*)" roleLabel="is referenced by" polymorphic="true">
                    <Class class="Element" />
                </Target>
            </ECRelationshipClass>
        <ECRelationshipClass typeName="ILinearlyLocatedAlongILinearElement" strength="referencing" modifier="None">
                <BaseClass>ElementRefersToElements</BaseClass>
                <Source multiplicity="(0..*)" polymorphic="true" roleLabel="along">
                    <Class class="ILinearlyLocated" />
                </Source>
                <Target multiplicity="(0..1)" polymorphic="true" roleLabel="is linear axis for">
                    <Class class="ILinearElement" />
                </Target>
            </ECRelationshipClass>
        <ECRelationshipClass typeName="GraphicalElement3dRepresentsElement" strength="referencing" modifier="None">
                <BaseClass>ElementRefersToElements</BaseClass>
                <Source multiplicity="(0..*)" roleLabel="represents" polymorphic="true">
                    <Class class="GraphicalElement3d" />
                </Source>
                <Target multiplicity="(0..*)" roleLabel="is represented by" polymorphic="true">
                    <Class class="Element" />
                </Target>
            </ECRelationshipClass>
        <ECEntityClass typeName="GraphicalElement3d" modifier="Abstract">
                <BaseClass>GeometricElement3d</BaseClass>
            </ECEntityClass>
        <ECEntityClass typeName="ILinearElement" modifier="Abstract">
                <ECCustomAttributes>
                    <IsMixin xmlns="CoreCustomAttributes.1.0.0">
                        <!-- Only subclasses of bis:Element can implement the IParentElement interface -->
                        <AppliesToEntityClass>Element</AppliesToEntityClass>
                    </IsMixin>
                </ECCustomAttributes>
                <ECProperty propertyName="StartValue" typeName="double" />
                <ECProperty propertyName="LengthValue" typeName="double" />
        </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("RoadRailPhysicalManyJoins.ecdb", schemaItem));
    
    Utf8String sql = "";

    // Original query with optimization turned off
    m_ecdb.GetECSqlConfig().SetOptimizationOption(OptimizationOptions::OptimizeJoinForClassIds, false);
    sql = GetHelper().ECSqlToSql(R"statement(
        SELECT
            COUNT(*)
        FROM
            RoadRailPhysical.Corridor c,
            RoadRailPhysical.TransportationSystem ts,
            RoadRailPhysical.CorridorPortionElement cp,
            RoadRailPhysical.PhysicalElement pe,
            RoadRailPhysical.ILinearlyLocatedAlongILinearElement lle,
            RoadRailPhysical.GraphicalElement3dRepresentsElement gre,
            RoadRailPhysical.GraphicalElement3d ge
        WHERE
            lle.TargetECInstanceId = 0x20000003e91
            AND c.ECInstanceId = lle.SourceECInstanceId
            AND c.ECInstanceId = ts.Model.Id
            AND cp.Model.Id = ts.ECInstanceId
            AND pe.Parent.Id = cp.ECInstanceId
            AND gre.TargetECInstanceId = pe.ECInstanceId
            AND ge.ECInstanceId = gre.SourceECInstanceId
        )statement");

    EXPECT_EQ(8, GetHelper().GetFrequencyCount(sql, "JOIN"));
    EXPECT_EQ(2, GetHelper().GetFrequencyCount(sql, "SourceECClassId"));
    EXPECT_EQ(2, GetHelper().GetFrequencyCount(sql, "TargetECClassId"));
    
    // Optimized query with optimization turned on
    m_ecdb.GetECSqlConfig().SetOptimizationOption(OptimizationOptions::OptimizeJoinForClassIds, true);
    sql = GetHelper().ECSqlToSql(R"statement(
        SELECT
            COUNT(*)
        FROM
            RoadRailPhysical.Corridor c,
            RoadRailPhysical.TransportationSystem ts,
            RoadRailPhysical.CorridorPortionElement cp,
            RoadRailPhysical.PhysicalElement pe,
            RoadRailPhysical.ILinearlyLocatedAlongILinearElement lle,
            RoadRailPhysical.GraphicalElement3dRepresentsElement gre,
            RoadRailPhysical.GraphicalElement3d ge
        WHERE
            lle.TargetECInstanceId = 0x20000003e91
            AND c.ECInstanceId = lle.SourceECInstanceId
            AND c.ECInstanceId = ts.Model.Id
            AND cp.Model.Id = ts.ECInstanceId
            AND pe.Parent.Id = cp.ECInstanceId
            AND gre.TargetECInstanceId = pe.ECInstanceId
            AND ge.ECInstanceId = gre.SourceECInstanceId
        )statement");

    EXPECT_EQ(4, GetHelper().GetFrequencyCount(sql, "JOIN"));
    EXPECT_EQ(0, GetHelper().GetFrequencyCount(sql, "SourceECClassId"));
    EXPECT_EQ(0, GetHelper().GetFrequencyCount(sql, "TargetECClassId"));

    // Improved query with optimization turned off
    m_ecdb.GetECSqlConfig().SetOptimizationOption(OptimizationOptions::OptimizeJoinForClassIds, false);
    sql = GetHelper().ECSqlToSql(R"statement(
        SELECT 
            ge.ECInstanceId, 
            ge.ECClassId 
        FROM 
            RoadRailPhysical.Element c, 
            RoadRailPhysical.Element ts, 
            RoadRailPhysical.Element cp, 
            RoadRailPhysical.Element pe, 
            RoadRailPhysical.ILinearlyLocatedAlongILinearElement lle, 
            RoadRailPhysical.GraphicalElement3dRepresentsElement gre, 
            RoadRailPhysical.Element ge  
        WHERE 
            c.ECClassId IS (RoadRailPhysical.Corridor)
            AND ts.ECClassId IS (RoadRailPhysical.TransportationSystem)
            AND cp.ECClassId IS (RoadRailPhysical.CorridorPortionElement)
            AND pe.ECClassId IS (RoadRailPhysical.PhysicalElement)
            AND ge.ECClassId IS (RoadRailPhysical.GraphicalElement3d)
            AND lle.TargetECInstanceId = 0x20000003e91 
            AND c.ECInstanceId = lle.SourceECInstanceId 
            AND c.ECInstanceId = ts.Model.Id 
            AND cp.Model.Id = ts.ECInstanceId 
            AND pe.Parent.Id = cp.ECInstanceId 
            AND gre.TargetECInstanceId = pe.ECInstanceId 
            AND ge.ECInstanceId = gre.SourceECInstanceId 
        ORDER BY ge.ECClassId
        )statement");

    EXPECT_EQ(4, GetHelper().GetFrequencyCount(sql, "JOIN"));
    EXPECT_EQ(2, GetHelper().GetFrequencyCount(sql, "SourceECClassId"));
    EXPECT_EQ(2, GetHelper().GetFrequencyCount(sql, "TargetECClassId"));

    // Improved query with optimization turned on
    m_ecdb.GetECSqlConfig().SetOptimizationOption(OptimizationOptions::OptimizeJoinForClassIds, true);
    sql = GetHelper().ECSqlToSql(R"statement(
        SELECT 
            ge.ECInstanceId, 
            ge.ECClassId 
        FROM 
            RoadRailPhysical.Element c, 
            RoadRailPhysical.Element ts, 
            RoadRailPhysical.Element cp, 
            RoadRailPhysical.Element pe, 
            RoadRailPhysical.ILinearlyLocatedAlongILinearElement lle, 
            RoadRailPhysical.GraphicalElement3dRepresentsElement gre, 
            RoadRailPhysical.Element ge  
        WHERE 
            c.ECClassId IS (RoadRailPhysical.Corridor)
            AND ts.ECClassId IS (RoadRailPhysical.TransportationSystem)
            AND cp.ECClassId IS (RoadRailPhysical.CorridorPortionElement)
            AND pe.ECClassId IS (RoadRailPhysical.PhysicalElement)
            AND ge.ECClassId IS (RoadRailPhysical.GraphicalElement3d)
            AND lle.TargetECInstanceId = 0x20000003e91 
            AND c.ECInstanceId = lle.SourceECInstanceId 
            AND c.ECInstanceId = ts.Model.Id 
            AND cp.Model.Id = ts.ECInstanceId 
            AND pe.Parent.Id = cp.ECInstanceId 
            AND gre.TargetECInstanceId = pe.ECInstanceId 
            AND ge.ECInstanceId = gre.SourceECInstanceId 
        ORDER BY ge.ECClassId
        )statement");

    EXPECT_EQ(0, GetHelper().GetFrequencyCount(sql, "JOIN"));
    EXPECT_EQ(0, GetHelper().GetFrequencyCount(sql, "SourceECClassId"));
    EXPECT_EQ(0, GetHelper().GetFrequencyCount(sql, "TargetECClassId"));
    
    m_ecdb.CloseDb();
    }
END_ECDBUNITTESTS_NAMESPACE
