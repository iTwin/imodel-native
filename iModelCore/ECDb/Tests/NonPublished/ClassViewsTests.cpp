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
    void _OnIssueReported(ECN::IssueSeverity severity, ECN::IssueCategory category, ECN::IssueType type, Utf8CP message) const override {
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
        <ECSchemaReference name='ClassViews' version='01.00.00' alias='classView' />
        <ECEntityClass typeName="SchemaClassesView" description="" displayLabel="" modifier="Abstract">
            <ECCustomAttributes>
                <View xmlns="ClassViews.01.00.00">
                    <Query>
                        SELECT
                            cd.ECInstanceId,
                            cd.ECClassId,
                            sc.Name SchemaName,
                            cd.Name ClassName
                        FROM meta.ECSchemaDef sc
                            JOIN meta.ECClassDef cd ON cd.Schema.Id=sc.ECInstanceId
                        GROUP BY sc.Name, cd.Name
                        LIMIT 10
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
        auto nativeSql = "SELECT [K0],[K1],[K2],[K3] FROM (SELECT [cd].[ECInstanceId] [K0],[cd].[ECClassId] [K1],[sc].[Name] [K2],[cd].[Name] [K3] FROM (SELECT [Id] ECInstanceId,35 ECClassId,[Name] FROM [main].[ec_Schema]) [sc] INNER JOIN (SELECT [Id] ECInstanceId,33 ECClassId,[SchemaId],[Name] FROM [main].[ec_Class]) [cd] ON [cd].[SchemaId]=[sc].[ECInstanceId]   GROUP BY [sc].[Name],[cd].[Name]  LIMIT 10) WHERE [K3]='View'";
        ASSERT_STREQ(nativeSql, stmt.GetNativeSql());
        ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);
        ASSERT_EQ(stmt.GetValueId<ECInstanceId>(0), ECInstanceId(27ull));
        ASSERT_EQ(stmt.GetValueId<ECN::ECClassId>(1), ECClassId(33ull));
        ASSERT_STREQ(stmt.GetValueText(2), "ClassViews");
        ASSERT_STREQ(stmt.GetValueText(3), "View");
    }
    if (true){
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM ts.SchemaClassesView"));
        ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);
        ASSERT_EQ(stmt.GetValueInt(0), 10);
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
    auto testSchema = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
    <ECSchema
            schemaName="test_schema"
            alias="ts"
            version="1.0.0"
            xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name='ClassViews' version='01.00.00' alias='classView' />
        <ECSchemaReference name='ECDbMeta' version='04.00.00' alias='meta' />
        <ECRelationshipClass typeName="SchemaClassesView" description="" displayLabel="" strength="referencing" modifier="Abstract">
            <ECCustomAttributes>
                <View xmlns="ClassViews.01.00.00">
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
        <ECSchemaReference name='ClassViews' version='01.00.00' alias='classView' />
        <ECSchemaReference name='ECDbMeta' version='04.00.00' alias='meta' />
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
                <View xmlns="ClassViews.01.00.00">
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
        auto nativeSql = "SELECT [K0],[K1],[K2_0],[K2_1] FROM (SELECT [ECClassDef].[ECInstanceId] [K0],[ECClassDef].[ECClassId] [K1],[ECClassDef].[SchemaId] [K2_0],[ECClassDef].[SchemaRelECClassId] [K2_1] FROM (SELECT [Id] ECInstanceId,33 ECClassId,[SchemaId],(CASE WHEN [SchemaId] IS NULL THEN NULL ELSE 34 END) [SchemaRelECClassId] FROM [main].[ec_Class]) [ECClassDef])";
        ASSERT_STREQ(nativeSql, stmt.GetNativeSql());
        ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);
        ASSERT_EQ(stmt.GetValueId<ECInstanceId>(0), ECInstanceId(1ull));
        ASSERT_EQ(stmt.GetValueId<ECN::ECClassId>(1), ECClassId(33ull));
        ECN::ECClassId relId;
        ASSERT_EQ(stmt.GetValueNavigation<ECInstanceId>(2, &relId), ECInstanceId(1ull));
        ASSERT_EQ(relId, ECClassId(34ull));
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
            <ECSchemaReference name='ClassViews' version='01.00.00' alias='classView' />
            <ECEntityClass typeName="SchemaView" description="" displayLabel="" modifier="Abstract">
                <ECCustomAttributes>
                    <View xmlns="ClassViews.01.00.00">
                        <Query>SELECT cd.ECInstanceId, cd.ECClassId FROM ts.SchemaView</Query>
                    </View>
                </ECCustomAttributes>
            </ECEntityClass>
        </ECSchema>)xml");

        listener.Reset();
        ASSERT_EQ(ERROR, ImportSchema(testSchema));
        ASSERT_STREQ("Total of 1 view classes were checked and 1 were found to be invalid.", listener.PopLastError().c_str());
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
            <ECSchemaReference name='ClassViews' version='01.00.00' alias='classView' />
            <ECEntityClass typeName="SchemaView" description="" displayLabel="" modifier="Abstract">
                <ECCustomAttributes>
                    <View xmlns="ClassViews.01.00.00">
                        <Query>SELECT cd.ECInstanceId, cd.ECClassId FROM ts.ClassView</Query>
                    </View>
                </ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="ClassView" description="" displayLabel="" modifier="Abstract">
                <ECCustomAttributes>
                    <View xmlns="ClassViews.01.00.00">
                        <Query>SELECT cd.ECInstanceId, cd.ECClassId FROM ts.SchemaView</Query>
                    </View>
                </ECCustomAttributes>
            </ECEntityClass>
        </ECSchema>)xml");

        listener.Reset();
        ASSERT_EQ(ERROR, ImportSchema(testSchema));
        ASSERT_STREQ("Total of 2 view classes were checked and 2 were found to be invalid.", listener.PopLastError().c_str());
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
            <ECSchemaReference name='ClassViews' version='01.00.00' alias='classView' />
            <ECEntityClass typeName="View1" description="" displayLabel="" modifier="Abstract">
                <ECCustomAttributes>
                    <View xmlns="ClassViews.01.00.00">
                        <Query>SELECT cd.ECInstanceId, cd.ECClassId FROM ts.View2</Query>
                    </View>
                </ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="View2" description="" displayLabel="" modifier="Abstract">
                <ECCustomAttributes>
                    <View xmlns="ClassViews.01.00.00">
                        <Query>SELECT cd.ECInstanceId, cd.ECClassId FROM ts.View3</Query>
                    </View>
                </ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="View3" description="" displayLabel="" modifier="Abstract">
                <ECCustomAttributes>
                    <View xmlns="ClassViews.01.00.00">
                        <Query>SELECT cd.ECInstanceId, cd.ECClassId FROM ts.View1</Query>
                    </View>
                </ECCustomAttributes>
            </ECEntityClass>
        </ECSchema>)xml");

        listener.Reset();
        ASSERT_EQ(ERROR, ImportSchema(testSchema));
        ASSERT_STREQ("Total of 3 view classes were checked and 3 were found to be invalid.", listener.PopLastError().c_str());
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
            <ECSchemaReference name='ClassViews' version='01.00.00' alias='classView' />
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
                    <View xmlns="ClassViews.01.00.00">
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
            <ECSchemaReference name='ClassViews' version='01.00.00' alias='classView' />
            <ECEntityClass typeName="P" modifier="Sealed">
                <ECProperty propertyName="intProp" typeName="int"/>
                <ECProperty propertyName="longProp" typeName="long"/>
            </ECEntityClass>
            <ECEntityClass typeName="P_View" description="" displayLabel="" modifier="Abstract">
                <ECCustomAttributes>
                    <View xmlns="ClassViews.01.00.00">
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
        ASSERT_STREQ("Total of 1 view classes were checked and 1 were found to be invalid.", listener.PopLastError().c_str());
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
            <ECSchemaReference name='ClassViews' version='01.00.00' alias='classView' />
            <ECEntityClass typeName="P" modifier="Sealed">
                <ECProperty propertyName="intProp" typeName="int"/>
                <ECProperty propertyName="longProp" typeName="long"/>
                <ECProperty propertyName="doubleProp" typeName="double"/>
            </ECEntityClass>
            <ECEntityClass typeName="P_View" description="" displayLabel="" modifier="Abstract">
                <ECCustomAttributes>
                    <View xmlns="ClassViews.01.00.00">
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
        ASSERT_STREQ("Total of 1 view classes were checked and 1 were found to be invalid.", listener.PopLastError().c_str());
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
            <ECSchemaReference name='ClassViews' version='01.00.00' alias='classView' />
            <ECEntityClass typeName="P" modifier="Sealed">
                <ECProperty propertyName="intProp" typeName="int"/>
                <ECProperty propertyName="longProp" typeName="long"/>
                <ECProperty propertyName="doubleProp" typeName="double"/>
            </ECEntityClass>
            <ECEntityClass typeName="P_View" description="" displayLabel="" modifier="Abstract">
                <ECCustomAttributes>
                    <View xmlns="ClassViews.01.00.00">
                        <Query>SELECT intProp, longProp, doubleProp FROM ts.P</Query>
                    </View>
                </ECCustomAttributes>
                <ECProperty propertyName="intProp" typeName="int"/>
                <ECProperty propertyName="longProp" typeName="long"/>
            </ECEntityClass>
        </ECSchema>)xml");

        listener.Reset();
        ASSERT_EQ(ERROR, ImportSchema(testSchema));
        ASSERT_STREQ("Total of 1 view classes were checked and 1 were found to be invalid.", listener.PopLastError().c_str());
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
            <ECSchemaReference name='ClassViews' version='01.00.00' alias='classView' />
            <ECEntityClass typeName="P" modifier="Sealed">
                <ECProperty propertyName="intProp" typeName="int"/>
                <ECProperty propertyName="longProp" typeName="long"/>
                <ECProperty propertyName="doubleProp" typeName="double"/>
            </ECEntityClass>
            <ECEntityClass typeName="P_View" description="" displayLabel="" modifier="Abstract">
                <ECCustomAttributes>
                    <View xmlns="ClassViews.01.00.00">
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
            <ECSchemaReference name='ClassViews' version='01.00.00' alias='classView' />
            <ECEntityClass typeName="P" modifier="Sealed">
                <ECProperty propertyName="intProp" typeName="int"/>
                <ECProperty propertyName="longProp" typeName="long"/>
                <ECProperty propertyName="doubleProp" typeName="double"/>
            </ECEntityClass>
            <ECEntityClass typeName="P_View" description="" displayLabel="" modifier="Abstract">
                <ECCustomAttributes>
                    <View xmlns="ClassViews.01.00.00">
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

END_ECDBUNITTESTS_NAMESPACE