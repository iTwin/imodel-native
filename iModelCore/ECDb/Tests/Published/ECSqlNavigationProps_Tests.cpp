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
    ECDbR ecdb = SetupECDb("ecsqlnavprops.ecdb", BeFileName(L"ECSql_NavigationProperties.01.00.ecschema.xml"), 3, ECDb::OpenParams(Db::OpenMode::ReadWrite));
    ASSERT_TRUE(ecdb.IsDbOpen());

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

    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT Code, Model FROM np.Element WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, elementKey.GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT * FROM np.Element WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, elementKey.GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    }
    }

END_ECDBUNITTESTS_NAMESPACE

