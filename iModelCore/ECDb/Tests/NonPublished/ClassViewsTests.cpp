/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

struct ClassViewsFixture : public ECDbTestFixture {};

struct TestIssueListener: ECN::IIssueListener {
    mutable std::vector<Utf8String> m_issues;
    void _OnIssueReported(ECN::IssueSeverity severity, ECN::IssueCategory category, ECN::IssueType type, ECN::IssueId issueId, Utf8CP message) const override {
        m_issues.push_back(message);
    }
    Utf8String const& GetLastError() const { return m_issues.back();}
    Utf8String PopLastError() {
        Utf8String str = m_issues.back();
        m_issues.pop_back();
        return str;
    }
    void Dump (Utf8CP listenerName) {
        Utf8String cppCode;
        for(auto it = m_issues.rbegin(); it != m_issues.rend(); ++it) {
            cppCode.append(Utf8PrintfString("ASSERT_STREQ(\"%s\", %s.PopLastError().c_str());\r\n", (*it).c_str(), listenerName));
        }
        printf("%s", cppCode.c_str());
    }
    void Reset() { m_issues.clear(); }
};

struct GetDbValueFunc final : ScalarFunction {
  private:
      Db const* m_db;
      void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override {
        if (nArgs != 3 || args[0].GetValueType() != DbValueType::TextVal || args[1].GetValueType() != DbValueType::TextVal || args[2].GetValueType() != DbValueType::IntegerVal) {
            return;
        }
        const auto tableName = args[0].GetValueText();
        const auto columnName= args[1].GetValueText();
        const auto rowId = args[2].GetValueId<ECInstanceId>();
        Utf8String sql = SqlPrintfString("SELECT [%s] FROM [%s] WHERE [ROWID]=%s", columnName, tableName, rowId.ToHexStr().c_str()).GetUtf8CP();
        auto stmt = m_db->GetCachedStatement(sql.c_str());
        if (stmt.IsValid() && stmt->Step() == BE_SQLITE_ROW) {
          ctx.SetResultValue(stmt->GetDbValue(0));
        }
      }

