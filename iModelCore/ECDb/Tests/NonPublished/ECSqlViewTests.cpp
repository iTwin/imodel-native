/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

struct ECSqlView : public ECDbTestFixture {
    // Json test
};
auto ClassViewSchema_1_0 = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
    <ECSchema
            schemaName="ClassView"
            alias="ecdbview"
            version="1.0.0"
            xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <!--
            Persistence method determine where data is stored.
                Permanent: data go into regular table.
                Temporay:  data go into a temp table
        -->
        <ECEnumeration typeName="PersistenceMethod" backingTypeName="string" isStrict="true">
            <ECEnumerator name="Permanent"  value="Permanent" displayLabel="Permanent"/>
        </ECEnumeration>
        <!--
            Refresh method determine how view data is updated
                Recompute: delete and reinsert data.
        -->
        <ECEnumeration typeName="RefreshMethod" backingTypeName="string" isStrict="true">
            <ECEnumerator name="Recompute"  value="Recompute" displayLabel="Recompute"/>
        </ECEnumeration>

        <!-- Data is store in a table either permanently or temporay -->
        <ECCustomAttributeClass typeName="PersistedView" modifier="Sealed" appliesTo="EntityClass,RelationshipClass">
            <ECProperty propertyName="PersistenceMethod" typeName="PersistenceMethod"  description=""/>
            <ECProperty propertyName="RefreshMethod"     typeName="RefreshMethod"      description=""/>
            <ECProperty propertyName="Query"             typeName="string"             description=""/>
            <ECArrayProperty propertyName="PropertyMaps" typeName="string" minOccurs="0" maxOccurs="unbounded"/>
        </ECCustomAttributeClass>

        <!-- View class is replaced by view query when used in ECSql -->
        <ECCustomAttributeClass typeName="TransientView" modifier="Sealed" appliesTo="EntityClass,RelationshipClass">
            <ECProperty propertyName="Query"             typeName="string"             description=""/>
        </ECCustomAttributeClass>
    </ECSchema>)xml");
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlView, TransientView_Simple) {
    auto testSchema = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
    <ECSchema
            schemaName="test_schema"
            alias="ts"
            version="1.0.0"
            xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name='ClassView' version='01.00.00' alias='classView' />
        <ECEntityClass typeName="SchemaClassesView" description="" displayLabel="" modifier="Abstract">
            <ECCustomAttributes>
                <TransientView xmlns="ClassView.01.00.00">
                    <Query>SELECT cd.ECInstanceId,  sc.Name SchemaName, cd.Name ClassName FROM meta.ECSchemaDef sc JOIN meta.ECClassDef cd ON cd.Schema.Id=sc.ECInstanceId GROUP BY sc.Name, cd.Name LIMIT 10</Query>
                </TransientView>
           </ECCustomAttributes>
            <ECProperty propertyName="SchemaName" typeName="string" />
            <ECProperty propertyName="ClassName"  typeName="string"/>
        </ECEntityClass>
    </ECSchema>)xml");

    ASSERT_EQ(SUCCESS, SetupECDb("test.ecdb", ClassViewSchema_1_0));
    ASSERT_EQ(SUCCESS, ImportSchema(testSchema));
    if (true){
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId, ECClassId, SchemaName, ClassName FROM ts.SchemaClassesView WHERE ClassName='PersistedView'"));
        auto nativeSql = "SELECT [K3],[K4],[K5],[K6] FROM (SELECT [K0] [K3],0x4b [K4],[K1] [K5],[K2] [K6] FROM (SELECT [cd].[ECInstanceId] [K0],[sc].[Name] [K1],[cd].[Name] [K2] FROM (SELECT [Id] ECInstanceId,34 ECClassId,[Name] FROM [main].[ec_Schema]) [sc] INNER JOIN (SELECT [Id] ECInstanceId,32 ECClassId,[SchemaId],[Name] FROM [main].[ec_Class]) [cd] ON [cd].[SchemaId]=[sc].[ECInstanceId]   GROUP BY [sc].[Name],[cd].[Name]  LIMIT 10)) WHERE [K6]='PersistedView'";
       //  printf("%s\n", stmt.GetNativeSql());
        ASSERT_STREQ(nativeSql, stmt.GetNativeSql());
        ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);
        ASSERT_EQ(stmt.GetValueId<ECInstanceId>(0), ECInstanceId(73ull));
        ASSERT_EQ(stmt.GetValueId<ECN::ECClassId>(1), ECClassId(75ull));
        ASSERT_STREQ(stmt.GetValueText(2), "ClassView");
        ASSERT_STREQ(stmt.GetValueText(3), "PersistedView");
    }
    if (true){
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM ts.SchemaClassesView"));
        printf("%s\n", stmt.GetNativeSql());
        ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);
        ASSERT_EQ(stmt.GetValueInt(0), 10);
    }
    ASSERT_EQ(SUCCESS, m_ecdb.RefreshViews());
    if (true){
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM ts.SchemaClassesView"));
        ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);
        ASSERT_EQ(stmt.GetValueInt(0), 10);
    }
    if (true){
        // should fail to prepare insert against a view
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "Insert into ts.SchemaClassesView(SchemaName, ClassName) Values( ?, ?)"));
    }
    if (true){
        // should fail to prepare insert against a view
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "update ts.SchemaClassesView set schemaname =?"));
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlView, PersistedView_Simple) {
    auto testSchema = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
    <ECSchema
            schemaName="test_schema"
            alias="ts"
            version="1.0.0"
            xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name='ClassView' version='01.00.00' alias='classView' />
        <ECEntityClass typeName="SchemaClassesView" description="" displayLabel="" modifier="Sealed">
            <ECCustomAttributes>
                <PersistedView xmlns="ClassView.01.00.00">
                    <Query>
                        SELECT cd.ECInstanceId, sc.Name, cd.Name
                        FROM meta.ECSchemaDef sc
                            JOIN meta.ECClassDef cd ON cd.Schema.Id=sc.ECInstanceId GROUP BY sc.Name, cd.Name LIMIT 10
                    </Query>
                    <PersistenceMethod>Permanent</PersistenceMethod>
                    <RefreshMethod>Recompute</RefreshMethod>
                    <PropertyMaps>
                        <string>ECInstanceId</string>
                        <string>SchemaName</string>
                        <string>ClassName</string>
                    </PropertyMaps>
                </PersistedView>
                <DbIndexList xmlns="ECDbMap.02.00.00">
                    <Indexes>
                        <DbIndex>
                            <Name>uniqueIndexSchemaNameAndClassName</Name>
                            <IsUnique>True</IsUnique>
                            <Properties>
                                <string>SchemaName</string>
                                <string>ClassName</string>
                            </Properties>
                        </DbIndex>
                    </Indexes>
                </DbIndexList>
            </ECCustomAttributes>
            <ECProperty propertyName="SchemaName" typeName="string" />
            <ECProperty propertyName="ClassName"  typeName="string"/>
        </ECEntityClass>
    </ECSchema>)xml");

    ASSERT_EQ(SUCCESS, SetupECDb("test.ecdb", ClassViewSchema_1_0));
    ASSERT_EQ(SUCCESS, ImportSchema(testSchema));

    if (true){
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM ts.SchemaClassesView"));
        ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);
        ASSERT_EQ(stmt.GetValueInt(0), 0);
    }
    ASSERT_EQ(SUCCESS, m_ecdb.RefreshViews());
    if (true){
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM ts.SchemaClassesView"));
        ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);
        ASSERT_EQ(stmt.GetValueInt(0), 10);
    }
    if (true){
        // should fail to prepare insert against a view
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "Insert into ts.SchemaClassesView(SchemaName, ClassName) Values( ?, ?)"));
    }
    if (true){
        // should fail to prepare insert against a view
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "update ts.SchemaClassesView set schemaname =?"));
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlView, PersistedView_DataVersion) {
    auto testSchema = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
    <ECSchema
            schemaName="test_schema"
            alias="ts"
            version="1.0.0"
            xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name='ClassView' version='01.00.00' alias='classView' />
        <ECEntityClass typeName="Person" description="" displayLabel="" modifier="Sealed">
            <ECProperty propertyName="FirstName" typeName="string" />
            <ECProperty propertyName="LastName" typeName="string" />
            <ECProperty propertyName="Age"  typeName="integer"/>
        </ECEntityClass>
        <ECEntityClass typeName="PersonView" description="" displayLabel="" modifier="Sealed">
            <ECCustomAttributes>
                <PersistedView xmlns="ClassView.01.00.00">
                    <Query>
                        SELECT FirstName || ' ' || LastName, Age FROM ts.Person ORDER BY LastName, FirstName
                    </Query>
                    <PersistenceMethod>Permanent</PersistenceMethod>
                    <RefreshMethod>Recompute</RefreshMethod>
                    <PropertyMaps>
                        <string>FullName</string>
                        <string>Age</string>
                    </PropertyMaps>
                </PersistedView>
            </ECCustomAttributes>
            <ECProperty propertyName="FullName" typeName="string" />
            <ECProperty propertyName="Age"  typeName="integer"/>
        </ECEntityClass>
    </ECSchema>)xml");

    auto insertRow = [&](Utf8CP firstName, Utf8CP lastName, int age) {
        ECSqlStatement stmt;
        EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Person(FirstName, LastName, Age) VALUES(?, ?, ?)"));
        EXPECT_EQ(ECSqlStatus::Success, stmt.BindText(1, firstName, IECSqlBinder::MakeCopy::No));
        EXPECT_EQ(BE_SQLITE_DONE, stmt.Step());
    };

    auto countViewRows= [&]() {
        ECSqlStatement stmt;
        EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM ts.PersonView"));
        EXPECT_EQ(BE_SQLITE_ROW, stmt.Step());
        return stmt.GetValueInt(0);
    };

    ASSERT_EQ(SUCCESS, SetupECDb("test.ecdb", ClassViewSchema_1_0));
    ASSERT_EQ(SUCCESS, ImportSchema(testSchema));
    m_ecdb.SaveChanges();
    ASSERT_EQ(4ull, m_ecdb.GetDataVersion());

    // following will set view version to 5 and db version will be 5 as well
    ASSERT_EQ(SUCCESS, m_ecdb.RefreshViews());
    ASSERT_EQ(0, countViewRows());
    m_ecdb.SaveChanges();
    ASSERT_EQ(5ull, m_ecdb.GetDataVersion());

    // following will not change view version  as data has not changed.
    ASSERT_EQ(SUCCESS, m_ecdb.RefreshViews());
    ASSERT_EQ(0, countViewRows());
    m_ecdb.SaveChanges();
    ASSERT_EQ(5ull, m_ecdb.GetDataVersion());

    // following will add 3 rows and should change view version cause it to update on next refresh call
    insertRow("Megan", "Bird", 28);
    insertRow("Alex", "Yang", 30);
    insertRow("Kristin", "Perry", 40);
    m_ecdb.SaveChanges();
    ASSERT_EQ(6ull, m_ecdb.GetDataVersion());
    
    // following should see updated view with 3 rows
    ASSERT_EQ(SUCCESS, m_ecdb.RefreshViews());
    ASSERT_EQ(3, countViewRows());
    m_ecdb.SaveChanges();
    ASSERT_EQ(7ull, m_ecdb.GetDataVersion());

    // following should not change any thing and view version should stay same
    ASSERT_EQ(SUCCESS, m_ecdb.RefreshViews());
    ASSERT_EQ(3, countViewRows());
    m_ecdb.SaveChanges();
    ASSERT_EQ(7ull, m_ecdb.GetDataVersion());
}

