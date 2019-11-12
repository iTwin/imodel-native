
/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

struct TableSpaceTestFixture : ECDbTestFixture {};

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Krischan.Eberle                  11/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(TableSpaceTestFixture, AttachedTableSpace)
    {
    BeFileName attachedECDbPath;

    {
    //attach file
    BeTest::GetHost().GetOutputRoot(attachedECDbPath);
    attachedECDbPath.AppendToPath(L"AttachedTableSpace.ecdb.attached");
    if (attachedECDbPath.DoesPathExist())
        ASSERT_EQ(BeFileNameStatus::Success, attachedECDbPath.BeDeleteFile());

    ECDb attached;
    ASSERT_EQ(BE_SQLITE_OK, attached.CreateNewDb(attachedECDbPath));

    TestHelper testHelper(attached);
    ASSERT_EQ(SUCCESS, testHelper.ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="Parent" modifier="None">
                <ECProperty propertyName="Name" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="Child" modifier="Sealed">
                <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward"/>
                <ECProperty propertyName="Name" typeName="string" />
            </ECEntityClass>
           <ECRelationshipClass typeName="Rel" strength="Referencing" modifier="Sealed">
              <Source multiplicity="(0..1)" polymorphic="False" roleLabel="has">
                  <Class class ="Parent" />
              </Source>
              <Target multiplicity="(0..*)" polymorphic="False" roleLabel="is referenced by">
                  <Class class="Child" />
              </Target>
           </ECRelationshipClass>
        </ECSchema>)xml")));

    ECInstanceKey parentKey;
    ASSERT_EQ(BE_SQLITE_DONE, testHelper.ExecuteInsertECSql(parentKey, "INSERT INTO ts.Parent(Name) VALUES('Parent 1')"));
    ASSERT_EQ(BE_SQLITE_DONE, testHelper.ExecuteECSql(Utf8PrintfString("INSERT INTO ts.Child(Parent.Id,Name) VALUES(%s,'Child 1')",
                                                                       parentKey.GetInstanceId().ToString().c_str()).c_str()));

    ASSERT_EQ(BE_SQLITE_OK, attached.SaveChanges());
    attached.CloseDb();
    }


    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("attachedtablespace.ecdb"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, GetHelper().PrepareECSql("SELECT * FROM ts.Parent"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.AttachDb(attachedECDbPath.GetNameUtf8().c_str(), "attached"));
    ASSERT_TRUE(m_ecdb.IsTransactionActive());

    ASSERT_EQ(ECSqlStatus::Success, GetHelper().PrepareECSql("SELECT * FROM ts.Parent"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, GetHelper().PrepareECSql("SELECT * FROM main.ts.Parent"));
    ASSERT_EQ(ECSqlStatus::Success, GetHelper().PrepareECSql("SELECT * FROM attached.ts.Parent"));

    ASSERT_EQ(BE_SQLITE_ERROR, GetHelper().ExecuteECSql("INSERT INTO ts.Parent(Name) VALUES('Parent 2')"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, GetHelper().PrepareECSql("INSERT INTO main.ts.Parent(Name) VALUES('Parent 2')"));
    ASSERT_EQ(BE_SQLITE_ERROR, GetHelper().ExecuteECSql("INSERT INTO attached.ts.Parent(Name) VALUES('Parent 3')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.Parent(ECInstanceId,Name) VALUES(100,'Parent 2')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO attached.ts.Parent(ECInstanceId,Name) VALUES(101,'Parent 2')"));

    ASSERT_EQ(BE_SQLITE_ERROR, GetHelper().ExecuteECSql("INSERT INTO ts.Child(Name) VALUES('Child 2')"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, GetHelper().PrepareECSql("INSERT INTO main.ts.Child(Name) VALUES('Child 2')"));
    ASSERT_EQ(BE_SQLITE_ERROR, GetHelper().ExecuteECSql("INSERT INTO attached.ts.Child(Name) VALUES('Child 3')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.Child(ECInstanceId,Name) VALUES(200,'Child 2')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO attached.ts.Child(ECInstanceId,Name) VALUES(201,'Child 2')"));

    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    ASSERT_TRUE(m_ecdb.IsTransactionActive());

    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb(ECDb::OpenParams(ECDb::OpenMode::Readonly)));
    ASSERT_TRUE(m_ecdb.IsTransactionActive());
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.AttachDb(attachedECDbPath.GetNameUtf8().c_str(), "attached"));
    ASSERT_TRUE(m_ecdb.IsTransactionActive());
    ASSERT_EQ(ECSqlStatus::Error, GetHelper().PrepareECSql("INSERT INTO ts.Parent(Name) VALUES('Parent 2')")) << "file opened read-only which applies to attached file as well";
    ASSERT_EQ(ECSqlStatus::Error, GetHelper().PrepareECSql("INSERT INTO attached.ts.Parent(Name) VALUES('Parent 2')")) << "file opened read-only which applies to attached file as well";
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Krischan.Eberle                  12/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(TableSpaceTestFixture, AttachWithoutApi)
    {
    BeFileName attachedECDbPath;

    {
    //attach file
    BeTest::GetHost().GetOutputRoot(attachedECDbPath);
    attachedECDbPath.AppendToPath(L"AttachedTableSpace.ecdb.attached");
    if (attachedECDbPath.DoesPathExist())
        ASSERT_EQ(BeFileNameStatus::Success, attachedECDbPath.BeDeleteFile());

    ECDb attached;
    ASSERT_EQ(BE_SQLITE_OK, attached.CreateNewDb(attachedECDbPath));

    TestHelper testHelper(attached);
    ASSERT_EQ(SUCCESS, testHelper.ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="Parent" modifier="None">
                <ECProperty propertyName="Name" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="Child" modifier="Sealed">
                <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward"/>
                <ECProperty propertyName="Name" typeName="string" />
            </ECEntityClass>
           <ECRelationshipClass typeName="Rel" strength="Referencing" modifier="Sealed">
              <Source multiplicity="(0..1)" polymorphic="False" roleLabel="has">
                  <Class class ="Parent" />
              </Source>
              <Target multiplicity="(0..*)" polymorphic="False" roleLabel="is referenced by">
                  <Class class="Child" />
              </Target>
           </ECRelationshipClass>
        </ECSchema>)xml")));

    ECInstanceKey parentKey;
    ASSERT_EQ(BE_SQLITE_DONE, testHelper.ExecuteInsertECSql(parentKey, "INSERT INTO ts.Parent(Name) VALUES('Parent 1')"));
    ASSERT_EQ(BE_SQLITE_DONE, testHelper.ExecuteECSql(Utf8PrintfString("INSERT INTO ts.Child(Parent.Id,Name) VALUES(%s,'Child 1')",
                                                                       parentKey.GetInstanceId().ToString().c_str()).c_str()));

    ASSERT_EQ(BE_SQLITE_OK, attached.SaveChanges());
    attached.CloseDb();
    }


    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("attachedtablespace.ecdb"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, GetHelper().PrepareECSql("SELECT * FROM ts.Parent"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, GetHelper().PrepareECSql("SELECT * FROM attached.ts.Parent"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.AttachDb(attachedECDbPath.GetNameUtf8().c_str(), "attached"));
    ASSERT_TRUE(m_ecdb.IsTransactionActive()) << "Transaction is expected to be restarted after attach/detach";

    ASSERT_EQ(ECSqlStatus::Success, GetHelper().PrepareECSql("SELECT * FROM ts.Parent"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, GetHelper().PrepareECSql("SELECT * FROM main.ts.Parent"));
    ASSERT_EQ(ECSqlStatus::Success, GetHelper().PrepareECSql("SELECT * FROM attached.ts.Parent"));

    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.DetachDb("attached"));
    ASSERT_TRUE(m_ecdb.IsTransactionActive()) << "Transaction is expected to be restarted after attach/detach";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, GetHelper().PrepareECSql("SELECT * FROM ts.Parent")) << "after detaching";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, GetHelper().PrepareECSql("SELECT * FROM attached.ts.Parent")) << "after detaching";

    //now attach with plain SQL. ECDb will not be notified about the attachment. Therefore queries will not work
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb(ECDb::OpenParams(ECDb::OpenMode::Readonly, DefaultTxn::No)));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.TryExecuteSql(Utf8PrintfString("ATTACH '%s' AS attached", attachedECDbPath.GetNameUtf8().c_str()).c_str()));
    ASSERT_FALSE(m_ecdb.IsTransactionActive()) << "Transaction is not expected to be restarted when attach is called via SQL (as BeSQLite is by-passed)";
    Savepoint sp(m_ecdb, "");
    ASSERT_EQ(ECSqlStatus::InvalidECSql, GetHelper().PrepareECSql("SELECT * FROM ts.Parent")) << "after attaching via SQL";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, GetHelper().PrepareECSql("SELECT * FROM attached.ts.Parent")) << "after attaching via SQL";
    sp.Cancel();
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Krischan.Eberle                  11/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(TableSpaceTestFixture, ECDbAndNonECDbAttachedTableSpace)
    {
    BeFileName attachedECDbPath, attachedDbPath;

    {
    //attach Db
    BeTest::GetHost().GetOutputRoot(attachedDbPath);
    attachedDbPath.AppendToPath(L"attachedtablespace_attached.db");
    if (attachedDbPath.DoesPathExist())
        ASSERT_EQ(BeFileNameStatus::Success, attachedDbPath.BeDeleteFile());

    Db attachedDb;
    ASSERT_EQ(BE_SQLITE_OK, attachedDb.CreateNewDb(attachedDbPath));
    ASSERT_EQ(BE_SQLITE_OK, attachedDb.SaveChanges());
    attachedDb.CloseDb();

    //attach ECDb 
    BeTest::GetHost().GetOutputRoot(attachedECDbPath);
    attachedECDbPath.AppendToPath(L"attachedtablespace_attached.ecdb");
    if (attachedECDbPath.DoesPathExist())
        ASSERT_EQ(BeFileNameStatus::Success, attachedECDbPath.BeDeleteFile());

    ECDb attached;
    ASSERT_EQ(BE_SQLITE_OK, attached.CreateNewDb(attachedECDbPath));

    TestHelper testHelper(attached);
    ASSERT_EQ(SUCCESS, testHelper.ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="Parent" modifier="None">
                <ECProperty propertyName="Name" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="Child" modifier="Sealed">
                <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward"/>
                <ECProperty propertyName="Name" typeName="string" />
            </ECEntityClass>
           <ECRelationshipClass typeName="Rel" strength="Referencing" modifier="Sealed">
              <Source multiplicity="(0..1)" polymorphic="False" roleLabel="has">
                  <Class class ="Parent" />
              </Source>
              <Target multiplicity="(0..*)" polymorphic="False" roleLabel="is referenced by">
                  <Class class="Child" />
              </Target>
           </ECRelationshipClass>
        </ECSchema>)xml")));

    ECInstanceKey parentKey;
    ASSERT_EQ(BE_SQLITE_DONE, testHelper.ExecuteInsertECSql(parentKey, "INSERT INTO ts.Parent(Name) VALUES('Parent 1')"));
    ASSERT_EQ(BE_SQLITE_DONE, testHelper.ExecuteECSql(Utf8PrintfString("INSERT INTO ts.Child(Parent.Id,Name) VALUES(%s,'Child 1')",
                                                                       parentKey.GetInstanceId().ToString().c_str()).c_str()));

    ASSERT_EQ(BE_SQLITE_OK, attached.SaveChanges());
    attached.CloseDb();
    }


    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("attachedtablespace_primary.ecdb"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, GetHelper().PrepareECSql("SELECT * FROM ts.Parent"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.AttachDb(attachedECDbPath.GetNameUtf8().c_str(), "attached"));
    ASSERT_TRUE(m_ecdb.IsTransactionActive()) << "Transaction is expected to be restarted after attach/detach";
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.AttachDb(attachedDbPath.GetNameUtf8().c_str(), "attacheddb"));
    ASSERT_TRUE(m_ecdb.IsTransactionActive()) << "Transaction is expected to be restarted after attach/detach";

    ASSERT_EQ(ECSqlStatus::Success, GetHelper().PrepareECSql("SELECT * FROM ts.Parent"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, GetHelper().PrepareECSql("SELECT * FROM main.ts.Parent"));
    ASSERT_EQ(ECSqlStatus::Success, GetHelper().PrepareECSql("SELECT * FROM attached.ts.Parent"));

    ASSERT_EQ(BE_SQLITE_ERROR, GetHelper().ExecuteECSql("INSERT INTO ts.Parent(Name) VALUES('Parent 2')"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, GetHelper().PrepareECSql("INSERT INTO main.ts.Parent(Name) VALUES('Parent 2')"));
    ASSERT_EQ(BE_SQLITE_ERROR, GetHelper().ExecuteECSql("INSERT INTO attached.ts.Parent(Name) VALUES('Parent 3')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.Parent(ECInstanceId,Name) VALUES(100,'Parent 2')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO attached.ts.Parent(ECInstanceId,Name) VALUES(101,'Parent 2')"));

    ASSERT_EQ(BE_SQLITE_ERROR, GetHelper().ExecuteECSql("INSERT INTO ts.Child(Name) VALUES('Child 2')"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, GetHelper().PrepareECSql("INSERT INTO main.ts.Child(Name) VALUES('Child 2')"));
    ASSERT_EQ(BE_SQLITE_ERROR, GetHelper().ExecuteECSql("INSERT INTO attached.ts.Child(Name) VALUES('Child 3')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.Child(ECInstanceId,Name) VALUES(200,'Child 2')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO attached.ts.Child(ECInstanceId,Name) VALUES(201,'Child 2')"));

    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());

    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb(ECDb::OpenParams(ECDb::OpenMode::Readonly)));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.AttachDb(attachedECDbPath.GetNameUtf8().c_str(), "attached"));
    ASSERT_TRUE(m_ecdb.IsTransactionActive()) << "Transaction is expected to be restarted after attach/detach";
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.AttachDb(attachedDbPath.GetNameUtf8().c_str(), "attacheddb"));
    ASSERT_TRUE(m_ecdb.IsTransactionActive()) << "Transaction is expected to be restarted after attach/detach";
    ASSERT_EQ(ECSqlStatus::Error, GetHelper().PrepareECSql("INSERT INTO ts.Parent(Name) VALUES('Parent 2')")) << "file opened read-only which applies to attached file as well";
    ASSERT_EQ(ECSqlStatus::Error, GetHelper().PrepareECSql("INSERT INTO attached.ts.Parent(Name) VALUES('Parent 2')")) << "file opened read-only which applies to attached file as well";
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Krischan.Eberle                  11/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(TableSpaceTestFixture, AttachedTableSpace_IndexesAndRels)
    {
    BeFileName settingsDbPath;

    ECInstanceKey fooKey, settingKey, linkTableRel;

    {
    //attach settings file
    BeTest::GetHost().GetOutputRoot(settingsDbPath);
    settingsDbPath.AppendToPath(L"AttachedTableSpace.ecdb.settings");
    if (settingsDbPath.DoesPathExist())
        ASSERT_EQ(BeFileNameStatus::Success, settingsDbPath.BeDeleteFile());

    ECDb settingsDb;
    ASSERT_EQ(BE_SQLITE_OK, settingsDb.CreateNewDb(settingsDbPath));

    TestHelper testHelper(settingsDb);
    ASSERT_EQ(SUCCESS, testHelper.ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Foo" modifier="None">
                <ECProperty propertyName="Name" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="SessionSetting" modifier="Sealed">
                <ECCustomAttributes>
                    <DbIndexList xmlns="ECDbMap.02.00">
                       <Indexes>
                           <DbIndex>
                                <Name>uix_sessionsetting_name</Name>
                                <IsUnique>True</IsUnique>
                                <Properties>
                                    <string>Name</string>
                                </Properties>
                           </DbIndex>
                           <DbIndex>
                                <Name>ix_sessionsetting_val</Name>
                                <Properties>
                                    <string>Val</string>
                                </Properties>
                           </DbIndex>
                       </Indexes>
                    </DbIndexList>
                </ECCustomAttributes>
                <ECNavigationProperty propertyName="Foo" relationshipName="Rel" direction="Backward"/>
                <ECProperty propertyName="Name" typeName="string" />
                <ECProperty propertyName="Val" typeName="binary" />
            </ECEntityClass>
           <ECRelationshipClass typeName="Rel" strength="Referencing" modifier="Sealed">
              <Source multiplicity="(0..1)" polymorphic="False" roleLabel="has">
                  <Class class ="Foo" />
              </Source>
              <Target multiplicity="(0..*)" polymorphic="False" roleLabel="is referenced by">
                  <Class class="SessionSetting" />
              </Target>
           </ECRelationshipClass>
           <ECRelationshipClass typeName="LinkTableRel" strength="Referencing" modifier="Sealed">
                <ECCustomAttributes>
                    <LinkTableRelationshipMap xmlns="ECDbMap.02.00">
                        <CreateForeignKeyConstraints>false</CreateForeignKeyConstraints>
                    </LinkTableRelationshipMap>
                </ECCustomAttributes>
              <Source multiplicity="(0..*)" polymorphic="False" roleLabel="has">
                  <Class class ="Foo" />
              </Source>
              <Target multiplicity="(0..*)" polymorphic="False" roleLabel="is referenced by">
                  <Class class="SessionSetting" />
              </Target>
           </ECRelationshipClass>
        </ECSchema>)xml")));

    ASSERT_EQ(BE_SQLITE_DONE, testHelper.ExecuteInsertECSql(fooKey, "INSERT INTO ts.Foo(Name) VALUES('Foo 1')"));
    ASSERT_EQ(BE_SQLITE_DONE, testHelper.ExecuteInsertECSql(settingKey, Utf8PrintfString("INSERT INTO ts.SessionSetting(Foo.Id,Name,Val) VALUES(%s,'Logging1',true)",
                                                                                         fooKey.GetInstanceId().ToString().c_str()).c_str()));
    ASSERT_EQ(BE_SQLITE_DONE, testHelper.ExecuteInsertECSql(linkTableRel, Utf8PrintfString("INSERT INTO ts.LinkTableRel(SourceECInstanceId,TargetECInstanceId) VALUES(%s,%s)",
                                                                                           fooKey.GetInstanceId().ToString().c_str(), settingKey.GetInstanceId().ToString().c_str()).c_str()));

    ASSERT_EQ(BE_SQLITE_OK, settingsDb.SaveChanges());
    settingsDb.CloseDb();
    }

    auto assertSelectSettings = [this, &fooKey] (int expectedRowCount)
        {
        ECSqlStatus expectedStat = expectedRowCount >= 0 ? ECSqlStatus::Success : ECSqlStatus::InvalidECSql;
        Utf8String assertMessage = expectedRowCount >= 0 ? Utf8PrintfString("TableSpace is attached (%d expected rows)", expectedRowCount) : Utf8String("TableSpace not attached");
        ECSqlStatement stmt;
        ASSERT_EQ(expectedStat, stmt.Prepare(m_ecdb, "SELECT count(*) FROM ts.SessionSetting")) << assertMessage;
        if (expectedStat != ECSqlStatus::Success)
            return;

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << assertMessage;
        ASSERT_EQ(expectedRowCount, stmt.GetValueInt(0)) << stmt.GetECSql() << assertMessage;
        };

    auto assertSelectLinkTable = [this, &fooKey, &settingKey, &linkTableRel] (int expectedRowCount)
        {
        ECSqlStatus expectedStat = expectedRowCount >= 0 ? ECSqlStatus::Success : ECSqlStatus::InvalidECSql;
        Utf8String assertMessage = expectedRowCount >= 0 ? Utf8PrintfString("TableSpace is attached (%d expected rows)", expectedRowCount) : Utf8String("TableSpace not attached");
        ECSqlStatement stmt;
        ASSERT_EQ(expectedStat, stmt.Prepare(m_ecdb, "SELECT count(*) FROM ts.LinkTableRel")) << assertMessage;
        if (expectedStat != ECSqlStatus::Success)
            return;

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << assertMessage;
        ASSERT_EQ(expectedRowCount, stmt.GetValueInt(0)) << stmt.GetECSql() << assertMessage;
        };


    auto assertIndexes = [this] (bool expectedToExist)
        {
        ASSERT_TRUE(GetHelper().GetIndexNamesForTable("ts_Foo", "settings").empty()) << "Indexes on ts_Foo are never expected";
        if (expectedToExist)
            ASSERT_EQ(std::vector<Utf8String>({"ix_sessionsetting_val", "uix_sessionsetting_name"}), GetHelper().GetIndexNamesForTable("ts_SessionSetting", "settings"));
        };

    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("AttachedTableSpace.ecdb"));

    assertSelectSettings(-1);
    assertSelectLinkTable(-1);
    assertIndexes(false);

    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.AttachDb(settingsDbPath.GetNameUtf8().c_str(), "settings"));
    assertSelectSettings(1);
    assertSelectLinkTable(1);
    assertIndexes(true);

    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    assertSelectSettings(-1);
    assertSelectLinkTable(-1);
    assertIndexes(false);


    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.AttachDb(settingsDbPath.GetNameUtf8().c_str(), "settings"));
    assertSelectSettings(1);
    assertSelectLinkTable(1);
    assertIndexes(true);

    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql(Utf8PrintfString("INSERT INTO ts.SessionSetting(ECInstanceId,Foo.Id,Name,Val) VALUES(1000,%s,'Debugging',false)",
                                                                        fooKey.GetInstanceId().ToString().c_str()).c_str()));

    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(linkTableRel, Utf8PrintfString("INSERT INTO ts.LinkTableRel(ECInstanceId,SourceECInstanceId,TargetECInstanceId) VALUES(2000,%s,1000)",
                                                                                            fooKey.GetInstanceId().ToString().c_str()).c_str()));

    assertSelectSettings(2);
    assertSelectLinkTable(2);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Krischan.Eberle                  11/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(TableSpaceTestFixture, AttachedTableSpace_PhysicalForeignKey)
    {
    BeFileName settingsDbPath;

    {
    //attach settings file
    BeTest::GetHost().GetOutputRoot(settingsDbPath);
    settingsDbPath.AppendToPath(L"AttachedTableSpace_PhysicalForeignKey.ecdb.settings");
    if (settingsDbPath.DoesPathExist())
        ASSERT_EQ(BeFileNameStatus::Success, settingsDbPath.BeDeleteFile());

    ECDb settingsDb;
    ASSERT_EQ(BE_SQLITE_OK, settingsDb.CreateNewDb(settingsDbPath));

    ASSERT_EQ(SUCCESS, TestHelper(settingsDb).ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Foo" modifier="Sealed">
                <ECProperty propertyName="Name" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="SessionSetting" modifier="Sealed">
              <ECNavigationProperty propertyName="Foo" relationshipName="Rel" direction="Backward">
                <ECCustomAttributes>
                    <ForeignKeyConstraint xmlns="ECDbMap.02.00">
                        <OnDeleteAction>Cascade</OnDeleteAction>
                    </ForeignKeyConstraint>
                </ECCustomAttributes>
              </ECNavigationProperty>
                <ECProperty propertyName="Name" typeName="string" />
                <ECProperty propertyName="Val" typeName="binary" />
            </ECEntityClass>
           <ECRelationshipClass typeName="Rel" strength="Embedding" modifier="Sealed">
              <Source multiplicity="(0..1)" polymorphic="False" roleLabel="has">
                  <Class class ="Foo" />
              </Source>
              <Target multiplicity="(0..*)" polymorphic="False" roleLabel="is referenced by">
                  <Class class="SessionSetting" />
              </Target>
           </ECRelationshipClass>
        </ECSchema>)xml")));
    settingsDb.CloseDb();
    }

    auto attachSettingsFile = [this, &settingsDbPath] () { return m_ecdb.AttachDb(settingsDbPath.GetNameUtf8().c_str(), "settings"); };

    auto getRelCount = [this] ()
        {
        ECSqlStatement stmt;
        if (ECSqlStatus::Success != stmt.Prepare(m_ecdb, "SELECT count(*) FROM ts.Rel") || BE_SQLITE_ROW != stmt.Step())
            return -1;

        return stmt.GetValueInt(0);
        };

    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("AttachedTableSpace_PhysicalForeignKey.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, attachSettingsFile());
    ASSERT_TRUE(GetHelper().TableSpaceExists("settings"));
    ASSERT_EQ(0, getRelCount());

    ECInstanceKey fooKey, settingKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(fooKey, "INSERT INTO ts.Foo(ECInstanceId,Name) VALUES(100,'Foo 1')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(settingKey, "INSERT INTO ts.SessionSetting(ECInstanceId,Foo.Id,Name,Val) VALUES(200,100,'Logging1',true)"));

    ASSERT_EQ(1, getRelCount()) << "After one insert";
    m_ecdb.SaveChanges();

    //expected to fail prepare as file is not attached yet
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    ASSERT_FALSE(GetHelper().TableSpaceExists("settings"));
    ASSERT_EQ(-1, getRelCount()) << "After reopening without attaching";
    ASSERT_EQ(BE_SQLITE_OK, attachSettingsFile());
    ASSERT_TRUE(GetHelper().TableSpaceExists("settings"));
    ASSERT_EQ(1, getRelCount()) << "After reattaching and reopening";

    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(settingKey, Utf8PrintfString("INSERT INTO ts.SessionSetting(ECInstanceId,Foo.Id,Name,Val) VALUES(1000,%s,'Debugging',false)",
                                                                                          fooKey.GetInstanceId().ToString().c_str()).c_str()));
    ASSERT_EQ(2, getRelCount()) << "After second insert";
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());

    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    ASSERT_FALSE(GetHelper().TableSpaceExists("settings"));
    ASSERT_EQ(-1, getRelCount()) << "After reopening without attaching";
    ASSERT_EQ(BE_SQLITE_OK, attachSettingsFile());
    ASSERT_TRUE(GetHelper().TableSpaceExists("settings"));
    ASSERT_EQ(2, getRelCount()) << "After reattaching and reopening";

    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("DELETE FROM ts.Foo"));
    ASSERT_EQ(0, getRelCount()) << "Cascading delete after deleting Foo";
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Krischan.Eberle                  07/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(TableSpaceTestFixture, DetachTableSpace)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("detachtablespace.ecdb"));

    BeFileName attachedECDbPath;

    {
    //create attachment ECDb 
    BeTest::GetHost().GetOutputRoot(attachedECDbPath);
    attachedECDbPath.AppendToPath(L"attachedtablespace_attached.ecdb");
    if (attachedECDbPath.DoesPathExist())
        ASSERT_EQ(BeFileNameStatus::Success, attachedECDbPath.BeDeleteFile());

    ECDb attached;
    ASSERT_EQ(BE_SQLITE_OK, attached.CreateNewDb(attachedECDbPath));
    }
    // test that the current default transaction remains active after attach/detach calls
    // (because internally attach/detach commits and restarts the transaction)
    ASSERT_TRUE(m_ecdb.IsTransactionActive());
    //calling detach for a table space that doesn't exist
    ASSERT_FALSE(GetHelper().TableSpaceExists("doesnotexist"));
    ASSERT_TRUE(m_ecdb.IsTransactionActive());
    ASSERT_EQ(BE_SQLITE_ERROR, m_ecdb.DetachDb("doesnotexist"));
    ASSERT_TRUE(m_ecdb.IsTransactionActive());

    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.AttachDb(attachedECDbPath.GetNameUtf8().c_str(), "testtablespace"));
    ASSERT_TRUE(m_ecdb.IsTransactionActive());
    ASSERT_TRUE(GetHelper().TableSpaceExists("testtablespace"));
    ASSERT_TRUE(m_ecdb.IsTransactionActive());
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.DetachDb("testtablespace"));
    ASSERT_TRUE(m_ecdb.IsTransactionActive());
    ASSERT_FALSE(GetHelper().TableSpaceExists("testtablespace"));
    ASSERT_TRUE(m_ecdb.IsTransactionActive());
    ASSERT_EQ(BE_SQLITE_ERROR, m_ecdb.DetachDb("testtablespace"));
    ASSERT_TRUE(m_ecdb.IsTransactionActive());
    ASSERT_FALSE(GetHelper().TableSpaceExists("testtablespace"));
    ASSERT_TRUE(m_ecdb.IsTransactionActive());
    }