  public:
      GetDbValueFunc(DbCR db):ScalarFunction("get_val", 3), m_db(&db) {}
      ~GetDbValueFunc() {}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassViewsFixture, prepare_view_and_check_validate_sql_and_data) {
    auto testSchema = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
    <ECSchema
            schemaName="test_schema"
            alias="ts"
            version="1.0.0"
            xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name='ECDbMap' version='02.00.03' alias='ecdbmap' />
        <ECEntityClass typeName="SchemaClassesView" description="" displayLabel="" modifier="Abstract">
            <ECCustomAttributes>
                <View xmlns="ECDbMap.02.00.03">
                    <Query>
                        SELECT
                            cd.ECInstanceId,
                            cd.ECClassId,
                            sc.Name SchemaName,
                            cd.Name ClassName
                        FROM meta.ECSchemaDef sc
                            JOIN meta.ECClassDef cd ON cd.Schema.Id=sc.ECInstanceId
                        GROUP BY sc.Name, cd.Name
                        LIMIT 50
                    </Query>
                </View>
           </ECCustomAttributes>
            <ECProperty propertyName="SchemaName" typeName="string" />
            <ECProperty propertyName="ClassName"  typeName="string"/>
        </ECEntityClass>
    </ECSchema>)xml");

    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("test.ecdb"));
    ASSERT_EQ(SUCCESS, ImportSchema(testSchema));

    if (true){
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId, ECClassId, SchemaName, ClassName FROM ts.SchemaClassesView WHERE ClassName='View'"));
        auto nativeSql = "SELECT [K0],[K1],[K2],[K3] FROM (SELECT [cd].[ECInstanceId] [K0],[cd].[ECClassId] [K1],[sc].[Name] [K2],[cd].[Name] [K3] FROM (SELECT [Id] ECInstanceId,38 ECClassId,[Name] FROM [main].[ec_Schema]) [sc] INNER JOIN (SELECT [Id] ECInstanceId,36 ECClassId,[SchemaId],[Name] FROM [main].[ec_Class]) [cd] ON [cd].[SchemaId]=[sc].[ECInstanceId]   GROUP BY [sc].[Name],[cd].[Name]  LIMIT 50) [SchemaClassesView] WHERE [K3]='View'";
        ASSERT_STREQ(nativeSql, stmt.GetNativeSql());
        ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);
        //ASSERT_EQ(stmt.GetValueId<ECInstanceId>(0), ECInstanceId(27ull));
        //ASSERT_EQ(stmt.GetValueId<ECN::ECClassId>(1), ECClassId(33ull));
        ASSERT_STREQ(stmt.GetValueText(2), "ECDbMap");
        ASSERT_STREQ(stmt.GetValueText(3), "View");
    }
    if (true){
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM ts.SchemaClassesView"));
        ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);
        ASSERT_EQ(stmt.GetValueInt(0), 50);
    }
    if (true){
        // should fail to prepare insert against a view
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "Insert into ts.SchemaClassesView(SchemaName, ClassName) Values(?, ?)"));
    }
    if (true){
        // should fail to prepare insert against a view
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "update ts.SchemaClassesView set schemaname = ?"));
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassViewsFixture, linktable_relationship_view) {
    NativeLogging::Logging::SetLogger(&NativeLogging::ConsoleLogger::GetLogger());
    NativeLogging::ConsoleLogger::GetLogger().SetSeverity("ECDb", BentleyApi::NativeLogging::LOG_TRACE);
    NativeLogging::ConsoleLogger::GetLogger().SetSeverity("ECObjectsNative", BentleyApi::NativeLogging::LOG_TRACE);
    auto testSchema = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
    <ECSchema
            schemaName="test_schema"
            alias="ts"
            version="1.0.0"
            xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name='ECDbMap' version='02.00.03' alias='ecdbmap' />
        <ECSchemaReference name='ECDbMeta' version='04.00.02' alias='meta' />
        <ECRelationshipClass typeName="SchemaClassesView" description="" displayLabel="" strength="referencing" modifier="Abstract">
            <ECCustomAttributes>
                <View xmlns="ECDbMap.02.00.03">
                    <Query>
                        SELECT * FROM meta.SchemaOwnsClasses
                    </Query>
                </View>
            </ECCustomAttributes>
            <Source multiplicity="(1..1)" roleLabel="contains" polymorphic="false">
                <Class class="meta:ECSchemaDef"/>
            </Source>
            <Target multiplicity="(0..*)" roleLabel="is defined in" polymorphic="false">
                <Class class="meta:ECClassDef"/>
            </Target>
        </ECRelationshipClass>
    </ECSchema>)xml");

    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("test.ecdb"));
    ASSERT_EQ(SUCCESS, ImportSchema(testSchema));
    if (true){
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * FROM ts.SchemaClassesView"));
        auto nativeSql = "SELECT [K0],[K1],[K2],[K3],[K4],[K5] FROM (SELECT [SchemaOwnsClasses].[ECInstanceId] [K0],[SchemaOwnsClasses].[ECClassId] [K1],[SchemaOwnsClasses].[SourceECInstanceId] [K2],[SchemaOwnsClasses].[SourceECClassId] [K3],[SchemaOwnsClasses].[TargetECInstanceId] [K4],[SchemaOwnsClasses].[TargetECClassId] [K5] FROM (SELECT [ec_Class].[Id] ECInstanceId,34 ECClassId,[ec_Class].[SchemaId] SourceECInstanceId,35 SourceECClassId,[ec_Class].[Id] TargetECInstanceId,33 TargetECClassId FROM [main].[ec_Class] WHERE [ec_Class].[SchemaId] IS NOT NULL) [SchemaOwnsClasses])";
        ASSERT_STREQ(nativeSql, stmt.GetNativeSql());
        ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);
        ASSERT_EQ(stmt.GetValueId<ECInstanceId>(0), ECInstanceId(1ull));
        ASSERT_EQ(stmt.GetValueId<ECN::ECClassId>(1), ECClassId(34ull));
        ASSERT_EQ(stmt.GetValueId<ECInstanceId>(2), ECInstanceId(1ull));
        ASSERT_EQ(stmt.GetValueId<ECClassId>(3), ECClassId(35ull));
        ASSERT_EQ(stmt.GetValueId<ECInstanceId>(4), ECInstanceId(1ull));
        ASSERT_EQ(stmt.GetValueId<ECClassId>(5), ECClassId(33ull));
    }
    ASSERT_FALSE(m_ecdb.TableExists("ts_SchemaClassesView")) << "A abstract linktable is still mapped to a table.";
}
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassViewsFixture, return_nav_prop_from_view_query) {
    auto testSchema = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
    <ECSchema
            schemaName="test_schema"
            alias="ts"
            version="1.0.0"
            xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name='ECDbMap' version='02.00.03' alias='ecdbmap' />
        <ECSchemaReference name='ECDbMeta' version='04.00.02' alias='meta' />
        <ECRelationshipClass typeName="SchemaClassesView" description="" displayLabel="" strength="referencing" modifier="Abstract">
            <Source multiplicity="(1..1)" roleLabel="contains" polymorphic="false">
                <Class class="meta:ECSchemaDef"/>
            </Source>
            <Target multiplicity="(0..*)" roleLabel="is defined in" polymorphic="false">
                <Class class="ClassDefView"/>
            </Target>
        </ECRelationshipClass>
        <ECEntityClass typeName="ClassDefView" description="" displayLabel="" modifier="Abstract">
            <ECCustomAttributes>
                <View xmlns="ECDbMap.02.00.03">
                    <Query>SELECT ECInstanceId, ECClassId, Schema FROM meta.ECClassDef</Query>
                </View>
            </ECCustomAttributes>
            <ECNavigationProperty propertyName="Schema" relationshipName="SchemaClassesView" direction="backward"/>
        </ECEntityClass>
    </ECSchema>)xml");

    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("test.ecdb"));
    ASSERT_EQ(SUCCESS, ImportSchema(testSchema));
    if (true){
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * FROM ts.ClassDefView"));
        auto nativeSql = "SELECT [K0],[K1],[K2_0],[K2_1] FROM (SELECT [ECClassDef].[ECInstanceId] [K0],[ECClassDef].[ECClassId] [K1],[ECClassDef].[SchemaId] [K2_0],[ECClassDef].[SchemaRelECClassId] [K2_1] FROM (SELECT [Id] ECInstanceId,36 ECClassId,[SchemaId],(CASE WHEN [SchemaId] IS NULL THEN NULL ELSE 37 END) [SchemaRelECClassId] FROM [main].[ec_Class]) [ECClassDef]) [ClassDefView]";
        ASSERT_STREQ(nativeSql, stmt.GetNativeSql());
        ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);
        ASSERT_EQ(stmt.GetValueId<ECInstanceId>(0), ECInstanceId(1ull));
        ASSERT_EQ(stmt.GetValueId<ECN::ECClassId>(1), ECClassId(36ull));
        ECN::ECClassId relId;
        ASSERT_EQ(stmt.GetValueNavigation<ECInstanceId>(2, &relId), ECInstanceId(1ull));
        ASSERT_EQ(relId, ECClassId(37ull));
    }
}
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassViewsFixture, fail_when_view_reference_itself_directly_or_indirectly) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("test.ecdb"));
    TestIssueListener listener;
    m_ecdb.AddIssueListener(listener);

    if ("direct cyclic dependent view") {
        auto testSchema = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema
                schemaName="test_schema"
                alias="ts"
                version="1.0.0"
                xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name='ECDbMap' version='02.00.03' alias='ecdbmap' />
            <ECEntityClass typeName="SchemaView" description="" displayLabel="" modifier="Abstract">
                <ECCustomAttributes>
                    <View xmlns="ECDbMap.02.00.03">
                        <Query>SELECT cd.ECInstanceId, cd.ECClassId FROM ts.SchemaView</Query>
                    </View>
                </ECCustomAttributes>
            </ECEntityClass>
        </ECSchema>)xml");

        listener.Reset();
        ASSERT_EQ(ERROR, ImportSchema(testSchema));
        ASSERT_STREQ("Total of 4 view classes were checked and 1 were found to be invalid.", listener.PopLastError().c_str());
        ASSERT_STREQ("Invalid view class 'test_schema:SchemaView'. Failed to prepare view query (SELECT cd.ECInstanceId, cd.ECClassId FROM ts.SchemaView)", listener.PopLastError().c_str());
        ASSERT_STREQ("Invalid View Class 'test_schema:SchemaView'. View ECSQL failed to parse.", listener.PopLastError().c_str());
        ASSERT_STREQ("Invalid View Class 'test_schema:SchemaView'. View query references itself recusively (test_schema:SchemaView -> test_schema:SchemaView).", listener.PopLastError().c_str());
        m_ecdb.AbandonChanges();
    }

    if ("indirect cyclic dependent view") {
        auto testSchema = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema
                schemaName="test_schema"
                alias="ts"
                version="1.0.0"
                xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name='ECDbMap' version='02.00.03' alias='ecdbmap' />
            <ECEntityClass typeName="SchemaView" description="" displayLabel="" modifier="Abstract">
                <ECCustomAttributes>
                    <View xmlns="ECDbMap.02.00.03">
                        <Query>SELECT cd.ECInstanceId, cd.ECClassId FROM ts.ClassView</Query>
                    </View>
                </ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="ClassView" description="" displayLabel="" modifier="Abstract">
                <ECCustomAttributes>
                    <View xmlns="ECDbMap.02.00.03">
                        <Query>SELECT cd.ECInstanceId, cd.ECClassId FROM ts.SchemaView</Query>
                    </View>
                </ECCustomAttributes>
            </ECEntityClass>
        </ECSchema>)xml");

        listener.Reset();
        ASSERT_EQ(ERROR, ImportSchema(testSchema));
        ASSERT_STREQ("Total of 5 view classes were checked and 2 were found to be invalid.", listener.PopLastError().c_str());
        ASSERT_STREQ("Invalid view class 'test_schema:SchemaView'. Failed to prepare view query (SELECT cd.ECInstanceId, cd.ECClassId FROM ts.ClassView)", listener.PopLastError().c_str());
        ASSERT_STREQ("Invalid View Class 'test_schema:ClassView'. View ECSQL failed to parse.", listener.PopLastError().c_str());
        ASSERT_STREQ("Invalid View Class 'test_schema:SchemaView'. View ECSQL failed to parse.", listener.PopLastError().c_str());
        ASSERT_STREQ("Invalid View Class 'test_schema:ClassView'. View query references itself recusively (test_schema:ClassView -> test_schema:SchemaView -> test_schema:ClassView).", listener.PopLastError().c_str());
        ASSERT_STREQ("Invalid view class 'test_schema:ClassView'. Failed to prepare view query (SELECT cd.ECInstanceId, cd.ECClassId FROM ts.SchemaView)", listener.PopLastError().c_str());
        ASSERT_STREQ("Invalid View Class 'test_schema:SchemaView'. View ECSQL failed to parse.", listener.PopLastError().c_str());
        ASSERT_STREQ("Invalid View Class 'test_schema:ClassView'. View ECSQL failed to parse.", listener.PopLastError().c_str());
        ASSERT_STREQ("Invalid View Class 'test_schema:SchemaView'. View query references itself recusively (test_schema:SchemaView -> test_schema:ClassView -> test_schema:SchemaView).", listener.PopLastError().c_str());
        m_ecdb.AbandonChanges();
    }
    if ("indirect cyclic dependent view") {
        auto testSchema = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema
                schemaName="test_schema"
                alias="ts"
                version="1.0.0"
                xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name='ECDbMap' version='02.00.03' alias='ecdbmap' />
            <ECEntityClass typeName="View1" description="" displayLabel="" modifier="Abstract">
                <ECCustomAttributes>
                    <View xmlns="ECDbMap.02.00.03">
                        <Query>SELECT cd.ECInstanceId, cd.ECClassId FROM ts.View2</Query>
                    </View>
                </ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="View2" description="" displayLabel="" modifier="Abstract">
                <ECCustomAttributes>
                    <View xmlns="ECDbMap.02.00.03">
                        <Query>SELECT cd.ECInstanceId, cd.ECClassId FROM ts.View3</Query>
                    </View>
                </ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="View3" description="" displayLabel="" modifier="Abstract">
                <ECCustomAttributes>
                    <View xmlns="ECDbMap.02.00.03">
                        <Query>SELECT cd.ECInstanceId, cd.ECClassId FROM ts.View1</Query>
                    </View>
                </ECCustomAttributes>
            </ECEntityClass>
        </ECSchema>)xml");

        listener.Reset();
        ASSERT_EQ(ERROR, ImportSchema(testSchema));
        ASSERT_STREQ("Total of 6 view classes were checked and 3 were found to be invalid.", listener.PopLastError().c_str());
        ASSERT_STREQ("Invalid view class 'test_schema:View3'. Failed to prepare view query (SELECT cd.ECInstanceId, cd.ECClassId FROM ts.View1)", listener.PopLastError().c_str());
        ASSERT_STREQ("Invalid View Class 'test_schema:View1'. View ECSQL failed to parse.", listener.PopLastError().c_str());
        ASSERT_STREQ("Invalid View Class 'test_schema:View2'. View ECSQL failed to parse.", listener.PopLastError().c_str());
        ASSERT_STREQ("Invalid View Class 'test_schema:View3'. View ECSQL failed to parse.", listener.PopLastError().c_str());
        ASSERT_STREQ("Invalid View Class 'test_schema:View1'. View query references itself recusively (test_schema:View1 -> test_schema:View2 -> test_schema:View3 -> test_schema:View1).", listener.PopLastError().c_str());
        ASSERT_STREQ("Invalid view class 'test_schema:View2'. Failed to prepare view query (SELECT cd.ECInstanceId, cd.ECClassId FROM ts.View3)", listener.PopLastError().c_str());
        ASSERT_STREQ("Invalid View Class 'test_schema:View3'. View ECSQL failed to parse.", listener.PopLastError().c_str());
        ASSERT_STREQ("Invalid View Class 'test_schema:View1'. View ECSQL failed to parse.", listener.PopLastError().c_str());
        ASSERT_STREQ("Invalid View Class 'test_schema:View2'. View ECSQL failed to parse.", listener.PopLastError().c_str());
        ASSERT_STREQ("Invalid View Class 'test_schema:View3'. View query references itself recusively (test_schema:View3 -> test_schema:View1 -> test_schema:View2 -> test_schema:View3).", listener.PopLastError().c_str());
        ASSERT_STREQ("Invalid view class 'test_schema:View1'. Failed to prepare view query (SELECT cd.ECInstanceId, cd.ECClassId FROM ts.View2)", listener.PopLastError().c_str());
        ASSERT_STREQ("Invalid View Class 'test_schema:View2'. View ECSQL failed to parse.", listener.PopLastError().c_str());
        ASSERT_STREQ("Invalid View Class 'test_schema:View3'. View ECSQL failed to parse.", listener.PopLastError().c_str());
        ASSERT_STREQ("Invalid View Class 'test_schema:View1'. View ECSQL failed to parse.", listener.PopLastError().c_str());
        ASSERT_STREQ("Invalid View Class 'test_schema:View2'. View query references itself recusively (test_schema:View2 -> test_schema:View3 -> test_schema:View1 -> test_schema:View2).", listener.PopLastError().c_str());
    }
}
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassViewsFixture, all_specified_view_properties_must_return_by_view_query) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("test.ecdb"));
    TestIssueListener listener;
    m_ecdb.AddIssueListener(listener);

    if ("view and query prop matches") {
        auto testSchema = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema
                schemaName="test_schema"
                alias="ts"
                version="1.0.0"
                xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name='ECDbMap' version='02.00.03' alias='ecdbmap' />
            <ECEntityClass typeName="P" modifier="Sealed">
                <ECProperty propertyName="intProp" typeName="int"/>
                <ECProperty propertyName="longProp" typeName="long"/>
                <ECProperty propertyName="doubleProp" typeName="double"/>
                <ECProperty propertyName="stringProp" typeName="string"/>
                <ECProperty propertyName="dateTimeProp" typeName="dateTime"/>
                <ECProperty propertyName="binaryProp" typeName="binary"/>
                <ECProperty propertyName="booleanProp" typeName="boolean"/>
                <ECProperty propertyName="point2dProp" typeName="point2d"/>
                <ECProperty propertyName="point3dProp" typeName="point3d"/>
            </ECEntityClass>
            <ECEntityClass typeName="P_View" description="" displayLabel="" modifier="Abstract">
                <ECCustomAttributes>
                    <View xmlns="ECDbMap.02.00.03">
                        <Query>SELECT * FROM ts.P</Query>
                    </View>
                </ECCustomAttributes>
                <ECProperty propertyName="intProp" typeName="int"/>
                <ECProperty propertyName="longProp" typeName="long"/>
                <ECProperty propertyName="doubleProp" typeName="double"/>
                <ECProperty propertyName="stringProp" typeName="string"/>
                <ECProperty propertyName="dateTimeProp" typeName="dateTime"/>
                <ECProperty propertyName="binaryProp" typeName="binary"/>
                <ECProperty propertyName="booleanProp" typeName="boolean"/>
                <ECProperty propertyName="point2dProp" typeName="point2d"/>
                <ECProperty propertyName="point3dProp" typeName="point3d"/>
            </ECEntityClass>
        </ECSchema>)xml");

        listener.Reset();
        ASSERT_EQ(SUCCESS, ImportSchema(testSchema));
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * FROM ts.P_View"));
        ASSERT_EQ(stmt.GetColumnCount(), 11);
        m_ecdb.AbandonChanges();
    }
    if ("view class has property missing in view query") {
        auto testSchema = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema
                schemaName="test_schema"
                alias="ts"
                version="1.0.0"
                xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name='ECDbMap' version='02.00.03' alias='ecdbmap' />
            <ECEntityClass typeName="P" modifier="Sealed">
                <ECProperty propertyName="intProp" typeName="int"/>
                <ECProperty propertyName="longProp" typeName="long"/>
            </ECEntityClass>
            <ECEntityClass typeName="P_View" description="" displayLabel="" modifier="Abstract">
                <ECCustomAttributes>
                    <View xmlns="ECDbMap.02.00.03">
                        <Query>SELECT intProp, longProp FROM ts.P</Query>
                    </View>
                </ECCustomAttributes>
                <ECProperty propertyName="intProp" typeName="int"/>
                <ECProperty propertyName="longProp" typeName="long"/>
                <ECProperty propertyName="doubleProp" typeName="double"/>
            </ECEntityClass>
        </ECSchema>)xml");

        listener.Reset();
        ASSERT_EQ(ERROR, ImportSchema(testSchema));
        ASSERT_STREQ("Total of 4 view classes were checked and 1 were found to be invalid.", listener.PopLastError().c_str());
        ASSERT_STREQ("Invalid view class 'test_schema:P_View'. View class has property 'doubleProp' which is not returned by view query.", listener.PopLastError().c_str());
        m_ecdb.AbandonChanges();
    }
    if ("view class has property that has different type then returned by query") {
        auto testSchema = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema
                schemaName="test_schema"
                alias="ts"
                version="1.0.0"
                xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name='ECDbMap' version='02.00.03' alias='ecdbmap' />
            <ECEntityClass typeName="P" modifier="Sealed">
                <ECProperty propertyName="intProp" typeName="int"/>
                <ECProperty propertyName="longProp" typeName="long"/>
                <ECProperty propertyName="doubleProp" typeName="double"/>
            </ECEntityClass>
            <ECEntityClass typeName="P_View" description="" displayLabel="" modifier="Abstract">
                <ECCustomAttributes>
                    <View xmlns="ECDbMap.02.00.03">
                        <Query>SELECT intProp, longProp, doubleProp FROM ts.P</Query>
                    </View>
                </ECCustomAttributes>
                <ECProperty propertyName="intProp" typeName="int"/>
                <ECProperty propertyName="longProp" typeName="long"/>
                <ECProperty propertyName="doubleProp" typeName="string"/>
            </ECEntityClass>
        </ECSchema>)xml");

        listener.Reset();
        ASSERT_EQ(ERROR, ImportSchema(testSchema));
        ASSERT_STREQ("Total of 4 view classes were checked and 1 were found to be invalid.", listener.PopLastError().c_str());
        ASSERT_STREQ("Invalid view class 'test_schema:P_View'. View class property 'doubleProp' type does not match the type returned by view query ('string' <> 'double').", listener.PopLastError().c_str());
        listener.Dump("listener");
        m_ecdb.AbandonChanges();
    }
    if ("query return data property that is not present in view class") {
        auto testSchema = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema
                schemaName="test_schema"
                alias="ts"
                version="1.0.0"
                xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name='ECDbMap' version='02.00.03' alias='ecdbmap' />
            <ECEntityClass typeName="P" modifier="Sealed">
                <ECProperty propertyName="intProp" typeName="int"/>
                <ECProperty propertyName="longProp" typeName="long"/>
                <ECProperty propertyName="doubleProp" typeName="double"/>
            </ECEntityClass>
            <ECEntityClass typeName="P_View" description="" displayLabel="" modifier="Abstract">
                <ECCustomAttributes>
                    <View xmlns="ECDbMap.02.00.03">
                        <Query>SELECT intProp, longProp, doubleProp FROM ts.P</Query>
                    </View>
                </ECCustomAttributes>
                <ECProperty propertyName="intProp" typeName="int"/>
                <ECProperty propertyName="longProp" typeName="long"/>
            </ECEntityClass>
        </ECSchema>)xml");

        listener.Reset();
        ASSERT_EQ(ERROR, ImportSchema(testSchema));
        ASSERT_STREQ("Total of 4 view classes were checked and 1 were found to be invalid.", listener.PopLastError().c_str());
        ASSERT_STREQ("Invalid view class 'test_schema:P_View'. View query return property 'doubleProp' which not defined in view class or is a invalid system property.", listener.PopLastError().c_str());
        listener.Dump("listener");
        m_ecdb.AbandonChanges();
    }
    if ("system properties can be returned by view query for entity class") {
        auto testSchema = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema
                schemaName="test_schema"
                alias="ts"
                version="1.0.0"
                xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name='ECDbMap' version='02.00.03' alias='ecdbmap' />
            <ECEntityClass typeName="P" modifier="Sealed">
                <ECProperty propertyName="intProp" typeName="int"/>
                <ECProperty propertyName="longProp" typeName="long"/>
                <ECProperty propertyName="doubleProp" typeName="double"/>
            </ECEntityClass>
            <ECEntityClass typeName="P_View" description="" displayLabel="" modifier="Abstract">
                <ECCustomAttributes>
                    <View xmlns="ECDbMap.02.00.03">
                        <Query>SELECT ECInstanceId, ECClassId, intProp, longProp, doubleProp FROM ts.P</Query>
                    </View>
                </ECCustomAttributes>
                <ECProperty propertyName="intProp" typeName="int"/>
                <ECProperty propertyName="longProp" typeName="long"/>
                <ECProperty propertyName="doubleProp" typeName="double"/>
            </ECEntityClass>
        </ECSchema>)xml");

        listener.Reset();
        ASSERT_EQ(SUCCESS, ImportSchema(testSchema));
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId, ECClassId FROM ts.P_View"));
        ASSERT_EQ(stmt.GetColumnCount(), 2);
        m_ecdb.AbandonChanges();
    }
    if ("if system property is not selected view query, the select query will fail") {
        auto testSchema = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema
                schemaName="test_schema"
                alias="ts"
                version="1.0.0"
                xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name='ECDbMap' version='02.00.03' alias='ecdbmap' />
            <ECEntityClass typeName="P" modifier="Sealed">
                <ECProperty propertyName="intProp" typeName="int"/>
                <ECProperty propertyName="longProp" typeName="long"/>
                <ECProperty propertyName="doubleProp" typeName="double"/>
            </ECEntityClass>
            <ECEntityClass typeName="P_View" description="" displayLabel="" modifier="Abstract">
                <ECCustomAttributes>
                    <View xmlns="ECDbMap.02.00.03">
                        <Query>SELECT intProp, longProp, doubleProp FROM ts.P</Query>
                    </View>
                </ECCustomAttributes>
                <ECProperty propertyName="intProp" typeName="int"/>
                <ECProperty propertyName="longProp" typeName="long"/>
                <ECProperty propertyName="doubleProp" typeName="double"/>
            </ECEntityClass>
        </ECSchema>)xml");

        listener.Reset();
        ASSERT_EQ(SUCCESS, ImportSchema(testSchema));
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT intProp, longProp, doubleProp FROM ts.P_View"));
        stmt.Finalize();
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT ECInstanceId, ECClassId FROM ts.P_View"));
        stmt.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * FROM ts.P_View"));
        ASSERT_EQ(stmt.GetColumnCount(), 3) << "system prop is not returned as its not selected ";
        stmt.Finalize();
        m_ecdb.AbandonChanges();
    }

}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassViewsFixture, complex_data) {
    ASSERT_EQ(SUCCESS, SetupECDb("complex_data.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECSchemaReference name="ECDbMap" version="02.00.03" alias="ecdbmap" />
                <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA" />
                <ECStructClass typeName="struct_p" description="Struct with primitive props (default mappings)">
                    <ECProperty propertyName="b" typeName="boolean" />
                    <ECProperty propertyName="bi" typeName="binary" />
                    <ECProperty propertyName="d" typeName="double" />
                    <ECProperty propertyName="dt" typeName="dateTime" />
                    <ECProperty propertyName="dtUtc" typeName="dateTime">
                        <ECCustomAttributes>
                            <DateTimeInfo xmlns="CoreCustomAttributes.01.00.00">
                                <DateTimeKind>Utc</DateTimeKind>
                            </DateTimeInfo>
                        </ECCustomAttributes>
                    </ECProperty>
                    <ECProperty propertyName="i" typeName="int" />
                    <ECProperty propertyName="l" typeName="long" />
                    <ECProperty propertyName="s" typeName="string" />
                    <ECProperty propertyName="p2d" typeName="point2d" />
                    <ECProperty propertyName="p3d" typeName="point3d" />
                    <ECProperty propertyName="geom" typeName="Bentley.Geometry.Common.IGeometry" />
                </ECStructClass>
                <ECStructClass typeName="struct_pa" description="Primitive array">
                    <ECArrayProperty propertyName="b_array" typeName="boolean" minOccurs="0" maxOccurs="unbounded" />
                    <ECArrayProperty propertyName="bi_array" typeName="binary" minOccurs="0" maxOccurs="unbounded" />
                    <ECArrayProperty propertyName="d_array" typeName="double" minOccurs="0" maxOccurs="unbounded" />
                    <ECArrayProperty propertyName="dt_array" typeName="dateTime" minOccurs="0" maxOccurs="unbounded" />
                    <ECArrayProperty propertyName="dtUtc_array" typeName="dateTime" minOccurs="0" maxOccurs="unbounded">
                        <ECCustomAttributes>
                            <DateTimeInfo xmlns="CoreCustomAttributes.01.00.00">
                                <DateTimeKind>Utc</DateTimeKind>
                            </DateTimeInfo>
                        </ECCustomAttributes>
                    </ECArrayProperty>
                    <ECArrayProperty propertyName="i_array" typeName="int" minOccurs="0" maxOccurs="unbounded" />
                    <ECArrayProperty propertyName="l_array" typeName="long" minOccurs="0" maxOccurs="unbounded" />
                    <ECArrayProperty propertyName="s_array" typeName="string" minOccurs="0" maxOccurs="unbounded" />
                    <ECArrayProperty propertyName="p2d_array" typeName="point2d" minOccurs="0" maxOccurs="unbounded" />
                    <ECArrayProperty propertyName="p3d_array" typeName="point3d" minOccurs="0" maxOccurs="unbounded" />
                    <ECArrayProperty propertyName="geom_array" typeName="Bentley.Geometry.Common.IGeometry" minOccurs="0" maxOccurs="unbounded" />
                </ECStructClass>
                <ECEntityClass typeName="e_mix"
                    description="Cover all primitive, primitive array, struct of primitive, array of struct">
                    <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00.03">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                        <ShareColumns xmlns="ECDbMap.02.00.03">
                            <MaxSharedColumnsBeforeOverflow>500</MaxSharedColumnsBeforeOverflow>
                            <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>
                        </ShareColumns>
                    </ECCustomAttributes>
                    <ECNavigationProperty propertyName="parent" relationshipName="e_mix_has_base_mix" direction="Backward">
                        <ECCustomAttributes>
                            <ForeignKeyConstraint xmlns="ECDbMap.02.00.03">
                                <OnDeleteAction>Cascade</OnDeleteAction>
                                <OnUpdateAction>Cascade</OnUpdateAction>
                            </ForeignKeyConstraint>
                        </ECCustomAttributes>
                    </ECNavigationProperty>
                    <ECProperty propertyName="b" typeName="boolean" />
                    <ECProperty propertyName="bi" typeName="binary" />
                    <ECProperty propertyName="d" typeName="double" />
                    <ECProperty propertyName="dt" typeName="dateTime" />
                    <ECProperty propertyName="dtUtc" typeName="dateTime">
                        <ECCustomAttributes>
                            <DateTimeInfo xmlns="CoreCustomAttributes.01.00.00">
                                <DateTimeKind>Utc</DateTimeKind>
                            </DateTimeInfo>
                        </ECCustomAttributes>
                    </ECProperty>
                    <ECProperty propertyName="i" typeName="int" />
                    <ECProperty propertyName="l" typeName="long" />
                    <ECProperty propertyName="s" typeName="string" />
                    <ECProperty propertyName="p2d" typeName="point2d" />
                    <ECProperty propertyName="p3d" typeName="point3d" />
                    <ECProperty propertyName="geom" typeName="Bentley.Geometry.Common.IGeometry" />
                    <ECArrayProperty propertyName="b_array" typeName="boolean" minOccurs="0" maxOccurs="unbounded" />
                    <ECArrayProperty propertyName="bi_array" typeName="binary" minOccurs="0" maxOccurs="unbounded" />
                    <ECArrayProperty propertyName="d_array" typeName="double" minOccurs="0" maxOccurs="unbounded" />
                    <ECArrayProperty propertyName="dt_array" typeName="dateTime" minOccurs="0" maxOccurs="unbounded" />
                    <ECArrayProperty propertyName="dtUtc_array" typeName="dateTime" minOccurs="0"
                        maxOccurs="unbounded">
                        <ECCustomAttributes>
                            <DateTimeInfo xmlns="CoreCustomAttributes.01.00.00">
                                <DateTimeKind>Utc</DateTimeKind>
                            </DateTimeInfo>
                        </ECCustomAttributes>
                    </ECArrayProperty>
                    <ECArrayProperty propertyName="i_array" typeName="int" minOccurs="0" maxOccurs="unbounded" />
                    <ECArrayProperty propertyName="l_array" typeName="long" minOccurs="0" maxOccurs="unbounded" />
                    <ECArrayProperty propertyName="s_array" typeName="string" minOccurs="0" maxOccurs="unbounded" />
                    <ECArrayProperty propertyName="p2d_array" typeName="point2d" minOccurs="0" maxOccurs="unbounded" />
                    <ECArrayProperty propertyName="p3d_array" typeName="point3d" minOccurs="0" maxOccurs="unbounded" />
                    <ECArrayProperty propertyName="geom_array" typeName="Bentley.Geometry.Common.IGeometry" minOccurs="0" maxOccurs="unbounded" />
                    <ECStructProperty propertyName="p" typeName="struct_p" />
                    <ECStructProperty propertyName="pa" typeName="struct_pa" />
                    <ECStructArrayProperty propertyName="array_of_p" typeName="struct_p" minOccurs="0" maxOccurs="unbounded" />
                    <ECStructArrayProperty propertyName="array_of_pa" typeName="struct_pa" minOccurs="0" maxOccurs="unbounded" />
                </ECEntityClass>
                <ECRelationshipClass typeName="e_mix_has_base_mix" strength="Embedding" modifier="Sealed">
                    <Source multiplicity="(0..1)" polymorphic="false" roleLabel="e_mix">
                        <Class class="e_mix" />
                    </Source>
                    <Target multiplicity="(0..*)" polymorphic="false" roleLabel="e_mix">
                        <Class class="e_mix" />
                    </Target>
                </ECRelationshipClass>
            </ECSchema>)xml")));
    m_ecdb.Schemas().CreateClassViewsInDb();
    m_ecdb.SaveChanges();
    ASSERT_FALSE(IsECSqlExperimentalFeaturesEnabled(m_ecdb));
    ASSERT_TRUE(EnableECSqlExperimentalFeatures(m_ecdb, true));

    ECInstanceKey instKey;
    if ("insert data") {
        ECSqlStatement stmt;
        auto rc = stmt.Prepare(m_ecdb, R"sql(
            insert into ts.e_mix(
                parent,
                b, bi, d, dt, dtUtc, i, l, s, p2d, p3d, geom,
                b_array, bi_array, d_array, dt_array, dtUtc_array,
                i_array, l_array, s_array, p2d_array, p3d_array, geom_array,
                p, pa, array_of_p, array_of_pa)
            values(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)
        )sql");

        const auto b = true;
        const uint8_t bi[] = {0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21};
        const double d = PI;
        const auto dt = DateTime(DateTime::Kind::Unspecified, 2017, 1, 17, 0, 0);
        const auto dtUtc = DateTime(DateTime::Kind::Utc, 2018, 2, 17, 0, 0);
        const int i = 0xfffafaff;
        const int64_t l = 0xfffafaffafffffff;
        const auto s = std::string{"Hello, World!"};
        const auto p2d = DPoint2d::From(22.33, -21.34);
        const auto p3d = DPoint3d::From(12.13, -42.34, -93.12);
        auto geom = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, 1.0, 1.0, 1.0)));
        const bool b_array[] = {true, false, true};
        const std::vector<std::vector<uint8_t>> bi_array = {
            {0x48, 0x65, 0x6},
            {0x48, 0x65, 0x6},
            {0x48, 0x65, 0x6}
        };
        const std::vector<double> d_array = {123.3434, 345.223, -532.123};
        const std::vector<DateTime> dt_array = {
            DateTime(DateTime::Kind::Unspecified, 2017, 1, 14, 0, 0),
            DateTime(DateTime::Kind::Unspecified, 2018, 1, 13, 0, 0),
            DateTime(DateTime::Kind::Unspecified, 2019, 1, 11, 0, 0),
        };
        const std::vector<DateTime> dtUtc_array = {
            DateTime(DateTime::Kind::Utc, 2017, 1, 17, 0, 0),
            DateTime(DateTime::Kind::Utc, 2018, 1, 11, 0, 0),
            DateTime(DateTime::Kind::Utc, 2019, 1, 10, 0, 0),
        };
        const std::vector<int> i_array = {3842, -4923, 8291};
        const std::vector<int64_t> l_array = {384242, -234923, 528291};
        const std::vector<std::string> s_array = {"Bentley", "System"};
        const std::vector<DPoint2d> p2d_array = {
            DPoint2d::From(22.33 , -81.17),
            DPoint2d::From(-42.74,  16.29),
            DPoint2d::From(77.45 , -32.98),
        };
        const std::vector<DPoint3d> p3d_array = {
            DPoint3d::From( 84.13,  99.23, -121.75),
            DPoint3d::From(-90.34,  45.75, -452.34),
            DPoint3d::From(-12.54, -84.23, -343.45),
        };
        const std::vector<IGeometryPtr> geom_array = {
            IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, 4.0, 2.1, 1.2))),
            IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, 1.1, 2.5, 4.2))),
            IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, 9.1, 3.6, 3.8))),
        };
        int idx = 0;
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindNull(++idx));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindBoolean(++idx, true));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindBlob(++idx, (void const*)bi, (int)sizeof(bi), IECSqlBinder::MakeCopy::No));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(++idx, d));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindDateTime(++idx, dt));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindDateTime(++idx, dtUtc));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(++idx, i));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(++idx, l));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(++idx, s.c_str(), IECSqlBinder::MakeCopy::No));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint2d(++idx, p2d));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint3d(++idx, p3d));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindGeometry(++idx, *geom));
        if (auto v = &stmt.GetBinder(++idx)) {
            for(auto& m : b_array)
                ASSERT_EQ(ECSqlStatus::Success, v->AddArrayElement().BindBoolean(m));
        }
        if (auto v = &stmt.GetBinder(++idx)) {
            for(auto& m : bi_array)
                ASSERT_EQ(ECSqlStatus::Success, v->AddArrayElement().BindBlob((void const*)&m[0], (int)m.size(), IECSqlBinder::MakeCopy::No));
        }
        if (auto v = &stmt.GetBinder(++idx)) {
            for(auto& m : d_array)
                ASSERT_EQ(ECSqlStatus::Success, v->AddArrayElement().BindDouble(m));
        }
        if (auto v = &stmt.GetBinder(++idx)) {
            for(auto& m : dt_array)
                ASSERT_EQ(ECSqlStatus::Success, v->AddArrayElement().BindDateTime(m));
        }
        if (auto v = &stmt.GetBinder(++idx)) {
            for(auto& m : dtUtc_array)
                ASSERT_EQ(ECSqlStatus::Success, v->AddArrayElement().BindDateTime(m));
        }
        if (auto v = &stmt.GetBinder(++idx)) {
            for(auto& m : i_array)
                ASSERT_EQ(ECSqlStatus::Success, v->AddArrayElement().BindInt(m));
        }
        if (auto v = &stmt.GetBinder(++idx)) {
            for(auto& m : l_array)
                ASSERT_EQ(ECSqlStatus::Success, v->AddArrayElement().BindInt64(m));
        }
        if (auto v = &stmt.GetBinder(++idx)) {
            for(auto& m : s_array)
                ASSERT_EQ(ECSqlStatus::Success, v->AddArrayElement().BindText(m.c_str(), IECSqlBinder::MakeCopy::No));
        }
        if (auto v = &stmt.GetBinder(++idx)) {
            for(auto& m : p2d_array)
                ASSERT_EQ(ECSqlStatus::Success, v->AddArrayElement().BindPoint2d(m));
        }
        if (auto v = &stmt.GetBinder(++idx)) {
            for(auto& m : p3d_array)
                ASSERT_EQ(ECSqlStatus::Success, v->AddArrayElement().BindPoint3d(m));
        }
        if (auto v = &stmt.GetBinder(++idx)) {
            for(auto& m : geom_array)
                ASSERT_EQ(ECSqlStatus::Success, v->AddArrayElement().BindGeometry(*m));
        }
        if (auto v = &stmt.GetBinder(++idx)) {
            ASSERT_EQ(ECSqlStatus::Success, (*v)["b"].BindBoolean(b));
            ASSERT_EQ(ECSqlStatus::Success, (*v)["bi"].BindBlob((void const*)bi, (int)sizeof(bi), IECSqlBinder::MakeCopy::No));
            ASSERT_EQ(ECSqlStatus::Success, (*v)["d"].BindDouble(d));
            ASSERT_EQ(ECSqlStatus::Success, (*v)["dt"].BindDateTime(dt));
            ASSERT_EQ(ECSqlStatus::Success, (*v)["dtUtc"].BindDateTime(dtUtc));
            ASSERT_EQ(ECSqlStatus::Success, (*v)["i"].BindInt(i));
            ASSERT_EQ(ECSqlStatus::Success, (*v)["l"].BindInt64(l));
            ASSERT_EQ(ECSqlStatus::Success, (*v)["s"].BindText(s.c_str(), IECSqlBinder::MakeCopy::No));
            ASSERT_EQ(ECSqlStatus::Success, (*v)["p2d"].BindPoint2d(p2d));
            ASSERT_EQ(ECSqlStatus::Success, (*v)["p3d"].BindPoint3d(p3d));
            ASSERT_EQ(ECSqlStatus::Success, (*v)["geom"].BindGeometry(*geom));
        }
        if (auto v = &stmt.GetBinder(++idx)) {
            for(auto& m : b_array)
                ASSERT_EQ(ECSqlStatus::Success, (*v)["b_array"].AddArrayElement().BindBoolean(m));
            for(auto& m : bi_array)
                ASSERT_EQ(ECSqlStatus::Success, (*v)["bi_array"].AddArrayElement().BindBlob((void const*)&m[0], (int)m.size(), IECSqlBinder::MakeCopy::No));
            for(auto& m : d_array)
                ASSERT_EQ(ECSqlStatus::Success, (*v)["d_array"].AddArrayElement().BindDouble(m));
            for(auto& m : dt_array)
                ASSERT_EQ(ECSqlStatus::Success, (*v)["dt_array"].AddArrayElement().BindDateTime(m));
            for(auto& m : dtUtc_array)
                ASSERT_EQ(ECSqlStatus::Success, (*v)["dtUtc_array"].AddArrayElement().BindDateTime(m));
            for(auto& m : i_array)
                ASSERT_EQ(ECSqlStatus::Success, (*v)["i_array"].AddArrayElement().BindInt(m));
            for(auto& m : l_array)
                ASSERT_EQ(ECSqlStatus::Success, (*v)["l_array"].AddArrayElement().BindInt64(m));
            for(auto& m : s_array)
                ASSERT_EQ(ECSqlStatus::Success, (*v)["s_array"].AddArrayElement().BindText(m.c_str(), IECSqlBinder::MakeCopy::No));
            for(auto& m : p2d_array)
                ASSERT_EQ(ECSqlStatus::Success, (*v)["p2d_array"].AddArrayElement().BindPoint2d(m));
            for(auto& m : p3d_array)
                ASSERT_EQ(ECSqlStatus::Success, (*v)["p3d_array"].AddArrayElement().BindPoint3d(m));
            for(auto& m : geom_array)
                ASSERT_EQ(ECSqlStatus::Success, (*v)["geom_array"].AddArrayElement().BindGeometry(*m));
        }
        if (auto o = &stmt.GetBinder(++idx)) {
            for (int n = 0; n < 2;++n) {
                auto& v = o->AddArrayElement();
                ASSERT_EQ(ECSqlStatus::Success, v["b"].BindBoolean(b_array[n]));
                ASSERT_EQ(ECSqlStatus::Success, v["bi"].BindBlob((void const*)&bi_array[n][0], (int)bi_array[n].size(), IECSqlBinder::MakeCopy::No));
                ASSERT_EQ(ECSqlStatus::Success, v["d"].BindDouble(d_array[n]));
                ASSERT_EQ(ECSqlStatus::Success, v["dt"].BindDateTime(dt_array[n]));
                ASSERT_EQ(ECSqlStatus::Success, v["dtUtc"].BindDateTime(dtUtc_array[n]));
                ASSERT_EQ(ECSqlStatus::Success, v["i"].BindInt(i_array[n]));
                ASSERT_EQ(ECSqlStatus::Success, v["l"].BindInt64(l_array[n]));
                ASSERT_EQ(ECSqlStatus::Success, v["s"].BindText(s_array[n].c_str(), IECSqlBinder::MakeCopy::No));
                ASSERT_EQ(ECSqlStatus::Success, v["p2d"].BindPoint2d(p2d_array[n]));
                ASSERT_EQ(ECSqlStatus::Success, v["p3d"].BindPoint3d(p3d_array[n]));
                ASSERT_EQ(ECSqlStatus::Success, v["geom"].BindGeometry(*geom_array[n]));
            }
        }
        if (auto o = &stmt.GetBinder(++idx)) {
            for (int n = 0; n < 2;++n) {
                auto& v = o->AddArrayElement();
                for(auto& m : b_array)
                    ASSERT_EQ(ECSqlStatus::Success, v["b_array"].AddArrayElement().BindBoolean(m));
                for(auto& m : bi_array)
                    ASSERT_EQ(ECSqlStatus::Success, v["bi_array"].AddArrayElement().BindBlob((void const*)&m[0], (int)m.size(), IECSqlBinder::MakeCopy::No));
                for(auto& m : d_array)
                    ASSERT_EQ(ECSqlStatus::Success, v["d_array"].AddArrayElement().BindDouble(m));
                for(auto& m : dt_array)
                    ASSERT_EQ(ECSqlStatus::Success, v["dt_array"].AddArrayElement().BindDateTime(m));
                for(auto& m : dtUtc_array)
                    ASSERT_EQ(ECSqlStatus::Success, v["dtUtc_array"].AddArrayElement().BindDateTime(m));
                for(auto& m : i_array)
                    ASSERT_EQ(ECSqlStatus::Success, v["i_array"].AddArrayElement().BindInt(m));
                for(auto& m : l_array)
                    ASSERT_EQ(ECSqlStatus::Success, v["l_array"].AddArrayElement().BindInt64(m));
                for(auto& m : s_array)
                    ASSERT_EQ(ECSqlStatus::Success, v["s_array"].AddArrayElement().BindText(m.c_str(), IECSqlBinder::MakeCopy::No));
                for(auto& m : p2d_array)
                    ASSERT_EQ(ECSqlStatus::Success, v["p2d_array"].AddArrayElement().BindPoint2d(m));
                for(auto& m : p3d_array)
                    ASSERT_EQ(ECSqlStatus::Success, v["p3d_array"].AddArrayElement().BindPoint3d(m));
                for(auto& m : geom_array)
                    ASSERT_EQ(ECSqlStatus::Success, v["geom_array"].AddArrayElement().BindGeometry(*m));
            }
        }
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(instKey));
        m_ecdb.SaveChanges();
    }
   //! HARD_CODED_IDS
   // do not indent the following string as it increases the size of string and compiler will error out.
    BeJsDocument expected;
    expected.Parse(R"json([
  [
    "0x1",
    "0x57",
    null,
    true,
    "encoding=base64;SGVsbG8sIFdvcmxkIQ==",
    3.141592653589793,
    "2017-01-17T00:00:00.000",
    "2018-02-17T00:00:00.000Z",
    -328961,
    -1412873783869441.0,
    "Hello, World!",
    {
      "X":22.33,
      "Y":-21.34
    },
    {
      "X":12.13,
      "Y":-42.34,
      "Z":-93.12
    },
    {
      "lineSegment":[
        [
          0,
          0,
          0
        ],
        [
          1,
          1,
          1
        ]
      ]
    },
    [
      true,
      false,
      true
    ],
    [
      "encoding=base64;SGUG",
      "encoding=base64;SGUG",
      "encoding=base64;SGUG"
    ],
    [
      123.3434,
      345.223,
      -532.123
    ],
    [
      "2017-01-14T00:00:00.000",
      "2018-01-13T00:00:00.000",
      "2019-01-11T00:00:00.000"
    ],
    [
      "2017-01-17T00:00:00.000Z",
      "2018-01-11T00:00:00.000Z",
      "2019-01-10T00:00:00.000Z"
    ],
    [
      3842,
      -4923,
      8291
    ],
    [
      384242.0,
      -234923.0,
      528291.0
    ],
    [
      "Bentley",
      "System"
    ],
    [
      {
        "X":22.33,
        "Y":-81.17
      },
      {
        "X":-42.74,
        "Y":16.29
      },
      {
        "X":77.45,
        "Y":-32.98
      }
    ],
    [
      {
        "X":84.13,
        "Y":99.23,
        "Z":-121.75
      },
      {
        "X":-90.34,
        "Y":45.75,
        "Z":-452.34
      },
      {
        "X":-12.54,
        "Y":-84.23,
        "Z":-343.45
      }
    ],
    [
      {
        "lineSegment":[
          [
            0,
            0,
            0
          ],
          [
            4,
            2.1,
            1.2
          ]
        ]
      },
      {
        "lineSegment":[
          [
            0,
            0,
            0
          ],
          [
            1.1,
            2.5,
            4.2
          ]
        ]
      },
      {
        "lineSegment":[
          [
            0,
            0,
            0
          ],
          [
            9.1,
            3.6,
            3.8
          ]
        ]
      }
    ],
    {
      "b":true,
      "bi":"encoding=base64;SGVsbG8sIFdvcmxkIQ==",
      "d":3.141592653589793,
      "dt":"2017-01-17T00:00:00.000",
      "dtUtc":"2018-02-17T00:00:00.000Z",
      "geom":{
        "lineSegment":[
          [
            0,
            0,
            0
          ],
          [
            1,
            1,
            1
          ]
        ]
      },
      "i":-328961,
      "l":-1412873783869441.0,
      "p2d":{
        "X":22.33,
        "Y":-21.34
      },
      "p3d":{
        "X":12.13,
        "Y":-42.34,
        "Z":-93.12
      },
      "s":"Hello, World!"
    },
    {
      "b_array":[
        true,
        false,
        true
      ],
      "bi_array":[
        "encoding=base64;SGUG",
        "encoding=base64;SGUG",
        "encoding=base64;SGUG"
      ],
      "d_array":[
        123.3434,
        345.223,
        -532.123
      ],
      "dt_array":[
        "2017-01-14T00:00:00.000",
        "2018-01-13T00:00:00.000",
        "2019-01-11T00:00:00.000"
      ],
      "dtUtc_array":[
        "2017-01-17T00:00:00.000Z",
        "2018-01-11T00:00:00.000Z",
        "2019-01-10T00:00:00.000Z"
      ],
      "geom_array":[
        {
          "lineSegment":[
            [
              0,
              0,
              0
            ],
            [
              4,
              2.1,
              1.2
            ]
          ]
        },
        {
          "lineSegment":[
            [
              0,
              0,
              0
            ],
            [
              1.1,
              2.5,
              4.2
            ]
          ]
        },
        {
          "lineSegment":[
            [
              0,
              0,
              0
            ],
            [
              9.1,
              3.6,
              3.8
            ]
          ]
        }
      ],
      "i_array":[
        3842,
        -4923,
        8291
      ],
      "l_array":[
        384242.0,
        -234923.0,
        528291.0
      ],
      "p2d_array":[
        {
          "X":22.33,
          "Y":-81.17
        },
        {
          "X":-42.74,
          "Y":16.29
        },
        {
          "X":77.45,
          "Y":-32.98
        }
      ],
      "p3d_array":[
        {
          "X":84.13,
          "Y":99.23,
          "Z":-121.75
        },
        {
          "X":-90.34,
          "Y":45.75,
          "Z":-452.34
        },
        {
          "X":-12.54,
          "Y":-84.23,
          "Z":-343.45
        }
      ],
      "s_array":[
        "Bentley",
        "System"
      ]
    },
    [
      {
        "b":true,
        "bi":"encoding=base64;SGUG",
        "d":123.3434,
        "dt":"2017-01-14T00:00:00.000",
        "dtUtc":"2017-01-17T00:00:00.000Z",
        "i":3842,
        "l":384242.0,
        "s":"Bentley",
        "p2d":{
          "X":22.33,
          "Y":-81.17
        },
        "p3d":{
          "X":84.13,
          "Y":99.23,
          "Z":-121.75
        },
        "geom":{
          "lineSegment":[
            [
              0,
              0,
              0
            ],
            [
              4,
              2.1,
              1.2
            ]
          ]
        }
      },
      {
        "b":false,
        "bi":"encoding=base64;SGUG",
        "d":345.223,
        "dt":"2018-01-13T00:00:00.000",
        "dtUtc":"2018-01-11T00:00:00.000Z",
        "i":-4923,
        "l":-234923.0,
        "s":"System",
        "p2d":{
          "X":-42.74,
          "Y":16.29
        },
        "p3d":{
          "X":-90.34,
          "Y":45.75,
          "Z":-452.34
        },
        "geom":{
          "lineSegment":[
            [
              0,
              0,
              0
            ],
            [
              1.1,
              2.5,
              4.2
            ]
          ]
        }
      }
    ],
    [
      {
        "b_array":[
          true,
          false,
          true
        ],
        "bi_array":[
          "encoding=base64;SGUG",
          "encoding=base64;SGUG",
          "encoding=base64;SGUG"
        ],
        "d_array":[
          123.3434,
          345.223,
          -532.123
        ],
        "dt_array":[
          "2017-01-14T00:00:00.000",
          "2018-01-13T00:00:00.000",
          "2019-01-11T00:00:00.000"
        ],
        "dtUtc_array":[
          "2017-01-17T00:00:00.000Z",
          "2018-01-11T00:00:00.000Z",
          "2019-01-10T00:00:00.000Z"
        ],
        "i_array":[
          3842,
          -4923,
          8291
        ],
        "l_array":[
          384242.0,
          -234923.0,
          528291.0
        ],
        "s_array":[
          "Bentley",
          "System"
        ],
        "p2d_array":[
          {
            "X":22.33,
            "Y":-81.17
          },
          {
            "X":-42.74,
            "Y":16.29
          },
          {
            "X":77.45,
            "Y":-32.98
          }
        ],
        "p3d_array":[
          {
            "X":84.13,
            "Y":99.23,
            "Z":-121.75
          },
          {
            "X":-90.34,
            "Y":45.75,
            "Z":-452.34
          },
          {
            "X":-12.54,
            "Y":-84.23,
            "Z":-343.45
          }
        ],
        "geom_array":[
          {
            "lineSegment":[
              [
                0,
                0,
                0
              ],
              [
                4,
                2.1,
                1.2
              ]
            ]
          },
          {
            "lineSegment":[
              [
                0,
                0,
                0
              ],
              [
                1.1,
                2.5,
                4.2
              ]
            ]
          },
          {
            "lineSegment":[
              [
                0,
                0,
                0
              ],
              [
                9.1,
                3.6,
                3.8
              ]
            ]
          }
        ]
      },
      {
        "b_array":[
          true,
          false,
          true
        ],
        "bi_array":[
          "encoding=base64;SGUG",
          "encoding=base64;SGUG",
          "encoding=base64;SGUG"
        ],
        "d_array":[
          123.3434,
          345.223,
          -532.123
        ],
        "dt_array":[
          "2017-01-14T00:00:00.000",
          "2018-01-13T00:00:00.000",
          "2019-01-11T00:00:00.000"
        ],
        "dtUtc_array":[
          "2017-01-17T00:00:00.000Z",
          "2018-01-11T00:00:00.000Z",
          "2019-01-10T00:00:00.000Z"
        ],
        "i_array":[
          3842,
          -4923,
          8291
        ],
        "l_array":[
          384242.0,
          -234923.0,
          528291.0
        ],
        "s_array":[
          "Bentley",
          "System"
        ],
        "p2d_array":[
          {
            "X":22.33,
            "Y":-81.17
          },
          {
            "X":-42.74,
            "Y":16.29
          },
          {
            "X":77.45,
            "Y":-32.98
          }
        ],
        "p3d_array":[
          {
            "X":84.13,
            "Y":99.23,
            "Z":-121.75
          },
          {
            "X":-90.34,
            "Y":45.75,
            "Z":-452.34
          },
          {
            "X":-12.54,
            "Y":-84.23,
            "Z":-343.45
          }
        ],
        "geom_array":[
          {
            "lineSegment":[
              [
                0,
                0,
                0
              ],
              [
                4,
                2.1,
                1.2
              ]
            ]
          },
          {
            "lineSegment":[
              [
                0,
                0,
                0
              ],
              [
                1.1,
                2.5,
                4.2
              ]
            ]
          },
          {
            "lineSegment":[
              [
                0,
                0,
                0
              ],
              [
                9.1,
                3.6,
                3.8
              ]
            ]
          }
        ]
      }
    ]
  ]
])json");
    if ("proxy view") {
        auto testSchema = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema
                schemaName="v1"
                alias="v1"
                version="1.0.0"
                xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name='ECDbMap' version='02.00.03' alias='ecdbmap' />
            <ECSchemaReference name="TestSchema" version="01.00.00" alias="ts" />
            <ECEntityClass typeName="EMixProxyView" description="" displayLabel="" modifier="Abstract">
                <ECCustomAttributes>
                    <View xmlns="ECDbMap.02.00.03">
                        <Query>SELECT * FROM ts.e_mix</Query>
                    </View>
                </ECCustomAttributes>
                <ECNavigationProperty propertyName="parent" relationshipName="EMixHasBase" direction="Backward">
                </ECNavigationProperty>
                <ECProperty propertyName="b" typeName="boolean" />
                <ECProperty propertyName="bi" typeName="binary" />
                <ECProperty propertyName="d" typeName="double" />
                <ECProperty propertyName="dt" typeName="dateTime" />
                <ECProperty propertyName="dtUtc" typeName="dateTime">
                    <ECCustomAttributes>
                        <DateTimeInfo xmlns="CoreCustomAttributes.01.00.00">
                            <DateTimeKind>Utc</DateTimeKind>
                        </DateTimeInfo>
                    </ECCustomAttributes>
                </ECProperty>
                <ECProperty propertyName="i" typeName="int" />
                <ECProperty propertyName="l" typeName="long" />
                <ECProperty propertyName="s" typeName="string" />
                <ECProperty propertyName="p2d" typeName="point2d" />
                <ECProperty propertyName="p3d" typeName="point3d" />
                <ECProperty propertyName="geom" typeName="Bentley.Geometry.Common.IGeometry" />
                <ECArrayProperty propertyName="b_array" typeName="boolean" minOccurs="0" maxOccurs="unbounded" />
                <ECArrayProperty propertyName="bi_array" typeName="binary" minOccurs="0" maxOccurs="unbounded" />
                <ECArrayProperty propertyName="d_array" typeName="double" minOccurs="0" maxOccurs="unbounded" />
                <ECArrayProperty propertyName="dt_array" typeName="dateTime" minOccurs="0" maxOccurs="unbounded" />
                <ECArrayProperty propertyName="dtUtc_array" typeName="dateTime" minOccurs="0"
                    maxOccurs="unbounded">
                    <ECCustomAttributes>
                        <DateTimeInfo xmlns="CoreCustomAttributes.01.00.00">
                            <DateTimeKind>Utc</DateTimeKind>
                        </DateTimeInfo>
                    </ECCustomAttributes>
                </ECArrayProperty>
                <ECArrayProperty propertyName="i_array" typeName="int" minOccurs="0" maxOccurs="unbounded" />
                <ECArrayProperty propertyName="l_array" typeName="long" minOccurs="0" maxOccurs="unbounded" />
                <ECArrayProperty propertyName="s_array" typeName="string" minOccurs="0" maxOccurs="unbounded" />
                <ECArrayProperty propertyName="p2d_array" typeName="point2d" minOccurs="0" maxOccurs="unbounded" />
                <ECArrayProperty propertyName="p3d_array" typeName="point3d" minOccurs="0" maxOccurs="unbounded" />
                <ECArrayProperty propertyName="geom_array" typeName="Bentley.Geometry.Common.IGeometry" minOccurs="0" maxOccurs="unbounded" />
                <ECStructProperty propertyName="p" typeName="ts:struct_p" />
                <ECStructProperty propertyName="pa" typeName="ts:struct_pa" />
                <ECStructArrayProperty propertyName="array_of_p" typeName="ts:struct_p" minOccurs="0" maxOccurs="unbounded" />
                <ECStructArrayProperty propertyName="array_of_pa" typeName="ts:struct_pa" minOccurs="0" maxOccurs="unbounded" />
            </ECEntityClass>
            <ECRelationshipClass typeName="EMixHasBase" strength="Embedding" modifier="Sealed">
                <Source multiplicity="(0..1)" polymorphic="false" roleLabel="EMixProxyView">
                    <Class class="EMixProxyView" />
                </Source>
                <Target multiplicity="(0..*)" polymorphic="false" roleLabel="EMixProxyView">
                    <Class class="EMixProxyView" />
                </Target>
            </ECRelationshipClass>
        </ECSchema>)xml");
        ASSERT_EQ(SUCCESS, ImportSchema(testSchema));
        m_ecdb.SaveChanges();
        auto& mgr = ConcurrentQueryMgr::GetInstance(m_ecdb);
        auto queryResponse = mgr.Enqueue(ECSqlRequest::MakeRequest("SELECT * FROM v1.EMixProxyView")).Get();
        auto queryResultJson = ((ECSqlResponse*)queryResponse.get())->asJsonString();
        BeJsDocument actualJs;
        actualJs.Parse(queryResultJson);
        EXPECT_STRCASEEQ(expected.Stringify(StringifyFormat::Indented).c_str(), actualJs.Stringify(StringifyFormat::Indented).c_str());
    }
}

END_ECDBUNITTESTS_NAMESPACE