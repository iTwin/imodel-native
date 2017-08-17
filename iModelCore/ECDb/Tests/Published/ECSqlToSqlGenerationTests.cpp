/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECSqlToSqlGenerationTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

struct ECSqlToSqlGenerationTests : ECDbTestFixture {};
//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlToSqlGenerationTests, PolymorphicSqlExpr)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("PolymorphicSqlExpr.ecdb", SchemaItem(
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
    {//BaseClass
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * FROM ts.BaseClass"));
    ASSERT_STREQ("SELECT [BaseClass].[ECInstanceId],[BaseClass].[ECClassId],[BaseClass].[ps1] FROM (SELECT [ts_BaseClass].[Id] ECInstanceId,[ts_BaseClass].[ECClassId],[ts_BaseClass].[ps1] FROM [ts_BaseClass]) [BaseClass]", stmt.GetNativeSql());
    }

    {//SubClass
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * FROM ts.D_A"));
    ASSERT_TRUE(Utf8String(stmt.GetNativeSql()).Contains("INNER JOIN ec_cache_ClassHierarchy [CHC_ts_BaseClass] ON [CHC_ts_BaseClass].[ClassId]=[ts_BaseClass].ECClassId AND [CHC_ts_BaseClass].[BaseClassId]"));
    }

    {//ONLY
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * FROM ONLY ts.D_A"));
    ASSERT_STREQ("SELECT [D_A].[ECInstanceId],[D_A].[ECClassId],[D_A].[ps1],[D_A].[ps2],[D_A].[ps3] FROM (SELECT [Id] ECInstanceId,[ECClassId],[ps1],[ps2],[ps3] FROM [ts_BaseClass] WHERE [ts_BaseClass].ECClassId=57) [D_A]", stmt.GetNativeSql());
    ASSERT_TRUE(Utf8String(stmt.GetNativeSql()).Contains("WHERE [ts_BaseClass].ECClassId="));
    }

    {//NoECClassIdFilter
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * FROM ts.D_A ECSQLOPTIONS NoECClassIdFilter"));
    ASSERT_TRUE(Utf8String(stmt.GetNativeSql()).Contains("FROM [ts_BaseClass]) [D_A]"));
    }
    }

END_ECDBUNITTESTS_NAMESPACE