//---------------------------------------------------------------------------------------
// @bsiMethod                                     Krischan.Eberle                  01/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(TableSpaceTestFixture, JsonAdapter)
    {
    BeFileName attachedDbPath;

    ECInstanceKey parentKey, childKey, relKey;

    {
    //attach settings file
    BeTest::GetHost().GetOutputRoot(attachedDbPath);
    attachedDbPath.AppendToPath(L"AttachedTableSpace_PhysicalForeignKey.ecdb.settings");
    if (attachedDbPath.DoesPathExist())
        ASSERT_EQ(BeFileNameStatus::Success, attachedDbPath.BeDeleteFile());

    ECDb attachedDb;
    ASSERT_EQ(BE_SQLITE_OK, attachedDb.CreateNewDb(attachedDbPath));

    TestHelper attachedTestHelper(attachedDb);
    ASSERT_EQ(SUCCESS, attachedTestHelper.ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Parent" modifier="Sealed">
                <ECProperty propertyName="Name" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="Child" modifier="Sealed">
                <ECProperty propertyName="Name" typeName="string" />
                <ECNavigationProperty propertyName="Parent" relationshipName="FkRel" direction="Backward"/>
            </ECEntityClass>
           <ECRelationshipClass typeName="FkRel" strength="Embedding" modifier="Sealed">
              <Source multiplicity="(0..1)" polymorphic="False" roleLabel="has">
                  <Class class ="Parent" />
              </Source>
              <Target multiplicity="(0..*)" polymorphic="False" roleLabel="is referenced by">
                  <Class class="Child" />
              </Target>
           </ECRelationshipClass>
           <ECRelationshipClass typeName="LinkTableRel" strength="Referencing" modifier="Sealed">
              <Source multiplicity="(0..*)" polymorphic="False" roleLabel="has">
                  <Class class ="Parent" />
              </Source>
              <Target multiplicity="(0..*)" polymorphic="False" roleLabel="is referenced by">
                  <Class class="Parent" />
              </Target>
           </ECRelationshipClass>
        </ECSchema>)xml")));

    ASSERT_EQ(BE_SQLITE_DONE, attachedTestHelper.ExecuteInsertECSql(parentKey, "INSERT INTO TestSchema.Parent(Name) VALUES('Parent 1')"));
    ASSERT_EQ(BE_SQLITE_DONE, attachedTestHelper.ExecuteInsertECSql(childKey, Utf8PrintfString("INSERT INTO TestSchema.Child(Name,Parent.Id) VALUES('Child',%s)",parentKey.GetInstanceId().ToString().c_str()).c_str()));
    ASSERT_EQ(BE_SQLITE_DONE, attachedTestHelper.ExecuteInsertECSql(relKey, Utf8PrintfString("INSERT INTO TestSchema.LinkTableRel(SourceECInstanceId,TargetECInstanceId) VALUES(%s,%s)", parentKey.GetInstanceId().ToString().c_str(), parentKey.GetInstanceId().ToString().c_str()).c_str()));
    ASSERT_EQ(BE_SQLITE_OK, attachedDb.SaveChanges());
    attachedDb.CloseDb();
    }

    Utf8String parentId = parentKey.GetInstanceId().ToHexStr();
    Utf8String childId = childKey.GetInstanceId().ToHexStr();
    Utf8String relId = relKey.GetInstanceId().ToHexStr();

    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("attachedtablespace_json.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.AttachDb(attachedDbPath.GetNameUtf8().c_str(), "att"));

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId,ECClassId,Name FROM TestSchema.Parent"));
    JsonECSqlSelectAdapter adapter(stmt);
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    JsonValue row;
    ASSERT_EQ(SUCCESS, adapter.GetRow(row.m_value));
    EXPECT_EQ(JsonValue(Utf8PrintfString(R"({"id":"%s", "className":"TestSchema.Parent","Name":"Parent 1"})", parentId.c_str())), row);
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId,ECClassId,Parent,Name FROM TestSchema.Child"));
    JsonECSqlSelectAdapter adapter(stmt);
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    JsonValue row;
    ASSERT_EQ(SUCCESS, adapter.GetRow(row.m_value));
    EXPECT_EQ(JsonValue(Utf8PrintfString(R"({"id":"%s", "className":"TestSchema.Child","Name":"Child", "Parent":{"id":"%s", "relClassName":"TestSchema.FkRel"}})", childId.c_str(), parentId.c_str())), row);
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId,ECClassId,SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId FROM TestSchema.LinkTableRel"));
    JsonECSqlSelectAdapter adapter(stmt);
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    JsonValue row;
    ASSERT_EQ(SUCCESS, adapter.GetRow(row.m_value));
    EXPECT_EQ(JsonValue(Utf8PrintfString(R"({"id":"%s", "className":"TestSchema.LinkTableRel","sourceId":"%s","sourceClassName":"TestSchema.Parent","targetId":"%s", "targetClassName":"TestSchema.Parent"})",
                                         relId.c_str(), parentId.c_str(), parentId.c_str())), row);
    }

    }

END_ECDBUNITTESTS_NAMESPACE