// Add issue listener
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
TEST_F(ECSqlView, TransientView_Validation_Checks) {
    ASSERT_EQ(SUCCESS, SetupECDb("test.ecdb", ClassViewSchema_1_0));

    TestIssueListener listener;
    m_ecdb.AddIssueListener(listener);
    // ===================================================================================
    ASSERT_EQ(ERROR, ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
    <ECSchema
            schemaName="test_schema"
            alias="ts"
            version="1.0.0"
            xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name='ClassView' version='01.00.00' alias='classView' />
        <ECEntityClass typeName="SchemaClassesView1" description="" displayLabel="" modifier="Abstract">
            <ECCustomAttributes>
                <TransientView xmlns="ClassView.01.00.00">
                    <Query>
                        SELECT cd.ECInstanceId, sc.Name, cd.Name
                        FROM ts.SchemaClassesView2
                    </Query>
                </TransientView>
            </ECCustomAttributes>
            <ECProperty propertyName="SchemaName" typeName="string" />
            <ECProperty propertyName="ClassName"  typeName="string"/>
        </ECEntityClass>
        <ECEntityClass typeName="SchemaClassesView2" description="" displayLabel="" modifier="Abstract">
            <ECCustomAttributes>
                <TransientView xmlns="ClassView.01.00.00">
                    <Query>
                        SELECT cd.ECInstanceId, sc.Name, cd.Name
                        FROM ts.SchemaClassesView1
                    </Query>
                </TransientView>
            </ECCustomAttributes>
            <ECProperty propertyName="SchemaName" typeName="string" />
            <ECProperty propertyName="ClassName"  typeName="string"/>
        </ECEntityClass>        
    </ECSchema>)xml"))) << "Transient view cannot reference other views";
    ASSERT_STREQ("Failed to import ECClass 'test_schema:SchemaClassesView2'.", listener.PopLastError().c_str());
    ASSERT_STREQ("Failed to import ECClass '[test_schema].[SchemaClassesView2]'. It has view definition custom attribute but not configured correctly.", listener.PopLastError().c_str());
    ASSERT_STREQ("Invalid view class 'test_schema:SchemaClassesView2'. View query contain reference to another view class 'test_schema:SchemaClassesView1' which is not supported right now.", listener.PopLastError().c_str());
    listener.Reset();

    // ===================================================================================
    ASSERT_EQ(ERROR, ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
    <ECSchema
            schemaName="test_schema"
            alias="ts"
            version="1.0.0"
            xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name='ClassView' version='01.00.00' alias='classView' />
        <ECEntityClass typeName="SchemaClassesView" description="" displayLabel="" modifier="Abstract">
            <ECCustomAttributes>
                <TransientView xmlns="ClassView.01.00.00">
                    <Query>
                        SELECT cd.ECInstanceId, sc.Name, cd.Name
                        FROM ts.SchemaClassesView
                    </Query>
                </TransientView>
            </ECCustomAttributes>
            <ECProperty propertyName="SchemaName" typeName="string" />
            <ECProperty propertyName="ClassName"  typeName="string"/>
        </ECEntityClass>
    </ECSchema>)xml"))) << "Transient view cannot reference it self";
    ASSERT_STREQ("Detected invalid views definitions for test_schema:SchemaClassesView.", listener.PopLastError().c_str());
    ASSERT_STREQ("Invalid view class 'test_schema:SchemaClassesView'. View query contain self reference to view class which is not allowed.", listener.PopLastError().c_str());
    listener.Reset();

    // ===================================================================================
    ASSERT_EQ(ERROR, ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
    <ECSchema
            schemaName="test_schema"
            alias="ts"
            version="1.0.0"
            xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name='ClassView' version='01.00.00' alias='classView' />
        <ECEntityClass typeName="SchemaClassesView" description="" displayLabel="" modifier="Sealed">
            <ECCustomAttributes>
                <TransientView xmlns="ClassView.01.00.00">
                    <Query>
                        SELECT cd.ECInstanceId, sc.Name, cd.Name
                        FROM meta.ECSchemaDef sc
                            JOIN meta.ECClassDef cd ON cd.Schema.Id=sc.ECInstanceId GROUP BY sc.Name, cd.Name LIMIT 10
                    </Query>
                </TransientView>
            </ECCustomAttributes>
            <ECProperty propertyName="SchemaName" typeName="string" />
            <ECProperty propertyName="ClassName"  typeName="string"/>
        </ECEntityClass>
    </ECSchema>)xml"))) << "Transient view class must be abstract";
    ASSERT_STREQ("Failed to import ECClass 'test_schema:SchemaClassesView'.", listener.PopLastError().c_str());
    ASSERT_STREQ("Failed to import ECClass '[test_schema].[SchemaClassesView]'. It has view definition custom attribute but not configured correctly.", listener.PopLastError().c_str());
    ASSERT_STREQ("Invalid view class '[test_schema].[SchemaClassesView]'. 'TransientView' must be applied to a 'Abstract' class.", listener.PopLastError().c_str());
    listener.Reset();

    // ===================================================================================
    ASSERT_EQ(ERROR, ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
    <ECSchema
            schemaName="test_schema"
            alias="ts"
            version="1.0.0"
            xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name='ClassView' version='01.00.00' alias='classView' />
        <ECEntityClass typeName="SchemaClassesView" description="" displayLabel="" modifier="Abstract">
            <ECCustomAttributes>
                <TransientView xmlns="ClassView.01.00.00">
                    <Query></Query>
                </TransientView>
            </ECCustomAttributes>
            <ECProperty propertyName="SchemaName" typeName="string" />
            <ECProperty propertyName="ClassName"  typeName="string"/>
        </ECEntityClass>
    </ECSchema>)xml"))) << "Transient view class must be abstract";
    ASSERT_STREQ("Failed to import ECClass 'test_schema:SchemaClassesView'.", listener.PopLastError().c_str());
    ASSERT_STREQ("Failed to import ECClass '[test_schema].[SchemaClassesView]'. It has view definition custom attribute but not configured correctly.", listener.PopLastError().c_str());
    ASSERT_STREQ("Invalid view class '[test_schema].[SchemaClassesView]'. 'Query' is required attribute for PersistedView/TransientView defintion custom attribute.", listener.PopLastError().c_str());
    listener.Reset();

    // ===================================================================================
    ASSERT_EQ(ERROR, ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
    <ECSchema
            schemaName="test_schema"
            alias="ts"
            version="1.0.0"
            xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name='ClassView' version='01.00.00' alias='classView' />
        <ECEntityClass typeName="SchemaClassesView" description="" displayLabel="" modifier="Abstract">
            <ECCustomAttributes>
                <TransientView xmlns="ClassView.01.00.00">
                    <Query>Update table</Query>
                </TransientView>
            </ECCustomAttributes>
            <ECProperty propertyName="SchemaName" typeName="string" />
            <ECProperty propertyName="ClassName"  typeName="string"/>
        </ECEntityClass>
    </ECSchema>)xml"))) << "Transient view class must be abstract";
    ASSERT_STREQ("Detected invalid views definitions for test_schema:SchemaClassesView.", listener.PopLastError().c_str());
    ASSERT_STREQ("Invalid view class '[test_schema].[SchemaClassesView]'. Failed to prepare query.", listener.PopLastError().c_str());
    ASSERT_STREQ("Failed to parse ECSQL 'Update table': syntax error", listener.PopLastError().c_str());
    listener.Reset();
    // ===================================================================================
    ASSERT_EQ(ERROR, ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
    <ECSchema
            schemaName="test_schema"
            alias="ts"
            version="1.0.0"
            xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name='ClassView' version='01.00.00' alias='classView' />
        <ECEntityClass typeName="SchemaClassesView" description="" displayLabel="" modifier="Abstract">
            <ECCustomAttributes>
                <TransientView xmlns="ClassView.01.00.00">
                    <Query>
                        SELECT cd.ECInstanceId, sc.Name, cd.Name
                        FROM meta.ECSchemaDef sc
                            JOIN meta.ECClassDef cd ON cd.Schema.Id=sc.ECInstanceId GROUP BY sc.Name, cd.Name LIMIT 10
                    </Query>
                </TransientView>
            </ECCustomAttributes>
            <ECProperty propertyName="SchemaName" typeName="string" />
            <ECProperty propertyName="ClassName"  typeName="string"/>
        </ECEntityClass>
    </ECSchema>)xml"))) << "Transient view class must be abstract";
    ASSERT_STREQ("Detected invalid views definitions for test_schema:SchemaClassesView.", listener.PopLastError().c_str());
    ASSERT_STREQ("Invalid view class '[test_schema].[SchemaClassesView]'. class does not have property with name 'Name' as defined in query at index 1", listener.PopLastError().c_str());
    ASSERT_STREQ("Invalid view class '[test_schema].[SchemaClassesView]'. class does not have property with name 'Name' as defined in query at index 1", listener.PopLastError().c_str());

    listener.Reset();
}
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlView, PersistedView_Validation_Checks) {
    ASSERT_EQ(SUCCESS, SetupECDb("test.ecdb", ClassViewSchema_1_0));


    TestIssueListener listener;
    m_ecdb.AddIssueListener(listener);
    // ===================================================================================
    ASSERT_EQ(ERROR, ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
    <ECSchema
            schemaName="test_schema"
            alias="ts"
            version="1.0.0"
            xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name='ClassView' version='01.00.00' alias='classView' />
        <ECEntityClass typeName="SchemaClassesView1" description="" displayLabel="" modifier="Sealed">
            <ECCustomAttributes>
                <PersistedView xmlns="ClassView.01.00.00">
                    <Query>
                        SELECT cd.ECInstanceId, sc.name, cd.Name 
                        FROM ts.SchemaClassesView2
                    </Query>
                    <PersistenceMethod>Permanent</PersistenceMethod>
                    <RefreshMethod>Recompute</RefreshMethod>
                    <PropertyMaps>
                        <string>ECInstanceId</string>
                        <string>SchemaName</string>
                        <string>ClassName</string>
                    </PropertyMaps>
                </PersistedView>
            </ECCustomAttributes>
            <ECProperty propertyName="SchemaName" typeName="string" />
            <ECProperty propertyName="ClassName"  typeName="string"/>
        </ECEntityClass>
        <ECEntityClass typeName="SchemaClassesView2" description="" displayLabel="" modifier="Sealed">
            <ECCustomAttributes>
                <PersistedView xmlns="ClassView.01.00.00">
                    <Query>
                        SELECT cd.ECInstanceId, sc.name, cd.Name 
                        FROM ts.SchemaClassesView1
                    </Query>
                    <PersistenceMethod>Permanent</PersistenceMethod>
                    <RefreshMethod>Recompute</RefreshMethod>
                    <PropertyMaps>
                        <string>ECInstanceId</string>
                        <string>SchemaName</string>
                        <string>ClassName</string>
                    </PropertyMaps>
                </PersistedView>
            </ECCustomAttributes>
            <ECProperty propertyName="SchemaName" typeName="string" />
            <ECProperty propertyName="ClassName"  typeName="string"/>
        </ECEntityClass>        
    </ECSchema>)xml"))) << "Transient view cannot reference other views";
    ASSERT_STREQ("Failed to import ECClass 'test_schema:SchemaClassesView2'.", listener.PopLastError().c_str());
    ASSERT_STREQ("Failed to import ECClass '[test_schema].[SchemaClassesView2]'. It has view definition custom attribute but not configured correctly.", listener.PopLastError().c_str());
    ASSERT_STREQ("Invalid view class 'test_schema:SchemaClassesView2'. View query contain reference to another view class 'test_schema:SchemaClassesView1' which is not supported right now.", listener.PopLastError().c_str());
    listener.Reset();

    // ===================================================================================
    ASSERT_EQ(ERROR, ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
    <ECSchema
            schemaName="test_schema"
            alias="ts"
            version="1.0.0"
            xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name='ClassView' version='01.00.00' alias='classView' />
        <ECEntityClass typeName="SchemaClassesView" description="" displayLabel="" modifier="Sealed">
            <ECCustomAttributes>
                <PersistedView xmlns="ClassView.01.00.00">
                    <Query>
                        SELECT cd.ECInstanceId, sc.name, cd.Name 
                        FROM ts.SchemaClassesView
                    </Query>
                    <PersistenceMethod>Permanent</PersistenceMethod>
                    <RefreshMethod>Recompute</RefreshMethod>
                    <PropertyMaps>
                        <string>ECInstanceId</string>
                        <string>SchemaName</string>
                        <string>ClassName</string>
                    </PropertyMaps>
                </PersistedView>
            </ECCustomAttributes>
            <ECProperty propertyName="SchemaName" typeName="string" />
            <ECProperty propertyName="ClassName"  typeName="string"/>
        </ECEntityClass>
    </ECSchema>)xml"))) << "Transient view cannot reference it self";
    ASSERT_STREQ("Detected invalid views definitions for test_schema:SchemaClassesView.", listener.PopLastError().c_str());
    ASSERT_STREQ("Invalid view class 'test_schema:SchemaClassesView'. View query contain self reference to view class which is not allowed.", listener.PopLastError().c_str());
    listener.Reset();

    // ===================================================================================
    ASSERT_EQ(ERROR, ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
    <ECSchema
            schemaName="test_schema"
            alias="ts"
            version="1.0.0"
            xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name='ClassView' version='01.00.00' alias='classView' />
        <ECEntityClass typeName="SchemaClassesView" description="" displayLabel="" modifier="Sealed">
            <ECCustomAttributes>
                <PersistedView xmlns="ClassView.01.00.00">
                    <Query>wrong query</Query>
                    <PersistenceMethod>Permanent</PersistenceMethod>
                    <RefreshMethod>Recompute</RefreshMethod>
                    <PropertyMaps>
                        <string>ECInstanceId</string>
                        <string>SchemaName</string>
                        <string>ClassName</string>
                    </PropertyMaps>
                </PersistedView>
            </ECCustomAttributes>
            <ECProperty propertyName="SchemaName" typeName="string" />
            <ECProperty propertyName="ClassName"  typeName="string"/>
        </ECEntityClass>
    </ECSchema>)xml"))) << "Persisted view must specify a query";
    listener.Dump("listener");
    listener.Reset();

    // ===================================================================================
    ASSERT_EQ(ERROR, ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
    <ECSchema
            schemaName="test_schema"
            alias="ts"
            version="1.0.0"
            xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name='ClassView' version='01.00.00' alias='classView' />
        <ECEntityClass typeName="SchemaClassesView" description="" displayLabel="" modifier="Sealed">
            <ECCustomAttributes>
                <PersistedView xmlns="ClassView.01.00.00">
                    <Query></Query>
                    <PersistenceMethod>Permanent</PersistenceMethod>
                    <RefreshMethod>Recompute</RefreshMethod>
                    <PropertyMaps>
                        <string>ECInstanceId</string>
                        <string>SchemaName</string>
                        <string>ClassName</string>
                    </PropertyMaps>
                </PersistedView>
            </ECCustomAttributes>
            <ECProperty propertyName="SchemaName" typeName="string" />
            <ECProperty propertyName="ClassName"  typeName="string"/>
        </ECEntityClass>
    </ECSchema>)xml"))) << "Persisted view must specify a query";
    ASSERT_STREQ("Failed to import ECClass 'test_schema:SchemaClassesView'.", listener.PopLastError().c_str());
    ASSERT_STREQ("Failed to import ECClass '[test_schema].[SchemaClassesView]'. It has view definition custom attribute but not configured correctly.", listener.PopLastError().c_str());
    ASSERT_STREQ("Invalid view class '[test_schema].[SchemaClassesView]'. 'Query' is required attribute for PersistedView/TransientView defintion custom attribute.", listener.PopLastError().c_str());
    listener.Reset();
    // ===================================================================================
    ASSERT_EQ(ERROR, ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
    <ECSchema
            schemaName="test_schema"
            alias="ts"
            version="1.0.0"
            xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name='ClassView' version='01.00.00' alias='classView' />
        <ECEntityClass typeName="SchemaClassesView" description="" displayLabel="" modifier="Sealed">
            <ECCustomAttributes>
                <PersistedView xmlns="ClassView.01.00.00">
                    <Query>
                        SELECT cd.ECInstanceId, sc.Name, cd.Name
                        FROM meta.ECSchemaDef sc
                            JOIN meta.ECClassDef cd ON cd.Schema.Id=sc.ECInstanceId GROUP BY sc.Name, cd.Name LIMIT 10
                    </Query>
                    <PersistenceMethod></PersistenceMethod>
                    <RefreshMethod>Recompute</RefreshMethod>
                    <PropertyMaps>
                        <string>ECInstanceId</string>
                        <string>SchemaName</string>
                        <string>ClassName</string>
                    </PropertyMaps>
                </PersistedView>
            </ECCustomAttributes>
            <ECProperty propertyName="SchemaName" typeName="string" />
            <ECProperty propertyName="ClassName"  typeName="string"/>
        </ECEntityClass>
    </ECSchema>)xml"))) << "Persisted view must specify a PersistenceMethod";
    ASSERT_STREQ("Failed to import ECClass 'test_schema:SchemaClassesView'.", listener.PopLastError().c_str());
    ASSERT_STREQ("Failed to import ECClass '[test_schema].[SchemaClassesView]'. It has view definition custom attribute but not configured correctly.", listener.PopLastError().c_str());
    ASSERT_STREQ("Invalid view class '[test_schema].[SchemaClassesView]'. 'PersistenceMethod' is required attribute for PersistedView custom attribute.", listener.PopLastError().c_str());
    listener.Reset();
    // ===================================================================================
    ASSERT_EQ(ERROR, ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
    <ECSchema
            schemaName="test_schema"
            alias="ts"
            version="1.0.0"
            xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name='ClassView' version='01.00.00' alias='classView' />
        <ECEntityClass typeName="SchemaClassesView" description="" displayLabel="" modifier="Sealed">
            <ECCustomAttributes>
                <PersistedView xmlns="ClassView.01.00.00">
                    <Query>
                        SELECT cd.ECInstanceId, sc.Name, cd.Name
                        FROM meta.ECSchemaDef sc
                            JOIN meta.ECClassDef cd ON cd.Schema.Id=sc.ECInstanceId GROUP BY sc.Name, cd.Name LIMIT 10
                    </Query>
                    <PersistenceMethod>Permanent</PersistenceMethod>
                    <RefreshMethod></RefreshMethod>
                    <PropertyMaps>
                        <string>ECInstanceId</string>
                        <string>SchemaName</string>
                        <string>ClassName</string>
                    </PropertyMaps>
                </PersistedView>
            </ECCustomAttributes>
            <ECProperty propertyName="SchemaName" typeName="string" />
            <ECProperty propertyName="ClassName"  typeName="string"/>
        </ECEntityClass>
    </ECSchema>)xml"))) << "Persisted view must specify a RefreshMethod";
    ASSERT_STREQ("Failed to import ECClass 'test_schema:SchemaClassesView'.", listener.PopLastError().c_str());
    ASSERT_STREQ("Failed to import ECClass '[test_schema].[SchemaClassesView]'. It has view definition custom attribute but not configured correctly.", listener.PopLastError().c_str());
    ASSERT_STREQ("Invalid view class '[test_schema].[SchemaClassesView]'. 'RefreshMethod' is required attribute for PersistedView custom attribute.", listener.PopLastError().c_str());
    listener.Reset();
    // ===================================================================================
    ASSERT_EQ(ERROR, ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
    <ECSchema
            schemaName="test_schema"
            alias="ts"
            version="1.0.0"
            xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name='ClassView' version='01.00.00' alias='classView' />
        <ECEntityClass typeName="SchemaClassesView" description="" displayLabel="" modifier="Sealed">
            <ECCustomAttributes>
                <PersistedView xmlns="ClassView.01.00.00">
                    <Query>
                        SELECT cd.ECInstanceId, sc.Name, cd.Name
                        FROM meta.ECSchemaDef sc
                            JOIN meta.ECClassDef cd ON cd.Schema.Id=sc.ECInstanceId GROUP BY sc.Name, cd.Name LIMIT 10
                    </Query>
                    <PersistenceMethod>Permanent</PersistenceMethod>
                    <RefreshMethod>Recompute</RefreshMethod>
                    <PropertyMaps>
                        <string>ECInstanceId</string>
                        <string>SchemaName</string>
                        <string>ClassName</string>
                        <string>FakeProperty</string>
                    </PropertyMaps>
                </PersistedView>
            </ECCustomAttributes>
            <ECProperty propertyName="SchemaName" typeName="string" />
            <ECProperty propertyName="ClassName"  typeName="string"/>
        </ECEntityClass>
    </ECSchema>)xml"))) << "Persisted view must with wrong property map";
    ASSERT_STREQ("Failed to import ECClass 'test_schema:SchemaClassesView'.", listener.PopLastError().c_str());
    ASSERT_STREQ("Failed to import ECClass '[test_schema].[SchemaClassesView]'. It has view definition custom attribute but not configured correctly.", listener.PopLastError().c_str());
    ASSERT_STREQ("Invalid view class '[test_schema].[SchemaClassesView]'. 'FakeProperty property not found in class.", listener.PopLastError().c_str());
    listener.Reset();
    // ===================================================================================
    ASSERT_EQ(ERROR, ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
    <ECSchema
            schemaName="test_schema"
            alias="ts"
            version="1.0.0"
            xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name='ClassView' version='01.00.00' alias='classView' />
        <ECEntityClass typeName="SchemaClassesView" description="" displayLabel="" modifier="Sealed">
            <ECCustomAttributes>
                <PersistedView xmlns="ClassView.01.00.00">
                    <Query>
                        SELECT cd.ECInstanceId, sc.Name, cd.Name
                        FROM meta.ECSchemaDef sc
                            JOIN meta.ECClassDef cd ON cd.Schema.Id=sc.ECInstanceId GROUP BY sc.Name, cd.Name LIMIT 10
                    </Query>
                    <PersistenceMethod>Permanent</PersistenceMethod>
                    <RefreshMethod>Recompute</RefreshMethod>
                    <PropertyMaps>
                        <string>ECInstanceId</string>
                        <string>SchemaName</string>
                        <string>ClassName</string>
                    </PropertyMaps>
                </PersistedView>
            </ECCustomAttributes>
            <ECProperty propertyName="SchemaName1" typeName="string" />
            <ECProperty propertyName="ClassName"  typeName="string"/>
        </ECEntityClass>
    </ECSchema>)xml"))) << "Persisted view must with wrong property map";
    ASSERT_STREQ("Failed to import ECClass 'test_schema:SchemaClassesView'.", listener.PopLastError().c_str());
    ASSERT_STREQ("Failed to import ECClass '[test_schema].[SchemaClassesView]'. It has view definition custom attribute but not configured correctly.", listener.PopLastError().c_str());
    ASSERT_STREQ("Invalid view class '[test_schema].[SchemaClassesView]'. 'SchemaName property not found in class.", listener.PopLastError().c_str());
    listener.Reset();
    // ===================================================================================
    ASSERT_EQ(ERROR, ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
    <ECSchema
            schemaName="test_schema"
            alias="ts"
            version="1.0.0"
            xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name='ClassView' version='01.00.00' alias='classView' />
        <ECEntityClass typeName="SchemaClassesView" description="" displayLabel="" modifier="Sealed">
            <ECCustomAttributes>
                <PersistedView xmlns="ClassView.01.00.00">
                    <Query>
                        SELECT cd.ECInstanceId, sc.Name, cd.Name
                        FROM meta.ECSchemaDef sc
                            JOIN meta.ECClassDef cd ON cd.Schema.Id=sc.ECInstanceId GROUP BY sc.Name, cd.Name LIMIT 10
                    </Query>
                    <PersistenceMethod>Permanent</PersistenceMethod>
                    <RefreshMethod>Recompute</RefreshMethod>
                    <PropertyMaps>
                    </PropertyMaps>
                </PersistedView>
            </ECCustomAttributes>
            <ECProperty propertyName="SchemaName" typeName="string" />
            <ECProperty propertyName="ClassName"  typeName="string"/>
        </ECEntityClass>
    </ECSchema>)xml"))) << "Persisted view must with wrong property map";
    ASSERT_STREQ("Failed to import ECClass 'test_schema:SchemaClassesView'.", listener.PopLastError().c_str());
    ASSERT_STREQ("Failed to import ECClass '[test_schema].[SchemaClassesView]'. It has view definition custom attribute but not configured correctly.", listener.PopLastError().c_str());
    ASSERT_STREQ("Invalid view class '[test_schema].[SchemaClassesView]'. 'PropertyMap' is required attribute for PersistedView custom attribute.", listener.PopLastError().c_str());

     listener.Reset();
    // ===================================================================================
    ASSERT_EQ(ERROR, ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
    <ECSchema
            schemaName="test_schema"
            alias="ts"
            version="1.0.0"
            xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name='ClassView' version='01.00.00' alias='classView' />
        <ECEntityClass typeName="SchemaClassesView" description="" displayLabel="" modifier="Sealed">
            <ECCustomAttributes>
                <PersistedView xmlns="ClassView.01.00.00">
                    <Query>
                        SELECT cd.ECInstanceId, sc.Name, cd.Name
                        FROM meta.ECSchemaDef sc
                            JOIN meta.ECClassDef cd ON cd.Schema.Id=sc.ECInstanceId GROUP BY sc.Name, cd.Name LIMIT 10
                    </Query>
                    <PersistenceMethod>Permanent</PersistenceMethod>
                    <RefreshMethod>Recompute</RefreshMethod>
                    <PropertyMaps>
                        <string>ECInstanceId</string>
                        <string>SchemaName</string>
                        <string>ClassName</string>
                    </PropertyMaps>
                </PersistedView>
            </ECCustomAttributes>
            <ECProperty propertyName="SchemaName" typeName="double"/>
            <ECProperty propertyName="ClassName"  typeName="string"/>
        </ECEntityClass>
    </ECSchema>)xml"))) << "Persisted view must with wrong property map";
    ASSERT_STREQ("Detected invalid views definitions for test_schema:SchemaClassesView.", listener.PopLastError().c_str());
    ASSERT_STREQ("Invalid view class '[test_schema].[SchemaClassesView]'. View property 'SchemaName' is incompatible with query property at index 2.", listener.PopLastError().c_str());
    ASSERT_STREQ("Invalid view class '[test_schema].[SchemaClassesView]'. View property 'SchemaName' is incompatible with query property at index 2.", listener.PopLastError().c_str());
}



END_ECDBUNITTESTS_NAMESPACE