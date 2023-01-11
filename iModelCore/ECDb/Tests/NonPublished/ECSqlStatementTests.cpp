/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "NestedStructArrayTestSchemaHelper.h"
#include <Bentley/Base64Utilities.h>
#include <cmath>
#include <algorithm>
#include <set>
#include <BeRapidJson/BeRapidJson.h>

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

struct ECSqlStatementTestFixture : ECDbTestFixture {};

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, ECSqlStatus)
    {
    ECSqlStatus empty;
    ASSERT_EQ(ECSqlStatus::Status::Success, empty.Get());
    ASSERT_EQ(ECSqlStatus::Status::Success, empty) << "implicit cast operator";
    ASSERT_FALSE(empty.IsSQLiteError());
    ASSERT_EQ(BE_SQLITE_OK, empty.GetSQLiteError());
    ASSERT_TRUE(ECSqlStatus::Success == empty);
    ASSERT_FALSE(ECSqlStatus::Success != empty);

    ECSqlStatus invalidECSql = ECSqlStatus::InvalidECSql;
    ASSERT_EQ(ECSqlStatus::Status::InvalidECSql, invalidECSql.Get());
    ASSERT_EQ(ECSqlStatus::Status::InvalidECSql, invalidECSql) << "implicit cast operator";
    ASSERT_FALSE(invalidECSql.IsSQLiteError());
    ASSERT_EQ(BE_SQLITE_OK, invalidECSql.GetSQLiteError());
    ASSERT_TRUE(ECSqlStatus::InvalidECSql == invalidECSql);
    ASSERT_FALSE(ECSqlStatus::InvalidECSql != invalidECSql);

    ECSqlStatus error = ECSqlStatus::Error;
    ASSERT_EQ(ECSqlStatus::Status::Error, error.Get());
    ASSERT_EQ(ECSqlStatus::Status::Error, error) << "implicit cast operator";
    ASSERT_FALSE(error.IsSQLiteError());
    ASSERT_EQ(BE_SQLITE_OK, error.GetSQLiteError());
    ASSERT_TRUE(ECSqlStatus::Error == error);
    ASSERT_FALSE(ECSqlStatus::Error != error);

    ECSqlStatus sqliteError(BE_SQLITE_CONSTRAINT_CHECK);
    ASSERT_EQ(ECSqlStatus::Status::SQLiteError, sqliteError.Get());
    ASSERT_EQ(ECSqlStatus::Status::SQLiteError, sqliteError) << "implicit cast operator";
    ASSERT_TRUE(sqliteError.IsSQLiteError());
    ASSERT_EQ(BE_SQLITE_CONSTRAINT_CHECK, sqliteError.GetSQLiteError());

    ASSERT_TRUE(ECSqlStatus(BE_SQLITE_CONSTRAINT_CHECK) == sqliteError);
    ASSERT_FALSE(ECSqlStatus(BE_SQLITE_CONSTRAINT_CHECK) != sqliteError);

    ECSqlStatus sqliteError2(BE_SQLITE_BUSY);
    ASSERT_EQ(ECSqlStatus::Status::SQLiteError, sqliteError2.Get());
    ASSERT_EQ(ECSqlStatus::Status::SQLiteError, sqliteError2) << "implicit cast operator";
    ASSERT_TRUE(sqliteError2.IsSQLiteError());
    ASSERT_EQ(BE_SQLITE_BUSY, sqliteError2.GetSQLiteError());
    ASSERT_TRUE(ECSqlStatus(BE_SQLITE_BUSY) == sqliteError2);
    ASSERT_FALSE(ECSqlStatus(BE_SQLITE_BUSY) != sqliteError2);

    ASSERT_FALSE(sqliteError == sqliteError2);
    ASSERT_TRUE(sqliteError != sqliteError2);
    ASSERT_FALSE(sqliteError2 == sqliteError);
    ASSERT_TRUE(sqliteError2 != sqliteError);

    ASSERT_FALSE(empty == invalidECSql);
    ASSERT_TRUE(empty != invalidECSql);

    ASSERT_FALSE(empty == error);
    ASSERT_TRUE(empty != error);

    ASSERT_FALSE(empty == sqliteError);
    ASSERT_TRUE(empty != sqliteError);

    ASSERT_FALSE(empty == sqliteError2);
    ASSERT_TRUE(empty != sqliteError2);

    ASSERT_FALSE(invalidECSql == error);
    ASSERT_TRUE(invalidECSql != error);

    ASSERT_FALSE(invalidECSql == sqliteError);
    ASSERT_TRUE(invalidECSql != sqliteError);

    ASSERT_FALSE(invalidECSql == sqliteError2);
    ASSERT_TRUE(invalidECSql != sqliteError2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, CTECrash) {
    ASSERT_EQ(SUCCESS, SetupECDb("cte_crash.ecdb"));
    auto ecsql = R"(
        WITH RECURSIVE
        F (A) AS (SELECT 1),
        S (A) AS (SELECT * FROM F UNION SELECT 1 FROM S WHERE S.A = 1)
        SELECT A FROM S
    )";
    for (int i = 0; i < 1; ++i) {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_STREQ( stmt.GetNativeSql(), "WITH F(A) AS (SELECT 1),S(A) AS (SELECT F.A FROM F UNION SELECT 1 FROM S WHERE S.A=1)\nSELECT S.A FROM S");
    }
}
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, DisableFunction) {
    ASSERT_EQ(SUCCESS, SetupECDb("disabledFunc.ecdb"));
    struct IssueListener: ECN::IIssueListener {
        mutable bvector<Utf8String> m_issues;
        void _OnIssueReported(ECN::IssueSeverity severity, ECN::IssueCategory category, ECN::IssueType type, Utf8CP message) const override {
            m_issues.push_back(message);
        }
        Utf8String const& GetLastError() const { return m_issues.back();}
    };
    IssueListener listener;
    m_ecdb.AddIssueListener(listener);

    auto getExpectedErrorMessage =[](Utf8String functionName) -> Utf8String {
        return SqlPrintfString("Failed to prepare function with name '%s': Function is disabled by application.", functionName.c_str()).GetUtf8CP();
    };
    auto assertFunc = [&](Utf8StringCR ecsql, Utf8String functionNameInECSql, Utf8String disableFunc) {
        ECSqlStatement stmt;

        //TEST A - Just prepare ecsql as is to verify it can be prepared.
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str())) << "ECSQL should succeed without disabling the function"
            << "\nECDb (err)" << listener.GetLastError().c_str();
        stmt.Finalize();

        //TEST B - Disable function and prepare the ecsql, with expectation that it will fail and puts out a expected error message.
        m_ecdb.GetECSqlConfig().GetDisableFunctions().Add(disableFunc);
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, ecsql.c_str())) << "ECSQL should now fail when function is disabled";
        Utf8String erroMessageExpected = getExpectedErrorMessage(functionNameInECSql);
        ASSERT_STREQ(listener.GetLastError().c_str(), erroMessageExpected.c_str()) << "Perpare should fail with expected error message";
        stmt.Finalize();

        //TEST C - ReEnable the function and reprepare the ecsql and expect it to be successful
        m_ecdb.GetECSqlConfig().GetDisableFunctions().Remove(disableFunc);
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str())) << "ECSQL should succeed after enabling the function";
    };

    assertFunc("SELECT sqlite_version()", "sqlite_version", "sqlite_version");
    assertFunc("SELECT sqlite_version()", "sqlite_version", "SQLITE_VERSION");
    assertFunc("SELECT randomblob(1923)", "randomblob", "randomblob");
    assertFunc("SELECT randomblob(1923)", "randomblob", "RANDOMBLOB");
}
/*---------------------------------------------------------------------------------**//**
* @bsimethod
* ECSqlStatementTestFixture.BugFix
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, NestedValConstructor) {
    ASSERT_EQ(SUCCESS, SetupECDb("RowConstructor.ecdb"));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,"SELECT * FROM (SELECT 1001 a)"));
    ASSERT_STREQ(stmt.GetNativeSql(), "SELECT [K0] FROM (SELECT 1001 [K0])");
}
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, PopulateECSql_TestDbWithTestData)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ECSqlStatementTests.ecdb", SchemaItem::CreateForFile("ECSqlStatementTests.01.00.00.ecschema.xml")));
    NestedStructArrayTestSchemaHelper::PopulateECSqlStatementTestsDb(m_ecdb);
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, SelectBitwiseOperators)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("bitwise.ecdb"));
    auto testOp = [&] (Utf8CP test)
        {
        ECSqlStatement stmt;
        EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT %s FROM meta.ECClassDef LIMIT 1", test)));
        EXPECT_EQ(BE_SQLITE_ROW, stmt.Step());
        return stmt.GetValueInt64(0);
        };

    ASSERT_EQ(testOp("1 | 1"), 1);
    ASSERT_EQ(testOp("1 | 2"), 3);
    ASSERT_EQ(testOp("0x00ff | 0xace"), 0x0aff);
    ASSERT_EQ(testOp("0x1 | 0x2 | 0x4 | 0x8 "), 0xf);
    ASSERT_EQ(testOp("0x1 & 0x1 | 0x8"), 0x9);
    ASSERT_EQ(testOp("0x1 & 0x2"), 0x0);
    ASSERT_EQ(testOp("0x2 & 0x2"), 0x2);
    ASSERT_EQ(testOp("1 << 1"), 0x2);
    ASSERT_EQ(testOp("1 << 2"), 0x4);
    ASSERT_EQ(testOp("1 << 3"), 0x8);
    ASSERT_EQ(testOp("1 << 4"), 0x10);
    ASSERT_EQ(testOp("1 >> 1"), 0x0);
    ASSERT_EQ(testOp("2 >> 1"), 0x1);
    ASSERT_EQ(testOp("4 >> 1"), 0x2);
    ASSERT_EQ(testOp("8 >> 1"), 0x4);
    ASSERT_EQ(testOp("~1"), -2);
    ASSERT_EQ(testOp("~2"), -3);
    ASSERT_EQ(testOp("~0xff"), -256);
    }
TEST_F(ECSqlStatementTestFixture, WhereBitwiseOperators)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("bitwise.ecdb"));
    auto testOp = [&] (Utf8CP test)
        {
        ECSqlStatement stmt;
        return stmt.Prepare(m_ecdb, SqlPrintfString("SELECT 1 FROM meta.ECClassDef WHERE %s LIMIT 1", test));
        };

    EXPECT_EQ(ECSqlStatus::Success, testOp("1"));
    EXPECT_EQ(ECSqlStatus::Success, testOp("~1"));
    EXPECT_EQ(ECSqlStatus::Success, testOp("1&1"));
    EXPECT_EQ(ECSqlStatus::Success, testOp("1|1&1"));
    EXPECT_EQ(ECSqlStatus::Success, testOp("1+1"));

    EXPECT_EQ(ECSqlStatus::Success, testOp("invirtualset(?,1)"));
    EXPECT_EQ(ECSqlStatus::Success, testOp("invirtualset(?,~1)"));
    EXPECT_EQ(ECSqlStatus::Success, testOp("invirtualset(?,1&1)"));
    EXPECT_EQ(ECSqlStatus::Success, testOp("invirtualset(?,1|1&1)"));
    EXPECT_EQ(ECSqlStatus::Success, testOp("invirtualset(?,1+1)"));

    EXPECT_EQ(ECSqlStatus::Success, testOp("ECInstanceId IN (?,1)"));
    EXPECT_EQ(ECSqlStatus::Success, testOp("ECInstanceId IN (?,~1)"));
    EXPECT_EQ(ECSqlStatus::Success, testOp("ECInstanceId IN (?,1&1)"));
    EXPECT_EQ(ECSqlStatus::Success, testOp("ECInstanceId IN (?,1|1&1)"));
    EXPECT_EQ(ECSqlStatus::Success, testOp("ECInstanceId IN (?,1+1)"));

    EXPECT_EQ(ECSqlStatus::Success, testOp("1>1"));
    EXPECT_EQ(ECSqlStatus::Success, testOp("1>~1"));
    EXPECT_EQ(ECSqlStatus::Success, testOp("1>1&1"));
    EXPECT_EQ(ECSqlStatus::Success, testOp("1>1|1&1"));
    EXPECT_EQ(ECSqlStatus::Success, testOp("1>1+1"));

    // issue with grammer where left side recursion cause boolean exp and value exp to hit '(' as prefix.
    // the only way around it to compbine boolean/value exp into one.
    EXPECT_EQ(ECSqlStatus::InvalidECSql, testOp("(1)=1"));
    EXPECT_EQ(ECSqlStatus::InvalidECSql, testOp("(1)=(1)"));
    EXPECT_EQ(ECSqlStatus::InvalidECSql, testOp("((1)=(1))"));

    EXPECT_EQ(ECSqlStatus::Success, testOp("(1+1)=1+1"));
    EXPECT_EQ(ECSqlStatus::Success, testOp("(1+1)=(1+1)"));
    EXPECT_EQ(ECSqlStatus::Success, testOp("((1+1)=(1+1))"));

    EXPECT_EQ(ECSqlStatus::Success, testOp("(~1)=~1"));
    EXPECT_EQ(ECSqlStatus::Success, testOp("(~1)=(~1)"));
    EXPECT_EQ(ECSqlStatus::Success, testOp("((~1)=(~1))"));

    EXPECT_EQ(ECSqlStatus::Success, testOp("(~1)=~1 and (~1)=~1"));
    EXPECT_EQ(ECSqlStatus::Success, testOp("(~1)=(~1) and (~1)=(~1)"));
    EXPECT_EQ(ECSqlStatus::Success, testOp("((~1)=(~1)) and ((~1)=(~1))"));
    EXPECT_EQ(ECSqlStatus::Success, testOp("(~1=1) and (~1=1)"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, NavPropAlias)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("NavPropAlias.ecdb"));
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT Schema, Schema MySchema FROM meta.ECClassDef"));
    int rowCount = 0;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        rowCount++;
        IECSqlValue const& unaliasedVal = stmt.GetValue(0);
        IECSqlValue const& aliasedVal = stmt.GetValue(1);

        ECClassId unaliasedRelClassId, aliasedRelClassId;
        EXPECT_EQ(unaliasedVal.GetNavigation(&unaliasedRelClassId), aliasedVal.GetNavigation(&aliasedRelClassId));
        EXPECT_EQ(unaliasedRelClassId, aliasedRelClassId);

        ECSqlColumnInfoCR unaliasedColInfo = unaliasedVal.GetColumnInfo();
        ECSqlColumnInfoCR aliasedColInfo = aliasedVal.GetColumnInfo();

        EXPECT_FALSE(unaliasedColInfo.IsGeneratedProperty());
        EXPECT_TRUE(aliasedColInfo.IsGeneratedProperty());
        EXPECT_STREQ("Schema", unaliasedColInfo.GetPropertyPath().ToString().c_str());
        EXPECT_STREQ("MySchema", aliasedColInfo.GetPropertyPath().ToString().c_str());
        EXPECT_STRNE(unaliasedColInfo.GetRootClass().GetClass().GetFullName(), aliasedColInfo.GetRootClass().GetClass().GetFullName());

        ASSERT_TRUE(unaliasedColInfo.GetProperty() != nullptr);
        ASSERT_TRUE(unaliasedColInfo.GetProperty()->GetIsNavigation());
        ASSERT_TRUE(unaliasedColInfo.GetProperty()->GetAsNavigationProperty()->GetRelationshipClass() != nullptr);

        ECPropertyCP aliasedColInfoProp = aliasedColInfo.GetProperty();
        ASSERT_TRUE(aliasedColInfoProp != nullptr);
        ASSERT_TRUE(aliasedColInfoProp->GetIsNavigation());
        ASSERT_TRUE(aliasedColInfoProp->GetAsNavigationProperty()->GetRelationshipClass() != nullptr);
        EXPECT_STREQ("MySchema", aliasedColInfoProp->GetName().c_str());
        EXPECT_STREQ("MySchema", aliasedColInfoProp->GetDisplayLabel().c_str());
        EXPECT_STREQ(unaliasedColInfo.GetProperty()->GetAsNavigationProperty()->GetRelationshipClass()->GetFullName(), aliasedColInfoProp->GetAsNavigationProperty()->GetRelationshipClass()->GetFullName());
        EXPECT_EQ(unaliasedColInfo.GetProperty()->GetAsNavigationProperty()->GetDirection(), aliasedColInfoProp->GetAsNavigationProperty()->GetDirection());
        }

    ASSERT_GT(rowCount, 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, SelectAsterisk)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("SelectAsterisk.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
                    <ECSchemaReference name="ECDbFileInfo" version="02.00.01" alias="ecdbf" />
                    <ECEntityClass typeName="A">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Name" typeName="string" />
                        <ECProperty propertyName="Size" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="B">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECNavigationProperty propertyName="A" relationshipName="AOwnsB" direction="Backward"/>
                        <ECProperty propertyName="Name" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="SubB">
                        <BaseClass>B</BaseClass>
                        <ECProperty propertyName="SubName" typeName="string" />
                     </ECEntityClass>
                    <ECRelationshipClass typeName="AOwnsB" modifier="Sealed" strength="referencing">
                      <Source multiplicity="(0..1)" roleLabel="is extracted from" polymorphic="false">
                        <Class class="A"/>
                      </Source>
                      <Target multiplicity="(0..*)" roleLabel="refers to" polymorphic="false">
                        <Class class="B"/>
                      </Target>
                    </ECRelationshipClass>
                    <ECRelationshipClass typeName="ALinksB" modifier="Sealed" strength="referencing">
                      <Source multiplicity="(0..*)" roleLabel="is extracted from" polymorphic="false">
                        <Class class="A"/>
                      </Source>
                      <Target multiplicity="(0..*)" roleLabel="refers to" polymorphic="false">
                        <Class class="B"/>
                      </Target>
                    </ECRelationshipClass>
                    <ECRelationshipClass typeName="SubBLinksFileInfo" modifier="Sealed" strength="referencing">
                      <Source multiplicity="(0..*)" roleLabel="is extracted from" polymorphic="false">
                        <Class class="SubB"/>
                      </Source>
                      <Target multiplicity="(0..*)" roleLabel="refers to" polymorphic="false">
                        <Class class="ecdbf:ExternalFileInfo"/>
                      </Target>
                      <ECProperty propertyName="Priority" typeName="int" />
                    </ECRelationshipClass>
                </ECSchema>)xml")));

    ECInstanceKey aKey, bKey, subBKey, fileInfoKey, aLinksBKey, subBLinksFileInfoKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(aKey, "INSERT INTO ts.A(Name,Size) VALUES('A-1',100)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(bKey, Utf8PrintfString("INSERT INTO ts.B(Name,A.Id) VALUES('B-1',%" PRIu64 ")", aKey.GetInstanceId().GetValue()).c_str()));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(subBKey, Utf8PrintfString("INSERT INTO ts.SubB(Name,SubName,A.Id) VALUES('B-2','Sub 1',%" PRIu64 ")", aKey.GetInstanceId().GetValue()).c_str()));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(fileInfoKey, "INSERT INTO ecdbf.ExternalFileInfo(Name) VALUES('DataFile-1')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(aLinksBKey, Utf8PrintfString("INSERT INTO ts.ALinksB(SourceECInstanceId,TargetECInstanceId) VALUES(%" PRIu64 ",%" PRIu64 ")",
                                                                                          aKey.GetInstanceId().GetValue(), bKey.GetInstanceId().GetValue()).c_str()));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(subBLinksFileInfoKey, Utf8PrintfString("INSERT INTO ts.SubBLinksFileInfo(SourceECInstanceId,TargetECInstanceId, Priority) VALUES(%" PRIu64 ",%" PRIu64 ", 400)",
                                                                                          subBKey.GetInstanceId().GetValue(), fileInfoKey.GetInstanceId().GetValue()).c_str()));

    auto retrieveRow = [] (ECSqlStatement const& stmt)
        {
        JsonValue json;
        for (int i = 0; i < stmt.GetColumnCount(); i++)
            {
            if (stmt.IsValueNull(i))
                continue;

            ECSqlColumnInfo const& colInfo = stmt.GetColumnInfo(i);
            Utf8String colName = colInfo.GetPropertyPath().ToString();

            if (colInfo.GetDataType().IsNavigation())
                json.m_value[colName.c_str()] = (uint32_t) stmt.GetValueNavigation<ECInstanceId>(i).GetValue();
            if (colInfo.GetDataType() == PRIMITIVETYPE_Integer || colInfo.GetDataType() == PRIMITIVETYPE_Long)
                json.m_value[colName.c_str()] = stmt.GetValueInt64(i);
            else if (colInfo.GetDataType() == PRIMITIVETYPE_String)
                json.m_value[colName.c_str()] = stmt.GetValueText(i);
            }

        return json;
        };

    ECClassId aOwnsBClassId = m_ecdb.Schemas().GetClassId("TestSchema", "AOwnsB");
    ASSERT_TRUE(aOwnsBClassId.IsValid());

    ECSqlStatement stmt;
    for (Utf8CP ecsql : std::vector<Utf8CP> {"SELECT * FROM ts.A", "SELECT * FROM (SELECT * FROM ts.A)"})
        {
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql)) << ecsql;
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
        EXPECT_EQ(4, stmt.GetColumnCount()) << stmt.GetECSql();
        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json({"ECInstanceId":%s, "ECClassId":%s, "Name":"A-1", "Size": 100})json", aKey.GetInstanceId().ToString().c_str(),
                                             aKey.GetClassId().ToString().c_str())), retrieveRow(stmt)) << stmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
        stmt.Finalize();
        }

    for (Utf8CP ecsql : std::vector<Utf8CP> {"SELECT * FROM ts.B ORDER BY Name", "SELECT * FROM (SELECT * FROM ts.B) ORDER BY Name"})
        {
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql)) << ecsql;
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
        EXPECT_EQ(4, stmt.GetColumnCount()) << stmt.GetECSql();
        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json({"ECInstanceId":%s, "ECClassId":%s, "Name":"B-1", "A":%s})json",
                                             bKey.GetInstanceId().ToString().c_str(),
                                             bKey.GetClassId().ToString().c_str(),
                                             aKey.GetInstanceId().ToString().c_str())), retrieveRow(stmt)) << stmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json({"ECInstanceId":%s, "ECClassId":%s, "Name":"B-2", "A":%s})json",
                                             subBKey.GetInstanceId().ToString().c_str(),
                                             subBKey.GetClassId().ToString().c_str(),
                                             aKey.GetInstanceId().ToString().c_str())), retrieveRow(stmt)) << stmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
        stmt.Finalize();
        }

    for (Utf8CP ecsql : std::vector<Utf8CP> {"SELECT * FROM ts.AOwnsB ORDER BY SourceECClassId, TargetECClassId", "SELECT * FROM (SELECT * FROM ts.AOwnsB ORDER BY SourceECClassId, TargetECClassId)"})
        {
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql)) << ecsql;
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
        EXPECT_EQ(6, stmt.GetColumnCount()) << stmt.GetECSql();
        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json({"ECInstanceId":%s, "ECClassId":%s, "SourceECInstanceId":%s, "TargetECInstanceId":%s, "SourceECClassId":%s, "TargetECClassId":%s})json",
                                             bKey.GetInstanceId().ToString().c_str(), // nav prop rels don't have their own id, ECDb hands out the instance's id holding the nav prop
                                             aOwnsBClassId.ToString().c_str(),
                                             aKey.GetInstanceId().ToString().c_str(),
                                             bKey.GetInstanceId().ToString().c_str(),
                                             aKey.GetClassId().ToString().c_str(),
                                             bKey.GetClassId().ToString().c_str()
        )), retrieveRow(stmt)) << stmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json({"ECInstanceId":%s, "ECClassId":%s, "SourceECInstanceId":%s,"TargetECInstanceId":%s, "SourceECClassId":%s, "TargetECClassId":%s})json",
                                             subBKey.GetInstanceId().ToString().c_str(), // nav prop rels don't have their own id, ECDb hands out the instance's id holding the nav prop
                                             aOwnsBClassId.ToString().c_str(),
                                             aKey.GetInstanceId().ToString().c_str(),
                                             subBKey.GetInstanceId().ToString().c_str(),
                                             aKey.GetClassId().ToString().c_str(),
                                             subBKey.GetClassId().ToString().c_str()
        )), retrieveRow(stmt)) << stmt.GetECSql();

        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
        stmt.Finalize();
        }

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * FROM (SELECT * FROM ts.AOwnsB) ORDER BY SourceECClassId, TargetECClassId"));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * FROM (SELECT SourceECClassId, TargetECClassId FROM ts.AOwnsB ORDER BY TargetECInstanceId) ORDER BY SourceECClassId,TargetECClassId"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    EXPECT_EQ(2, stmt.GetColumnCount()) << stmt.GetECSql();
    EXPECT_EQ(JsonValue(Utf8PrintfString(R"json({"SourceECClassId":%s, "TargetECClassId":%s})json",
                                         aKey.GetClassId().ToString().c_str(),
                                         bKey.GetClassId().ToString().c_str())), retrieveRow(stmt)) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    EXPECT_EQ(JsonValue(Utf8PrintfString(R"json({"SourceECClassId":%s, "TargetECClassId":%s})json",
                                         aKey.GetClassId().ToString().c_str(),
                                         subBKey.GetClassId().ToString().c_str())), retrieveRow(stmt)) << stmt.GetECSql();

    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    stmt.Finalize();


    for (Utf8CP ecsql : std::vector<Utf8CP> {"SELECT * FROM ts.ALinksB", "SELECT * FROM (SELECT * FROM ts.ALinksB)"})
        {
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql)) << ecsql;
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
        EXPECT_EQ(6, stmt.GetColumnCount()) << stmt.GetECSql();
        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json({"ECInstanceId":%s, "ECClassId":%s, "SourceECInstanceId":%s, "TargetECInstanceId":%s, "SourceECClassId":%s, "TargetECClassId":%s})json",
                                             aLinksBKey.GetInstanceId().ToString().c_str(),
                                             aLinksBKey.GetClassId().ToString().c_str(),
                                             aKey.GetInstanceId().ToString().c_str(),
                                             bKey.GetInstanceId().ToString().c_str(),
                                             aKey.GetClassId().ToString().c_str(),
                                             bKey.GetClassId().ToString().c_str()
        )), retrieveRow(stmt)) << stmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
        stmt.Finalize();
        }

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * FROM (SELECT SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM ts.ALinksB) ORDER BY SourceECClassId, TargetECClassId"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    EXPECT_EQ(4, stmt.GetColumnCount()) << stmt.GetECSql();
    EXPECT_EQ(JsonValue(Utf8PrintfString(R"json({"SourceECInstanceId":%s, "SourceECClassId":%s, "TargetECInstanceId":%s, "TargetECClassId":%s})json",
                                            aKey.GetInstanceId().ToString().c_str(),
                                            aKey.GetClassId().ToString().c_str(),
                                            bKey.GetInstanceId().ToString().c_str(),
                                            bKey.GetClassId().ToString().c_str()
    )), retrieveRow(stmt)) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    stmt.Finalize();

    for (Utf8CP ecsql : std::vector<Utf8CP> {"SELECT * FROM ts.SubBLinksFileInfo", "SELECT * FROM (SELECT * FROM ts.SubBLinksFileInfo)"})
        {
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql)) << ecsql;
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
        EXPECT_EQ(7, stmt.GetColumnCount()) << stmt.GetECSql();
        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json({"ECInstanceId":%s, "ECClassId":%s, "SourceECInstanceId":%s, "TargetECInstanceId":%s, "Priority":400, "SourceECClassId":%s, "TargetECClassId":%s})json",
                                             subBLinksFileInfoKey.GetInstanceId().ToString().c_str(),
                                             subBLinksFileInfoKey.GetClassId().ToString().c_str(),
                                             subBKey.GetInstanceId().ToString().c_str(),
                                             fileInfoKey.GetInstanceId().ToString().c_str(),
                                             subBKey.GetClassId().ToString().c_str(),
                                             fileInfoKey.GetClassId().ToString().c_str()
        )), retrieveRow(stmt)) << stmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
        stmt.Finalize();
        }

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * FROM (SELECT SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId, Priority FROM ts.SubBLinksFileInfo) ORDER BY SourceECClassId, TargetECClassId"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    EXPECT_EQ(5, stmt.GetColumnCount()) << stmt.GetECSql();
    EXPECT_EQ(JsonValue(Utf8PrintfString(R"json({"SourceECInstanceId":%s, "SourceECClassId":%s, "TargetECInstanceId":%s, "TargetECClassId":%s, "Priority":400})json",
                                         subBKey.GetInstanceId().ToString().c_str(),
                                         subBKey.GetClassId().ToString().c_str(),
                                         fileInfoKey.GetInstanceId().ToString().c_str(),
                                         fileInfoKey.GetClassId().ToString().c_str()
    )), retrieveRow(stmt)) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    stmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, SelectAsteriskOnRelationshipClass)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("SelectAsteriskOnRelationshipClass.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
                    <ECEntityClass typeName="A">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Name" typeName="string" />
                        <ECProperty propertyName="Size" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="B">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECNavigationProperty propertyName="A" relationshipName="AOwnsB" direction="Backward"/>
                        <ECProperty propertyName="Name" typeName="string" />
                     </ECEntityClass>
                    <ECRelationshipClass typeName="AOwnsB" modifier="Sealed" strength="referencing">
                      <Source multiplicity="(0..1)" roleLabel="is extracted from" polymorphic="false">
                        <Class class="A"/>
                      </Source>
                      <Target multiplicity="(0..*)" roleLabel="refers to" polymorphic="false">
                        <Class class="B"/>
                      </Target>
                    </ECRelationshipClass>
                </ECSchema>)xml")));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * FROM ts.AOwnsB"));
    bool hasSourceECClassId = false, hasTargetECClassId = false;
    const int columnCount = stmt.GetColumnCount();
    for (int i = 0; i < columnCount; i++)
        {
        ECSqlColumnInfo const& colInfo = stmt.GetColumnInfo(i);
        Utf8String colName = colInfo.GetProperty()->GetName().c_str();
        if (colName.CompareTo("SourceECClassId") == 0)
            hasSourceECClassId = true;

        if (colName.CompareTo("TargetECClassId") == 0)
            hasTargetECClassId = true;
        }

    ASSERT_EQ(true, hasSourceECClassId) << "Select * should include SourceECClassId";
    ASSERT_EQ(true, hasTargetECClassId) << "Select * should include TargetECClassId";;
    stmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, SelectAsteriskAndViewGenerator)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("SelectAsterisk.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
                    <ECSchemaReference name="ECDbFileInfo" version="02.00.01" alias="ecdbf" />
                    <ECEntityClass typeName="A">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Name" typeName="string" />
                        <ECProperty propertyName="Size" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="B">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECNavigationProperty propertyName="A" relationshipName="AOwnsB" direction="Backward"/>
                        <ECProperty propertyName="Name" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="SubB">
                        <BaseClass>B</BaseClass>
                        <ECProperty propertyName="SubName" typeName="string" />
                     </ECEntityClass>
                    <ECRelationshipClass typeName="AOwnsB" modifier="Sealed" strength="referencing">
                      <Source multiplicity="(0..1)" roleLabel="is extracted from" polymorphic="false">
                        <Class class="A"/>
                      </Source>
                      <Target multiplicity="(0..*)" roleLabel="refers to" polymorphic="false">
                        <Class class="B"/>
                      </Target>
                    </ECRelationshipClass>
                    <ECRelationshipClass typeName="ALinksB" modifier="Sealed" strength="referencing">
                      <Source multiplicity="(0..*)" roleLabel="is extracted from" polymorphic="false">
                        <Class class="A"/>
                      </Source>
                      <Target multiplicity="(0..*)" roleLabel="refers to" polymorphic="false">
                        <Class class="B"/>
                      </Target>
                    </ECRelationshipClass>
                    <ECRelationshipClass typeName="SubBLinksFileInfo" modifier="Sealed" strength="referencing">
                      <Source multiplicity="(0..*)" roleLabel="is extracted from" polymorphic="false">
                        <Class class="SubB"/>
                      </Source>
                      <Target multiplicity="(0..*)" roleLabel="refers to" polymorphic="false">
                        <Class class="ecdbf:ExternalFileInfo"/>
                      </Target>
                      <ECProperty propertyName="Priority" typeName="int" />
                    </ECRelationshipClass>
                </ECSchema>)xml")));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * FROM ts.AOwnsB"));
    EXPECT_STRCASEEQ("SELECT [AOwnsB].[ECInstanceId],[AOwnsB].[ECClassId],[AOwnsB].[SourceECInstanceId],[AOwnsB].[SourceECClassId],[AOwnsB].[TargetECInstanceId],[AOwnsB].[TargetECClassId] FROM "
        "(SELECT [ts_B].[Id] ECInstanceId,76 ECClassId,[ts_B].[AId] SourceECInstanceId,[ts_A].[ECClassId] SourceECClassId,[ts_B].[Id] TargetECInstanceId,[ts_B].[ECClassId] TargetECClassId FROM [main].[ts_B] "
        "INNER JOIN [main].[ts_A] ON [ts_A].[Id]=[ts_B].[AId] WHERE [ts_B].[AId] IS NOT NULL) [AOwnsB]", stmt.GetNativeSql()) << stmt.GetECSql();
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * FROM ts.AOwnsB WHERE SourceECClassId=?"));
    EXPECT_STRCASEEQ("SELECT [AOwnsB].[ECInstanceId],[AOwnsB].[ECClassId],[AOwnsB].[SourceECInstanceId],[AOwnsB].[SourceECClassId],[AOwnsB].[TargetECInstanceId],[AOwnsB].[TargetECClassId] FROM "
        "(SELECT [ts_B].[Id] ECInstanceId,76 ECClassId,[ts_B].[AId] SourceECInstanceId,[ts_A].[ECClassId] SourceECClassId,[ts_B].[Id] TargetECInstanceId,[ts_B].[ECClassId] TargetECClassId FROM [main].[ts_B] "
        "INNER JOIN [main].[ts_A] ON [ts_A].[Id]=[ts_B].[AId] WHERE [ts_B].[AId] IS NOT NULL) [AOwnsB] "
        "WHERE [AOwnsB].[SourceECClassId]=:_ecdb_sqlparam_ix1_col1", stmt.GetNativeSql()) << stmt.GetECSql();
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT SourceECInstanceId,SourceECClassId FROM ts.AOwnsB"));
    EXPECT_STRCASEEQ("SELECT [AOwnsB].[SourceECInstanceId],[AOwnsB].[SourceECClassId] FROM "
                     "(SELECT [ts_B].[Id] ECInstanceId,76 ECClassId,[ts_B].[AId] SourceECInstanceId,[ts_A].[ECClassId] SourceECClassId,[ts_B].[Id] TargetECInstanceId,[ts_B].[ECClassId] TargetECClassId FROM [main].[ts_B] "
                     "INNER JOIN [main].[ts_A] ON [ts_A].[Id]=[ts_B].[AId] WHERE [ts_B].[AId] IS NOT NULL) [AOwnsB]", stmt.GetNativeSql()) << stmt.GetECSql();
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT TargetECInstanceId,TargetECClassId FROM ts.AOwnsB"));
    EXPECT_STRCASEEQ("SELECT [AOwnsB].[TargetECInstanceId],[AOwnsB].[TargetECClassId] FROM "
                     "(SELECT [ts_B].[Id] ECInstanceId,76 ECClassId,[ts_B].[AId] SourceECInstanceId,[ts_A].[ECClassId] SourceECClassId,[ts_B].[Id] TargetECInstanceId,[ts_B].[ECClassId] TargetECClassId FROM [main].[ts_B] "
                     "INNER JOIN [main].[ts_A] ON [ts_A].[Id]=[ts_B].[AId] WHERE [ts_B].[AId] IS NOT NULL) [AOwnsB]", stmt.GetNativeSql()) << stmt.GetECSql();
    stmt.Finalize();


    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * FROM ts.ALinksB"));
    EXPECT_STRCASEEQ("SELECT [ALinksB].[ECInstanceId],[ALinksB].[ECClassId],[ALinksB].[SourceECInstanceId],[ALinksB].[SourceECClassId],[ALinksB].[TargetECInstanceId],[ALinksB].[TargetECClassId] FROM "
        "(SELECT [ts_ALinksB].[Id] [ECInstanceId],74 [ECClassId],[ts_ALinksB].[SourceId] [SourceECInstanceId],[SourceECClassPrimaryTable].[ECClassId] SourceECClassId,[ts_ALinksB].[TargetId] [TargetECInstanceId],[TargetECClassPrimaryTable].[ECClassId] TargetECClassId FROM [main].[ts_ALinksB] "
        "INNER JOIN [main].[ts_A] [SourceECClassPrimaryTable] ON [SourceECClassPrimaryTable].[Id]=[ts_ALinksB].[SourceId] "
        "INNER JOIN [main].[ts_B] [TargetECClassPrimaryTable] ON [TargetECClassPrimaryTable].[Id]=[ts_ALinksB].[TargetId]) [ALinksB]", stmt.GetNativeSql()) << stmt.GetECSql();
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * FROM ts.ALinksB WHERE SourceECInstanceId=? AND SourceECClassId=?"));
    EXPECT_STRCASEEQ("SELECT [ALinksB].[ECInstanceId],[ALinksB].[ECClassId],[ALinksB].[SourceECInstanceId],[ALinksB].[SourceECClassId],[ALinksB].[TargetECInstanceId],[ALinksB].[TargetECClassId] FROM "
        "(SELECT [ts_ALinksB].[Id] [ECInstanceId],74 [ECClassId],[ts_ALinksB].[SourceId] [SourceECInstanceId],[SourceECClassPrimaryTable].[ECClassId] SourceECClassId,[ts_ALinksB].[TargetId] [TargetECInstanceId],[TargetECClassPrimaryTable].[ECClassId] TargetECClassId FROM [main].[ts_ALinksB] "
        "INNER JOIN [main].[ts_A] [SourceECClassPrimaryTable] ON [SourceECClassPrimaryTable].[Id]=[ts_ALinksB].[SourceId] "
        "INNER JOIN [main].[ts_B] [TargetECClassPrimaryTable] ON [TargetECClassPrimaryTable].[Id]=[ts_ALinksB].[TargetId]) [ALinksB] "
        "WHERE [ALinksB].[SourceECInstanceId]=:_ecdb_sqlparam_ix1_col1 AND [ALinksB].[SourceECClassId]=:_ecdb_sqlparam_ix2_col1", stmt.GetNativeSql()) << stmt.GetECSql();
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * FROM ts.ALinksB WHERE SourceECClassId=?"));
    EXPECT_STRCASEEQ("SELECT [ALinksB].[ECInstanceId],[ALinksB].[ECClassId],[ALinksB].[SourceECInstanceId],[ALinksB].[SourceECClassId],[ALinksB].[TargetECInstanceId],[ALinksB].[TargetECClassId] FROM "
                     "(SELECT [ts_ALinksB].[Id] [ECInstanceId],74 [ECClassId],[ts_ALinksB].[SourceId] [SourceECInstanceId],[SourceECClassPrimaryTable].[ECClassId] SourceECClassId,[ts_ALinksB].[TargetId] [TargetECInstanceId],[TargetECClassPrimaryTable].[ECClassId] TargetECClassId FROM [main].[ts_ALinksB] "
                     "INNER JOIN [main].[ts_A] [SourceECClassPrimaryTable] ON [SourceECClassPrimaryTable].[Id]=[ts_ALinksB].[SourceId] "
                     "INNER JOIN [main].[ts_B] [TargetECClassPrimaryTable] ON [TargetECClassPrimaryTable].[Id]=[ts_ALinksB].[TargetId]) [ALinksB] "
                     "WHERE [ALinksB].[SourceECClassId]=:_ecdb_sqlparam_ix1_col1", stmt.GetNativeSql()) << stmt.GetECSql();
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * FROM ts.ALinksB WHERE TargetECInstanceId=? AND TargetECClassId=?"));
    EXPECT_STRCASEEQ("SELECT [ALinksB].[ECInstanceId],[ALinksB].[ECClassId],[ALinksB].[SourceECInstanceId],[ALinksB].[SourceECClassId],[ALinksB].[TargetECInstanceId],[ALinksB].[TargetECClassId] FROM "
                     "(SELECT [ts_ALinksB].[Id] [ECInstanceId],74 [ECClassId],[ts_ALinksB].[SourceId] [SourceECInstanceId],[SourceECClassPrimaryTable].[ECClassId] SourceECClassId,[ts_ALinksB].[TargetId] [TargetECInstanceId],[TargetECClassPrimaryTable].[ECClassId] TargetECClassId FROM [main].[ts_ALinksB] "
                     "INNER JOIN [main].[ts_A] [SourceECClassPrimaryTable] ON [SourceECClassPrimaryTable].[Id]=[ts_ALinksB].[SourceId] "
                     "INNER JOIN [main].[ts_B] [TargetECClassPrimaryTable] ON [TargetECClassPrimaryTable].[Id]=[ts_ALinksB].[TargetId]) [ALinksB] "
                     "WHERE [ALinksB].[TargetECInstanceId]=:_ecdb_sqlparam_ix1_col1 AND [ALinksB].[TargetECClassId]=:_ecdb_sqlparam_ix2_col1", stmt.GetNativeSql()) << stmt.GetECSql();
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * FROM ts.ALinksB WHERE TargetECClassId=?"));
    EXPECT_STRCASEEQ("SELECT [ALinksB].[ECInstanceId],[ALinksB].[ECClassId],[ALinksB].[SourceECInstanceId],[ALinksB].[SourceECClassId],[ALinksB].[TargetECInstanceId],[ALinksB].[TargetECClassId] FROM "
                     "(SELECT [ts_ALinksB].[Id] [ECInstanceId],74 [ECClassId],[ts_ALinksB].[SourceId] [SourceECInstanceId],[SourceECClassPrimaryTable].[ECClassId] SourceECClassId,[ts_ALinksB].[TargetId] [TargetECInstanceId],[TargetECClassPrimaryTable].[ECClassId] TargetECClassId FROM [main].[ts_ALinksB] "
                     "INNER JOIN [main].[ts_A] [SourceECClassPrimaryTable] ON [SourceECClassPrimaryTable].[Id]=[ts_ALinksB].[SourceId] "
                     "INNER JOIN [main].[ts_B] [TargetECClassPrimaryTable] ON [TargetECClassPrimaryTable].[Id]=[ts_ALinksB].[TargetId]) [ALinksB] "
                     "WHERE [ALinksB].[TargetECClassId]=:_ecdb_sqlparam_ix1_col1", stmt.GetNativeSql()) << stmt.GetECSql();
    stmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, InsertWithoutPropClause)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("InsertWithoutPropClause.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
                    <ECSchemaReference name="ECDbFileInfo" version="02.00.01" alias="ecdbf" />
                    <ECEntityClass typeName="A">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Name" typeName="string" />
                        <ECProperty propertyName="Size" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="B">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECNavigationProperty propertyName="A" relationshipName="AOwnsB" direction="Backward"/>
                        <ECProperty propertyName="Name" typeName="string" />
                     </ECEntityClass>
                    <ECEntityClass typeName="SubB">
                        <BaseClass>B</BaseClass>
                        <ECProperty propertyName="SubName" typeName="string" />
                     </ECEntityClass>
                    <ECRelationshipClass typeName="AOwnsB" modifier="Sealed" strength="referencing">
                      <Source multiplicity="(0..1)" roleLabel="is extracted from" polymorphic="false">
                        <Class class="A"/>
                      </Source>
                      <Target multiplicity="(0..*)" roleLabel="refers to" polymorphic="false">
                        <Class class="B"/>
                      </Target>
                    </ECRelationshipClass>
                    <ECRelationshipClass typeName="ALinksB" modifier="Sealed" strength="referencing">
                      <Source multiplicity="(0..*)" roleLabel="is extracted from" polymorphic="false">
                        <Class class="A"/>
                      </Source>
                      <Target multiplicity="(0..*)" roleLabel="refers to" polymorphic="false">
                        <Class class="B"/>
                      </Target>
                    </ECRelationshipClass>
                    <ECRelationshipClass typeName="SubBLinksFileInfo" modifier="Sealed" strength="referencing">
                      <Source multiplicity="(0..*)" roleLabel="is extracted from" polymorphic="false">
                        <Class class="SubB"/>
                      </Source>
                      <Target multiplicity="(0..*)" roleLabel="refers to" polymorphic="false">
                        <Class class="ecdbf:ExternalFileInfo"/>
                      </Target>
                      <ECProperty propertyName="Priority" typeName="int" />
                    </ECRelationshipClass>
                </ECSchema>)xml")));


    ECInstanceKey aKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(aKey, "INSERT INTO ts.A VALUES('A-1',100)"));

    // verify
    EXPECT_EQ(BE_SQLITE_ROW, GetHelper().ExecuteECSql(Utf8PrintfString("SELECT 1 FROM ts.A WHERE ECInstanceId=%" PRIu64 " AND ECClassId=%" PRIu64 " AND Name='A-1' AND Size=100",
                                                                              aKey.GetInstanceId().GetValue(), aKey.GetClassId().GetValue()).c_str()));

    ECInstanceKey bKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.B VALUES(?,'B-1')"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(1, aKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(bKey));
    // verify
    EXPECT_EQ(BE_SQLITE_ROW, GetHelper().ExecuteECSql(Utf8PrintfString("SELECT 1 FROM ts.B WHERE ECInstanceId=%" PRIu64 " AND ECClassId=%" PRIu64 " AND A.Id=%" PRIu64 " AND Name='B-1'",
                                                                       bKey.GetInstanceId().GetValue(), bKey.GetClassId().GetValue(), aKey.GetInstanceId().GetValue()).c_str()));
    }

    ECInstanceKey subBKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.SubB VALUES(?,'B-2','Sub 1')"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(1, aKey.GetInstanceId()));

    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(subBKey));
    // verify
    EXPECT_EQ(BE_SQLITE_ROW, GetHelper().ExecuteECSql(Utf8PrintfString("SELECT 1 FROM ts.SubB WHERE ECInstanceId=%" PRIu64 " AND ECClassId=%" PRIu64 " AND A.Id=%" PRIu64 " AND Name='B-2' AND SubName='Sub 1'",
                                                                       subBKey.GetInstanceId().GetValue(), subBKey.GetClassId().GetValue(), aKey.GetInstanceId().GetValue()).c_str()));
    }

    ECInstanceKey fileInfoKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(fileInfoKey, "INSERT INTO ecdbf.ExternalFileInfo VALUES('DataFile-1',1000,'A test file',TIMESTAMP '2018-11-23T00:00Z',ecdbf.StandardRootFolderType.TemporaryFolder,'files/large')"));
    // verify
    EXPECT_EQ(BE_SQLITE_ROW, GetHelper().ExecuteECSql(Utf8PrintfString("SELECT 1 FROM ecdbf.ExternalFileInfo WHERE ECInstanceId=%" PRIu64 " AND ECClassId=%" PRIu64 " AND Name='DataFile-1' AND Size=1000 AND Description='A test file' AND LastModified=TIMESTAMP '2018-11-23T00:00Z' AND RootFolder=ecdbf.StandardRootFolderType.TemporaryFolder AND RelativePath='files/large'",
                                                                              fileInfoKey.GetInstanceId().GetValue(), fileInfoKey.GetClassId().GetValue()).c_str()));

    {
    ECSqlStatement stmt;

    // SourceECClassId/TargetECClassId is skipped
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "INSERT INTO ts.ALinksB VALUES(?,-1,?,-1)"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "INSERT INTO ts.ALinksB VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "INSERT INTO ts.ALinksB VALUES(?,?,?)"));

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.ALinksB VALUES(?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, aKey.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, bKey.GetInstanceId()));

    ECInstanceKey aLinksBKey;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(aLinksBKey));
    // verify
    EXPECT_EQ(BE_SQLITE_ROW, GetHelper().ExecuteECSql(Utf8PrintfString("SELECT 1 FROM ts.ALinksB WHERE ECInstanceId=%" PRIu64 " AND ECClassId=%" PRIu64 " AND SourceECInstanceId=%" PRIu64 " AND SourceECClassId=%" PRIu64 " AND TargetECInstanceId=%" PRIu64 " AND TargetECClassId=%" PRIu64,
                                                                       aLinksBKey.GetInstanceId().GetValue(), aLinksBKey.GetClassId().GetValue(),
                                                                       aKey.GetInstanceId().GetValue(), aKey.GetClassId().GetValue(),
                                                                       bKey.GetInstanceId().GetValue(), bKey.GetClassId().GetValue()).c_str()));
    }

    {
    ECSqlStatement stmt;

    // SourceECClassId/TargetECClassId is skipped
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "INSERT INTO ts.SubBLinksFileInfo VALUES(400,?,-1,?,-1)"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "INSERT INTO ts.SubBLinksFileInfo VALUES(400,?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "INSERT INTO ts.SubBLinksFileInfo VALUES(400,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "INSERT INTO ts.SubBLinksFileInfo VALUES(?,?,?,?)"));

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.SubBLinksFileInfo VALUES(?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, 400)) << "Priority is expected to be the first prop";
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, subBKey.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, fileInfoKey.GetInstanceId()));
    ECInstanceKey subBLinksFileInfoKey;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(subBLinksFileInfoKey));
    // verify
    EXPECT_EQ(BE_SQLITE_ROW, GetHelper().ExecuteECSql(Utf8PrintfString("SELECT 1 FROM ts.SubBLinksFileInfo WHERE ECInstanceId=%" PRIu64 " AND ECClassId=%" PRIu64 " AND SourceECInstanceId=%" PRIu64 " AND SourceECClassId=%" PRIu64 " AND TargetECInstanceId=%" PRIu64 " AND TargetECClassId=%" PRIu64 " AND Priority=400",
                                                                       subBLinksFileInfoKey.GetInstanceId().GetValue(), subBLinksFileInfoKey.GetClassId().GetValue(),
                                                                       subBKey.GetInstanceId().GetValue(), subBKey.GetClassId().GetValue(),
                                                                       fileInfoKey.GetInstanceId().GetValue(), fileInfoKey.GetClassId().GetValue()).c_str()));
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, ClassAliases)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("classaliases.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECEntityClass typeName="A">
                        <ECProperty propertyName="Name" typeName="string" />
                        <ECProperty propertyName="Size" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="B">
                        <ECProperty propertyName="Name" typeName="string" />
                        <ECProperty propertyName="A" typeName="int" />
                     </ECEntityClass>
                </ECSchema>)xml")));

    ECInstanceKey a1Key, a2Key, b1Key, b2Key;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(a1Key, "INSERT INTO ts.A(Name,Size) VALUES('A1', 10)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(a2Key, "INSERT INTO ts.A(Name,Size) VALUES('A2', 20)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(b1Key, "INSERT INTO ts.B(Name,A) VALUES('B1', 11)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(b2Key, "INSERT INTO ts.B(Name,A) VALUES('B2', 22)"));
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT a1.Name, ts.A.Name FROM ts.A a1, ts.A WHERE a1.Size=20 AND ts.A.Size=10 ORDER BY a1.Name, ts.A.Name"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_STREQ("A2", stmt.GetValueText(0)) << stmt.GetECSql();
    ASSERT_STREQ("A1", stmt.GetValueText(1)) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT DISTINCT A.Name FROM ts.A C, ts.B A ORDER BY A.Name"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_STREQ("B1", stmt.GetValueText(0)) << "A is expected to refer to alias A. ECSQL: " << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_STREQ("B2", stmt.GetValueText(0)) << "A is expected to refer to alias A. ECSQL: " << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT DISTINCT A.Name FROM ts.A B, ts.B A ORDER BY A.Name"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_STREQ("B1", stmt.GetValueText(0)) << "A is expected to refer to alias A. ECSQL: " << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_STREQ("B2", stmt.GetValueText(0)) << "A is expected to refer to alias A. ECSQL: " << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }


    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT DISTINCT A FROM ts.A C, ts.B A ORDER BY A"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(11, stmt.GetValueInt(0)) << "A is expected to refer to Property A in class B. ECSQL: " << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(22, stmt.GetValueInt(0)) << "A is expected to refer to Property A in class B. ECSQL: " << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT DISTINCT Size FROM ts.A C, ts.B A ORDER BY Size"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(10, stmt.GetValueInt(0)) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(20, stmt.GetValueInt(0)) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT a1.Name, ts.A.Name FROM ts.A a1, ts.A WHERE a1.Size=20 AND A.Size=10 ORDER BY a1.Name, A.Name"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_STREQ("A2", stmt.GetValueText(0)) << stmt.GetECSql();
    ASSERT_STREQ("A1", stmt.GetValueText(1)) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, UnionTests)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ECSqlStatementTests.ecdb", SchemaItem::CreateForFile("ECSqlStatementTests.01.00.00.ecschema.xml")));
    NestedStructArrayTestSchemaHelper::PopulateECSqlStatementTestsDb(m_ecdb);

    struct PowSqlFunction final : ScalarFunction
        {
        private:

            void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
                {
                if (args[0].IsNull() || args[1].IsNull())
                    {
                    ctx.SetResultError("Arguments to POW must not be NULL", -1);
                    return;
                    }

                double base = args[0].GetValueDouble();
                double exp = args[1].GetValueDouble();

                double res = std::pow(base, exp);
                ctx.SetResultDouble(res);
                }

        public:
            PowSqlFunction() : ScalarFunction("POW2", 2, DbValueType::FloatVal) {}
        };

    int rowCount;
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM (SELECT CompanyName FROM ECST.Supplier UNION ALL SELECT CompanyName FROM ECST.Shipper)"));
    ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);
    int count = stmt.GetValueInt(0);
    EXPECT_EQ(6, count);
    stmt.Finalize();

    //Select Statement containing Union All Clause and also Order By clause
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT CompanyName, Phone FROM ECST.Supplier UNION ALL SELECT CompanyName, Phone FROM ECST.Shipper ORDER BY Phone"));
    rowCount = 0;
    Utf8CP expectedContactNames = "ABCD-Rio Grand-GHIJ-Rio Grand-Rue Perisnon-Salguero-";
    Utf8String actualContactNames;
    while (stmt.Step() != BE_SQLITE_DONE)
        {
        actualContactNames.append(stmt.GetValueText(0));
        actualContactNames.append("-");
        rowCount++;
        }
    ASSERT_STREQ(expectedContactNames, actualContactNames.c_str()) << stmt.GetECSql();
    ASSERT_EQ(6, rowCount);
    stmt.Finalize();

    //Select Statement using UNION Clause, so we should get only distinct results
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT City FROM ECST.Supplier UNION SELECT City FROM ECST.Customer ORDER BY City"));
    rowCount = 0;
    Utf8CP expectedCityNames = "ALASKA-AUSTIN-CA-MD-NC-SAN JOSE-";
    Utf8String actualCityNames;
    while (stmt.Step() != BE_SQLITE_DONE)
        {
        actualCityNames.append(stmt.GetValueText(0));
        actualCityNames.append("-");
        rowCount++;
        }
    ASSERT_STREQ(expectedCityNames, actualCityNames.c_str()) << stmt.GetECSql();
    ASSERT_EQ(6, rowCount);
    stmt.Finalize();

    //Select Statement Using UNION ALL Clause so we should get even Duplicate Results
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT City FROM ECST.Supplier UNION ALL SELECT City FROM ECST.Customer ORDER BY City"));
    rowCount = 0;
    expectedCityNames = "ALASKA-AUSTIN-CA-MD-NC-SAN JOSE-SAN JOSE-";
    actualCityNames.clear();
    while (stmt.Step() != BE_SQLITE_DONE)
        {
        actualCityNames.append(stmt.GetValueText(0));
        actualCityNames.append("-");
        rowCount++;
        }
    ASSERT_STREQ(expectedCityNames, actualCityNames.c_str()) << stmt.GetECSql();
    ASSERT_EQ(7, rowCount);
    stmt.Finalize();

    //use Custom Scaler function in union query
    PowSqlFunction func;
    ASSERT_EQ(0, m_ecdb.AddFunction(func));
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "Select POW2(ECInstanceId, 2), ECClassId, ECInstanceId From ECST.Supplier UNION ALL Select POW2(ECInstanceId, 2), ECClassId, ECInstanceId From ECST.Customer ORDER BY ECInstanceId"));
    rowCount = 0;
    while (stmt.Step() != BE_SQLITE_DONE)
        {
        int base = stmt.GetValueInt(2);
        ASSERT_EQ(std::pow(base, 2), stmt.GetValueInt(0));
        rowCount++;
        }
    ASSERT_EQ(7, rowCount);
    stmt.Finalize();

    //use aggregate function in Union Query
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT Count(*), AVG(Phone) FROM (SELECT Phone FROM ECST.Supplier UNION ALL SELECT Phone FROM ECST.Customer)"));
    ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);
    ASSERT_EQ(7, stmt.GetValueInt(0));
    ASSERT_EQ(1400, stmt.GetValueInt(1));
    stmt.Finalize();

    //Use GROUP BY clause in Union Query
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT COUNT(*), Phone FROM (SELECT ECClassId, Phone FROM ECST.Supplier UNION ALL SELECT ECClassId, Phone FROM ECST.Customer) GROUP BY ECClassId ORDER BY Phone"));

    //Get Row one
    ASSERT_TRUE(stmt.Step() == BE_SQLITE_ROW);
    ASSERT_EQ(3, stmt.GetValueInt(0));
    ASSERT_EQ(1100, stmt.GetValueDouble(1));

    //Get Row two
    ASSERT_TRUE(stmt.Step() == BE_SQLITE_ROW);
    ASSERT_EQ(4, stmt.GetValueInt(0));
    ASSERT_EQ(1400, stmt.GetValueDouble(1));

    ASSERT_TRUE(stmt.Step() == BE_SQLITE_DONE);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, ExceptTests)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ECSqlStatementTests.ecdb", SchemaItem::CreateForFile("ECSqlStatementTests.01.00.00.ecschema.xml")));
    NestedStructArrayTestSchemaHelper::PopulateECSqlStatementTestsDb(m_ecdb);

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT CompanyName FROM ECST.Supplier EXCEPT SELECT CompanyName FROM ECST.Shipper"));
    int rowCount = 0;
    Utf8CP expectedContactNames = "ABCD-GHIJ-";
    Utf8String actualContactNames;
    while (stmt.Step() != BE_SQLITE_DONE)
        {
        actualContactNames.append(stmt.GetValueText(0));
        actualContactNames.append("-");
        rowCount++;
        }
    ASSERT_STREQ(expectedContactNames, actualContactNames.c_str()) << stmt.GetECSql();
    ASSERT_EQ(2, rowCount);

    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ContactTitle FROM ECST.Customer EXCEPT SELECT ContactTitle FROM ECST.Supplier"));
    rowCount = 0;
    expectedContactNames = "AM-Adm-SPIELMANN-";
    actualContactNames.clear();
    while (stmt.Step() != BE_SQLITE_DONE)
        {
        actualContactNames.append(stmt.GetValueText(0));
        actualContactNames.append("-");
        rowCount++;
        }
    ASSERT_STREQ(expectedContactNames, actualContactNames.c_str()) << stmt.GetECSql();
    ASSERT_EQ(3, rowCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, IntersectTests)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ECSqlStatementTests.ecdb", SchemaItem::CreateForFile("ECSqlStatementTests.01.00.00.ecschema.xml")));
    NestedStructArrayTestSchemaHelper::PopulateECSqlStatementTestsDb(m_ecdb);

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT CompanyName FROM ECST.Supplier INTERSECT SELECT CompanyName FROM ECST.Shipper ORDER BY CompanyName"));
    int rowCount = 0;
    Utf8CP expectedContactNames = "Rio Grand";
    Utf8String actualContactNames;
    while (stmt.Step() != BE_SQLITE_DONE)
        {
        actualContactNames.append(stmt.GetValueText(0));
        rowCount++;
        }
    ASSERT_STREQ(expectedContactNames, actualContactNames.c_str());
    ASSERT_EQ(1, rowCount);
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ContactTitle FROM ECST.Supplier INTERSECT SELECT ContactTitle FROM ECST.Customer ORDER BY ContactTitle"));
    rowCount = 0;
    expectedContactNames = "Brathion";
    actualContactNames.clear();
    while (stmt.Step() != BE_SQLITE_DONE)
        {
        actualContactNames.append(stmt.GetValueText(0));
        rowCount++;
        }
    ASSERT_STREQ(expectedContactNames, actualContactNames.c_str());
    ASSERT_EQ(1, rowCount);
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, QuoteTest)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("StartupCompany.ecdb", SchemaItem::CreateForFile("StartupCompany.02.00.00.ecschema.xml")));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO stco.ClassWithPrimitiveProperties (stringProp) VALUES('''a''a''')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT stringProp FROM stco.ClassWithPrimitiveProperties WHERE stringProp = '''a''a'''"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE ONLY stco.ClassWithPrimitiveProperties SET stringProp = '''g''''g'''"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT stringProp FROM stco.ClassWithPrimitiveProperties WHERE stringProp = '''g''''g'''"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, IsNull)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlisnull.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
              <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
              <ECStructClass typeName="PrimStruct">
                    <ECProperty propertyName="i" typeName="int" />
                    <ECProperty propertyName="l" typeName="long" />
                    <ECProperty propertyName="p2d" typeName="Point2d" />
                    <ECProperty propertyName="p3d" typeName="Point3d" />
                    <ECProperty propertyName="geom" typeName="Bentley.Geometry.Common.IGeometry" />
              </ECStructClass>
              <ECStructClass typeName="MyStruct">
                    <ECStructProperty propertyName="pstruct" typeName="PrimStruct" />
                    <ECArrayProperty propertyName="l_array" typeName="long" />
                    <ECArrayProperty propertyName="geom_array" typeName="Bentley.Geometry.Common.IGeometry" />
                    <ECStructArrayProperty propertyName="struct_array" typeName="PrimStruct" />
              </ECStructClass>
              <ECEntityClass typeName="Class_NoSharedCols">
                    <ECProperty propertyName="I" typeName="int" />
                    <ECProperty propertyName="L" typeName="long" />
                    <ECProperty propertyName="Geom" typeName="Bentley.Geometry.Common.IGeometry" />
                    <ECProperty propertyName="P2D" typeName="Point2d" />
                    <ECProperty propertyName="P3D" typeName="Point3d" />
                    <ECArrayProperty propertyName="L_Array" typeName="long" />
                    <ECArrayProperty propertyName="Geom_Array" typeName="Bentley.Geometry.Common.IGeometry" />
                    <ECStructProperty propertyName="Struct" typeName="MyStruct" />
                    <ECStructArrayProperty propertyName="Struct_Array" typeName="MyStruct" />
             </ECEntityClass>
             <ECEntityClass typeName="Class_SharedCols_NoOverflow">
                   <ECCustomAttributes>
                        <ClassMap xmlns='ECDbMap.02.00'>
                             <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                        <ShareColumns xmlns='ECDbMap.02.00'>
                              <MaxSharedColumnsBeforeOverflow>100</MaxSharedColumnsBeforeOverflow>
                        </ShareColumns>
                    </ECCustomAttributes>
                    <ECProperty propertyName="I" typeName="int" />
                    <ECProperty propertyName="L" typeName="long" />
                    <ECProperty propertyName="Geom" typeName="Bentley.Geometry.Common.IGeometry" />
                    <ECProperty propertyName="P2D" typeName="Point2d" />
                    <ECProperty propertyName="P3D" typeName="Point3d" />
                    <ECArrayProperty propertyName="L_Array" typeName="long" />
                    <ECArrayProperty propertyName="Geom_Array" typeName="Bentley.Geometry.Common.IGeometry" />
                    <ECStructProperty propertyName="Struct" typeName="MyStruct" />
                    <ECStructArrayProperty propertyName="Struct_Array" typeName="MyStruct" />
             </ECEntityClass>
             <ECEntityClass typeName="Class_SharedCols_Overflow">
                   <ECCustomAttributes>
                        <ClassMap xmlns='ECDbMap.02.00'>
                             <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                        <ShareColumns xmlns='ECDbMap.02.00'>
                        </ShareColumns>
                    </ECCustomAttributes>
                    <ECProperty propertyName="I" typeName="int" />
                    <ECProperty propertyName="L" typeName="long" />
                    <ECProperty propertyName="Geom" typeName="Bentley.Geometry.Common.IGeometry" />
                    <ECProperty propertyName="P2D" typeName="Point2d" />
                    <ECProperty propertyName="P3D" typeName="Point3d" />
                    <ECArrayProperty propertyName="L_Array" typeName="long" />
                    <ECArrayProperty propertyName="Geom_Array" typeName="Bentley.Geometry.Common.IGeometry" />
                    <ECStructProperty propertyName="Struct" typeName="MyStruct" />
                    <ECStructArrayProperty propertyName="Struct_Array" typeName="MyStruct" />
             </ECEntityClass>
        </ECSchema>)xml")));

    std::vector<Utf8CP> testClassNames {"Class_NoSharedCols", "Class_SharedCols_NoOverflow", "Class_SharedCols_Overflow"};


    //*** all values null
    for (Utf8CP testClassName : testClassNames)
        {
        Utf8String ecsql;
        ecsql.Sprintf("INSERT INTO ts.%s(ECInstanceId) VALUES(NULL)", testClassName);
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str();

        ECInstanceKey key;
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << stmt.GetECSql();
        stmt.Finalize();

        ecsql.Sprintf("SELECT Struct, Struct.struct_array, Struct_Array, "
                      "I, Geom, P2D, P2D.X, P2D.Y, P3D, P3D.X, P3D.Y, P3D.Z, "
                      "Struct.pstruct, Struct.pstruct.i, Struct.pstruct.geom, Struct.pstruct.p2d, Struct.pstruct.p2d.X, Struct.pstruct.p2d.Y, "
                      "Struct.pstruct.p3d, Struct.pstruct.p3d.X, Struct.pstruct.p3d.Y, Struct.pstruct.p3d.Z, "
                      "Struct.l_array FROM ts.%s WHERE ECInstanceId=%s",
                      testClassName, key.GetInstanceId().ToString().c_str());

        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str();
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
        for (int i = 0; i < stmt.GetColumnCount(); i++)
            {
            ASSERT_TRUE(stmt.IsValueNull(i)) << "no values bound to " << stmt.GetECSql();
            }

        const int expectedMembersCount = (int) m_ecdb.Schemas().GetClass("TestSchema", "MyStruct")->GetPropertyCount(true);
        IECSqlValue const& structVal = stmt.GetValue(0);
        int actualMembersCount = 0;
        for (IECSqlValue const& memberVal : structVal.GetStructIterable())
            {
            actualMembersCount++;
            ASSERT_TRUE(memberVal.IsNull());
            }
        ASSERT_EQ(expectedMembersCount, actualMembersCount);

        IECSqlValue const& structMemberStructArrayVal = stmt.GetValue(1);
        ASSERT_EQ(0, structMemberStructArrayVal.GetArrayLength());

        IECSqlValue const& structArrayVal = stmt.GetValue(2);
        ASSERT_EQ(0, structArrayVal.GetArrayLength());
        }

    //*** array with two null elements
    for (Utf8CP testClassName : testClassNames)
        {
        Utf8String ecsql;
        ecsql.Sprintf("INSERT INTO ts.%s(L_Array, Struct.l_array, Struct.struct_array, Struct_Array) "
                      "VALUES(?,?,?,?)", testClassName);
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str();

        for (int i = 1; i <= 4; i++)
            {
            IECSqlBinder& arrayBinder = stmt.GetBinder(i);
            ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindNull());
            ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindNull());
            }

        ECInstanceKey key;
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << stmt.GetECSql();
        stmt.Finalize();

        ecsql.Sprintf("SELECT L_Array, Struct.l_array, Struct.struct_array, Struct_Array FROM ts.%s WHERE ECInstanceId=%s",
                      testClassName, key.GetInstanceId().ToString().c_str());

        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str();
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
        for (int i = 0; i < stmt.GetColumnCount(); i++)
            {
            IECSqlValue const& val = stmt.GetValue(i);
            ASSERT_FALSE(val.IsNull()) << i << " " << stmt.GetECSql();
            ASSERT_EQ(2, val.GetArrayLength());
            for (IECSqlValue const& elementVal : val.GetArrayIterable())
                {
                ASSERT_TRUE(elementVal.IsNull()) << i << " " << stmt.GetECSql();

                if (val.GetColumnInfo().GetDataType().IsStructArray())
                    {
                    for (IECSqlValue const& memberVal : elementVal.GetStructIterable())
                        {
                        ASSERT_TRUE(memberVal.IsNull());
                        }
                    }
                }
            }
        }

    //*** nested struct array being null
    for (Utf8CP testClassName : testClassNames)
        {
        Utf8String ecsql;
        ecsql.Sprintf("INSERT INTO ts.%s(Struct_Array) VALUES(?)", testClassName);
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str();

        IECSqlBinder& elementBinder = stmt.GetBinder(1).AddArrayElement();
        ASSERT_EQ(ECSqlStatus::Success, elementBinder["struct_array"].BindNull());

        ECInstanceKey key;
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << stmt.GetECSql();
        stmt.Finalize();

        ecsql.Sprintf("SELECT Struct_Array FROM ts.%s WHERE ECInstanceId=%s",
                      testClassName, key.GetInstanceId().ToString().c_str());

        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str();
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();

        ASSERT_FALSE(stmt.IsValueNull(0));
        IECSqlValue const& outerArrayVal = stmt.GetValue(0);
        ASSERT_EQ(1, outerArrayVal.GetArrayLength());
        IECSqlValue const& outerArrayElementVal = *outerArrayVal.GetArrayIterable().begin();
        ASSERT_TRUE(outerArrayElementVal.IsNull()) << stmt.GetECSql();
        int memberCount = 0;
        for (IECSqlValue const& memberVal : outerArrayElementVal.GetStructIterable())
            {
            memberCount++;
            ASSERT_TRUE(memberVal.IsNull());
            }
        ASSERT_EQ((int) outerArrayElementVal.GetColumnInfo().GetStructType()->GetPropertyCount(true), memberCount) << "Test class: " << testClassName << " struct: " << outerArrayElementVal.GetColumnInfo().GetStructType()->GetName().c_str();
        }

    //*** nested struct being partially set
    for (Utf8CP testClassName : testClassNames)
        {
        Utf8String ecsql;
        ecsql.Sprintf("INSERT INTO ts.%s(Struct_Array) VALUES(?)", testClassName);
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str();

        IECSqlBinder& arrayElementBinder = stmt.GetBinder(1).AddArrayElement();
        //Set StructArray[0].struct_array[0].i=3
        IECSqlBinder& nestedArrayElementBinder = arrayElementBinder["struct_array"].AddArrayElement();
        ASSERT_EQ(ECSqlStatus::Success, nestedArrayElementBinder["i"].BindInt(3));

        ECInstanceKey key;
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << stmt.GetECSql();
        stmt.Finalize();

        ecsql.Sprintf("SELECT Struct_Array FROM ts.%s WHERE ECInstanceId=%s",
                      testClassName, key.GetInstanceId().ToString().c_str());

        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str();
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();

        ASSERT_FALSE(stmt.IsValueNull(0));
        IECSqlValue const& arrayVal = stmt.GetValue(0);
        ASSERT_EQ(1, arrayVal.GetArrayLength());
        IECSqlValue const& structArrayElementVal = *arrayVal.GetArrayIterable().begin();
        ASSERT_FALSE(structArrayElementVal.IsNull()) << stmt.GetECSql();
        for (IECSqlValue const& memberVal : structArrayElementVal.GetStructIterable())
            {
            if (memberVal.GetColumnInfo().GetProperty()->GetName().Equals("struct_array"))
                {
                ASSERT_EQ(1, memberVal.GetArrayLength()) << "struct_array";
                int memberCount = 0;
                IECSqlValue const& nestedStructVal = *memberVal.GetArrayIterable().begin();
                for (IECSqlValue const& nestedMemberVal : nestedStructVal.GetStructIterable())
                    {
                    memberCount++;
                    if (nestedMemberVal.GetColumnInfo().GetProperty()->GetName().Equals("i"))
                        ASSERT_EQ(3, nestedMemberVal.GetInt());
                    else
                        ASSERT_TRUE(nestedMemberVal.IsNull());
                    }
                ASSERT_EQ((int) nestedStructVal.GetColumnInfo().GetStructType()->GetPropertyCount(true), memberCount);
                }
            else
                ASSERT_TRUE(memberVal.IsNull());

            }
        }

    // points partially null
    for (Utf8CP testClassName : testClassNames)
        {
        Utf8String ecsql;
        ecsql.Sprintf("INSERT INTO ts.%s(P2D.X, P3D.Y, Struct.pstruct.p2d.X, Struct.pstruct.p3d.Y) VALUES(?,?,?,?)", testClassName);
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str();

        ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(1, 3.14)) << ecsql.c_str();
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(2, 3.14)) << ecsql.c_str();
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(3, 3.14)) << ecsql.c_str();
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(4, 3.14)) << ecsql.c_str();

        ECInstanceKey key;
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << stmt.GetECSql();
        stmt.Finalize();

        ecsql.Sprintf("SELECT P2D, P2D.X, P2D.Y, P3D.X, P3D.Y, P3D.Z, Struct.pstruct.p2d.X, Struct.pstruct.p2d.Y, Struct.pstruct.p3d.X, Struct.pstruct.p3d.Y, Struct.pstruct.p3d.Z FROM ts.%s WHERE ECInstanceId=%s",
                      testClassName, key.GetInstanceId().ToString().c_str());

        std::set<Utf8String> notNullItems {"P2D", "P2D.X", "P3D.Y", "Struct.pstruct.p2d.X", "Struct.pstruct.p3d.Y"};
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str();
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
        for (int i = 0; i < stmt.GetColumnCount(); i++)
            {
            IECSqlValue const& val = stmt.GetValue(i);
            Utf8String propPath = val.GetColumnInfo().GetPropertyPath().ToString();
            const bool expectedToBeNull = notNullItems.find(propPath) == notNullItems.end();
            ASSERT_EQ(expectedToBeNull, val.IsNull()) << "Select clause item " << i << " in " << stmt.GetECSql();
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, pragma_ecdb_version)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("pragma.ecdb"));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "PRAGMA ecdb_ver"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_STREQ(stmt.GetValueText(0), m_ecdb.GetECDbProfileVersion().ToString().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, IsNullForIncompletePoints)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("IsNullForIncompletePoints.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'> "
        "    <ECEntityClass typeName='Element' modifier='None'>"
        "        <ECProperty propertyName='Code' typeName='string'/>"
        "        <ECProperty propertyName='P2D' typeName='point2d'/>"
        "        <ECProperty propertyName='P3D' typeName='point3d'/>"
        "    </ECEntityClass>"
        "</ECSchema>")));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Element (P2D.X, Code) VALUES (21.5,'C1')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT P2D, P3D FROM ts.Element WHERE Code='C1'"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    IECSqlValue const& point2D = stmt.GetValue(0);
    IECSqlValue const& point3D = stmt.GetValue(1);

    //IsNull only returns true if all coordinates cols are NULL.
    ASSERT_FALSE(point2D.IsNull());
    ASSERT_TRUE(point3D.IsNull());
    }
//-----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, SubQueryAliasCheck)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("TopLevelAliasCheck.ecdb", SchemaItem(R"(
    <ECSchema schemaName="Test" alias="test" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECStructClass typeName="MyStruct" modifier="Sealed">
            <ECProperty propertyName="Bl"   typeName="binary"/>
            <ECProperty propertyName="Bo"   typeName="boolean"/>
            <ECProperty propertyName="D"    typeName="double"/>
            <ECProperty propertyName="Dt"   typeName="dateTime"/>
            <ECProperty propertyName="G"    typeName="Bentley.Geometry.Common.IGeometry"/>
            <ECProperty propertyName="I"    typeName="int"/>
            <ECProperty propertyName="L"    typeName="long"/>
            <ECProperty propertyName="P2d"  typeName="Point2d"/>
            <ECProperty propertyName="P3d"  typeName="Point3d"/>
            <ECProperty propertyName="S"    typeName="string"/>
        </ECStructClass>
        <ECEntityClass typeName="Foo" modifier="Sealed">
            <ECProperty propertyName="Bl"   typeName="binary"/>
            <ECProperty propertyName="Bo"   typeName="boolean"/>
            <ECProperty propertyName="D"    typeName="double"/>
            <ECProperty propertyName="Dt"   typeName="dateTime"/>
            <ECProperty propertyName="G"    typeName="Bentley.Geometry.Common.IGeometry"/>
            <ECProperty propertyName="I"    typeName="int"/>
            <ECProperty propertyName="L"    typeName="long"/>
            <ECProperty propertyName="P2d"  typeName="Point2d"/>
            <ECProperty propertyName="P3d"  typeName="Point3d"/>
            <ECProperty propertyName="S"    typeName="string"/>
            <ECStructProperty propertyName="Struct" typeName="MyStruct"/>
        </ECEntityClass>
    </ECSchema>
    )")));

    auto prepare = [&](Utf8CP ecsql, std::function<void(ECSqlStatus,ECSqlStatement&)> testFunc) {
        ECSqlStatement stmt;
        ECSqlStatus rc = stmt.Prepare(m_ecdb, ecsql);
        testFunc(rc, stmt);
        stmt.Finalize();
        };

    auto assertCol = [](ECSqlStatement &stmt, int i, std::function<void(ECSqlColumnInfoCR)> func) {
        func(stmt.GetValue(i).GetColumnInfo());
    };

    auto fooClassId = m_ecdb.Schemas().GetClassId("test", "Foo");
    auto fooSchemaId = m_ecdb.Schemas().GetSchema("test")->GetId();
    prepare("SELECT * FROM (SELECT s.ECInstanceId, c.ECInstanceId, c.Name, s.Name FROM meta.ECClassDef c JOIN meta.ECSchemaDef s ON c.Schema.Id=s.ECInstanceId WHERE s.Name='Test' AND c.Name='Foo')", [&](ECSqlStatus rc, ECSqlStatement &stmt) {
        printf("%s\n", stmt.GetNativeSql());
        // Verify row
        ASSERT_EQ(ECSqlStatus::Success, rc) << stmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(fooSchemaId, stmt.GetValueId<ECN::ECClassId>(0));
        ASSERT_EQ(fooClassId, stmt.GetValueId<ECN::ECSchemaId>(1));
        // foo
        EXPECT_STRCASEEQ("Foo", stmt.GetValueText(2));
        EXPECT_STRCASEEQ("Test", stmt.GetValueText(3));

        assertCol(stmt, 0, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "ECInstanceId");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "ECInstanceId");
            EXPECT_EQ(info.IsGeneratedProperty(), false);
            EXPECT_EQ(info.IsSystemProperty(), true);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Long);
        });
        assertCol(stmt, 1, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "ECInstanceId");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "ECInstanceId");
            EXPECT_EQ(info.IsGeneratedProperty(), false);
            EXPECT_EQ(info.IsSystemProperty(), true);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Long);
        });
        assertCol(stmt, 2, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "Name");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "Name");
            EXPECT_EQ(info.IsGeneratedProperty(), false);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_String);
        });
        assertCol(stmt, 3, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "Name");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "Name");
            EXPECT_EQ(info.IsGeneratedProperty(), false);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_String);
        });
    });

    prepare("SELECT s.ECInstanceId, c.ECInstanceId, c.Name, s.Name FROM meta.ECClassDef c JOIN meta.ECSchemaDef s ON c.Schema.Id=s.ECInstanceId WHERE s.Name='Test' AND c.Name='Foo'", [&](ECSqlStatus rc, ECSqlStatement &stmt) {
        ASSERT_EQ(ECSqlStatus::Success, rc) << stmt.GetECSql();

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(fooSchemaId, stmt.GetValueId<ECN::ECClassId>(0));
        ASSERT_EQ(fooClassId, stmt.GetValueId<ECN::ECSchemaId>(1));
        EXPECT_STRCASEEQ("Foo", stmt.GetValueText(2));
        EXPECT_STRCASEEQ("Test", stmt.GetValueText(3));

        assertCol(stmt, 0, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "ECInstanceId");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "ECInstanceId");
            EXPECT_EQ(info.IsGeneratedProperty(), false);
            EXPECT_EQ(info.IsSystemProperty(), true);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Long);
        });
        assertCol(stmt, 1, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "ECInstanceId");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "ECInstanceId");
            EXPECT_EQ(info.IsGeneratedProperty(), false);
            EXPECT_EQ(info.IsSystemProperty(), true);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Long);
        });
        assertCol(stmt, 2, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "Name");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "Name");
            EXPECT_EQ(info.IsGeneratedProperty(), false);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_String);
        });
        assertCol(stmt, 3, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "Name");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "Name");
            EXPECT_EQ(info.IsGeneratedProperty(), false);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_String);
        });
    });
    }
//-----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, TopLevelAliasCheck)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("TopLevelAliasCheck.ecdb", SchemaItem(R"(
    <ECSchema schemaName="Test" alias="test" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECStructClass typeName="MyStruct" modifier="Sealed">
            <ECProperty propertyName="Bl"   typeName="binary"/>
            <ECProperty propertyName="Bo"   typeName="boolean"/>
            <ECProperty propertyName="D"    typeName="double"/>
            <ECProperty propertyName="Dt"   typeName="dateTime"/>
            <ECProperty propertyName="G"    typeName="Bentley.Geometry.Common.IGeometry"/>
            <ECProperty propertyName="I"    typeName="int"/>
            <ECProperty propertyName="L"    typeName="long"/>
            <ECProperty propertyName="P2d"  typeName="Point2d"/>
            <ECProperty propertyName="P3d"  typeName="Point3d"/>
            <ECProperty propertyName="S"    typeName="string"/>
        </ECStructClass>
        <ECEntityClass typeName="Foo" modifier="Sealed">
            <ECProperty propertyName="Bl"   typeName="binary"/>
            <ECProperty propertyName="Bo"   typeName="boolean"/>
            <ECProperty propertyName="D"    typeName="double"/>
            <ECProperty propertyName="Dt"   typeName="dateTime"/>
            <ECProperty propertyName="G"    typeName="Bentley.Geometry.Common.IGeometry"/>
            <ECProperty propertyName="I"    typeName="int"/>
            <ECProperty propertyName="L"    typeName="long"/>
            <ECProperty propertyName="P2d"  typeName="Point2d"/>
            <ECProperty propertyName="P3d"  typeName="Point3d"/>
            <ECProperty propertyName="S"    typeName="string"/>
            <ECStructProperty propertyName="Struct" typeName="MyStruct"/>
        </ECEntityClass>
    </ECSchema>
    )")));

    /** Insert a data ===================================================*/
    uint8_t bl[] = {0x10, 0x20, 0x30, 0x40, 0x50};
    auto bl_size = 5;
    auto bo = true;
    auto d = 3.12;
    auto dt = DateTime::GetCurrentTimeUtc();
    uint8_t g[] = {0x1f, 0x2f, 0x3f, 0x4f, 0x5f, 0x22};
    auto g_size = 6;
    auto i = 123;
    auto l = 0xffffffffa;
    auto p2d = DPoint2d::From(23.22, 31.11);
    auto p3d = DPoint3d::From(41.33, 41.13, 12.25);
    auto s = Utf8String("Hello, World");

    uint8_t s_bl[] = {0x1f, 0x2f, 0x3f, 0x4f, 0x5f};
    auto s_bl_size = 5;
    auto s_bo = false;
    auto s_d = 31.1223;
    auto s_dt = DateTime::GetCurrentTimeUtc();
    uint8_t s_g[] = {0xaf, 0xaf, 0xaf, 0xaf, 0xaf, 0xa2};
    auto s_g_size = 6;
    auto s_i = 123411;
    auto s_l = 0xffffab;
    auto s_p2d = DPoint2d::From(53.22, 31.11);
    auto s_p3d = DPoint3d::From(11.33, 31.13, 12.25);
    auto s_s = Utf8String("From Struct, Hello, World");


    ECSqlStatement insertStmt;
    insertStmt.Prepare(m_ecdb,"INSERT INTO test.Foo (Bl, Bo, D, Dt, G, I, L, P2d, P3d, S, Struct) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    insertStmt.BindBlob(1, (void*)&bl, bl_size, IECSqlBinder::MakeCopy::No);
    insertStmt.BindBoolean(2, bo);
    insertStmt.BindDouble(3, d);
    insertStmt.BindDateTime(4, dt);
    insertStmt.BindBlob(5, (void*)g, g_size, IECSqlBinder::MakeCopy::No);
    insertStmt.BindInt(6, i);
    insertStmt.BindInt64(7, l);
    insertStmt.BindPoint2d(8, p2d);
    insertStmt.BindPoint3d(9, p3d);
    insertStmt.BindText(10, s.c_str(), IECSqlBinder::MakeCopy::No);
    auto& st = insertStmt.GetBinder(11);

    st["Bl"].BindBlob((void*)s_bl, s_bl_size, IECSqlBinder::MakeCopy::No);
    st["Bo"].BindBoolean(s_bo);
    st["D"].BindDouble(s_d);
    st["Dt"].BindDateTime(s_dt);
    st["G"].BindBlob((void*)s_g, s_g_size, IECSqlBinder::MakeCopy::No);
    st["I"].BindInt(s_i);
    st["L"].BindInt64(s_l);
    st["P2d"].BindPoint2d(s_p2d);
    st["P3d"].BindPoint3d(s_p3d);
    st["S"].BindText(s_s.c_str(), IECSqlBinder::MakeCopy::No);

    ECInstanceKey pk;
    ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step(pk));
    /** Insert a data ===================================================*/

    auto prepare = [&](Utf8CP ecsql, std::function<void(ECSqlStatus,ECSqlStatement&)> testFunc) {
        ECSqlStatement stmt;
        ECSqlStatus rc = stmt.Prepare(m_ecdb, ecsql);
        testFunc(rc, stmt);
        stmt.Finalize();
        };

    auto assertCol = [](ECSqlStatement &stmt, int i, std::function<void(ECSqlColumnInfoCR)> func) {
        func(stmt.GetValue(i).GetColumnInfo());
    };

    prepare("SELECT * FROM (SELECT Struct.Bl, Struct.Bo, Struct.D, Struct.Dt, Struct.I, Struct.P2d, Struct.P3d, Struct.S FROM test.Foo WHERE ECInstanceId=?)", [&](ECSqlStatus rc, ECSqlStatement &stmt) {
        ASSERT_EQ(ECSqlStatus::Success, rc) << stmt.GetECSql();

        stmt.BindId(1, pk.GetInstanceId());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
        ASSERT_EQ(0, memcmp(stmt.GetValueBlob(0), (void*)s_bl, s_bl_size));
        ASSERT_EQ(stmt.GetValueBoolean(1), s_bo);
        ASSERT_EQ(stmt.GetValueDouble(2), s_d);
        ASSERT_TRUE(stmt.GetValueDateTime(3).Equals(s_dt, true));
        ASSERT_EQ(stmt.GetValueInt(4), s_i);
        ASSERT_EQ(stmt.GetValuePoint2d(5), s_p2d);
        ASSERT_EQ(stmt.GetValuePoint3d(6), s_p3d);
        ASSERT_STRCASEEQ(stmt.GetValueText(7), s_s.c_str());
    });

    prepare("select e.* from (select bl t_bl, bo t_bo, d t_d, dt t_dt, g t_g, i t_i, l t_l, p2d t_p2d, p3d t_p3d, s t_s, struct.bl s_bl, struct.bo s_bo, struct.d s_d, struct.dt s_dt, struct.g s_g, struct.i s_i, struct.l s_l, struct.p2d s_p2d, struct.p3d s_p3d, struct.s s_s, struct s_st from test.foo) e" ,
        [&](ECSqlStatus rc, ECSqlStatement& stmt) {
        ASSERT_EQ(ECSqlStatus::Success, rc) << stmt.GetECSql();
        ASSERT_EQ(21, stmt.GetColumnCount());


        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
        ASSERT_EQ(0, memcmp(stmt.GetValueBlob(0), (void*)bl, bl_size));
        ASSERT_EQ(stmt.GetValueBoolean(1), bo);
        ASSERT_EQ(stmt.GetValueDouble(2), d);
        ASSERT_TRUE(stmt.GetValueDateTime(3).Equals(dt, true));
        ASSERT_EQ(0, memcmp(stmt.GetValueBlob(4), (void*)g, g_size));
        ASSERT_EQ(stmt.GetValueInt(5), i);
        ASSERT_EQ(stmt.GetValueInt64(6), l);
        ASSERT_EQ(stmt.GetValuePoint2d(7), p2d);
        ASSERT_EQ(stmt.GetValuePoint3d(8), p3d);
        ASSERT_STRCASEEQ(stmt.GetValueText(9), s.c_str());

        ASSERT_EQ(0, memcmp(stmt.GetValueBlob(10), (void*)s_bl, s_bl_size));
        ASSERT_EQ(stmt.GetValueBoolean(11), s_bo);
        ASSERT_EQ(stmt.GetValueDouble(12), s_d);
        ASSERT_TRUE(stmt.GetValueDateTime(13).Equals(s_dt, true));
        ASSERT_EQ(0, memcmp(stmt.GetValueBlob(14), (void*)s_g, s_g_size));
        ASSERT_EQ(stmt.GetValueInt(15), s_i);
        ASSERT_EQ(stmt.GetValueInt64(16), s_l);
        ASSERT_EQ(stmt.GetValuePoint2d(17), s_p2d);
        ASSERT_EQ(stmt.GetValuePoint3d(18), s_p3d);
        ASSERT_STRCASEEQ(stmt.GetValueText(19), s_s.c_str());

        auto &st = stmt.GetValue(20);
        ASSERT_EQ(0, memcmp(st["Bl"].GetBlob(), (void*)s_bl, s_bl_size));
        ASSERT_EQ(st["Bo"].GetBoolean(), s_bo);
        ASSERT_EQ(st["D"].GetDouble(), s_d);
        ASSERT_TRUE(st["Dt"].GetDateTime().Equals(s_dt, true));
        ASSERT_EQ(0, memcmp(st["G"].GetBlob(), (void*)s_g, s_g_size));
        ASSERT_EQ(st["I"].GetInt(), s_i);
        ASSERT_EQ(st["L"].GetInt64(), s_l);
        ASSERT_EQ(st["P2D"].GetPoint2d(), s_p2d);
        ASSERT_EQ(st["P3D"].GetPoint3d(), s_p3d);
        ASSERT_STRCASEEQ(st["S"].GetText(), s_s.c_str());

        int i = -1;
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "t_bl");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "t_bl");
            EXPECT_EQ(info.IsGeneratedProperty(), true);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Binary);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "t_bo");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "t_bo");
            EXPECT_EQ(info.IsGeneratedProperty(), true);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Boolean);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "t_d");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "t_d");
            EXPECT_EQ(info.IsGeneratedProperty(), true);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Double);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "t_dt");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "t_dt");
            EXPECT_EQ(info.IsGeneratedProperty(), true);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_DateTime);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "t_g");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "t_g");
            EXPECT_EQ(info.IsGeneratedProperty(), true);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_IGeometry);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "t_i");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "t_i");
            EXPECT_EQ(info.IsGeneratedProperty(), true);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Integer);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "t_l");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "t_l");
            EXPECT_EQ(info.IsGeneratedProperty(), true);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Long);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "t_p2d");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "t_p2d");
            EXPECT_EQ(info.IsGeneratedProperty(), true);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Point2d);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "t_p3d");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "t_p3d");
            EXPECT_EQ(info.IsGeneratedProperty(), true);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Point3d);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "t_s");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "t_s");
            EXPECT_EQ(info.IsGeneratedProperty(), true);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_String);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "s_bl");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "s_bl");
            EXPECT_EQ(info.IsGeneratedProperty(), true);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Binary);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "s_bo");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "s_bo");
            EXPECT_EQ(info.IsGeneratedProperty(), true);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Boolean);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "s_d");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "s_d");
            EXPECT_EQ(info.IsGeneratedProperty(), true);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Double);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "s_dt");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "s_dt");
            EXPECT_EQ(info.IsGeneratedProperty(), true);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_DateTime);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "s_g");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "s_g");
            EXPECT_EQ(info.IsGeneratedProperty(), true);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_IGeometry);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "s_i");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "s_i");
            EXPECT_EQ(info.IsGeneratedProperty(), true);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Integer);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "s_l");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "s_l");
            EXPECT_EQ(info.IsGeneratedProperty(), true);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Long);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "s_p2d");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "s_p2d");
            EXPECT_EQ(info.IsGeneratedProperty(), true);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Point2d);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "s_p3d");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "s_p3d");
            EXPECT_EQ(info.IsGeneratedProperty(), true);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Point3d);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "s_s");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "s_s");
            EXPECT_EQ(info.IsGeneratedProperty(), true);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_String);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "s_st");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "s_st");
            EXPECT_EQ(info.IsGeneratedProperty(), true);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsStruct(), true);
        });
        ASSERT_EQ(i, stmt.GetColumnCount() - 1);
        EXPECT_STRCASEEQ(stmt.GetNativeSql(), "SELECT [K0],[K1],[K2],[K3],[K4],[K5],[K6],[K7_0],[K7_1],[K8_0],[K8_1],[K8_2],[K9],[K10],[K11],[K12],[K13],[K14],[K15],[K16],[K17_0],[K17_1],[K18_0],[K18_1],[K18_2],[K19],[K20_0],[K20_1],[K20_2],[K20_3],[K20_4],[K20_5],[K20_6],[K20_7],[K20_8],[K20_9],[K20_10],[K20_11],[K20_12] FROM (SELECT [foo].[Bl] [K0],[foo].[Bo] [K1],[foo].[D] [K2],[foo].[Dt] [K3],[foo].[G] [K4],[foo].[I] [K5],[foo].[L] [K6],[foo].[P2d_X] [K7_0],[foo].[P2d_Y] [K7_1],[foo].[P3d_X] [K8_0],[foo].[P3d_Y] [K8_1],[foo].[P3d_Z] [K8_2],[foo].[S] [K9],[foo].[Struct_Bl] [K10],[foo].[Struct_Bo] [K11],[foo].[Struct_D] [K12],[foo].[Struct_Dt] [K13],[foo].[Struct_G] [K14],[foo].[Struct_I] [K15],[foo].[Struct_L] [K16],[foo].[Struct_P2d_X] [K17_0],[foo].[Struct_P2d_Y] [K17_1],[foo].[Struct_P3d_X] [K18_0],[foo].[Struct_P3d_Y] [K18_1],[foo].[Struct_P3d_Z] [K18_2],[foo].[Struct_S] [K19],[foo].[Struct_Bl] [K20_0],[foo].[Struct_Bo] [K20_1],[foo].[Struct_D] [K20_2],[foo].[Struct_Dt] [K20_3],[foo].[Struct_G] [K20_4],[foo].[Struct_I] [K20_5],[foo].[Struct_L] [K20_6],[foo].[Struct_P2d_X] [K20_7],[foo].[Struct_P2d_Y] [K20_8],[foo].[Struct_P3d_X] [K20_9],[foo].[Struct_P3d_Y] [K20_10],[foo].[Struct_P3d_Z] [K20_11],[foo].[Struct_S] [K20_12] FROM (SELECT [Id] ECInstanceId,73 ECClassId,[Bl],[Bo],[D],[Dt],[G],[I],[L],[P2d_X],[P2d_Y],[P3d_X],[P3d_Y],[P3d_Z],[S],[Struct_Bl],[Struct_Bo],[Struct_D],[Struct_Dt],[Struct_G],[Struct_I],[Struct_L],[Struct_P2d_X],[Struct_P2d_Y],[Struct_P3d_X],[Struct_P3d_Y],[Struct_P3d_Z],[Struct_S] FROM [main].[test_Foo]) [foo]) 'e'");
        });

    prepare("select bl t_bl, bo t_bo, d t_d, dt t_dt, g t_g, i t_i, l t_l, p2d t_p2d, p3d t_p3d, s t_s, struct.bl s_bl, struct.bo s_bo, struct.d s_d, struct.dt s_dt, struct.g s_g, struct.i s_i, struct.l s_l, struct.p2d s_p2d, struct.p3d s_p3d, struct.s s_s, struct s_st from test.foo" ,
        [&](ECSqlStatus rc, ECSqlStatement& stmt) {
        ASSERT_EQ(ECSqlStatus::Success, rc) << stmt.GetECSql();
        ASSERT_EQ(21, stmt.GetColumnCount());

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
        ASSERT_EQ(0, memcmp(stmt.GetValueBlob(0), (void*)bl, bl_size));
        ASSERT_EQ(stmt.GetValueBoolean(1), bo);
        ASSERT_EQ(stmt.GetValueDouble(2), d);
        ASSERT_TRUE(stmt.GetValueDateTime(3).Equals(dt, true));
        ASSERT_EQ(0, memcmp(stmt.GetValueBlob(4), (void*)g, g_size));
        ASSERT_EQ(stmt.GetValueInt(5), i);
        ASSERT_EQ(stmt.GetValueInt64(6), l);
        ASSERT_EQ(stmt.GetValuePoint2d(7), p2d);
        ASSERT_EQ(stmt.GetValuePoint3d(8), p3d);
        ASSERT_STRCASEEQ(stmt.GetValueText(9), s.c_str());

        ASSERT_EQ(0, memcmp(stmt.GetValueBlob(10), (void*)s_bl, s_bl_size));
        ASSERT_EQ(stmt.GetValueBoolean(11), s_bo);
        ASSERT_EQ(stmt.GetValueDouble(12), s_d);
        ASSERT_TRUE(stmt.GetValueDateTime(13).Equals(s_dt, true));
        ASSERT_EQ(0, memcmp(stmt.GetValueBlob(14), (void*)s_g, s_g_size));
        ASSERT_EQ(stmt.GetValueInt(15), s_i);
        ASSERT_EQ(stmt.GetValueInt64(16), s_l);
        ASSERT_EQ(stmt.GetValuePoint2d(17), s_p2d);
        ASSERT_EQ(stmt.GetValuePoint3d(18), s_p3d);
        ASSERT_STRCASEEQ(stmt.GetValueText(19), s_s.c_str());

        auto &st = stmt.GetValue(20);
        ASSERT_EQ(0, memcmp(st["Bl"].GetBlob(), (void*)s_bl, s_bl_size));
        ASSERT_EQ(st["Bo"].GetBoolean(), s_bo);
        ASSERT_EQ(st["D"].GetDouble(), s_d);
        ASSERT_TRUE(st["Dt"].GetDateTime().Equals(s_dt, true));
        ASSERT_EQ(0, memcmp(st["G"].GetBlob(), (void*)s_g, s_g_size));
        ASSERT_EQ(st["I"].GetInt(), s_i);
        ASSERT_EQ(st["L"].GetInt64(), s_l);
        ASSERT_EQ(st["P2D"].GetPoint2d(), s_p2d);
        ASSERT_EQ(st["P3D"].GetPoint3d(), s_p3d);
        ASSERT_STRCASEEQ(st["S"].GetText(), s_s.c_str());


        int i = -1;
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "t_bl");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "t_bl");
            EXPECT_EQ(info.IsGeneratedProperty(), true);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Binary);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "t_bo");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "t_bo");
            EXPECT_EQ(info.IsGeneratedProperty(), true);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Boolean);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "t_d");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "t_d");
            EXPECT_EQ(info.IsGeneratedProperty(), true);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Double);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "t_dt");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "t_dt");
            EXPECT_EQ(info.IsGeneratedProperty(), true);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_DateTime);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "t_g");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "t_g");
            EXPECT_EQ(info.IsGeneratedProperty(), true);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_IGeometry);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "t_i");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "t_i");
            EXPECT_EQ(info.IsGeneratedProperty(), true);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Integer);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "t_l");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "t_l");
            EXPECT_EQ(info.IsGeneratedProperty(), true);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Long);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "t_p2d");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "t_p2d");
            EXPECT_EQ(info.IsGeneratedProperty(), true);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Point2d);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "t_p3d");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "t_p3d");
            EXPECT_EQ(info.IsGeneratedProperty(), true);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Point3d);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "t_s");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "t_s");
            EXPECT_EQ(info.IsGeneratedProperty(), true);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_String);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "s_bl");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "s_bl");
            EXPECT_EQ(info.IsGeneratedProperty(), true);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Binary);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "s_bo");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "s_bo");
            EXPECT_EQ(info.IsGeneratedProperty(), true);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Boolean);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "s_d");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "s_d");
            EXPECT_EQ(info.IsGeneratedProperty(), true);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Double);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "s_dt");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "s_dt");
            EXPECT_EQ(info.IsGeneratedProperty(), true);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_DateTime);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "s_g");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "s_g");
            EXPECT_EQ(info.IsGeneratedProperty(), true);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_IGeometry);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "s_i");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "s_i");
            EXPECT_EQ(info.IsGeneratedProperty(), true);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Integer);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "s_l");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "s_l");
            EXPECT_EQ(info.IsGeneratedProperty(), true);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Long);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "s_p2d");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "s_p2d");
            EXPECT_EQ(info.IsGeneratedProperty(), true);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Point2d);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "s_p3d");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "s_p3d");
            EXPECT_EQ(info.IsGeneratedProperty(), true);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Point3d);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "s_s");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "s_s");
            EXPECT_EQ(info.IsGeneratedProperty(), true);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_String);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "s_st");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "s_st");
            EXPECT_EQ(info.IsGeneratedProperty(), true);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsStruct(), true);
        });
        ASSERT_EQ(i, stmt.GetColumnCount() - 1);
        EXPECT_STRCASEEQ(stmt.GetNativeSql(), "SELECT [foo].[Bl] [t_bl],[foo].[Bo] [t_bo],[foo].[D] [t_d],[foo].[Dt] [t_dt],[foo].[G] [t_g],[foo].[I] [t_i],[foo].[L] [t_l],[foo].[P2d_X] [t_p2d_0],[foo].[P2d_Y] [t_p2d_1],[foo].[P3d_X] [t_p3d_0],[foo].[P3d_Y] [t_p3d_1],[foo].[P3d_Z] [t_p3d_2],[foo].[S] [t_s],[foo].[Struct_Bl] [s_bl],[foo].[Struct_Bo] [s_bo],[foo].[Struct_D] [s_d],[foo].[Struct_Dt] [s_dt],[foo].[Struct_G] [s_g],[foo].[Struct_I] [s_i],[foo].[Struct_L] [s_l],[foo].[Struct_P2d_X] [s_p2d_0],[foo].[Struct_P2d_Y] [s_p2d_1],[foo].[Struct_P3d_X] [s_p3d_0],[foo].[Struct_P3d_Y] [s_p3d_1],[foo].[Struct_P3d_Z] [s_p3d_2],[foo].[Struct_S] [s_s],[foo].[Struct_Bl] [s_st_0],[foo].[Struct_Bo] [s_st_1],[foo].[Struct_D] [s_st_2],[foo].[Struct_Dt] [s_st_3],[foo].[Struct_G] [s_st_4],[foo].[Struct_I] [s_st_5],[foo].[Struct_L] [s_st_6],[foo].[Struct_P2d_X] [s_st_7],[foo].[Struct_P2d_Y] [s_st_8],[foo].[Struct_P3d_X] [s_st_9],[foo].[Struct_P3d_Y] [s_st_10],[foo].[Struct_P3d_Z] [s_st_11],[foo].[Struct_S] [s_st_12] FROM (SELECT [Id] ECInstanceId,73 ECClassId,[Bl],[Bo],[D],[Dt],[G],[I],[L],[P2d_X],[P2d_Y],[P3d_X],[P3d_Y],[P3d_Z],[S],[Struct_Bl],[Struct_Bo],[Struct_D],[Struct_Dt],[Struct_G],[Struct_I],[Struct_L],[Struct_P2d_X],[Struct_P2d_Y],[Struct_P3d_X],[Struct_P3d_Y],[Struct_P3d_Z],[Struct_S] FROM [main].[test_Foo]) [foo]");
        });

    prepare("select bl, bo, d, dt, g, i, l, p2d, p3d, s, struct.bl, struct.bo, struct.d, struct.dt, struct.g, struct.i, struct.l, struct.p2d, struct.p3d, struct.s, struct from test.foo", [&](ECSqlStatus rc, ECSqlStatement &stmt) {
        ASSERT_EQ(ECSqlStatus::Success, rc) << stmt.GetECSql();
        ASSERT_EQ(21, stmt.GetColumnCount());

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
        ASSERT_EQ(0, memcmp(stmt.GetValueBlob(0), (void*)bl, bl_size));
        ASSERT_EQ(stmt.GetValueBoolean(1), bo);
        ASSERT_EQ(stmt.GetValueDouble(2), d);
        ASSERT_TRUE(stmt.GetValueDateTime(3).Equals(dt, true));
        ASSERT_EQ(0, memcmp(stmt.GetValueBlob(4), (void*)g, g_size));
        ASSERT_EQ(stmt.GetValueInt(5), i);
        ASSERT_EQ(stmt.GetValueInt64(6), l);
        ASSERT_EQ(stmt.GetValuePoint2d(7), p2d);
        ASSERT_EQ(stmt.GetValuePoint3d(8), p3d);
        ASSERT_STRCASEEQ(stmt.GetValueText(9), s.c_str());

        ASSERT_EQ(0, memcmp(stmt.GetValueBlob(10), (void*)s_bl, s_bl_size));
        ASSERT_EQ(stmt.GetValueBoolean(11), s_bo);
        ASSERT_EQ(stmt.GetValueDouble(12), s_d);
        ASSERT_TRUE(stmt.GetValueDateTime(13).Equals(s_dt, true));
        ASSERT_EQ(0, memcmp(stmt.GetValueBlob(14), (void*)s_g, s_g_size));
        ASSERT_EQ(stmt.GetValueInt(15), s_i);
        ASSERT_EQ(stmt.GetValueInt64(16), s_l);
        ASSERT_EQ(stmt.GetValuePoint2d(17), s_p2d);
        ASSERT_EQ(stmt.GetValuePoint3d(18), s_p3d);
        ASSERT_STRCASEEQ(stmt.GetValueText(19), s_s.c_str());

        auto &st = stmt.GetValue(20);
        ASSERT_EQ(0, memcmp(st["Bl"].GetBlob(), (void*)s_bl, s_bl_size));
        ASSERT_EQ(st["Bo"].GetBoolean(), s_bo);
        ASSERT_EQ(st["D"].GetDouble(), s_d);
        ASSERT_TRUE(st["Dt"].GetDateTime().Equals(s_dt, true));
        ASSERT_EQ(0, memcmp(st["G"].GetBlob(), (void*)s_g, s_g_size));
        ASSERT_EQ(st["I"].GetInt(), s_i);
        ASSERT_EQ(st["L"].GetInt64(), s_l);
        ASSERT_EQ(st["P2D"].GetPoint2d(), s_p2d);
        ASSERT_EQ(st["P3D"].GetPoint3d(), s_p3d);
        ASSERT_STRCASEEQ(st["S"].GetText(), s_s.c_str());

        int i = -1;
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "bl");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "bl");
            EXPECT_EQ(info.IsGeneratedProperty(), false);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Binary);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "bo");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "bo");
            EXPECT_EQ(info.IsGeneratedProperty(), false);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Boolean);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "d");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "d");
            EXPECT_EQ(info.IsGeneratedProperty(), false);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Double);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "dt");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "dt");
            EXPECT_EQ(info.IsGeneratedProperty(), false);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_DateTime);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "g");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "g");
            EXPECT_EQ(info.IsGeneratedProperty(), false);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_IGeometry);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "i");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "i");
            EXPECT_EQ(info.IsGeneratedProperty(), false);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Integer);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "l");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "l");
            EXPECT_EQ(info.IsGeneratedProperty(), false);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Long);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "p2d");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "p2d");
            EXPECT_EQ(info.IsGeneratedProperty(), false);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Point2d);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "p3d");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "p3d");
            EXPECT_EQ(info.IsGeneratedProperty(), false);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Point3d);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "s");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "s");
            EXPECT_EQ(info.IsGeneratedProperty(), false);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_String);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "bl");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "struct.bl");
            EXPECT_EQ(info.IsGeneratedProperty(), false);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Binary);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "bo");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "struct.bo");
            EXPECT_EQ(info.IsGeneratedProperty(), false);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Boolean);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "d");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "struct.d");
            EXPECT_EQ(info.IsGeneratedProperty(), false);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Double);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "dt");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "struct.dt");
            EXPECT_EQ(info.IsGeneratedProperty(), false);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_DateTime);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "g");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "struct.g");
            EXPECT_EQ(info.IsGeneratedProperty(), false);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_IGeometry);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "i");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "struct.i");
            EXPECT_EQ(info.IsGeneratedProperty(), false);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Integer);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "l");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "struct.l");
            EXPECT_EQ(info.IsGeneratedProperty(), false);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Long);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "p2d");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "struct.p2d");
            EXPECT_EQ(info.IsGeneratedProperty(), false);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Point2d);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "p3d");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "struct.p3d");
            EXPECT_EQ(info.IsGeneratedProperty(), false);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_Point3d);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "s");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "struct.s");
            EXPECT_EQ(info.IsGeneratedProperty(), false);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsPrimitive(), true);
            EXPECT_EQ(info.GetDataType().GetPrimitiveType(), PRIMITIVETYPE_String);
        });
        assertCol(stmt, ++i, [](ECSqlColumnInfoCR info) {
            EXPECT_STRCASEEQ(info.GetProperty()->GetName().c_str(), "struct");
            EXPECT_STRCASEEQ(info.GetPropertyPath().ToString().c_str(), "struct");
            EXPECT_EQ(info.IsGeneratedProperty(), false);
            EXPECT_EQ(info.IsSystemProperty(), false);
            EXPECT_EQ(info.GetDataType().IsStruct(), true);
        });
        ASSERT_EQ(i, stmt.GetColumnCount() - 1);
        EXPECT_STRCASEEQ(stmt.GetNativeSql(), "SELECT [foo].[Bl],[foo].[Bo],[foo].[D],[foo].[Dt],[foo].[G],[foo].[I],[foo].[L],[foo].[P2d_X],[foo].[P2d_Y],[foo].[P3d_X],[foo].[P3d_Y],[foo].[P3d_Z],[foo].[S],[foo].[Struct_Bl],[foo].[Struct_Bo],[foo].[Struct_D],[foo].[Struct_Dt],[foo].[Struct_G],[foo].[Struct_I],[foo].[Struct_L],[foo].[Struct_P2d_X],[foo].[Struct_P2d_Y],[foo].[Struct_P3d_X],[foo].[Struct_P3d_Y],[foo].[Struct_P3d_Z],[foo].[Struct_S],[foo].[Struct_Bl],[foo].[Struct_Bo],[foo].[Struct_D],[foo].[Struct_Dt],[foo].[Struct_G],[foo].[Struct_I],[foo].[Struct_L],[foo].[Struct_P2d_X],[foo].[Struct_P2d_Y],[foo].[Struct_P3d_X],[foo].[Struct_P3d_Y],[foo].[Struct_P3d_Z],[foo].[Struct_S] FROM (SELECT [Id] ECInstanceId,73 ECClassId,[Bl],[Bo],[D],[Dt],[G],[I],[L],[P2d_X],[P2d_Y],[P3d_X],[P3d_Y],[P3d_Z],[S],[Struct_Bl],[Struct_Bo],[Struct_D],[Struct_Dt],[Struct_G],[Struct_I],[Struct_L],[Struct_P2d_X],[Struct_P2d_Y],[Struct_P3d_X],[Struct_P3d_Y],[Struct_P3d_Z],[Struct_S] FROM [main].[test_Foo]) [foo]");
    });
    }
//-----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, CompositPropertyAccessFromASubQuery)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("SubQueryAlias.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
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
        "        <ECStructProperty propertyName='StructProp' typeName='MyStruct' />"
        "        <ECNavigationProperty propertyName='Model' relationshipName='ModelHasElement' direction='Backward' />"
        "        <ECNavigationProperty propertyName='Parent' relationshipName='ElementOwnsChildElement' direction='Backward' />"
        "    </ECEntityClass>"
        "    <ECStructClass typeName = 'MyStruct'>"
        "        <ECProperty propertyName='SomeNumber' typeName='double' />"
        "        <ECProperty propertyName='SomeString' typeName='string' />"
        "    </ECStructClass>"
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
        "</ECSchema>")));

    auto prepare = [&](Utf8CP ecsql, std::function<void(ECSqlStatus,ECSqlStatement&)> testFunc) {
        ECSqlStatement stmt;
        ECSqlStatus rc = stmt.Prepare(m_ecdb, ecsql);
        testFunc(rc, stmt);
        stmt.Finalize();
        };

    prepare(R"(SELECT e1.ECInstanceId, e2.ECInstanceId FROM (SELECT ECInstanceId AS ECInstanceId FROM ts.Element) e1
                JOIN ts.Element e2 on e2.ECInstanceId = e1.ECInstanceId)",
                [](ECSqlStatus rc, ECSqlStatement &stmt) {
        Utf8String nativeSql = stmt.GetNativeSql();
        ASSERT_EQ(2, stmt.GetColumnCount());
        ASSERT_TRUE(nativeSql.StartsWith("SELECT [K0],[e2].[ECInstanceId] FROM (SELECT [Element].[ECInstanceId] [K0] FROM (SELECT [Id] ECInstanceId,[ECClassId] FROM [main].[ts_Element]) [Element]) 'e1' INNER JOIN (SELECT [Id] ECInstanceId,[ECClassId] FROM [main].[ts_Element]) [e2] ON [e2].[ECInstanceId]=[K0]"));
    });

    prepare("SELECT * FROM (SELECT COUNT(*) FROM ts.Element)",
        [](ECSqlStatus rc, ECSqlStatement& stmt) {
        ASSERT_EQ(ECSqlStatus::Success, rc) << stmt.GetECSql();
        Utf8String nativeSql = stmt.GetNativeSql();
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_TRUE(nativeSql.StartsWith("SELECT [K0] FROM (SELECT COUNT(*) [K0] FROM (SELECT [Id] ECInstanceId,[ECClassId] FROM [main].[ts_Element]) [Element])"));
        });

    /* bridge crash */
    prepare(R"(
        SELECT
            e.ECInstanceId,
            x.ECInstanceId,
            old.ECClassId
        FROM ts.Element e
            LEFT JOIN ts.Element x ON e.ECInstanceId = x.Model.Id
            LEFT JOIN (
                SELECT
                    e.ECInstanceId,
                    a.ECClassId
                FROM ts.Element e, ts.Element a, meta.ECClassDef c, meta.ECSchemaDef s
                WHERE
                    a.ECClassId = c.ECInstanceId AND
                    c.Schema.Id = s.ECInstanceId AND
                    (s.Name = 'OpenBridgeModelerCE' OR s.Name = 'DgnV8OpenRoadsDesigner') AND
                    e.ECInstanceId = a.Model.Id
                ) old ON e.ECInstanceId = old.ECInstanceId
        WHERE e.Model.Id = ? AND (x.ECInstanceId IS NOT NULL OR old.ECClassId IS NOT NULL)
        ORDER BY e.ECInstanceId
        )" ,
        [](ECSqlStatus rc, ECSqlStatement& stmt) {
        ASSERT_EQ(ECSqlStatus::Success, rc) << stmt.GetECSql();
        Utf8String nativeSql = stmt.GetNativeSql();
        ASSERT_EQ(3, stmt.GetColumnCount());
        ASSERT_TRUE(nativeSql.StartsWith("SELECT [e].[ECInstanceId],[x].[ECInstanceId],[K1] FROM (SELECT [Id] ECInstanceId,[ECClassId],[ModelId] FROM [main].[ts_Element]) [e] LEFT OUTER JOIN (SELECT [Id] ECInstanceId,[ECClassId],[ModelId] FROM [main].[ts_Element]) [x] ON [e].[ECInstanceId]=[x].[ModelId]  LEFT OUTER JOIN (SELECT [e].[ECInstanceId] [K0],[a].[ECClassId] [K1] FROM (SELECT [Id] ECInstanceId,[ECClassId] FROM [main].[ts_Element]) [e],(SELECT [Id] ECInstanceId,[ECClassId],[ModelId] FROM [main].[ts_Element]) [a],(SELECT [Id] ECInstanceId,"));
        ASSERT_TRUE(nativeSql.EndsWith("ECClassId,[Name] FROM [main].[ec_Schema]) [s] WHERE [a].[ECClassId]=[c].[ECInstanceId] AND [c].[SchemaId]=[s].[ECInstanceId] AND ([s].[Name]='OpenBridgeModelerCE' OR [s].[Name]='DgnV8OpenRoadsDesigner') AND [e].[ECInstanceId]=[a].[ModelId]) 'old' ON [e].[ECInstanceId]=[K0]  WHERE [e].[ModelId]=:_ecdb_sqlparam_ix1_col1 AND ([x].[ECInstanceId] IS NOT NULL OR [K1] IS NOT NULL) ORDER BY [e].[ECInstanceId]"));
        });

    /* Alias take priority over name of property in same context*/
    prepare(R"(
        SELECT  [/ECClassId/],0 AS [dummy],
                [/SkippedInstanceKeys/],
                [/DisplayLabel/],
                [/GroupedInstanceKeys/]
        FROM (
            SELECT
                [/ECClassId/],
                '[]'                AS [/SkippedInstanceKeys/],
                hex([/ECClassId/])  AS [/DisplayLabel/],
                hex([/ECClassId/])  AS [/GroupedInstanceKeys/]
            FROM (
                SELECT
                    '[]'                        AS [/SkippedInstanceKeys/],
                    [/search/].[/ECInstanceId/] AS [/ECInstanceId/],
                    [/search/].[/ECClassId/]    AS [/ECClassId/]
                FROM (
                    SELECT
                        ECInstanceId    AS [/ECInstanceId/],
                        ECClassId       AS [/ECClassId/]
                    FROM ts.Element) [/search/])
                    GROUP BY [/ECClassId/]
            ) ORDER BY length([/DisplayLabel/])
        )" ,
        [](ECSqlStatus rc, ECSqlStatement& stmt) {
        ASSERT_EQ(ECSqlStatus::Success, rc) << stmt.GetECSql();
        Utf8String nativeSql = stmt.GetNativeSql();
        ASSERT_EQ(5, stmt.GetColumnCount());
        ASSERT_TRUE(nativeSql.StartsWith("SELECT [K3],0 [dummy],[K4],[K5],[K6] FROM (SELECT [K2] [K3],'[]' [K4],hex([K2]) [K5],hex([K2]) [K6] FROM (SELECT '[]',[K0],[K1] [K2] FROM (SELECT [Element].[ECInstanceId] [K0],[Element].[ECClassId] [K1] FROM (SELECT [Id] ECInstanceId,[ECClassId] FROM [main].[ts_Element]) [Element]) '/search/')  GROUP BY [K2]) ORDER BY length([K5])"));
        });
    /* Alias take priority over name of property in same context*/
    prepare("SELECT StructProp.SomeNumber, StructProp.SomeString FROM (SELECT Code, StructProp FROM ts.Element) StructProp WHERE StructProp.SomeNumber = ?" ,
        [](ECSqlStatus rc, ECSqlStatement& stmt) {
        ASSERT_EQ(ECSqlStatus::Success, rc) << stmt.GetECSql();
        Utf8String nativeSql = stmt.GetNativeSql();
        ASSERT_EQ(2, stmt.GetColumnCount());
        ASSERT_TRUE(nativeSql.StartsWith("SELECT [K0_0],[K0_1] FROM (SELECT [Element].[Code],[Element].[StructProp_SomeNumber] [K0_0],[Element].[StructProp_SomeString] [K0_1] FROM (SELECT [Id] ECInstanceId,[ECClassId],[Code],[StructProp_SomeNumber],[StructProp_SomeString] FROM [main].[ts_Element]) [Element]) 'StructProp' WHERE [K0_0]=:_ecdb_sqlparam_ix1_col1"));
        });

    prepare("SELECT e.PI FROM (SELECT 3.12 AS PI FROM ts.Element) e",
            [](ECSqlStatus rc, ECSqlStatement &stmt) {
                ASSERT_EQ(ECSqlStatus::Success, rc) << stmt.GetECSql();
                Utf8String nativeSql = stmt.GetNativeSql();
                ASSERT_EQ(1, stmt.GetColumnCount());
                ASSERT_EQ(PrimitiveType::PRIMITIVETYPE_Double, stmt.GetValue(0).GetColumnInfo().GetDataType().GetPrimitiveType());
                ASSERT_TRUE(nativeSql.StartsWith("SELECT [K0] FROM (SELECT 3.12 [K0] FROM (SELECT [Id] ECInstanceId,[ECClassId] FROM [main].[ts_Element]) [Element]) 'e'"));
            });
    /* struct alias from sub query should can be used is top level query*/
    prepare("SELECT e.st.SomeString, e.st.SomeNumber FROM (SELECT StructProp st FROM ts.Element) e",
        [](ECSqlStatus rc, ECSqlStatement& stmt) {
        ASSERT_EQ(ECSqlStatus::Success, rc) << stmt.GetECSql();
        Utf8String nativeSql = stmt.GetNativeSql();
        ASSERT_EQ(2, stmt.GetColumnCount());
        ASSERT_EQ(PrimitiveType::PRIMITIVETYPE_String, stmt.GetValue(0).GetColumnInfo().GetDataType().GetPrimitiveType());
        ASSERT_EQ(PrimitiveType::PRIMITIVETYPE_Double, stmt.GetValue(1).GetColumnInfo().GetDataType().GetPrimitiveType());
        ASSERT_TRUE(nativeSql.StartsWith("SELECT [K0_1],[K0_0] FROM (SELECT [Element].[StructProp_SomeNumber] [K0_0],[Element].[StructProp_SomeString] [K0_1] FROM (SELECT [Id] ECInstanceId,[ECClassId],[StructProp_SomeNumber],[StructProp_SomeString] FROM [main].[ts_Element]) [Element]) 'e'"));
        });

    /* nav alias from sub query should can be used is top level query*/
    prepare("SELECT b.Id FROM (SELECT Model b FROM ts.Element)",
        [](ECSqlStatus rc, ECSqlStatement& stmt) {
        ASSERT_EQ(ECSqlStatus::Success, rc) << stmt.GetECSql();
        Utf8String nativeSql = stmt.GetNativeSql();
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_TRUE(nativeSql.StartsWith("SELECT [K0_0] FROM (SELECT [Element].[ModelId] [K0_0],[Element].[ModelRelECClassId] [K0_1] FROM "));
        });

    /* Alias take priority over name of property in same context*/
    prepare("SELECT StructProp.Code, StructProp FROM ts.Element StructProp",
        [](ECSqlStatus rc, ECSqlStatement& stmt) {
        ASSERT_EQ(ECSqlStatus::Success, rc) << stmt.GetECSql();
        Utf8String nativeSql = stmt.GetNativeSql();
        ASSERT_EQ(2, stmt.GetColumnCount());
        ASSERT_TRUE(nativeSql.StartsWith("SELECT [StructProp].[Code],[StructProp].[StructProp_SomeNumber],[StructProp].[StructProp_SomeString] FROM (SELECT [Id] ECInstanceId,[ECClassId],[Code],[StructProp_SomeNumber],[StructProp_SomeString] FROM [main].[ts_Element]) [StructProp]"));
        });

    /* Subquery wildcard selection should not cause generated properties*/
    prepare("SELECT * FROM (SELECT * FROM ts.Element)",
        [](ECSqlStatus rc, ECSqlStatement& stmt) {
        ASSERT_EQ(ECSqlStatus::Success, rc) << stmt.GetECSql();
        Utf8String nativeSql = stmt.GetNativeSql();
        ASSERT_EQ(6, stmt.GetColumnCount());
        for (int i = 0; i< stmt.GetColumnCount(); i++){
            ASSERT_EQ(false, stmt.GetValue(i).GetColumnInfo().IsGeneratedProperty());
        }
        ASSERT_TRUE(nativeSql.StartsWith("SELECT [K0],[K1],[K2],[K3_0],[K3_1],[K4_0],[K4_1],[K5_0],[K5_1] FROM (SELECT [Element].[ECInstanceId] [K0],[Element].[ECClassId] [K1],[Element].[Code] [K2],[Element].[StructProp_SomeNumber] [K3_0],[Element].[StructProp_SomeString] [K3_1],[Element].[ModelId] [K4_0],[Element].[ModelRelECClassId] [K4_1],[Element].[ParentId] [K5_0],[Element].[ParentRelECClassId] [K5_1] FROM ("));
        });

    /* able to select aliased property from subquery*/
    prepare("SELECT e.st.SomeString FROM (SELECT StructProp st FROM ts.Element) e",
        [](ECSqlStatus rc, ECSqlStatement& stmt) {
        ASSERT_EQ(ECSqlStatus::Success, rc) << stmt.GetECSql();
        Utf8String nativeSql = stmt.GetNativeSql();
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_EQ(PrimitiveType::PRIMITIVETYPE_String, stmt.GetValue(0).GetColumnInfo().GetDataType().GetPrimitiveType());
        ASSERT_TRUE(nativeSql.StartsWith("SELECT [K0_1] FROM (SELECT [Element].[StructProp_SomeNumber] [K0_0],[Element].[StructProp_SomeString] [K0_1] FROM (SELECT [Id] ECInstanceId,[ECClassId],[StructProp_SomeNumber],[StructProp_SomeString] FROM [main].[ts_Element]) [Element]) 'e'"));
        });

    /* wild card select from subquery set */
    prepare("SELECT * FROM  (SELECT ECInstanceId, ECClassId, Model.id, Model.RelECClassId FROM ts.Element)",
        [](ECSqlStatus rc, ECSqlStatement& stmt) {
        ASSERT_EQ(ECSqlStatus::Success, rc) << stmt.GetECSql();
        Utf8String nativeSql = stmt.GetNativeSql();
        ASSERT_EQ(4, stmt.GetColumnCount());
        ASSERT_TRUE(nativeSql.StartsWith("SELECT [K0],[K1],[K2],[K3] FROM (SELECT [Element].[ECInstanceId] [K0],[Element].[ECClassId] [K1],[Element].[ModelId] [K2],[Element].[ModelRelECClassId] [K3] FROM "));
        });

    // alias access is not supported
    prepare("SELECT e.b.Id FROM (SELECT Model b FROM ts.Element) e",
        [](ECSqlStatus rc, ECSqlStatement& stmt) {
        ASSERT_EQ(ECSqlStatus::Success, rc) << stmt.GetECSql();
        Utf8String nativeSql = stmt.GetNativeSql();
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_TRUE(nativeSql.StartsWith("SELECT [K0_0] FROM (SELECT [Element].[ModelId] [K0_0],[Element].[ModelRelECClassId] [K0_1] FROM (SELECT [Id] ECInstanceId,[ECClassId]"));
        });

    prepare("SELECT test.* FROM (SELECT Code AS C1 FROM ts.Element) test",
        [](ECSqlStatus rc, ECSqlStatement& stmt) {
        ASSERT_EQ(ECSqlStatus::Success, rc) << stmt.GetECSql();
        Utf8String nativeSql = stmt.GetNativeSql();
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_TRUE(nativeSql.StartsWith("SELECT [K0] FROM (SELECT [Element].[Code] [K0] FROM (SELECT [Id] ECInstanceId,[ECClassId],[Code] FROM [main].[ts_Element]) [Element]) 'test'"));
        });

    prepare("select * from (SELECT * FROM ts.Element) limit :sys_ecdb_count offset :sys_ecdb_offset",
        [](ECSqlStatus rc, ECSqlStatement& stmt) {
        ASSERT_EQ(ECSqlStatus::Success, rc) << stmt.GetECSql();
        Utf8String nativeSql = stmt.GetNativeSql();
        ASSERT_EQ(6, stmt.GetColumnCount());
        ASSERT_EQ(PrimitiveType::PRIMITIVETYPE_Long, stmt.GetValue(0).GetColumnInfo().GetDataType().GetPrimitiveType());
        ASSERT_TRUE(nativeSql.StartsWith("SELECT [K0],[K1],[K2],[K3_0],[K3_1],[K4_0],[K4_1],[K5_0],[K5_1] FROM (SELECT [Element].[ECInstanceId] [K0],[Element].[ECClassId] [K1],[Element].[Code] [K2],[Element].[StructProp_SomeNumber] [K3_0],[Element].[StructProp_SomeString] [K3_1],[Element].[ModelId] [K4_0],[Element].[ModelRelECClassId] [K4_1],[Element].[ParentId] [K5_0],[Element].[ParentRelECClassId] [K5_1] FROM"));
        ASSERT_TRUE(nativeSql.EndsWith("[ParentRelECClassId] FROM [main].[ts_Element]) [Element])  LIMIT :sys_ecdb_count_col1 OFFSET :sys_ecdb_offset_col1"));
        });

     prepare("SELECT e.Model.Id FROM (SELECT Model FROM ts.Element) e",
         [](ECSqlStatus rc, ECSqlStatement& stmt) {
        ASSERT_EQ(ECSqlStatus::Success, rc) << stmt.GetECSql();
        Utf8String nativeSql = stmt.GetNativeSql();
        ASSERT_EQ(1, stmt.GetColumnCount());
        ASSERT_EQ(PrimitiveType::PRIMITIVETYPE_Long, stmt.GetValue(0).GetColumnInfo().GetDataType().GetPrimitiveType());
        ASSERT_TRUE(nativeSql.StartsWith("SELECT [K0_0] FROM (SELECT [Element].[ModelId] [K0_0],[Element].[ModelRelECClassId] [K0_1] FROM"));
        });

     prepare("SELECT e.StructProp.SomeNumber FROM (SELECT StructProp FROM ts.Element) e",
         [](ECSqlStatus rc, ECSqlStatement& stmt) {
            ASSERT_EQ(ECSqlStatus::Success, rc) << stmt.GetECSql();
            Utf8String nativeSql = stmt.GetNativeSql();
            ASSERT_EQ(1, stmt.GetColumnCount());
            ASSERT_EQ(PrimitiveType::PRIMITIVETYPE_Double, stmt.GetValue(0).GetColumnInfo().GetDataType().GetPrimitiveType());
            ASSERT_TRUE(nativeSql.StartsWith("SELECT [K0_0] FROM (SELECT [Element].[StructProp_SomeNumber] [K0_0],[Element].[StructProp_SomeString] [K0_1] FROM"));
         });

     prepare("SELECT e.Model.Id FROM (SELECT * FROM ts.Element) e JOIN ts.Model m ON m.ECInstanceId = e.Model.Id",
         [](ECSqlStatus rc, ECSqlStatement& stmt) {
            ASSERT_EQ(ECSqlStatus::Success, rc) << stmt.GetECSql();
            Utf8String nativeSql = stmt.GetNativeSql();
            ASSERT_EQ(1, stmt.GetColumnCount());
            ASSERT_EQ(PrimitiveType::PRIMITIVETYPE_Long, stmt.GetValue(0).GetColumnInfo().GetDataType().GetPrimitiveType());
            ASSERT_TRUE(nativeSql.StartsWith("SELECT [K0_0] FROM (SELECT [Element].[ECInstanceId],[Element].[ECClassId],[Element].[Code],[Element].[StructProp_SomeNumber],[Element].[StructProp_SomeString],[Element].[ModelId] [K0_0],[Element].[ModelRelECClassId] [K0_1],[Element].[ParentId],[Element].[ParentRelECClassId] FROM"));
            ASSERT_TRUE(nativeSql.EndsWith(" FROM [main].[ts_Model]) [m] ON [m].[ECInstanceId]=[K0_0] "));
         });
    }
//---------------------------------------------------------------------------------------
// @bsitest
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, PrimitiveArrayUnsetMembers)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlprimarray_unsetmembers.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECEntityClass typeName="MyClass">
            <ECArrayProperty propertyName="Long_Array" typeName="long"/>
            <ECArrayProperty propertyName="String_Array" typeName="string"/>
            <ECArrayProperty propertyName="Blob_Array" typeName="binary"/>
            <ECArrayProperty propertyName="P3d_Array" typeName="Point3d"/>
        </ECEntityClass>
    </ECSchema>
    )xml")));

    const uint64_t testValue = BeBriefcaseBasedId(BeBriefcaseId(444), INT64_C(1432342)).GetValue();
    std::vector<std::pair<PrimitiveType, ECValue>> testValues {{PrimitiveType::PRIMITIVETYPE_Long, ECValue(testValue)},
    {PrimitiveType::PRIMITIVETYPE_String, ECValue(R"text(Hello, "world")text")},
    {PrimitiveType::PRIMITIVETYPE_Binary, ECValue((Byte const*) &testValue,sizeof(testValue))},
    {PrimitiveType::PRIMITIVETYPE_Point3d, ECValue(DPoint3d::From(1.0, -1.5, 10.3))}};

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.MyClass(Long_Array,String_Array,Blob_Array,P3d_Array) VALUES(?,?,?,?)"));

    for (int i = 0; i < 4; i++)
        {
        IECSqlBinder& arrayBinder = stmt.GetBinder(i + 1);
        //first element: don't bind anything
        arrayBinder.AddArrayElement();

        {
        //call BindNull on element
        IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
        ASSERT_EQ(ECSqlStatus::Success, elementBinder.BindNull());
        }

        {
        //call BindNull on element
        IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();
        std::pair<PrimitiveType, ECValue> const& testValue = testValues[(size_t) i];
        switch (testValue.first)
            {
                case PRIMITIVETYPE_Long:
                    ASSERT_EQ(ECSqlStatus::Success, elementBinder.BindInt64(testValue.second.GetLong()));
                    break;

                case PRIMITIVETYPE_String:
                    ASSERT_EQ(ECSqlStatus::Success, elementBinder.BindText(testValue.second.GetUtf8CP(), IECSqlBinder::MakeCopy::No));
                    break;

                case PRIMITIVETYPE_Binary:
                {
                size_t blobSize = 0;
                void const* blob = testValue.second.GetBinary(blobSize);
                ASSERT_EQ(ECSqlStatus::Success, elementBinder.BindBlob(blob, (int) blobSize, IECSqlBinder::MakeCopy::No));
                break;
                }

                case PRIMITIVETYPE_Point3d:
                    ASSERT_EQ(ECSqlStatus::Success, elementBinder.BindPoint3d(testValue.second.GetPoint3d()));
                    break;

                default:
                    FAIL() << "Test has to be adjusted to new primitive type";
            }
        }
        }


    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    stmt.Finalize();

    Statement validateStmt;
    ASSERT_EQ(BE_SQLITE_OK, validateStmt.Prepare(m_ecdb, "SELECT Long_Array,String_Array,Blob_Array,P3d_Array FROM ts_MyClass WHERE Id=?"));
    ASSERT_EQ(BE_SQLITE_OK, validateStmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, validateStmt.Step());


    for (int i = 0; i < 4; i++)
        {
        Utf8String actualJson(validateStmt.GetValueText(i));

        rapidjson::Document expectedJson(rapidjson::kArrayType);
        expectedJson.PushBack(rapidjson::Value(rapidjson::kNullType).Move(), expectedJson.GetAllocator());
        expectedJson.PushBack(rapidjson::Value(rapidjson::kNullType).Move(), expectedJson.GetAllocator());

        std::pair<PrimitiveType, ECValue> const& testValue = testValues[(size_t) i];
        switch (testValue.first)
            {
                case PRIMITIVETYPE_Long:
                    expectedJson.PushBack(rapidjson::Value(testValue.second.GetLong()).Move(), expectedJson.GetAllocator());
                    break;

                case PRIMITIVETYPE_String:
                {
                rapidjson::GenericStringRef<Utf8Char> stringValue(testValue.second.GetUtf8CP(), (rapidjson::SizeType) strlen(testValue.second.GetUtf8CP()));
                expectedJson.PushBack(rapidjson::Value(stringValue).Move(), expectedJson.GetAllocator());
                break;
                }

                case PRIMITIVETYPE_Binary:
                {
                size_t blobSize = 0;
                Byte const* blob = testValue.second.GetBinary(blobSize);

                rapidjson::Value val;
                ASSERT_EQ(SUCCESS, ECJsonUtilities::BinaryToJson(val, blob, blobSize, expectedJson.GetAllocator()));
                expectedJson.PushBack(val.Move(), expectedJson.GetAllocator());
                break;
                }

                case PRIMITIVETYPE_Point3d:
                {
                rapidjson::Value val;
                ASSERT_EQ(SUCCESS, ECJsonUtilities::Point3dToJson(val, testValue.second.GetPoint3d(), expectedJson.GetAllocator()));
                expectedJson.PushBack(val.Move(), expectedJson.GetAllocator());
                break;
                }
                default:
                    FAIL() << "Test has to be adjusted to new primitive type";

            }

        rapidjson::StringBuffer expectedJsonStr;
        rapidjson::Writer<rapidjson::StringBuffer> writer(expectedJsonStr);
        expectedJson.Accept(writer);
        ASSERT_STRCASEEQ(expectedJsonStr.GetString(), actualJson.c_str());
        }
    }

//---------------------------------------------------------------------------------------
// @bsitest
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, StructArrayUnsetMembers)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstructarray_unsetmembers.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
          <ECEntityClass typeName="MyClass">
                <ECStructArrayProperty propertyName="Locations" typeName="LocationStruct"/>
          </ECEntityClass>
          <ECStructClass typeName="LocationStruct">
                <ECProperty propertyName="Street" typeName="string"/>
                <ECStructProperty propertyName="City" typeName="CityStruct"/>
          </ECStructClass>
         <ECStructClass typeName="CityStruct">
               <ECProperty propertyName="Name" typeName="string"/>
               <ECProperty propertyName="State" typeName="string"/>
               <ECProperty propertyName="Country" typeName="string"/>
               <ECProperty propertyName="Zip" typeName="int"/>
         </ECStructClass>
        </ECSchema>
        )xml")));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.MyClass(Locations) VALUES(?)"));
    IECSqlBinder& structArrayBinder = stmt.GetBinder(1);
    //first element: don't bind anything
    structArrayBinder.AddArrayElement();

    {
    //call BindNull on element
    IECSqlBinder& elementBinder = structArrayBinder.AddArrayElement();
    ASSERT_EQ(ECSqlStatus::Success, elementBinder.BindNull());
    }

    {
    //bind to prim member in element
    IECSqlBinder& elementBinder = structArrayBinder.AddArrayElement();
    ASSERT_EQ(ECSqlStatus::Success, elementBinder["Street"].BindText("mainstreet", IECSqlBinder::MakeCopy::No));
    }

    {
    //bind null to prim member in element
    IECSqlBinder& elementBinder = structArrayBinder.AddArrayElement();
    ASSERT_EQ(ECSqlStatus::Success, elementBinder["Street"].BindNull());
    }

    {
    //call BindNull on struct member in element
    IECSqlBinder& elementBinder = structArrayBinder.AddArrayElement();
    ASSERT_EQ(ECSqlStatus::Success, elementBinder["City"].BindNull());
    }

    {
    //call BindNull on prim and struct member in element
    IECSqlBinder& elementBinder = structArrayBinder.AddArrayElement();
    ASSERT_EQ(ECSqlStatus::Success, elementBinder["Street"].BindNull());
    ASSERT_EQ(ECSqlStatus::Success, elementBinder["City"].BindNull());
    }

    {
    //bind to prim member in struct member in element
    IECSqlBinder& elementBinder = structArrayBinder.AddArrayElement();
    ASSERT_EQ(ECSqlStatus::Success, elementBinder["City"]["Zip"].BindInt(34000));
    }

    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    stmt.Finalize();

    Statement validateStmt;
    ASSERT_EQ(BE_SQLITE_OK, validateStmt.Prepare(m_ecdb, "SELECT Locations FROM ts_MyClass WHERE Id=?"));
    ASSERT_EQ(BE_SQLITE_OK, validateStmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, validateStmt.Step());
    Utf8String actualJson(validateStmt.GetValueText(0));
    actualJson.ReplaceAll(" ", "");
    ASSERT_STRCASEEQ(R"json([null,null,{"Street":"mainstreet"},{"Street":null},{"City":null},{"Street":null,"City":null},{"City":{"Zip":34000}}])json", actualJson.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, DateTimeCast)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("datetimecast.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    DateTime expectedDateOnly(2017, 2, 22);
    DateTime expectedDtUtc(DateTime::Kind::Utc, 2017, 2, 22, 10, 4, 2);

    ECInstanceKey key;
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.P(DateOnly,DtUtc,S) VALUES(?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDateTime(1, expectedDateOnly)) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDateTime(2, expectedDtUtc)) << stmt.GetECSql();
    double jd = -1.0;
    ASSERT_EQ(SUCCESS, expectedDateOnly.ToJulianDay(jd));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(3, jd)) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << stmt.GetECSql();
    stmt.Finalize();

    //casts strip away the date time info as it is not part of the actual value. Same when inserting a DateTime object into a non-DateTime property.
    //when retrieving those values, DateTime::Info is expected to be DateTime::Kind::Unspecified
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT DateOnly,DtUtc,CAST(DateOnly AS TEXT), CAST(DtUtc AS TEXT), CAST(DateOnly AS REAL), CAST(DtUtc AS REAL), CAST(DateOnly AS Date), CAST(DtUtc AS TimeStamp), S FROM ecsql.P WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, key.GetInstanceId())) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();

    ASSERT_EQ(9, stmt.GetColumnCount()) << stmt.GetECSql();

    EXPECT_TRUE(expectedDateOnly.Equals(stmt.GetValueDateTime(0))) << "Select clause item 0: Expected: " << expectedDateOnly.ToString().c_str() << " Actual: " << stmt.GetValueDateTime(0).ToString().c_str();
    EXPECT_TRUE(expectedDtUtc.Equals(stmt.GetValueDateTime(1))) << "Select clause item 1: Expected: " << expectedDtUtc.ToString().c_str() << " Actual: " << stmt.GetValueDateTime(1).ToString().c_str();

    for (int i = 2; i < stmt.GetColumnCount(); i++)
        {
        DateTime actual = stmt.GetValueDateTime(i);

        //Original DateTime::Info is always lost with cast or if not persisted in DateTime property
        EXPECT_EQ(DateTime::Component::DateAndTime, actual.GetInfo().GetComponent()) << "Select clause item " << i << ": Expected: " << expectedDateOnly.ToString().c_str() << " Actual: " << actual.ToString().c_str();
        EXPECT_EQ(DateTime::Kind::Unspecified, actual.GetInfo().GetKind()) << "Select clause item " << i << ": Expected: " << expectedDateOnly.ToString().c_str() << " Actual: " << actual.ToString().c_str();

        if (i % 2 == 0)
            EXPECT_TRUE(actual.Equals(DateTime(DateTime::Kind::Unspecified, expectedDateOnly.GetYear(), expectedDateOnly.GetMonth(), expectedDateOnly.GetDay(), 0, 0))) << "Select clause item " << i << " Actual: " << actual.ToString().c_str();
        else
            EXPECT_TRUE(actual.Equals(DateTime(DateTime::Kind::Unspecified, expectedDtUtc.GetYear(), expectedDtUtc.GetMonth(), expectedDtUtc.GetDay(), expectedDtUtc.GetHour(), expectedDtUtc.GetMinute(), expectedDtUtc.GetSecond()))) << "Select clause item " << i << " Actual: " << actual.ToString().c_str();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, NullLiteralForPoints)
    {
    const int rowCountPerClass = 3;
    ASSERT_EQ(SUCCESS, SetupECDb("nullliterals.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, PopulateECDb( rowCountPerClass));

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT CAST(NULL AS Point3D) FROM ecsql.PASpatial LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    ASSERT_EQ(PRIMITIVETYPE_Point3d, stmt.GetColumnInfo(0).GetDataType().GetPrimitiveType());
    ASSERT_TRUE(stmt.IsValueNull(0));
    }

    bmap<Utf8CP, bool> ecsqls;
    ecsqls["SELECT CAST(NULL AS Point3D) FROM ecsql.PASpatial "
        "UNION ALL "
        "SELECT P3D FROM ecsql.P"] = true;

    ecsqls["SELECT CAST(NULL AS Double) FROM ecsql.PASpatial "
        "UNION ALL "
        "SELECT P3D FROM ecsql.P"] = false;

    ecsqls["SELECT CAST(NULL AS Point2D) FROM ecsql.PASpatial "
        "UNION ALL "
        "SELECT P3D FROM ecsql.P"] = false;

    ecsqls["SELECT P3D FROM ecsql.P "
        "UNION ALL "
        "SELECT NULL FROM ecsql.PASpatial"] = true;

    ecsqls["SELECT NULL FROM ecsql.PASpatial "
        "UNION ALL "
        "SELECT P3D FROM ecsql.P"] = false;

    ecsqls["SELECT P3D FROM ecsql.P "
        "UNION ALL "
        "SELECT CAST(NULL AS Point3D) FROM ecsql.PASpatial"] = true;

    ecsqls["SELECT P3D FROM ecsql.P "
        "UNION ALL "
        "SELECT CAST(NULL AS Double) FROM ecsql.PASpatial"] = false;

    ecsqls["SELECT P3D FROM ecsql.P "
        "UNION ALL "
        "SELECT CAST(NULL AS Point2D) FROM ecsql.PASpatial"] = false;

    for (bpair<Utf8CP, bool> const& kvPair : ecsqls)
        {
        Utf8CP ecsql = kvPair.first;
        const bool expectedToSucceed = kvPair.second;

        if (!expectedToSucceed)
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, ecsql)) << ecsql;
            continue;
            }

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql)) << ecsql;

        int actualRowCount = 0;
        while (BE_SQLITE_ROW == stmt.Step())
            {
            ASSERT_EQ(PRIMITIVETYPE_Point3d, stmt.GetColumnInfo(0).GetDataType().GetPrimitiveType()) << ecsql;
            actualRowCount++;
            }
        ASSERT_EQ(rowCountPerClass * 2, actualRowCount) << ecsql;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, NullLiteralForStructs)
    {
    const int rowCountPerClass = 3;
    ASSERT_EQ(SUCCESS, SetupECDb("nullliterals.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, PopulateECDb( rowCountPerClass));

    ECClassCP structType = m_ecdb.Schemas().GetClass("ECSqlTest", "PStruct");
    ASSERT_TRUE(structType != nullptr);

    auto assertColumn = [] (ECClassCR expected, IECSqlValue const& colVal, bool checkIsNull)
        {
        ASSERT_TRUE(colVal.GetColumnInfo().GetDataType().IsStruct() && &colVal.GetColumnInfo().GetProperty()->GetAsStructProperty()->GetType() == &expected);

        if (checkIsNull)
            ASSERT_TRUE(colVal.IsNull());

        for (IECSqlValue const& memberVal : colVal.GetStructIterable())
            {
            if (checkIsNull)
                ASSERT_TRUE(memberVal.IsNull());
            }
        };

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT CAST(NULL AS ecsql.PStruct) FROM ecsql.PSA LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    assertColumn(*structType, stmt.GetValue(0), true);
    }

    bmap<Utf8CP, bool> ecsqls;
    ecsqls["SELECT CAST(NULL AS ecsql.PStruct) FROM ecsql.P "
        "UNION ALL "
        "SELECT PStructProp FROM ecsql.PSA"] = true;

    ecsqls["SELECT CAST(NULL AS ecsql.[PStruct]) FROM ecsql.P "
        "UNION ALL "
        "SELECT PStructProp FROM ecsql.PSA"] = true;

    ecsqls["SELECT CAST(NULL AS Double) FROM ecsql.PASpatial "
        "UNION ALL "
        "SELECT PStructProp FROM ecsql.PSA"] = false;

    ecsqls["SELECT NULL FROM ecsql.P "
        "UNION ALL "
        "SELECT PStructProp FROM ecsql.PSA"] = false;

    ecsqls["SELECT PStructProp FROM ecsql.PSA "
        "UNION ALL "
        "SELECT NULL FROM ecsql.P"] = true;

    for (bpair<Utf8CP, bool> const& kvPair : ecsqls)
        {
        Utf8CP ecsql = kvPair.first;
        const bool expectedToSucceed = kvPair.second;

        if (!expectedToSucceed)
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, ecsql)) << ecsql;
            continue;
            }

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql)) << ecsql;

        int actualRowCount = 0;
        while (BE_SQLITE_ROW == stmt.Step())
            {
            assertColumn(*structType, stmt.GetValue(0), false);
            actualRowCount++;
            }

        ASSERT_EQ(rowCountPerClass * 2, actualRowCount) << ecsql;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, NullLiteralForPrimArrays)
    {
    const int rowCountPerClass = 3;
    ASSERT_EQ(SUCCESS, SetupECDb("nullliterals.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, PopulateECDb( rowCountPerClass));


    auto assertColumn = [] (PrimitiveType expected, IECSqlValue const& colVal, bool checkIsNull)
        {
        ASSERT_TRUE(colVal.GetColumnInfo().GetDataType().IsPrimitiveArray());
        ASSERT_EQ(expected, colVal.GetColumnInfo().GetDataType().GetPrimitiveType());
        if (checkIsNull)
            {
            ASSERT_TRUE(colVal.IsNull());
            ASSERT_EQ(0, colVal.GetArrayLength());
            }
        };

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT CAST(NULL AS TIMESTAMP[]) FROM ecsql.PSA LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    assertColumn(PRIMITIVETYPE_DateTime, stmt.GetValue(0), true);
    }

    bmap<Utf8CP, bool> ecsqls;
    ecsqls["SELECT CAST(NULL AS TIMESTAMP[]) FROM ecsql.P "
        "UNION ALL "
        "SELECT Dt_Array FROM ecsql.PSA"] = true;

    ecsqls["SELECT CAST(NULL AS Double) FROM ecsql.P "
        "UNION ALL "
        "SELECT Dt_Array FROM ecsql.PSA"] = false;

    ecsqls["SELECT CAST(NULL AS TimeStamp) FROM ecsql.P "
        "UNION ALL "
        "SELECT Dt_Array FROM ecsql.PSA"] = false;

    ecsqls["SELECT NULL FROM ecsql.P "
        "UNION ALL "
        "SELECT Dt_Array FROM ecsql.PSA"] = false;

    ecsqls["SELECT Dt_Array FROM ecsql.PSA "
        "UNION ALL "
        "SELECT NULL FROM ecsql.P"] = true;

    for (bpair<Utf8CP, bool> const& kvPair : ecsqls)
        {
        Utf8CP ecsql = kvPair.first;
        const bool expectedToSucceed = kvPair.second;

        if (!expectedToSucceed)
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, ecsql)) << ecsql;
            continue;
            }

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql)) << ecsql;

        int actualRowCount = 0;
        while (BE_SQLITE_ROW == stmt.Step())
            {
            assertColumn(PRIMITIVETYPE_DateTime, stmt.GetValue(0), false);
            actualRowCount++;
            }

        ASSERT_EQ(rowCountPerClass * 2, actualRowCount) << ecsql;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, NullLiteralForStructArrays)
    {
    const int rowCountPerClass = 3;
    ASSERT_EQ(SUCCESS, SetupECDb("nullliterals.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, PopulateECDb( rowCountPerClass));

    ECClassCP structType = m_ecdb.Schemas().GetClass("ECSqlTest", "PStruct");
    ASSERT_TRUE(structType != nullptr);

    auto assertColumn = [] (ECClassCR expected, IECSqlValue const& colVal, bool checkIsNull)
        {
        ASSERT_TRUE(colVal.GetColumnInfo().GetDataType().IsStructArray());
        ASSERT_TRUE(&colVal.GetColumnInfo().GetProperty()->GetAsStructArrayProperty()->GetStructElementType() == &expected);
        if (checkIsNull)
            {
            ASSERT_TRUE(colVal.IsNull());
            ASSERT_EQ(0, colVal.GetArrayLength());
            }
        };

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT CAST(NULL AS ecsql.PStruct[]) FROM ecsql.PSA LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    assertColumn(*structType, stmt.GetValue(0), true);
    }

    bmap<Utf8CP, bool> ecsqls;
    ecsqls["SELECT CAST(NULL AS ecsql.PStruct[]) FROM ecsql.P "
        "UNION ALL "
        "SELECT PStruct_Array FROM ecsql.PSA"] = true;

    ecsqls["SELECT CAST(NULL AS [ecsql].[PStruct][]) FROM ecsql.P "
        "UNION ALL "
        "SELECT PStruct_Array FROM ecsql.PSA"] = true;

    ecsqls["SELECT CAST(NULL AS ecsql.PStruct) FROM ecsql.P "
        "UNION ALL "
        "SELECT PStruct_Array FROM ecsql.PSA"] = false;

    ecsqls["SELECT NULL FROM ecsql.P "
        "UNION ALL "
        "SELECT PStruct_Array FROM ecsql.PSA"] = false;

    ecsqls["SELECT PStruct_Array FROM ecsql.PSA "
        "UNION ALL "
        "SELECT NULL FROM ecsql.P"] = true;

    for (bpair<Utf8CP, bool> const& kvPair : ecsqls)
        {
        Utf8CP ecsql = kvPair.first;
        const bool expectedToSucceed = kvPair.second;

        if (!expectedToSucceed)
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, ecsql)) << ecsql;
            continue;
            }

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql)) << ecsql;

        int actualRowCount = 0;
        while (BE_SQLITE_ROW == stmt.Step())
            {
            assertColumn(*structType, stmt.GetValue(0), false);
            actualRowCount++;
            }

        ASSERT_EQ(rowCountPerClass * 2, actualRowCount) << ecsql;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, Coalesce)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ECSqlStatementTests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.P(I,S) VALUES(22, null)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.P(I,S) VALUES(null, 'Foo')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT I,COALESCE(I,S) FROM ecsql.P"));
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        if (stmt.IsValueNull(0))
            ASSERT_STREQ("Foo", stmt.GetValueText(1));
        else
            ASSERT_EQ(22, stmt.GetValueInt(1));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, NestedSelectStatementsTests)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ECSqlStatementTests.ecdb", SchemaItem::CreateForFile("ECSqlStatementTests.01.00.00.ecschema.xml")));
    NestedStructArrayTestSchemaHelper::PopulateECSqlStatementTestsDb(m_ecdb);

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ProductName, UnitPrice FROM ECST.Product WHERE UnitPrice > (SELECT AVG(UnitPrice) From ECST.Product) AND UnitPrice < 500"));
    ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);

    ECSqlStatement selectStmt;
    ASSERT_EQ(ECSqlStatus::Success, selectStmt.Prepare(m_ecdb, "SELECT ProductName From ECST.Product WHERE UnitPrice = ?"));
    ASSERT_EQ(ECSqlStatus::Success, selectStmt.BindDouble(1, stmt.GetValueDouble(1))) << "Binding Double value failed";
    ASSERT_TRUE(selectStmt.Step() == BE_SQLITE_ROW);
    ASSERT_STREQ(stmt.GetValueText(0), selectStmt.GetValueText(0));
    stmt.Finalize();

    //Using GetClassId in Nested Select statement
    ECClassId supplierClassId = m_ecdb.Schemas().GetClassId("ECST", "Supplier", SchemaLookupMode::ByAlias);
    ECClassId customerClassId = m_ecdb.Schemas().GetClassId("ECST", "Customer", SchemaLookupMode::ByAlias);
    ECClassId firstClassId = std::min<ECClassId>(supplierClassId, customerClassId);
    ECClassId secondClassId = std::max<ECClassId>(supplierClassId, customerClassId);
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECClassId, COUNT(*) FROM (SELECT ECClassId FROM ECST.Supplier UNION ALL SELECT ECClassId FROM ECST.Customer) GROUP BY ECClassId ORDER BY ECClassId"));
    ASSERT_TRUE(stmt.Step() == BE_SQLITE_ROW);
    ASSERT_EQ(firstClassId, stmt.GetValueId<ECClassId>(0));
    ASSERT_EQ(4, stmt.GetValueInt(1));
    ASSERT_TRUE(stmt.Step() == BE_SQLITE_ROW);
    ASSERT_EQ(secondClassId, stmt.GetValueId<ECClassId>(0));
    ASSERT_EQ(3, stmt.GetValueInt(1));
    ASSERT_TRUE(stmt.Step() == BE_SQLITE_DONE);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, PredicateFunctionsInNestedSelectStatement)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ECSqlStatementTests.ecdb", SchemaItem::CreateForFile("ECSqlStatementTests.01.00.00.ecschema.xml")));
    NestedStructArrayTestSchemaHelper::PopulateECSqlStatementTestsDb(m_ecdb);

    ECSqlStatement stmt;
    //Using Predicate function in nexted select statement
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT AVG(UnitPrice) FROM ECST.Product WHERE UnitPrice IN (SELECT UnitPrice FROM ECST.Product WHERE UnitPrice < (SELECT AVG(UnitPrice) FROM ECST.Product WHERE ProductAvailable))"));
    ASSERT_TRUE(stmt.Step() == BE_SQLITE_ROW);
    ASSERT_EQ(223, (int) stmt.GetValueDouble(0));
    ASSERT_TRUE(stmt.Step() == BE_SQLITE_DONE);
    stmt.Finalize();

    //Using NOT operator with predicate function in Nested Select statement
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT AVG(UnitPrice) FROM ECST.Product WHERE UnitPrice IN (SELECT UnitPrice FROM ECST.Product WHERE UnitPrice > (SELECT AVG(UnitPrice) FROM ECST.Product WHERE NOT ProductAvailable))"));
    ASSERT_TRUE(stmt.Step() == BE_SQLITE_ROW);
    ASSERT_EQ(619, (int) stmt.GetValueDouble(0));
    ASSERT_TRUE(stmt.Step() == BE_SQLITE_DONE);
    }

//-------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, ParametersInNestedSelectStatement)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("parametersinnestedselect.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ecsql.PSA(L,I) VALUES(33,123)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ecsql.PSA(L,I) VALUES(123456789,123)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ecsql.PSA(L,I) VALUES(123456789,124)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ecsql.PSA(L,I) VALUES(4444,123)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ecsql.P(I) VALUES(123)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ecsql.P(I) VALUES(124)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ecsql.P(I) VALUES(123)"));


    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT count(*) FROM ecsql.PSA WHERE L=123456789 AND I IN (SELECT I FROM ecsql.P WHERE I=123)"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    ASSERT_EQ(1, stmt.GetValueInt(0)) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT count(*) FROM ecsql.PSA WHERE L=? AND I IN (SELECT I FROM ecsql.P WHERE I=?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(1, 123456789)) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(2, 123)) << stmt.GetECSql();

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    ASSERT_EQ(1, stmt.GetValueInt(0)) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    }

    ECInstanceKey psaKey1, psaKey2, pKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(psaKey1, "INSERT INTO ecsql.PSA(L,S) VALUES(314,'Test PSA 1')"));
    Utf8String ecsql;
    ecsql.Sprintf("INSERT INTO ecsql.P(MyPSA.Id,S) VALUES(%s,'Test P')", psaKey1.GetInstanceId().ToString().c_str());
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(pKey, ecsql.c_str()));

    ecsql.Sprintf("INSERT INTO ecsql.PSA(L,I,S) VALUES(314,%s,'Test PSA 2')", pKey.GetInstanceId().ToString().c_str());
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(psaKey2, ecsql.c_str()));
    m_ecdb.SaveChanges();

    {
    Utf8String ecsqlWithoutParams;
    ecsqlWithoutParams.Sprintf("SELECT ECInstanceId FROM ecsql.PSA WHERE L=314 AND I IN (SELECT ECInstanceId FROM ecsql.P WHERE MyPSA.Id=%s)", psaKey1.GetInstanceId().ToString().c_str());

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsqlWithoutParams.c_str())) << ecsqlWithoutParams.c_str();

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    ASSERT_EQ(psaKey2.GetInstanceId(), stmt.GetValueId<ECInstanceId>(0)) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ecsql.PSA WHERE L=? AND I IN (SELECT ECInstanceId FROM ecsql.P WHERE MyPSA.Id=?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(1, 314)) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, psaKey1.GetInstanceId())) << stmt.GetECSql();

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    ASSERT_EQ(psaKey2.GetInstanceId(), stmt.GetValueId<ECInstanceId>(0)) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT PSA.ECInstanceId FROM ecsql.PSA, ecsql.P WHERE PSA.L=? AND PSA.I=P.ECInstanceId AND P.MyPSA.Id=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(1, 314)) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, psaKey1.GetInstanceId())) << stmt.GetECSql();

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    ASSERT_EQ(psaKey2.GetInstanceId(), stmt.GetValueId<ECInstanceId>(0)) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, GroupByClauseTests)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ECSqlStatementTests.ecdb", SchemaItem::CreateForFile("ECSqlStatementTests.01.00.00.ecschema.xml")));
    NestedStructArrayTestSchemaHelper::PopulateECSqlStatementTestsDb(m_ecdb);

    Utf8CP expectedProductsNames;
    Utf8String actualProductsNames;
    double expectedSumOfAvgPrices;
    double actualSumOfAvgPrices;
    ECSqlStatement stmt;
    //use of simple GROUP BY clause to find AVG(Price) from the Product table
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ProductName, AVG(UnitPrice) FROM ECST.Product GROUP BY ProductName ORDER BY ProductName"));
    expectedProductsNames = "Binder-Desk-Pen-Pen Set-Pencil-";
    expectedSumOfAvgPrices = 1895.67;
    actualSumOfAvgPrices = 0;
    while (stmt.Step() != BE_SQLITE_DONE)
        {
        actualProductsNames.append(stmt.GetValueText(0));
        actualProductsNames.append("-");
        actualSumOfAvgPrices += stmt.GetValueDouble(1);
        }
    ASSERT_STREQ(expectedProductsNames, actualProductsNames.c_str());
    ASSERT_EQ((int) expectedSumOfAvgPrices, (int) actualSumOfAvgPrices);
    stmt.Finalize();

    //using HAVING clause with GROUP BY clause
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ProductName, AVG(UnitPrice) FROM ECST.Product GROUP BY ProductName Having AVG(UnitPrice)>300.00 ORDER BY ProductName"));
    expectedProductsNames = "Binder-Pen-Pen Set-";
    actualProductsNames = "";
    expectedSumOfAvgPrices = 1556.62;
    actualSumOfAvgPrices = 0;
    while (stmt.Step() != BE_SQLITE_DONE)
        {
        actualProductsNames.append(stmt.GetValueText(0));
        actualProductsNames.append("-");
        actualSumOfAvgPrices += stmt.GetValueDouble(1);
        }
    ASSERT_STREQ(expectedProductsNames, actualProductsNames.c_str());
    ASSERT_EQ((int) expectedSumOfAvgPrices, (int) actualSumOfAvgPrices);
    stmt.Finalize();

    //combined Use of GROUP BY, HAVING and WHERE Clause
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ProductName, AVG(UnitPrice) FROM ECST.Product WHERE UnitPrice<500 GROUP BY ProductName Having AVG(UnitPrice)>200.00 ORDER BY ProductName"));
    expectedProductsNames = "Binder-Pen Set-";
    actualProductsNames = "";
    expectedSumOfAvgPrices = 666.84;
    actualSumOfAvgPrices = 0;
    while (stmt.Step() != BE_SQLITE_DONE)
        {
        actualProductsNames.append(stmt.GetValueText(0));
        actualProductsNames.append("-");
        actualSumOfAvgPrices += stmt.GetValueDouble(1);
        }
    ASSERT_EQ(expectedProductsNames, actualProductsNames);
    ASSERT_EQ((int) expectedSumOfAvgPrices, (int) actualSumOfAvgPrices);
    stmt.Finalize();

    //GROUP BY Clause with more then one parameters
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ProductName, AVG(UnitPrice), COUNT(ProductName) FROM ECST.Product GROUP BY ProductName, UnitPrice HAVING COUNT(ProductName)>1"));
    ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);
    ASSERT_EQ("Pen", (Utf8String) stmt.GetValueText(0));
    ASSERT_EQ(539, (int) stmt.GetValueDouble(1));
    ASSERT_EQ(3, stmt.GetValueInt(2));
    ASSERT_FALSE(stmt.Step() != BE_SQLITE_DONE);
    stmt.Finalize();
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, GroupByPoint)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ECSqlStatementTests.ecdb", SchemaItem(R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                      <ECEntityClass typeName="Foo" >
                        <ECProperty propertyName="Name" typeName="string" />
                        <ECProperty propertyName="Origin" typeName="Point3d" />
                      </ECEntityClass>
                 </ECSchema>)xml")));

    ECSqlStatement insertStmt;
    ASSERT_EQ(ECSqlStatus::Success, insertStmt.Prepare(m_ecdb, "INSERT INTO ts.Foo(Name,Origin) VALUES(?,?)"));

    DPoint3d pt1 = DPoint3d::From(1.0,2.0,3.0);
    DPoint3d pt2 = DPoint3d::From(-1.0, -2.0, 3.0);

    bmap<DPoint3d const*, int> ptCount;

    {
    ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindText(1, "Item 1", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindPoint3d(2, pt1));
    ptCount[&pt1] = 1;
    ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step());
    insertStmt.Reset();
    insertStmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindText(1, "Item 2", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindPoint3d(2, pt1));
    ptCount[&pt1]++;
    ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step());
    insertStmt.Reset();
    insertStmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindText(1, "Item 3", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindPoint3d(2, pt1));
    ptCount[&pt1]++;
    ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step());
    insertStmt.Reset();
    insertStmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindText(1, "Item 4", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindPoint3d(2, pt2));
    ptCount[&pt2] = 1;
    ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step());
    insertStmt.Reset();
    insertStmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindText(1, "Item 5", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step());
    }

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT Origin, count(*) FROM ts.Foo GROUP BY Origin"));
    int rowCount = 0;
    while (BE_SQLITE_ROW == statement.Step())
        {
        rowCount++;
        if (statement.IsValueNull(0))
            ASSERT_EQ(1, statement.GetValueInt(1)) << statement.GetECSql();
        else
            {
            DPoint3d actualPt = statement.GetValuePoint3d(0);
            if (pt1.AlmostEqual(actualPt))
                ASSERT_EQ(ptCount[&pt1], statement.GetValueInt(1)) << statement.GetECSql();
            else if (pt2.AlmostEqual(actualPt))
                ASSERT_EQ(ptCount[&pt2], statement.GetValueInt(1)) << statement.GetECSql();
            else
                FAIL();
            }
        }
    ASSERT_EQ(3, rowCount) << statement.GetECSql();
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, GroupByGeometry)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ECSqlStatementTests.ecdb", SchemaItem(R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                      <ECEntityClass typeName="Foo" >
                        <ECProperty propertyName="Name" typeName="string" />
                        <ECProperty propertyName="Geometry" typeName="Bentley.Geometry.Common.IGeometry" />
                      </ECEntityClass>
                 </ECSchema>)xml")));

    ECSqlStatement insertStmt;
    ASSERT_EQ(ECSqlStatus::Success, insertStmt.Prepare(m_ecdb, "INSERT INTO ts.Foo(Name,Geometry) VALUES(?,?)"));

    IGeometryPtr geometry1 = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, 1.0, 1.0, 1.0)));
    IGeometryPtr geometry2 = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(1.0, 0.0, 0.0, 1.0, 1.0, 1.0)));

    bmap<IGeometry*, int> geometryCount;
    {
    ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindText(1, "Item 1", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindGeometry(2, *geometry1));
    geometryCount[geometry1.get()] = 1;
    ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step());
    insertStmt.Reset();
    insertStmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindText(1, "Item 2", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindGeometry(2, *geometry1));
    geometryCount[geometry1.get()]++;
    ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step());
    insertStmt.Reset();
    insertStmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindText(1, "Item 3", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindGeometry(2, *geometry1));
    geometryCount[geometry1.get()]++;
    ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step());
    insertStmt.Reset();
    insertStmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindText(1, "Item 4", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindGeometry(2, *geometry2));
    geometryCount[geometry2.get()] = 1;
    ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step());

    insertStmt.Reset();
    insertStmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindText(1, "Item 5", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step());
    }

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT Geometry, count(*) FROM ts.Foo GROUP BY Geometry"));
    int rowCount = 0;
    while (BE_SQLITE_ROW == statement.Step())
        {
        rowCount++;
        if (statement.IsValueNull(0))
            ASSERT_EQ(1, statement.GetValueInt(1)) << statement.GetECSql();
        else
            {
            IGeometryPtr actualGeom = statement.GetValueGeometry(0);
            if (geometry1->IsSameStructureAndGeometry(*actualGeom))
                ASSERT_EQ(geometryCount[geometry1.get()], statement.GetValueInt(1)) << statement.GetECSql();
            else if (geometry2->IsSameStructureAndGeometry(*actualGeom))
                ASSERT_EQ(geometryCount[geometry2.get()], statement.GetValueInt(1)) << statement.GetECSql();
            else
                FAIL();
            }
        }
    ASSERT_EQ(3, rowCount) << statement.GetECSql();
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, GroupByStruct)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ECSqlStatementTests.ecdb", SchemaItem(R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                     <ECStructClass typeName="Person" >
                        <ECProperty propertyName="Name" typeName="string" />
                        <ECProperty propertyName="Age" typeName="int" />
                    </ECStructClass>
                      <ECEntityClass typeName="Foo" >
                        <ECProperty propertyName="Code" typeName="int" />
                        <ECStructProperty propertyName="Person" typeName="Person" />
                      </ECEntityClass>
                 </ECSchema>)xml")));

    ECSqlStatement insertStmt;
    ASSERT_EQ(ECSqlStatus::Success, insertStmt.Prepare(m_ecdb, "INSERT INTO ts.Foo(Code,Person.Name,Person.Age) VALUES(?,?,?)"));

    std::pair<Utf8CP, int> person1 = std::make_pair("John", 45);
    std::pair<Utf8CP, int> person2 = std::make_pair("Mary", 35);

    bmap<std::pair<Utf8CP, int>*, int> personCount;
    {
    ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindInt(1, 1));
    ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindText(2, person1.first, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindInt(3, person1.second));
    personCount[&person1] = 1;
    ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step());
    insertStmt.Reset();
    insertStmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindInt(1, 2));
    ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindText(2, person1.first, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindInt(3, person1.second));
    personCount[&person1]++;
    ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step());
    insertStmt.Reset();
    insertStmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindInt(1, 3));
    ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindText(2, person1.first, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindInt(3, person1.second));
    personCount[&person1]++;
    ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step());
    insertStmt.Reset();
    insertStmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindInt(1, 4));
    ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindText(2, person2.first, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindInt(3, person2.second));
    personCount[&person2] = 1;
    ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step());

    insertStmt.Reset();
    insertStmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindInt(1, 5));
    ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step());
    }

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT Person, count(*) FROM ts.Foo GROUP BY Person"));
    int rowCount = 0;
    while (BE_SQLITE_ROW == statement.Step())
        {
        rowCount++;
        if (statement.IsValueNull(0))
            ASSERT_EQ(1, statement.GetValueInt(1)) << statement.GetECSql();
        else
            {
            IECSqlValue const& actualPerson = statement.GetValue(0);
            if (strcmp(person1.first, actualPerson["Name"].GetText()) == 0)
                ASSERT_EQ(personCount[&person1], statement.GetValueInt(1)) << statement.GetECSql();
            else if (strcmp(person2.first, actualPerson["Name"].GetText()) == 0)
                ASSERT_EQ(personCount[&person2], statement.GetValueInt(1)) << statement.GetECSql();
            else
                FAIL();
            }
        }
    ASSERT_EQ(3, rowCount) << statement.GetECSql();
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, GroupByArray)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ECSqlStatementTests.ecdb", SchemaItem(R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                     <ECStructClass typeName="Person" >
                        <ECProperty propertyName="Name" typeName="string" />
                        <ECProperty propertyName="Age" typeName="int" />
                    </ECStructClass>
                      <ECEntityClass typeName="Foo" >
                        <ECStructArrayProperty propertyName="Persons" typeName="Person" />
                        <ECArrayProperty propertyName="Deadlines" typeName="dateTime" />
                        <ECArrayProperty propertyName="Vertexes" typeName="Point3d" />
                        <ECArrayProperty propertyName="Geometries" typeName="Bentley.Geometry.Common.IGeometry" />
                      </ECEntityClass>
                 </ECSchema>)xml")));

    const int nonNullRowCount = 3;
    ECSqlStatement insertStmt;
    ASSERT_EQ(ECSqlStatus::Success, insertStmt.Prepare(m_ecdb, "INSERT INTO ts.Foo(Persons,Deadlines,Vertexes,Geometries) VALUES(?,?,?,?)"));
    for (int i = 0; i < nonNullRowCount; i++)
        {
                {
                IECSqlBinder& binder = insertStmt.GetBinder(1);
                IECSqlBinder& element1Binder = binder.AddArrayElement();
                ASSERT_EQ(ECSqlStatus::Success, element1Binder["Name"].BindText("John", IECSqlBinder::MakeCopy::Yes));
                ASSERT_EQ(ECSqlStatus::Success, element1Binder["Age"].BindInt(40));
                IECSqlBinder& element2Binder = binder.AddArrayElement();
                ASSERT_EQ(ECSqlStatus::Success, element2Binder["Name"].BindText("Mary", IECSqlBinder::MakeCopy::Yes));
                ASSERT_EQ(ECSqlStatus::Success, element2Binder["Age"].BindInt(35));
                }

                {
                IECSqlBinder& binder = insertStmt.GetBinder(2);
                ASSERT_EQ(ECSqlStatus::Success, binder.AddArrayElement().BindDateTime(DateTime(2017, 07, 27)));
                ASSERT_EQ(ECSqlStatus::Success, binder.AddArrayElement().BindDateTime(DateTime(1971, 04, 30)));
                }

                {
                IECSqlBinder& binder = insertStmt.GetBinder(3);
                ASSERT_EQ(ECSqlStatus::Success, binder.AddArrayElement().BindPoint3d(DPoint3d::From(1.0, 2.0, 3.0)));
                ASSERT_EQ(ECSqlStatus::Success, binder.AddArrayElement().BindPoint3d(DPoint3d::From(-1.0, -2.0, -3.0)));
                }

                {
                IGeometryPtr line = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, 1.0, 1.0, 1.0)));
                IECSqlBinder& binder = insertStmt.GetBinder(4);
                ASSERT_EQ(ECSqlStatus::Success, binder.AddArrayElement().BindGeometry(*line));
                ASSERT_EQ(ECSqlStatus::Success, binder.AddArrayElement().BindGeometry(*line));
                ASSERT_EQ(ECSqlStatus::Success, binder.AddArrayElement().BindGeometry(*line));
                }

        ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step());
        insertStmt.Reset();
        insertStmt.ClearBindings();
        }

    //insert an empty row
    ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step());
    insertStmt.Finalize();

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT Persons, count(*) FROM ts.Foo GROUP BY Persons"));
    int rowCount = 0;
    while (BE_SQLITE_ROW == statement.Step())
        {
        rowCount++;
        if (statement.IsValueNull(0))
            ASSERT_EQ(1, statement.GetValueInt(1)) << statement.GetECSql();
        else
            ASSERT_EQ(nonNullRowCount, statement.GetValueInt(1)) << statement.GetECSql();
        }
    ASSERT_EQ(2, rowCount) << statement.GetECSql();
    }
    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT Deadlines, count(*) FROM ts.Foo GROUP BY Deadlines"));
    int rowCount = 0;
    while (BE_SQLITE_ROW == statement.Step())
        {
        rowCount++;
        if (statement.IsValueNull(0))
            ASSERT_EQ(1, statement.GetValueInt(1)) << statement.GetECSql();
        else
            ASSERT_EQ(nonNullRowCount, statement.GetValueInt(1)) << statement.GetECSql();
        }
    ASSERT_EQ(2, rowCount) << statement.GetECSql();
    }
    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT Vertexes, count(*) FROM ts.Foo GROUP BY Vertexes"));
    int rowCount = 0;
    while (BE_SQLITE_ROW == statement.Step())
        {
        rowCount++;
        if (statement.IsValueNull(0))
            ASSERT_EQ(1, statement.GetValueInt(1)) << statement.GetECSql();
        else
            ASSERT_EQ(nonNullRowCount, statement.GetValueInt(1)) << statement.GetECSql();
        }
    ASSERT_EQ(2, rowCount) << statement.GetECSql();
    }
    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT Geometries, count(*) FROM ts.Foo GROUP BY Geometries"));
    int rowCount = 0;
    while (BE_SQLITE_ROW == statement.Step())
        {
        rowCount++;
        if (statement.IsValueNull(0))
            ASSERT_EQ(1, statement.GetValueInt(1)) << statement.GetECSql();
        else
            ASSERT_EQ(nonNullRowCount, statement.GetValueInt(1)) << statement.GetECSql();
        }
    ASSERT_EQ(2, rowCount) << statement.GetECSql();
    }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, VerifyLiteralExpressionAsConstants)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ECSqlStatementTests.ecdb", SchemaItem::CreateForFile("ECSqlStatementTests.01.00.00.ecschema.xml")));

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ECST.Product (UnitPrice, ProductAvailable, ProductName) VALUES(100*5, true, 'Chair')"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ECST.Product (UnitPrice, ProductAvailable, ProductName) VALUES(100+1+ECClassId, true, 'Chair')"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ECST.Product (UnitPrice, ProductAvailable, ProductName) VALUES(1000/5, false, 'Table')"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ECST.Product (UnitPrice, ProductAvailable, ProductName) VALUES(100+2+ECClassId, false, 'Table')"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ECST.Product (UnitPrice, ProductAvailable, ProductName) VALUES(1000+100*5, true, 'LCD')"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT ProductName FROM ECST.Product WHERE UnitPrice=100+2+ECClassId"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_STREQ("Table", statement.GetValueText(0));
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT AVG(UnitPrice) FROM ECST.Product WHERE UnitPrice>ECClassId AND ProductName='Chair'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT ProductAvailable FROM ECST.Product WHERE UnitPrice>100+2+ECClassId AND ProductName='LCD'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_TRUE(statement.GetValueBoolean(0));
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT ProductAvailable FROM ECST.Product WHERE UnitPrice>100+2+ECClassId AND ProductName='LCD'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_TRUE(statement.GetValueBoolean(0));
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT ProductAvailable FROM ECST.Product WHERE UnitPrice=100+3+ECClassId OR ProductName='LCD'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_TRUE(statement.GetValueBoolean(0));
    statement.Finalize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, WrapWhereClauseInParams)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ECSqlStatementTests.ecdb", SchemaItem::CreateForFile("ECSqlStatementTests.01.00.00.ecschema.xml")));
    NestedStructArrayTestSchemaHelper::PopulateECSqlStatementTestsDb(m_ecdb);

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT Phone FROM ECST.Customer WHERE Country='USA' OR Company='ABC'"));
    Utf8String nativeSql = statement.GetNativeSql();
    ASSERT_TRUE(nativeSql.find("WHERE ([Customer].[Country]='USA' OR [Customer].[Company]='ABC')") != nativeSql.npos);
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT Phone FROM ECST.Customer WHERE Country='USA' AND Company='ABC'"));
    nativeSql = statement.GetNativeSql();
    ASSERT_TRUE(nativeSql.find("WHERE [Customer].[Country]='USA' AND [Customer].[Company]='ABC'") != nativeSql.npos);
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT Phone FROM ECST.Customer WHERE Country='USA' OR Country='DUBAI' AND ContactTitle='AM'"));
    nativeSql = statement.GetNativeSql();
    ASSERT_TRUE(nativeSql.find("WHERE ([Customer].[Country]='USA' OR [Customer].[Country]='DUBAI' AND [Customer].[ContactTitle]='AM')") != nativeSql.npos);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, HexLiteral)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("HexLiteral.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECEntityClass typeName="Sample" modifier="None">
                    <ECProperty propertyName="StringProp" typeName="string" />
                </ECEntityClass>
              </ECSchema>)xml")));

    ECInstanceKey actualKey;
    ECInstanceId expectedECInstanceId(UINT64_C(0x7FFFFFFFFFFFFFFF));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(actualKey, "INSERT INTO ts.Sample(ECInstanceId, StringProp) VALUES (0x7FFFFFFFFFFFFFFF, '0x7FFFFFFFFFFFFFFF')"));
    ASSERT_EQ(expectedECInstanceId, actualKey.GetInstanceId());

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT StringProp FROM ts.Sample WHERE ECInstanceId = 0x7FFFFFFFFFFFFFFF"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_STREQ("0x7FFFFFFFFFFFFFFF", stmt.GetValueText(0));

    expectedECInstanceId = ECInstanceId(UINT64_C(0x7ABCDEF) + 39421 - 0x43);
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(actualKey, "INSERT INTO ts.Sample(ECInstanceId, StringProp) VALUES (0x7ABCDEF + 39421 - 0x43, '0x7ABCDEF + 39421 - 0x43')"));
    ASSERT_EQ(expectedECInstanceId, actualKey.GetInstanceId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, BindECInstanceIdAsString)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("BindIdStrings.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECEntityClass typeName="Foo" modifier="None">
                    <ECProperty propertyName="Name" typeName="string" />
                </ECEntityClass>
              </ECSchema>)xml")));

    ECInstanceKey decimalKey, decimalStrKey, hexStrKey;
    ECSqlStatement insertStmt;
    ASSERT_EQ(ECSqlStatus::Success, insertStmt.Prepare(m_ecdb, "INSERT INTO ts.Foo(ECInstanceId, Name) VALUES (?,'Foo')"));
    ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindInt64(1, INT64_C(10))) << insertStmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step(decimalKey)) << "Bound decimal";
    insertStmt.Reset();
    insertStmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindText(1, "11", IECSqlBinder::MakeCopy::No)) << insertStmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step(decimalStrKey)) << "Bound decimal string";
    insertStmt.Reset();
    insertStmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindText(1, "0x12", IECSqlBinder::MakeCopy::No)) << insertStmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step(hexStrKey)) << "Bound hex string";
    insertStmt.Reset();
    insertStmt.ClearBindings();

    ECSqlStatement selStmt;
    ASSERT_EQ(ECSqlStatus::Success, selStmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ts.Foo WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, selStmt.BindId(1, decimalKey.GetInstanceId())) << "Inserted as decimal, bound as decimal";
    ASSERT_EQ(BE_SQLITE_ROW, selStmt.Step()) << "Inserted as decimal, bound as decimal";
    ASSERT_EQ(decimalKey.GetInstanceId(), selStmt.GetValueId<ECInstanceId>(0)) << "Inserted as decimal, bound as decimal";
    selStmt.Reset();
    selStmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, selStmt.BindText(1, decimalKey.GetInstanceId().ToString().c_str(), IECSqlBinder::MakeCopy::Yes)) << "Inserted as decimal, bound as decimal string";
    ASSERT_EQ(BE_SQLITE_ROW, selStmt.Step()) << "Inserted as decimal, bound as decimal string";
    ASSERT_EQ(decimalKey.GetInstanceId(), selStmt.GetValueId<ECInstanceId>(0)) << "Inserted as decimal, bound as decimal string";
    selStmt.Reset();
    selStmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, selStmt.BindText(1, decimalKey.GetInstanceId().ToHexStr().c_str(), IECSqlBinder::MakeCopy::Yes)) << "Inserted as decimal, bound as hexadecimal string";
    ASSERT_EQ(BE_SQLITE_ROW, selStmt.Step()) << "Inserted as decimal, bound as hexadecimal string";
    ASSERT_EQ(decimalKey.GetInstanceId(), selStmt.GetValueId<ECInstanceId>(0)) << "Inserted as decimal, bound as hexadecimal string";
    selStmt.Reset();
    selStmt.ClearBindings();


    ASSERT_EQ(ECSqlStatus::Success, selStmt.BindId(1, decimalStrKey.GetInstanceId())) << "Inserted as decimal string, bound as decimal";
    ASSERT_EQ(BE_SQLITE_ROW, selStmt.Step()) << "Inserted as decimal string, bound as decimal";
    ASSERT_EQ(decimalStrKey.GetInstanceId(), selStmt.GetValueId<ECInstanceId>(0)) << "Inserted as decimal string, bound as decimal";
    selStmt.Reset();
    selStmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, selStmt.BindText(1, decimalStrKey.GetInstanceId().ToString().c_str(), IECSqlBinder::MakeCopy::Yes)) << "Inserted as decimal string, bound as decimal string";
    ASSERT_EQ(BE_SQLITE_ROW, selStmt.Step()) << "Inserted as decimal string, bound as decimal string";
    ASSERT_EQ(decimalStrKey.GetInstanceId(), selStmt.GetValueId<ECInstanceId>(0)) << "Inserted as decimal string, bound as decimal string";
    selStmt.Reset();
    selStmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, selStmt.BindText(1, decimalStrKey.GetInstanceId().ToHexStr().c_str(), IECSqlBinder::MakeCopy::Yes)) << "Inserted as decimal string, bound as hexadecimal string";
    ASSERT_EQ(BE_SQLITE_ROW, selStmt.Step()) << "Inserted as decimal string, bound as hexadecimal string";
    ASSERT_EQ(decimalStrKey.GetInstanceId(), selStmt.GetValueId<ECInstanceId>(0)) << "Inserted as decimal string, bound as hexadecimal string";
    selStmt.Reset();
    selStmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, selStmt.BindId(1, hexStrKey.GetInstanceId())) << "Inserted as hexadecimal string, bound as decimal";
    ASSERT_EQ(BE_SQLITE_ROW, selStmt.Step()) << "Inserted as hexadecimal string, bound as decimal";
    ASSERT_EQ(hexStrKey.GetInstanceId(), selStmt.GetValueId<ECInstanceId>(0)) << "Inserted as hexadecimal string, bound as decimal";
    selStmt.Reset();
    selStmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, selStmt.BindText(1, hexStrKey.GetInstanceId().ToString().c_str(), IECSqlBinder::MakeCopy::Yes)) << "Inserted as hexadecimal string, bound as decimal string";
    ASSERT_EQ(BE_SQLITE_ROW, selStmt.Step()) << "Inserted as hexadecimal string, bound as decimal string";
    ASSERT_EQ(hexStrKey.GetInstanceId(), selStmt.GetValueId<ECInstanceId>(0)) << "Inserted as hexadecimal string, bound as decimal string";
    selStmt.Reset();
    selStmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, selStmt.BindText(1, hexStrKey.GetInstanceId().ToHexStr().c_str(), IECSqlBinder::MakeCopy::Yes)) << "Inserted as hexadecimal string, bound as hexadecimal string";
    ASSERT_EQ(BE_SQLITE_ROW, selStmt.Step()) << "Inserted as hexadecimal string, bound as hexadecimal string";
    ASSERT_EQ(hexStrKey.GetInstanceId(), selStmt.GetValueId<ECInstanceId>(0)) << "Inserted as hexadecimal string, bound as hexadecimal string";
    selStmt.Reset();
    selStmt.ClearBindings();


    ASSERT_EQ(ECSqlStatus::Success, selStmt.BindText(1, "", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, selStmt.Step()) << "Bound empty string";
    selStmt.Reset();
    selStmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, selStmt.BindText(1, "x", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, selStmt.Step()) << "Bound invalid hex string: x";
    selStmt.Reset();
    selStmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, selStmt.BindText(1, "0x", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, selStmt.Step()) << "Bound invalid hex string: 0x";
    selStmt.Reset();
    selStmt.ClearBindings();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, BindIdStrings)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("BindIdStrings.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="Foo" modifier="None">
                    <ECProperty propertyName="i" typeName="int" />
                    <ECProperty propertyName="l" typeName="long" />
                    <ECProperty propertyName="fooId" typeName="long" extendedTypeName="Id" />
                    <ECStructProperty propertyName="struct" typeName="Struct"/>
                    <ECArrayProperty propertyName="iArray" typeName="int"/>
                    <ECArrayProperty propertyName="lArray" typeName="long"/>
                    <ECArrayProperty propertyName="idArray" typeName="long" extendedTypeName="Id" />
                    <ECStructArrayProperty propertyName="structArray" typeName="Struct"/>
                </ECEntityClass>
                <ECStructClass typeName="Struct" modifier="None">
                    <ECProperty propertyName="i" typeName="int" />
                    <ECProperty propertyName="l" typeName="long" />
                    <ECProperty propertyName="fooId" typeName="long" extendedTypeName="Id" />
                </ECStructClass>
              </ECSchema>)xml")));

    constexpr int i = 20;
    constexpr int64_t l = INT64_C(20);
    BeInt64Id id(l);
    Utf8CP iStr = "20";
    Utf8CP iHexStr = "0x14";

    ECInstanceKey key;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Foo(i,l,fooId,struct,iArray,lArray,idArray,structArray) VALUES (20,20,20,?,?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(1)["i"].BindInt(i)) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(1)["l"].BindInt64(l)) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(1)["fooId"].BindId(id)) << stmt.GetECSql();

    // add 2 elements for each array prop
    for (int j = 0; j < 2; j++)
        {
        ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(2).AddArrayElement().BindInt(i)) << stmt.GetECSql();
        ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(3).AddArrayElement().BindInt64(l)) << stmt.GetECSql();
        ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(4).AddArrayElement().BindId(id)) << stmt.GetECSql();

        IECSqlBinder& structArrayElementBinder = stmt.GetBinder(5).AddArrayElement();
        ASSERT_EQ(ECSqlStatus::Success, structArrayElementBinder["i"].BindInt(i)) << stmt.GetECSql();
        ASSERT_EQ(ECSqlStatus::Success, structArrayElementBinder["l"].BindInt64(l)) << stmt.GetECSql();
        ASSERT_EQ(ECSqlStatus::Success, structArrayElementBinder["fooId"].BindId(id)) << stmt.GetECSql();
        }

    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << stmt.GetECSql();
    }

    // test that data was correctly inserted
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT i,l,fooId,struct,iArray,lArray,idArray,structArray FROM ts.Foo WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, key.GetInstanceId())) << stmt.GetECSql();
    EXPECT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    EXPECT_EQ(i, stmt.GetValueInt(0)) << stmt.GetECSql();
    EXPECT_EQ(l, stmt.GetValueInt64(1)) << stmt.GetECSql();
    EXPECT_EQ(id, stmt.GetValueId<BeInt64Id>(2)) << stmt.GetECSql();

    EXPECT_EQ(i, stmt.GetValue(3)["i"].GetInt()) << stmt.GetECSql();
    EXPECT_EQ(l, stmt.GetValue(3)["l"].GetInt64()) << stmt.GetECSql();
    EXPECT_EQ(id, stmt.GetValue(3)["fooId"].GetId<BeInt64Id>()) << stmt.GetECSql();

    EXPECT_EQ(2, stmt.GetValue(4).GetArrayLength()) << stmt.GetECSql();
    for (IECSqlValue const& arrayElementVal : stmt.GetValue(4).GetArrayIterable())
        {
        EXPECT_EQ(i, arrayElementVal.GetInt()) << stmt.GetECSql();
        }

    EXPECT_EQ(2, stmt.GetValue(5).GetArrayLength()) << stmt.GetECSql();
    for (IECSqlValue const& arrayElementVal : stmt.GetValue(5).GetArrayIterable())
        {
        EXPECT_EQ(l, arrayElementVal.GetInt64()) << stmt.GetECSql();
        }

    EXPECT_EQ(2, stmt.GetValue(6).GetArrayLength()) << stmt.GetECSql();
    for (IECSqlValue const& arrayElementVal : stmt.GetValue(6).GetArrayIterable())
        {
        EXPECT_EQ(id, arrayElementVal.GetId<BeInt64Id>()) << stmt.GetECSql();
        }

    EXPECT_EQ(2, stmt.GetValue(7).GetArrayLength()) << stmt.GetECSql();
    for (IECSqlValue const& arrayElementVal : stmt.GetValue(7).GetArrayIterable())
        {
        EXPECT_EQ(i, arrayElementVal["i"].GetInt()) << stmt.GetECSql();
        EXPECT_EQ(l, arrayElementVal["l"].GetInt64()) << stmt.GetECSql();
        EXPECT_EQ(id, arrayElementVal["fooId"].GetId<BeInt64Id>()) << stmt.GetECSql();
        }
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT 1 FROM ts.Foo WHERE i=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, i));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindInt(" << i << ")";
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(1, l));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindInt64(" << l << ")";
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, id));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindId(" << id.ToString() << ")";
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, iStr, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | BindText(" << iStr << ")";
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindText(" << iStr << ")";
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, iHexStr, IECSqlBinder::MakeCopy::No)) << " | BindText(" << iHexStr << ") | Hex strings only parsed for id props";
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << " | BindText(" << iHexStr << ") | Hex strings only parsed for id props";
    stmt.Reset();
    stmt.ClearBindings();
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT 1 FROM ts.Foo WHERE l=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, i));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindInt(" << i << ")";
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(1, l));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindInt64(" << l << ")";
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, id));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindId(" << id.ToString() << ")";
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, iStr, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | BindText(" << iStr << ")";
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindText(" << iStr << ")";
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, iHexStr, IECSqlBinder::MakeCopy::No)) << " | BindText(" << iHexStr << ") | Hex strings only parsed for id props";
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << " | BindText(" << iHexStr << ") | Hex strings only parsed for id props";
    stmt.Reset();
    stmt.ClearBindings();
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT 1 FROM ts.Foo WHERE fooId=?"));

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, id));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindId(" << id.ToString() << ")";
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, i));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindInt(" << i << ")";
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(1, l));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindInt64(" << l << ")";
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, iStr, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | BindText(" << iStr << ")";
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindText(" << iStr << ")";
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, iHexStr, IECSqlBinder::MakeCopy::No)) << " | BindText(" << iHexStr << ") | Hex strings only parsed for id props";
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindText(" << iHexStr << ")";
    stmt.Reset();
    stmt.ClearBindings();
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT 1 FROM ts.Foo WHERE struct.i=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, i));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindInt(" << i << ")";
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(1, l));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindInt64(" << l << ")";
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, iStr, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | BindText(" << iStr << ")";
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindText(" << iStr << ")";
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, iHexStr, IECSqlBinder::MakeCopy::No)) << " | BindText(" << iHexStr << ") | Hex strings only parsed for id props";
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << " | BindText(" << iHexStr << ") | Hex strings only parsed for id props";
    stmt.Reset();
    stmt.ClearBindings();
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT 1 FROM ts.Foo WHERE struct.l=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, i));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindInt(" << i << ")";
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(1, l));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindInt64(" << l << ")";
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, id));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindId(" << id.ToString() << ")";
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, iStr, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | BindText(" << iStr << ")";
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindText(" << iStr << ")";
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, iHexStr, IECSqlBinder::MakeCopy::No)) << " | BindText(" << iHexStr << ") | Hex strings only parsed for id props";
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << " | BindText(" << iHexStr << ") | Hex strings only parsed for id props";
    stmt.Reset();
    stmt.ClearBindings();
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT 1 FROM ts.Foo WHERE struct.fooId=?"));

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, id));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindId(" << id.ToString() << ")";
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, i));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindInt(" << i << ")";
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(1, l));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindInt64(" << l << ")";
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, iStr, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | BindText(" << iStr << ")";
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindText(" << iStr << ")";
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, iHexStr, IECSqlBinder::MakeCopy::No)) << " | BindText(" << iHexStr << ") | Hex strings only parsed for id props";
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindText(" << iHexStr << ")";
    stmt.Reset();
    stmt.ClearBindings();
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT 1 FROM ts.Foo WHERE iArray=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(1).AddArrayElement().BindInt(i));
    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(1).AddArrayElement().BindInt(i));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindInt(" << i << ")";
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(1).AddArrayElement().BindInt64(l));
    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(1).AddArrayElement().BindInt64(l));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindInt64(" << l << ")";
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(1).AddArrayElement().BindId(id));
    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(1).AddArrayElement().BindId(id));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindId(" << id.ToString() << ")";
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(1).AddArrayElement().BindText(iStr, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | BindText(" << iStr << ")";
    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(1).AddArrayElement().BindText(iStr, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | BindText(" << iStr << ")";
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << " | BindText(" << iStr << ") | For arrays we cannot leverage SQLite's type comparisons, so strings are not the same as numbers in arrays.";
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(1).AddArrayElement().BindText(iHexStr, IECSqlBinder::MakeCopy::No)) << " | BindText(" << iHexStr << ") | Hex strings only parsed for id props";
    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(1).AddArrayElement().BindText(iHexStr, IECSqlBinder::MakeCopy::No)) << " | BindText(" << iHexStr << ") | Hex strings only parsed for id props";
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << " | BindText(" << iHexStr << ") | hex strings are only parsed for Id props";
    stmt.Reset();
    stmt.ClearBindings();
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT 1 FROM ts.Foo WHERE lArray=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(1).AddArrayElement().BindInt(i));
    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(1).AddArrayElement().BindInt(i));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindInt(" << i << ")";
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(1).AddArrayElement().BindInt64(l));
    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(1).AddArrayElement().BindInt64(l));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindInt64(" << l << ")";
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(1).AddArrayElement().BindId(id));
    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(1).AddArrayElement().BindId(id));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindId(" << id.ToString() << ")";
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(1).AddArrayElement().BindText(iStr, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | BindText(" << iStr << ")";
    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(1).AddArrayElement().BindText(iStr, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | BindText(" << iStr << ")";
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << " | BindText(" << iStr << ") | For arrays we cannot leverage SQLite's type comparisons, so strings are not the same as numbers in arrays.";
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(1).AddArrayElement().BindText(iHexStr, IECSqlBinder::MakeCopy::No)) << " | BindText(" << iHexStr << ") | Hex strings only parsed for id props";
    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(1).AddArrayElement().BindText(iHexStr, IECSqlBinder::MakeCopy::No)) << " | BindText(" << iHexStr << ") | Hex strings only parsed for id props";
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << " | BindText(" << iHexStr << ") | hex strings are only parsed for Id props";
    stmt.Reset();
    stmt.ClearBindings();
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT 1 FROM ts.Foo WHERE idArray=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(1).AddArrayElement().BindInt(i));
    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(1).AddArrayElement().BindInt(i));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindInt(" << i << ")";
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(1).AddArrayElement().BindInt64(l));
    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(1).AddArrayElement().BindInt64(l));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindInt64(" << l << ")";
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(1).AddArrayElement().BindId(id));
    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(1).AddArrayElement().BindId(id));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindId(" << id.ToString() << ")";
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(1).AddArrayElement().BindText(iStr, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | BindText(" << iStr << ")";
    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(1).AddArrayElement().BindText(iStr, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | BindText(" << iStr << ")";
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindText(" << iStr << ").";
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(1).AddArrayElement().BindText(iHexStr, IECSqlBinder::MakeCopy::No)) << " | BindText(" << iHexStr << ") | Hex strings only parsed for id props";
    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(1).AddArrayElement().BindText(iHexStr, IECSqlBinder::MakeCopy::No)) << " | BindText(" << iHexStr << ") | Hex strings only parsed for id props";
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindText(" << iHexStr << ")";
    stmt.Reset();
    stmt.ClearBindings();
    }

    }

//-------------------------------------------------------------------------------------- -
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, BindDateTimeStrings)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("BindIdStrings.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
              <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA"/>
                <ECEntityClass typeName="Foo" modifier="None">
                    <ECProperty propertyName="dt" typeName="dateTime" />
                    <ECProperty propertyName="dtUtc" typeName="dateTime">
                      <ECCustomAttributes>
                        <DateTimeInfo xmlns="CoreCustomAttributes.01.00.00">
                            <DateTimeKind>Utc</DateTimeKind>
                        </DateTimeInfo>
                      </ECCustomAttributes>
                    </ECProperty>
                    <ECProperty propertyName="d" typeName="dateTime">
                      <ECCustomAttributes>
                          <DateTimeInfo xmlns="CoreCustomAttributes.01.00.00">
                              <DateTimeComponent>Date</DateTimeComponent>
                          </DateTimeInfo>
                      </ECCustomAttributes>
                    </ECProperty>
                    <ECProperty propertyName="s" typeName="string" />
                    <ECStructProperty propertyName="struct" typeName="Struct"/>
                    <ECArrayProperty propertyName="dtArray" typeName="dateTime"/>
                    <ECArrayProperty propertyName="dtUtcArray" typeName="dateTime">
                      <ECCustomAttributes>
                          <DateTimeInfo xmlns="CoreCustomAttributes.01.00.00">
                              <DateTimeKind>Utc</DateTimeKind>
                          </DateTimeInfo>
                      </ECCustomAttributes>
                    </ECArrayProperty>
                    <ECArrayProperty propertyName="dArray" typeName="dateTime">
                      <ECCustomAttributes>
                          <DateTimeInfo xmlns="CoreCustomAttributes.01.00.00">
                              <DateTimeComponent>Date</DateTimeComponent>
                          </DateTimeInfo>
                      </ECCustomAttributes>
                    </ECArrayProperty>
                    <ECArrayProperty propertyName="sArray" typeName="string" />
                    <ECStructArrayProperty propertyName="structArray" typeName="Struct"/>
                </ECEntityClass>
                <ECStructClass typeName="Struct" modifier="None">
                    <ECProperty propertyName="dt" typeName="dateTime" />
                    <ECProperty propertyName="dtUtc" typeName="dateTime">
                      <ECCustomAttributes>
                          <DateTimeInfo xmlns="CoreCustomAttributes.01.00.00">
                              <DateTimeKind>Utc</DateTimeKind>
                          </DateTimeInfo>
                      </ECCustomAttributes>
                    </ECProperty>
                    <ECProperty propertyName="d" typeName="dateTime">
                      <ECCustomAttributes>
                          <DateTimeInfo xmlns="CoreCustomAttributes.01.00.00">
                              <DateTimeComponent>Date</DateTimeComponent>
                          </DateTimeInfo>
                      </ECCustomAttributes>
                    </ECProperty>
                    <ECProperty propertyName="s" typeName="string" />
                </ECStructClass>
              </ECSchema>)xml")));

    Utf8CP dtStr = "2018-10-26T12:00:00";
    Utf8CP dtUtcStr = "2018-10-26T12:00:00Z";
    Utf8CP dStr = "2018-10-26";
    Utf8CP str = dtStr;

    DateTime dt, dtUtc, d;
    ASSERT_EQ(SUCCESS, DateTime::FromString(dt, dtStr));
    ASSERT_EQ(SUCCESS, DateTime::FromString(dtUtc, dtUtcStr));
    ASSERT_EQ(SUCCESS, DateTime::FromString(d, dStr));

    ECInstanceKey key;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Foo(dt,dtUtc,d,s,struct,dtArray,dtUtcArray,dArray,sArray,structArray) VALUES (?,?,?,?,?,?,?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDateTime(1, dt)) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDateTime(2, dtUtc)) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDateTime(3, d)) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(4, str, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql();

    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(5)["dt"].BindDateTime(dt)) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(5)["dtUtc"].BindDateTime(dtUtc)) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(5)["d"].BindDateTime(d)) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(5)["s"].BindText(str, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql();

    // add 2 elements for each array prop
    for (int j = 0; j < 2; j++)
        {
        ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(6).AddArrayElement().BindDateTime(dt)) << stmt.GetECSql();
        ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(7).AddArrayElement().BindDateTime(dtUtc)) << stmt.GetECSql();
        ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(8).AddArrayElement().BindDateTime(d)) << stmt.GetECSql();
        ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(9).AddArrayElement().BindText(str, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql();

        IECSqlBinder& structArrayElementBinder = stmt.GetBinder(10).AddArrayElement();
        ASSERT_EQ(ECSqlStatus::Success, structArrayElementBinder["dt"].BindDateTime(dt)) << stmt.GetECSql();
        ASSERT_EQ(ECSqlStatus::Success, structArrayElementBinder["dtUtc"].BindDateTime(dtUtc)) << stmt.GetECSql();
        ASSERT_EQ(ECSqlStatus::Success, structArrayElementBinder["d"].BindDateTime(d)) << stmt.GetECSql();
        ASSERT_EQ(ECSqlStatus::Success, structArrayElementBinder["s"].BindText(str, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql();
        }

    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << stmt.GetECSql();
    }

    // test that data was correctly inserted
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT dt,dtUtc,d,s,struct,dtArray,dtUtcArray,dArray,sArray,structArray FROM ts.Foo WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, key.GetInstanceId())) << stmt.GetECSql();
    EXPECT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();

    EXPECT_EQ(dt, stmt.GetValueDateTime(0)) << stmt.GetECSql();
    EXPECT_EQ(dtUtc, stmt.GetValueDateTime(1)) << stmt.GetECSql();
    EXPECT_EQ(d, stmt.GetValueDateTime(2)) << stmt.GetECSql();
    EXPECT_STREQ(str, stmt.GetValueText(3)) << stmt.GetECSql();

    EXPECT_EQ(dt, stmt.GetValue(4)["dt"].GetDateTime()) << stmt.GetECSql();
    EXPECT_EQ(dtUtc, stmt.GetValue(4)["dtUtc"].GetDateTime()) << stmt.GetECSql();
    EXPECT_EQ(d, stmt.GetValue(4)["d"].GetDateTime()) << stmt.GetECSql();
    EXPECT_STREQ(str, stmt.GetValue(4)["s"].GetText()) << stmt.GetECSql();

    EXPECT_EQ(2, stmt.GetValue(5).GetArrayLength()) << stmt.GetECSql();
    for (IECSqlValue const& arrayElementVal : stmt.GetValue(5).GetArrayIterable())
        {
        EXPECT_EQ(dt, arrayElementVal.GetDateTime()) << stmt.GetECSql();
        }

    EXPECT_EQ(2, stmt.GetValue(6).GetArrayLength()) << stmt.GetECSql();
    for (IECSqlValue const& arrayElementVal : stmt.GetValue(6).GetArrayIterable())
        {
        EXPECT_EQ(dtUtc, arrayElementVal.GetDateTime()) << stmt.GetECSql();
        }

    EXPECT_EQ(2, stmt.GetValue(7).GetArrayLength()) << stmt.GetECSql();
    for (IECSqlValue const& arrayElementVal : stmt.GetValue(7).GetArrayIterable())
        {
        EXPECT_EQ(d, arrayElementVal.GetDateTime()) << stmt.GetECSql();
        }

    EXPECT_EQ(2, stmt.GetValue(8).GetArrayLength()) << stmt.GetECSql();
    for (IECSqlValue const& arrayElementVal : stmt.GetValue(8).GetArrayIterable())
        {
        EXPECT_STREQ(str, arrayElementVal.GetText()) << stmt.GetECSql();
        }

    EXPECT_EQ(2, stmt.GetValue(9).GetArrayLength()) << stmt.GetECSql();
    for (IECSqlValue const& arrayElementVal : stmt.GetValue(9).GetArrayIterable())
        {
        EXPECT_EQ(dt, arrayElementVal["dt"].GetDateTime()) << stmt.GetECSql();
        EXPECT_EQ(dtUtc, arrayElementVal["dtUtc"].GetDateTime()) << stmt.GetECSql();
        EXPECT_EQ(d, arrayElementVal["d"].GetDateTime()) << stmt.GetECSql();
        EXPECT_STREQ(str, arrayElementVal["s"].GetText()) << stmt.GetECSql();
        }
    }

    {
    for (Utf8CP ecsql : std::vector<Utf8CP>({"SELECT 1 FROM ts.Foo WHERE dt=?", "SELECT 1 FROM ts.Foo WHERE struct.dt=?"}))
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindDateTime(1, dt)) << stmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindDateTime(" << dt.ToString() << ")";
        stmt.Reset();
        stmt.ClearBindings();

        ASSERT_EQ(ECSqlStatus::Success, stmt.BindDateTime(1, dtUtc)) << stmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindDateTime(" << dtUtc.ToString() << ")";
        stmt.Reset();
        stmt.ClearBindings();

        ASSERT_EQ(ECSqlStatus::Success, stmt.BindDateTime(1, d)) << stmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << " | BindDateTime(" << d.ToString() << ")";
        stmt.Reset();
        stmt.ClearBindings();

        ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, dt.ToString().c_str(), IECSqlBinder::MakeCopy::Yes)) << stmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindText(" << dt.ToString() << ") | date strings are parsed for date time props";
        stmt.Reset();
        stmt.ClearBindings();

        ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, dtUtc.ToString().c_str(), IECSqlBinder::MakeCopy::Yes)) << stmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindText(" << dtUtc.ToString() << ")";
        stmt.Reset();
        stmt.ClearBindings();

        ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, d.ToString().c_str(), IECSqlBinder::MakeCopy::Yes)) << stmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << " | BindText(" << d.ToString() << ")";
        stmt.Reset();
        stmt.ClearBindings();

        ASSERT_EQ(ECSqlStatus::Error, stmt.BindText(1, "Foo text", IECSqlBinder::MakeCopy::No)) << stmt.GetECSql();
        stmt.Reset();
        stmt.ClearBindings();
        }
    }

    {
    for (Utf8CP ecsql : std::vector<Utf8CP>({"SELECT 1 FROM ts.Foo WHERE dtUtc=?", "SELECT 1 FROM ts.Foo WHERE struct.dtUtc=?"}))
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));

        ASSERT_EQ(ECSqlStatus::Success, stmt.BindDateTime(1, dtUtc)) << stmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindDateTime(" << dtUtc.ToString() << ")";
        stmt.Reset();
        stmt.ClearBindings();

        ASSERT_EQ(ECSqlStatus::Error, stmt.BindDateTime(1, dt)) << stmt.GetECSql();
        stmt.Reset();
        stmt.ClearBindings();

        ASSERT_EQ(ECSqlStatus::Success, stmt.BindDateTime(1, d)) << stmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << " | BindText(" << d.ToString() << ")";
        stmt.Reset();
        stmt.ClearBindings();

        ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, dtUtc.ToString().c_str(), IECSqlBinder::MakeCopy::Yes)) << stmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindText(" << dtUtc.ToString() << ")";
        stmt.Reset();
        stmt.ClearBindings();

        ASSERT_EQ(ECSqlStatus::Error, stmt.BindText(1, dt.ToString().c_str(), IECSqlBinder::MakeCopy::Yes)) << stmt.GetECSql();
        stmt.Reset();
        stmt.ClearBindings();

        ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, d.ToString().c_str(), IECSqlBinder::MakeCopy::Yes)) << stmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << " | BindText(" << str << ") | date strings are parsed for date time props";
        stmt.Reset();
        stmt.ClearBindings();

        ASSERT_EQ(ECSqlStatus::Error, stmt.BindText(1, "Foo text", IECSqlBinder::MakeCopy::No)) << stmt.GetECSql();
        stmt.Reset();
        stmt.ClearBindings();
        }
    }

    {
    for (Utf8CP ecsql : std::vector<Utf8CP>({"SELECT 1 FROM ts.Foo WHERE d=?", "SELECT 1 FROM ts.Foo WHERE struct.d=?"}))
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));

        ASSERT_EQ(ECSqlStatus::Success, stmt.BindDateTime(1, d)) << stmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindDateTime(" << dtUtc.ToString() << ")";
        stmt.Reset();
        stmt.ClearBindings();

        ASSERT_EQ(ECSqlStatus::Success, stmt.BindDateTime(1, dt)) << stmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << " | BindDateTime(" << dt.ToString() << ")";
        stmt.Reset();
        stmt.ClearBindings();

        ASSERT_EQ(ECSqlStatus::Success, stmt.BindDateTime(1, dtUtc)) << stmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << " | BindDateTime(" << dtUtc.ToString() << ")";
        stmt.Reset();
        stmt.ClearBindings();

        ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, d.ToString().c_str(), IECSqlBinder::MakeCopy::Yes)) << stmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindText(" << d.ToString() << ")";
        stmt.Reset();
        stmt.ClearBindings();

        ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, dt.ToString().c_str(), IECSqlBinder::MakeCopy::Yes)) << stmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << " | BindText(" << dt.ToString() << ")";
        stmt.Reset();
        stmt.ClearBindings();

        ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, dtUtc.ToString().c_str(), IECSqlBinder::MakeCopy::Yes)) << stmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << " | BindText(" << dtUtc.ToString() << ")";
        stmt.Reset();
        stmt.ClearBindings();

        ASSERT_EQ(ECSqlStatus::Error, stmt.BindText(1, "Foo text", IECSqlBinder::MakeCopy::No)) << stmt.GetECSql();
        stmt.Reset();
        stmt.ClearBindings();
        }
    }

    //************
    {
    for (Utf8CP ecsql : std::vector<Utf8CP>({"SELECT 1 FROM ts.Foo WHERE s=?", "SELECT 1 FROM ts.Foo WHERE struct.s=?"}))
        {

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));

        ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, str, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindText(" << str << ")";
        stmt.Reset();
        stmt.ClearBindings();

        ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, dt.ToString().c_str(), IECSqlBinder::MakeCopy::Yes)) << stmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << " | BindText(" << dt.ToString() << ")";
        stmt.Reset();
        stmt.ClearBindings();

        ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, dtUtc.ToString().c_str(), IECSqlBinder::MakeCopy::Yes)) << stmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << " | BindText(" << dtUtc.ToString() << ")";
        stmt.Reset();
        stmt.ClearBindings();

        ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, d.ToString().c_str(), IECSqlBinder::MakeCopy::Yes)) << stmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << " | BindText(" << d.ToString() << ")";
        stmt.Reset();
        stmt.ClearBindings();

        ASSERT_EQ(ECSqlStatus::Error, stmt.BindDateTime(1, dt)) << stmt.GetECSql();
        ASSERT_EQ(ECSqlStatus::Error, stmt.BindDateTime(1, dtUtc)) << stmt.GetECSql();
        ASSERT_EQ(ECSqlStatus::Error, stmt.BindDateTime(1, d)) << stmt.GetECSql();
        stmt.Reset();
        stmt.ClearBindings();
        }
    }

    //************
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT 1 FROM ts.Foo WHERE dtArray=?"));
    {
    IECSqlBinder& arrayBinder = stmt.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindDateTime(dt)) << stmt.GetECSql() << " | BindDateTime(" << dt.ToString() << ")";
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindDateTime(dt)) << stmt.GetECSql() << " | BindDateTime(" << dt.ToString() << ")";
    EXPECT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindDateTime(" << dt.ToString() << ")";
    stmt.Reset();
    stmt.ClearBindings();
    }

    {
    IECSqlBinder& arrayBinder = stmt.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindDateTime(dtUtc)) << stmt.GetECSql() << " | BindDateTime(" << dtUtc.ToString() << ")";
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindDateTime(dtUtc)) << stmt.GetECSql() << " | BindDateTime(" << dtUtc.ToString() << ")";
    EXPECT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindDateTime(" << dtUtc.ToString() << ")";
    stmt.Reset();
    stmt.ClearBindings();
    }

    {
    IECSqlBinder& arrayBinder = stmt.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindDateTime(d)) << stmt.GetECSql() << " | BindDateTime(" << d.ToString() << ")";
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindDateTime(d)) << stmt.GetECSql() << " | BindDateTime(" << d.ToString() << ")";
    EXPECT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << " | BindDateTime(" << d.ToString() << ")";
    stmt.Reset();
    stmt.ClearBindings();
    }

    {
    IECSqlBinder& arrayBinder = stmt.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindText(dt.ToString().c_str(), IECSqlBinder::MakeCopy::Yes)) << stmt.GetECSql() << " | BindText(" << dt.ToString() << ")";
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindText(dt.ToString().c_str(), IECSqlBinder::MakeCopy::Yes)) << stmt.GetECSql() << " | BindText(" << dt.ToString() << ")";
    EXPECT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindText(" << dt.ToString() << ")";
    stmt.Reset();
    stmt.ClearBindings();
    }

    {
    IECSqlBinder& arrayBinder = stmt.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindText(dtUtc.ToString().c_str(), IECSqlBinder::MakeCopy::Yes)) << stmt.GetECSql() << " | BindText(" << dtUtc.ToString() << ")";
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindText(dtUtc.ToString().c_str(), IECSqlBinder::MakeCopy::Yes)) << stmt.GetECSql() << " | BindText(" << dtUtc.ToString() << ")";
    EXPECT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindText(" << dtUtc.ToString() << ")";
    stmt.Reset();
    stmt.ClearBindings();
    }

    {
    IECSqlBinder& arrayBinder = stmt.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindText(d.ToString().c_str(), IECSqlBinder::MakeCopy::Yes)) << stmt.GetECSql() << " | BindText(" << d.ToString() << ")";
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindText(d.ToString().c_str(), IECSqlBinder::MakeCopy::Yes)) << stmt.GetECSql() << " | BindText(" << d.ToString() << ")";
    EXPECT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << " | BindText(" << d.ToString() << ")";
    stmt.Reset();
    stmt.ClearBindings();
    }

    {
    IECSqlBinder& arrayBinder = stmt.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Error, arrayBinder.AddArrayElement().BindText("Foo string", IECSqlBinder::MakeCopy::No)) << stmt.GetECSql();
    }
    }

    //************
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT 1 FROM ts.Foo WHERE dtUtcArray=?"));
    {
    IECSqlBinder& arrayBinder = stmt.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindDateTime(dtUtc)) << stmt.GetECSql() << " | BindDateTime(" << dtUtc.ToString() << ")";
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindDateTime(dtUtc)) << stmt.GetECSql() << " | BindDateTime(" << dtUtc.ToString() << ")";
    EXPECT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindDateTime(" << dtUtc.ToString() << ")";
    stmt.Reset();
    stmt.ClearBindings();
    }

    {
    IECSqlBinder& arrayBinder = stmt.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindDateTime(d)) << stmt.GetECSql() << " | BindDateTime(" << d.ToString() << ")";
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindDateTime(d)) << stmt.GetECSql() << " | BindDateTime(" << d.ToString() << ")";
    EXPECT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << " | BindDateTime(" << d.ToString() << ")";
    stmt.Reset();
    stmt.ClearBindings();
    }

    {
    IECSqlBinder& arrayBinder = stmt.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindText(dtUtc.ToString().c_str(), IECSqlBinder::MakeCopy::Yes)) << stmt.GetECSql() << " | BindText(" << dtUtc.ToString() << ")";
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindText(dtUtc.ToString().c_str(), IECSqlBinder::MakeCopy::Yes)) << stmt.GetECSql() << " | BindText(" << dtUtc.ToString() << ")";
    EXPECT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindText(" << dtUtc.ToString() << ")";
    stmt.Reset();
    stmt.ClearBindings();
    }

    {
    IECSqlBinder& arrayBinder = stmt.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindText(d.ToString().c_str(), IECSqlBinder::MakeCopy::Yes)) << stmt.GetECSql() << " | BindText(" << d.ToString() << ")";
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindText(d.ToString().c_str(), IECSqlBinder::MakeCopy::Yes)) << stmt.GetECSql() << " | BindText(" << d.ToString() << ")";
    EXPECT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << " | BindText(" << d.ToString() << ")";
    stmt.Reset();
    stmt.ClearBindings();
    }

    {
    IECSqlBinder& arrayBinder = stmt.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Error, arrayBinder.AddArrayElement().BindDateTime(dt)) << stmt.GetECSql() << " | BindDateTime(" << dt.ToString() << ")";
    ASSERT_EQ(ECSqlStatus::Error, arrayBinder.AddArrayElement().BindText(dt.ToString().c_str(), IECSqlBinder::MakeCopy::Yes)) << stmt.GetECSql() << " | BindText(" << dt.ToString() << ")";
    ASSERT_EQ(ECSqlStatus::Error, arrayBinder.AddArrayElement().BindText("Foo string", IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | BindText('Foo string')";
    }

    }


    //************
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT 1 FROM ts.Foo WHERE dArray=?"));
    {
    IECSqlBinder& arrayBinder = stmt.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindDateTime(d)) << stmt.GetECSql() << " | BindDateTime(" << d.ToString() << ")";
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindDateTime(d)) << stmt.GetECSql() << " | BindDateTime(" << d.ToString() << ")";
    EXPECT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindDateTime(" << d.ToString() << ")";
    stmt.Reset();
    stmt.ClearBindings();
    }

    {
    IECSqlBinder& arrayBinder = stmt.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindDateTime(dt)) << stmt.GetECSql() << " | BindDateTime(" << dt.ToString() << ")";
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindDateTime(dt)) << stmt.GetECSql() << " | BindDateTime(" << dt.ToString() << ")";
    EXPECT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << " | BindDateTime(" << dt.ToString() << ")";
    stmt.Reset();
    stmt.ClearBindings();
    }

    {
    IECSqlBinder& arrayBinder = stmt.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindDateTime(dtUtc)) << stmt.GetECSql() << " | BindDateTime(" << dtUtc.ToString() << ")";
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindDateTime(dtUtc)) << stmt.GetECSql() << " | BindDateTime(" << dtUtc.ToString() << ")";
    EXPECT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << " | BindDateTime(" << dtUtc.ToString() << ")";
    stmt.Reset();
    stmt.ClearBindings();
    }

    {
    IECSqlBinder& arrayBinder = stmt.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindText(d.ToString().c_str(), IECSqlBinder::MakeCopy::Yes)) << stmt.GetECSql() << " | BindText(" << d.ToString() << ")";
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindText(d.ToString().c_str(), IECSqlBinder::MakeCopy::Yes)) << stmt.GetECSql() << " | BindText(" << d.ToString() << ")";
    EXPECT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindText(" << d.ToString() << ")";
    stmt.Reset();
    stmt.ClearBindings();
    }

    {
    IECSqlBinder& arrayBinder = stmt.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindText(dt.ToString().c_str(), IECSqlBinder::MakeCopy::Yes)) << stmt.GetECSql() << " | BindText(" << dt.ToString() << ")";
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindText(dt.ToString().c_str(), IECSqlBinder::MakeCopy::Yes)) << stmt.GetECSql() << " | BindText(" << dt.ToString() << ")";
    EXPECT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << " | BindText(" << dt.ToString() << ")";
    stmt.Reset();
    stmt.ClearBindings();
    }

    {
    IECSqlBinder& arrayBinder = stmt.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindText(dtUtc.ToString().c_str(), IECSqlBinder::MakeCopy::Yes)) << stmt.GetECSql() << " | BindText(" << dtUtc.ToString() << ")";
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindText(dtUtc.ToString().c_str(), IECSqlBinder::MakeCopy::Yes)) << stmt.GetECSql() << " | BindText(" << dtUtc.ToString() << ")";
    EXPECT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << " | BindText(" << dtUtc.ToString() << ")";
    stmt.Reset();
    stmt.ClearBindings();
    }

    {
    IECSqlBinder& arrayBinder = stmt.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Error, arrayBinder.AddArrayElement().BindText("Foo string", IECSqlBinder::MakeCopy::No)) << stmt.GetECSql();
    }


    }

    //************
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT 1 FROM ts.Foo WHERE sArray=?"));

    {
    IECSqlBinder& arrayBinder = stmt.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindText(str, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | BindText(" << str << ")";
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindText(str, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | BindText(" << str << ")";
    EXPECT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindText(" << str << ")";
    stmt.Reset();
    stmt.ClearBindings();
    }

    {
    IECSqlBinder& arrayBinder = stmt.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindText(dt.ToString().c_str(), IECSqlBinder::MakeCopy::Yes)) << stmt.GetECSql() << " | BindText(" << dt.ToString() << ")";
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindText(dt.ToString().c_str(), IECSqlBinder::MakeCopy::Yes)) << stmt.GetECSql() << " | BindText(" << dt.ToString() << ")";
    EXPECT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << " | BindText(" << dt.ToString() << ") | dt.ToString returns millisecons in string, but inserted was raw string without milliseconds";
    stmt.Reset();
    stmt.ClearBindings();
    }

    {
    IECSqlBinder& arrayBinder = stmt.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindText(dtUtc.ToString().c_str(), IECSqlBinder::MakeCopy::Yes)) << stmt.GetECSql() << " | BindText(" << dtUtc.ToString() << ")";
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindText(dtUtc.ToString().c_str(), IECSqlBinder::MakeCopy::Yes)) << stmt.GetECSql() << " | BindText(" << dtUtc.ToString() << ")";
    EXPECT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << " | BindText(" << dtUtc.ToString() << ")";
    stmt.Reset();
    stmt.ClearBindings();
    }

    {
    IECSqlBinder& arrayBinder = stmt.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindText(d.ToString().c_str(), IECSqlBinder::MakeCopy::Yes)) << stmt.GetECSql() << " | BindText(" << d.ToString() << ")";
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindText(d.ToString().c_str(), IECSqlBinder::MakeCopy::Yes)) << stmt.GetECSql() << " | BindText(" << d.ToString() << ")";
    EXPECT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << " | BindText(" << d.ToString() << ")";
    stmt.Reset();
    stmt.ClearBindings();
    }

    {
    IECSqlBinder& arrayBinder = stmt.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Error, arrayBinder.AddArrayElement().BindDateTime(dt)) << stmt.GetECSql() << " | BindDateTime(" << dt.ToString() << ")";
    ASSERT_EQ(ECSqlStatus::Error, arrayBinder.AddArrayElement().BindDateTime(dtUtc)) << stmt.GetECSql() << " | BindDateTime(" << dtUtc.ToString() << ")";
    ASSERT_EQ(ECSqlStatus::Error, arrayBinder.AddArrayElement().BindDateTime(d)) << stmt.GetECSql() << " | BindDateTime(" << d.ToString() << ")";
    stmt.Reset();
    stmt.ClearBindings();
    }

    }

    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, BindStringToGuidParameter)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("BindStringToGuidParameter.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECStructClass typeName="GuidStruct" modifier="None">
                <ECProperty propertyName="beguid" typeName="binary" extendedTypeName="BeGuid" />
                <ECProperty propertyName="guid" typeName="binary" extendedTypeName="Guid" />
                <ECProperty propertyName="bin" typeName="binary" />
            </ECStructClass>
            <ECEntityClass typeName="Foo" modifier="None">
                <ECProperty propertyName="beguid" typeName="binary" extendedTypeName="BeGuid" />
                <ECProperty propertyName="guid" typeName="binary" extendedTypeName="Guid" />
                <ECProperty propertyName="bin" typeName="binary" />
                <ECStructProperty propertyName="struct" typeName="GuidStruct" />
                <ECArrayProperty propertyName="beguidArray" typeName="binary" extendedTypeName="BeGuid" />
                <ECArrayProperty propertyName="guidArray" typeName="binary" extendedTypeName="Guid" />
                <ECArrayProperty propertyName="binArray" typeName="binary" />
                <ECStructArrayProperty propertyName="structArray" typeName="GuidStruct" />
            </ECEntityClass>
         </ECSchema>)xml")));

    Utf8CP guidStr = "8b8837f7-f53b-458d-be2c-11ea57065f88";
    BeGuid guid;
    ASSERT_EQ(SUCCESS, guid.FromString(guidStr));

    ECInstanceKey key;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Foo(beguid,guid,bin,struct,beguidArray,guidArray,binArray,structArray) VALUES (?,?,?,?,?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindGuid(1, guid, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindGuid(2, guid, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindGuid(3, guid, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql();

    IECSqlBinder& structBinder = stmt.GetBinder(4);
    ASSERT_EQ(ECSqlStatus::Success, structBinder["beguid"].BindGuid(guid, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, structBinder["guid"].BindGuid(guid, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, structBinder["bin"].BindGuid(guid, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql();

    //add 2 elements to each array prop
    for (int i = 0; i < 2; i++)
        {
        ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(5).AddArrayElement().BindGuid(guid, IECSqlBinder::MakeCopy::No));
        ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(6).AddArrayElement().BindGuid(guid, IECSqlBinder::MakeCopy::No));
        ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(7).AddArrayElement().BindGuid(guid, IECSqlBinder::MakeCopy::No));

        IECSqlBinder& structArrayElementBinder = stmt.GetBinder(8).AddArrayElement();
        ASSERT_EQ(ECSqlStatus::Success, structArrayElementBinder["beguid"].BindGuid(guid, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql();
        ASSERT_EQ(ECSqlStatus::Success, structArrayElementBinder["guid"].BindGuid(guid, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql();
        ASSERT_EQ(ECSqlStatus::Success, structArrayElementBinder["bin"].BindGuid(guid, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql();
        }

    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << stmt.GetECSql();
    }

    // test that data was correctly inserted
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT beguid,guid,bin,beguidArray,guidArray,binArray,struct,structArray FROM ts.Foo WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, key.GetInstanceId())) << stmt.GetECSql();
    EXPECT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    for (int i = 0; i < 3; i++)
        {
        EXPECT_STRCASEEQ(guidStr, stmt.GetValueGuid(i).ToString().c_str()) << "ECSQL " << stmt.GetECSql() << " Column index: " << i;

        EXPECT_EQ(2, stmt.GetValue(i + 3).GetArrayLength());
        for (IECSqlValue const& arrayElement : stmt.GetValue(i + 3).GetArrayIterable())
            {
            EXPECT_STRCASEEQ(guidStr, arrayElement.GetGuid().ToString().c_str()) << "ECSQL " << stmt.GetECSql() << " Column index: " << i + 3;
            }
        }

    EXPECT_STRCASEEQ(guidStr, stmt.GetValue(6)["beguid"].GetGuid().ToString().c_str());
    EXPECT_STRCASEEQ(guidStr, stmt.GetValue(6)["guid"].GetGuid().ToString().c_str());
    EXPECT_STRCASEEQ(guidStr, stmt.GetValue(6)["bin"].GetGuid().ToString().c_str());

    EXPECT_EQ(2, stmt.GetValue(7).GetArrayLength());
    for (IECSqlValue const& arrayElement : stmt.GetValue(7).GetArrayIterable())
        {
        EXPECT_STRCASEEQ(guidStr, arrayElement["beguid"].GetGuid().ToString().c_str());
        EXPECT_STRCASEEQ(guidStr, arrayElement["guid"].GetGuid().ToString().c_str());
        EXPECT_STRCASEEQ(guidStr, arrayElement["bin"].GetGuid().ToString().c_str());
        }
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT 1 FROM ts.Foo WHERE beguid=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindGuid(1, guid)) << stmt.GetECSql() << " | BindGuid(" << guidStr << ")";
    EXPECT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindGuid(" << guidStr << ")";
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, guidStr, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | BindText(" << guidStr << ") | GUID string only can be parsed for Guid parameters";
    EXPECT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindText(" << guidStr << ")";
    stmt.Reset();
    stmt.ClearBindings();
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT 1 FROM ts.Foo WHERE guid=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindGuid(1, guid)) << stmt.GetECSql() << " | BindGuid(" << guidStr << ")";
    EXPECT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindGuid(" << guidStr << ")";
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, guidStr, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | BindText(" << guidStr << ") | GUID string only can be parsed for Guid parameters";
    EXPECT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << " | BindText(" << guidStr << ")";
    stmt.Reset();
    stmt.ClearBindings();
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT 1 FROM ts.Foo WHERE bin=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindGuid(1, guid)) << stmt.GetECSql() << " | BindGuid(" << guidStr << ")";
    EXPECT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindGuid(" << guidStr << ")";
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, guidStr, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | BindText(" << guidStr << ") | GUID string only can be parsed for Guid parameters";
    EXPECT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << " | BindText(" << guidStr << ")";
    stmt.Reset();
    stmt.ClearBindings();
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT 1 FROM ts.Foo WHERE struct.beguid=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindGuid(1, guid)) << stmt.GetECSql() << " | BindGuid(" << guidStr << ")";
    EXPECT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindGuid(" << guidStr << ")";
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, guidStr, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | BindText(" << guidStr << ") | GUID string only can be parsed for Guid parameters";
    EXPECT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindText(" << guidStr << ")";
    stmt.Reset();
    stmt.ClearBindings();
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT 1 FROM ts.Foo WHERE struct.guid=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindGuid(1, guid)) << stmt.GetECSql() << " | BindGuid(" << guidStr << ")";
    EXPECT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindGuid(" << guidStr << ")";
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, guidStr, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | BindText(" << guidStr << ") | GUID string only can be parsed for Guid parameters";
    EXPECT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << " | BindText(" << guidStr << ")";
    stmt.Reset();
    stmt.ClearBindings();
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT 1 FROM ts.Foo WHERE struct.bin=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindGuid(1, guid)) << stmt.GetECSql() << " | BindGuid(" << guidStr << ")";
    EXPECT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindGuid(" << guidStr << ")";
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, guidStr, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | BindText(" << guidStr << ") | GUID string only can be parsed for Guid parameters";
    EXPECT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << " | BindText(" << guidStr << ")";
    stmt.Reset();
    stmt.ClearBindings();
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT 1 FROM ts.Foo WHERE beguidArray=?"));
    IECSqlBinder& arrayBinder = stmt.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindGuid(guid, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | BindGuid(" << guidStr << ")";
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindGuid(guid, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | BindGuid(" << guidStr << ")";
    EXPECT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindGuid(" << guidStr << ")";
    stmt.Reset();
    stmt.ClearBindings();

    IECSqlBinder& arrayBinder2 = stmt.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder2.AddArrayElement().BindText(guidStr, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | BindText(" << guidStr << ")";
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder2.AddArrayElement().BindText(guidStr, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | BindText(" << guidStr << ")";
    EXPECT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindText(" << guidStr << ")";
    stmt.Reset();
    stmt.ClearBindings();
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT 1 FROM ts.Foo WHERE guidArray=?"));
    IECSqlBinder& arrayBinder = stmt.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindGuid(guid, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | BindGuid(" << guidStr << ")";
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindGuid(guid, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | BindGuid(" << guidStr << ")";
    EXPECT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindGuid(" << guidStr << ")";
    stmt.Reset();
    stmt.ClearBindings();

    IECSqlBinder& arrayBinder2 = stmt.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder2.AddArrayElement().BindText(guidStr, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | BindText(" << guidStr << ")";
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder2.AddArrayElement().BindText(guidStr, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | BindText(" << guidStr << ")";
    EXPECT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << " | BindText(" << guidStr << ") | Guid is only parsed for props with BeGuid extended type name";
    stmt.Reset();
    stmt.ClearBindings();
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT 1 FROM ts.Foo WHERE binArray=?"));
    IECSqlBinder& arrayBinder = stmt.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindGuid(guid, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | BindGuid(" << guidStr << ")";
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindGuid(guid, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | BindGuid(" << guidStr << ")";
    EXPECT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | BindGuid(" << guidStr << ")";
    stmt.Reset();
    stmt.ClearBindings();

    IECSqlBinder& arrayBinder2 = stmt.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder2.AddArrayElement().BindText(guidStr, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | BindText(" << guidStr << ")";
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder2.AddArrayElement().BindText(guidStr, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | BindText(" << guidStr << ")";
    EXPECT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << " | BindText(" << guidStr << ") | Guid is only parsed for props with BeGuid extended type name";
    stmt.Reset();
    stmt.ClearBindings();
    }

    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, PolymorphicDelete)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("PolymorphicDeleteSharedTable.ecdb", SchemaItem::CreateForFile("NestedStructArrayTest.01.00.00.ecschema.xml")));
    NestedStructArrayTestSchemaHelper::PopulateNestedStructArrayDb(m_ecdb, true);

    ASSERT_FALSE(GetHelper().TableExists("nsat_DerivedA"));
    ASSERT_FALSE(GetHelper().TableExists("nsat_DoubleDerivedA"));
    ASSERT_FALSE(GetHelper().TableExists("nsat_DoubleDerivedC"));

    //Delete all Instances of the base class, all the structArrays and relationships should also be deleted.
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "DELETE FROM nsat.ClassA"));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    std::vector<Utf8CP> tableNames{"ClassA", "BaseHasDerivedA", "DerivedBHasChildren"};

    for (Utf8CP tableName : tableNames)
        {
        Utf8String selectSql = "SELECT count(*) FROM nsat_";
        selectSql.append(tableName);
        Statement stmt;
        ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, selectSql.c_str())) << selectSql.c_str();
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << selectSql.c_str();
        ASSERT_EQ(0, stmt.GetValueInt(0)) << "Table " << tableName << " is expected to be empty after DELETE FROM nsat.ClassA";
        stmt.Finalize();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, PolymorphicUpdateNoTph)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("PolymorphicUpdateNoTph.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECEntityClass typeName="Base" >
                    <ECProperty propertyName="BaseProp" typeName="int" />
                </ECEntityClass>
                <ECEntityClass typeName="Sub1">
                    <BaseClass>Base</BaseClass>
                    <ECProperty propertyName="SubProp" typeName="int" />
                </ECEntityClass>
                <ECEntityClass typeName="Sub2">
                    <BaseClass>Base</BaseClass>
                    <ECProperty propertyName="SubProp" typeName="int" />
                </ECEntityClass>
                <ECEntityClass typeName="BaseAbstract" modifier="Abstract">
                    <ECProperty propertyName="BaseProp" typeName="int" />
                </ECEntityClass>
                <ECEntityClass typeName="Sub10">
                    <BaseClass>BaseAbstract</BaseClass>
                    <ECProperty propertyName="SubProp" typeName="int" />
                </ECEntityClass>
                <ECEntityClass typeName="Sub20">
                    <BaseClass>BaseAbstract</BaseClass>
                    <ECProperty propertyName="SubProp" typeName="int" />
                </ECEntityClass>
              </ECSchema>)xml")));

    ECInstanceKey baseKey, sub1Key, sub10Key;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(baseKey, "INSERT INTO ts.Base(BaseProp) VALUES (100)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(sub1Key, "INSERT INTO ts.Sub1(BaseProp,SubProp) VALUES (100, 123)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(sub10Key, "INSERT INTO ts.Sub10(BaseProp,SubProp) VALUES (100, 123)"));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "UPDATE ts.Base SET BaseProp=200"));
    stmt.Finalize();

    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("UPDATE ONLY ts.Base SET BaseProp=200"));

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT BaseProp FROM ts.Base WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, baseKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(200, stmt.GetValueInt(0));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT BaseProp FROM ts.Sub1 WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, sub1Key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(100, stmt.GetValueInt(0));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "UPDATE ts.BaseAbstract SET BaseProp=200"));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "UPDATE ONLY ts.BaseAbstract SET BaseProp=200"));
    stmt.Finalize();


    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "DELETE FROM ts.Base WHERE ECInstanceId=?"));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "DELETE FROM ONLY ts.Base WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, baseKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT BaseProp FROM ts.Base WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, baseKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT BaseProp FROM ts.Sub1 WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, sub1Key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "DELETE FROM ts.BaseAbstract WHERE ECInstanceId=?"));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "DELETE FROM ONLY ts.BaseAbstract WHERE ECInstanceId=?"));
    stmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, PolymorphicUpdate)
    {
    SchemaItem testSchema(NestedStructArrayTestSchemaHelper::s_testSchemaXml);
    ASSERT_EQ(SUCCESS, SetupECDb("PolymorphicUpdateTest.ecdb", testSchema));

    NestedStructArrayTestSchemaHelper::PopulateNestedStructArrayDb(m_ecdb, false);

    //Updates the instances of ClassA
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "UPDATE nsat.ClassA SET T='UpdatedValue', I=2"));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    statement.Finalize();
    m_ecdb.SaveChanges();

    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT I,T FROM nsat_ClassA"));
    while (BE_SQLITE_ROW == stmt.Step())
        {
        ASSERT_EQ(2, stmt.GetValueInt(0)) << "Int value doesn't match";
        ASSERT_STREQ("UpdatedValue", stmt.GetValueText(1)) << "String value doesn't match";
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, DeleteWithNestedSelectStatements)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ECSqlStatementTests.ecdb", SchemaItem::CreateForFile("ECSqlStatementTests.01.00.00.ecschema.xml")));
    NestedStructArrayTestSchemaHelper::PopulateECSqlStatementTestsDb(m_ecdb);

    ECSqlStatement stmt;

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM ECST.Product"));
    ASSERT_TRUE(BE_SQLITE_ROW == stmt.Step());
    ASSERT_EQ(9, stmt.GetValueInt(0));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "DELETE FROM ECST.Product WHERE ProductName IN(SELECT ProductName FROM ECST.Product GROUP BY ProductName HAVING COUNT(ProductName)>2 AND ProductName IN(SELECT ProductName FROM ECST.Product WHERE UnitPrice >500))"));
    ASSERT_TRUE(BE_SQLITE_DONE == stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM ECST.Product"));
    ASSERT_TRUE(BE_SQLITE_ROW == stmt.Step());
    ASSERT_EQ(6, stmt.GetValueInt(0));
    stmt.Finalize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, UpdateWithNestedSelectStatments)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ECSqlStatementTests.ecdb", SchemaItem::CreateForFile("ECSqlStatementTests.01.00.00.ecschema.xml")));
    NestedStructArrayTestSchemaHelper::PopulateECSqlStatementTestsDb(m_ecdb);

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE ECST.Product SET ProductName='Laptop' WHERE ProductName IN(SELECT ProductName FROM ECST.Product GROUP BY ProductName HAVING COUNT(ProductName)>2 AND ProductName IN(SELECT ProductName FROM ECST.Product WHERE UnitPrice >500))"));
    ASSERT_TRUE(BE_SQLITE_DONE == stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM ECST.Product WHERE ProductName='Laptop'"));
    ASSERT_TRUE(BE_SQLITE_ROW == stmt.Step());
    ASSERT_EQ(3, stmt.GetValueInt(0));
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, InsertStructArray)
    {
    const int perClassRowCount = 10;
    ASSERT_EQ(SUCCESS, SetupECDb("ECSqlStatementTests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, PopulateECDb( perClassRowCount));
    Utf8CP ecsql = "INSERT INTO ecsql.PSA (L,PStruct_Array) VALUES(?, ?)";
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, ecsql)) << "Preparation of '" << ecsql << "' failed";

    statement.BindInt64(1, 1000);
    //add three array elements
    const int count = 3;

    IECSqlBinder& arrayBinder = statement.GetBinder(2);

    for (int i = 0; i < count; i++)
        {
        IECSqlBinder& arrayElementBinder = arrayBinder.AddArrayElement();
        ECSqlStatus stat = arrayElementBinder["d"].BindDouble(i * PI);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["i"].BindInt(i * 2);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["l"].BindInt64(i * 3);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["p2d"].BindPoint2d(DPoint2d::From(i, i + 1));
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["p3d"].BindPoint3d(DPoint3d::From(i, i + 1, i + 2));
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        }

    auto stepStatus = statement.Step();
    ASSERT_EQ(BE_SQLITE_DONE, stepStatus) << "Step for '" << ecsql << "' failed";
    }
//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, DeleteStructArray)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("PolymorphicUpdateTest.ecdb", SchemaItem::CreateForFile("NestedStructArrayTest.01.00.00.ecschema.xml")));

    auto in = NestedStructArrayTestSchemaHelper::CreateECInstances(m_ecdb, 1, "ClassP");

    int insertCount = 0;
    for (auto inst : in)
        {
        ECInstanceInserter inserter(m_ecdb, inst->GetClass(), nullptr);
        ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(*inst));
        insertCount++;
        }

    ECClassCP classP = m_ecdb.Schemas().GetClass("NestedStructArrayTest", "ClassP");
    ASSERT_TRUE(classP != nullptr);

    ECInstanceDeleter deleter(m_ecdb, *classP, nullptr);

    int deleteCount = 0;
    for (auto& inst : in)
        {
        ASSERT_EQ(BE_SQLITE_OK, deleter.Delete(*inst));
        deleteCount++;
        }

    //Verify Inserted Instance have been deleted.
    bvector<IECInstancePtr> out;
    ECSqlStatement stmt;
    auto prepareStatus = stmt.Prepare(m_ecdb, "SELECT * FROM ONLY nsat.ClassP ORDER BY ECInstanceId");
    ASSERT_TRUE(prepareStatus == ECSqlStatus::Success);
    ECInstanceECSqlSelectAdapter classPReader(stmt);
    ASSERT_FALSE(stmt.Step() == BE_SQLITE_ROW);
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, Int64InStructArrays)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("Int64InStructArrays.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='testSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECStructClass typeName='MyStruct' >"
        "        <ECProperty propertyName='I' typeName='int' />"
        "        <ECProperty propertyName='I64' typeName='long' />"
        "    </ECStructClass>"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECStructArrayProperty propertyName='StructArrayProp' typeName='MyStruct' />"
        "    </ECEntityClass>"
        "</ECSchema>")));

    BeBriefcaseBasedId id(BeBriefcaseId(123), INT64_C(4129813293));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Foo(StructArrayProp) VALUES(?)"));
    IECSqlBinder& arrayElementBinder = stmt.GetBinder(1).AddArrayElement();
    ASSERT_EQ(ECSqlStatus::Success, arrayElementBinder["I"].BindInt(123456));
    ASSERT_EQ(ECSqlStatus::Success, arrayElementBinder["I64"].BindInt64(id.GetValue()));
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << stmt.GetECSql();
    stmt.Finalize();

    Statement rawStmt;
    ASSERT_EQ(BE_SQLITE_OK, rawStmt.Prepare(m_ecdb, "SELECT StructArrayProp FROM ts_Foo WHERE Id=?"));
    ASSERT_EQ(BE_SQLITE_OK, rawStmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, rawStmt.Step()) << rawStmt.GetSql();
    rapidjson::Document actualStructArrayJson;
    ASSERT_TRUE(!actualStructArrayJson.Parse<0>(rawStmt.GetValueText(0)).HasParseError());

    ASSERT_TRUE(actualStructArrayJson.IsArray());
    ASSERT_EQ(123456, actualStructArrayJson[0]["I"]);
    ASSERT_EQ(id.GetValue(), actualStructArrayJson[0]["I64"]) << "Int64 are expected to not be stringified in the JSON";
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, ColumnInfoAndSystemProperties)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::Readonly)));
    ASSERT_EQ(SUCCESS, PopulateECDb( 3));
    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT ECInstanceId, ECClassId, MyPSA.Id, MyPSA.RelECClassId, P2D.X, P2D.Y, P3D.X, P3D.Y, P3D.Z FROM ecsql.P LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());

    ASSERT_EQ(9, statement.GetColumnCount());
    for (int i = 0; i < 9; i++)
        {
        ECSqlColumnInfo const& colInfo = statement.GetColumnInfo(i);
        EXPECT_TRUE(colInfo.IsSystemProperty()) << colInfo.GetPropertyPath().ToString();
        EXPECT_FALSE(colInfo.IsGeneratedProperty()) << colInfo.GetPropertyPath().ToString();
        EXPECT_TRUE(colInfo.GetDataType().IsPrimitive()) << colInfo.GetPropertyPath().ToString();

        if (i < 2)
            {
            ASSERT_STREQ("ClassECSqlSystemProperties", colInfo.GetProperty()->GetClass().GetName().c_str()) << colInfo.GetPropertyPath().ToString();
            ASSERT_EQ(PrimitiveType::PRIMITIVETYPE_Long, colInfo.GetDataType().GetPrimitiveType()) << colInfo.GetPropertyPath().ToString();
            }
        else if (i < 4)
            {
            ECClassCR navPropMemberClass = colInfo.GetProperty()->GetClass();
            ASSERT_STREQ("NavigationECSqlSystemProperties", navPropMemberClass.GetName().c_str()) << colInfo.GetPropertyPath().ToString();
            ASSERT_TRUE(navPropMemberClass.IsStructClass()) << colInfo.GetPropertyPath().ToString();
            ASSERT_EQ(PrimitiveType::PRIMITIVETYPE_Long, colInfo.GetDataType().GetPrimitiveType()) << colInfo.GetPropertyPath().ToString();
            }
        else
            {
            ECClassCR pointMemberClass = colInfo.GetProperty()->GetClass();
            ASSERT_STREQ("PointECSqlSystemProperties", pointMemberClass.GetName().c_str()) << colInfo.GetPropertyPath().ToString();
            ASSERT_TRUE(pointMemberClass.IsStructClass()) << colInfo.GetPropertyPath().ToString();
            ASSERT_EQ(PrimitiveType::PRIMITIVETYPE_Double, colInfo.GetDataType().GetPrimitiveType()) << colInfo.GetPropertyPath().ToString();
            }

        ASSERT_TRUE(colInfo.GetRootClass().GetAlias().empty()) << colInfo.GetPropertyPath().ToString();
        ASSERT_STREQ("P", colInfo.GetRootClass().GetClass().GetName().c_str()) << colInfo.GetPropertyPath().ToString();
        }
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT ECInstanceId, ECClassId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM ecsql.PSAHasPSA_NN LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());

    ASSERT_EQ(6, statement.GetColumnCount());
    for (int i = 0; i < 6; i++)
        {
        ECSqlColumnInfo const& colInfo = statement.GetColumnInfo(i);
        EXPECT_TRUE(colInfo.IsSystemProperty()) << colInfo.GetPropertyPath().ToString();
        EXPECT_FALSE(colInfo.IsGeneratedProperty()) << colInfo.GetPropertyPath().ToString();
        ASSERT_TRUE(colInfo.GetDataType().IsPrimitive()) << colInfo.GetPropertyPath().ToString();
        ASSERT_EQ(PrimitiveType::PRIMITIVETYPE_Long, colInfo.GetDataType().GetPrimitiveType()) << colInfo.GetPropertyPath().ToString();
        if (i < 2)
            ASSERT_STREQ("ClassECSqlSystemProperties", colInfo.GetProperty()->GetClass().GetName().c_str()) << colInfo.GetPropertyPath().ToString();
        else
            ASSERT_STREQ("RelationshipECSqlSystemProperties", colInfo.GetProperty()->GetClass().GetName().c_str()) << colInfo.GetPropertyPath().ToString();

        ASSERT_TRUE(colInfo.GetRootClass().GetAlias().empty()) << colInfo.GetPropertyPath().ToString();
        ASSERT_STREQ("PSAHasPSA_NN", colInfo.GetRootClass().GetClass().GetName().c_str()) << colInfo.GetPropertyPath().ToString();
        }
    }

    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, ColumnInfoGetOriginProperty)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::Readonly)));
    ASSERT_EQ(SUCCESS, PopulateECDb( 3));
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT ECInstanceId, I as MyInt, S as MyString FROM ecsql.P LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());

    ASSERT_EQ(3, statement.GetColumnCount());
    {
        auto& colInfo = statement.GetColumnInfo(0);
        EXPECT_TRUE(colInfo.IsSystemProperty()) << colInfo.GetPropertyPath().ToString();
        EXPECT_FALSE(colInfo.IsGeneratedProperty()) << colInfo.GetPropertyPath().ToString();
    }
    {
        auto& colInfo = statement.GetColumnInfo(1);
        EXPECT_TRUE(colInfo.IsGeneratedProperty()) << colInfo.GetPropertyPath().ToString();
        EXPECT_STREQ("MyInt", colInfo.GetProperty()->GetName().c_str());
        EXPECT_STREQ("I", colInfo.GetOriginProperty()->GetName().c_str());
    }
    {
        auto& colInfo = statement.GetColumnInfo(2);
        EXPECT_TRUE(colInfo.IsGeneratedProperty()) << colInfo.GetPropertyPath().ToString();
        EXPECT_STREQ("MyString", colInfo.GetProperty()->GetName().c_str());
        EXPECT_STREQ("S", colInfo.GetOriginProperty()->GetName().c_str());
    }
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, ColumnInfoGetOriginPropertyForNonPropertyColumns)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::Readonly)));
    ASSERT_EQ(SUCCESS, PopulateECDb( 3));
        {
        ECSqlStatement statement;
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT 100 as MyInt"));
        ASSERT_EQ(BE_SQLITE_ROW, statement.Step());

        ASSERT_EQ(1, statement.GetColumnCount());
            {
                auto& colInfo = statement.GetColumnInfo(1);
                EXPECT_FALSE(colInfo.IsGeneratedProperty()) << colInfo.GetPropertyPath().ToString();
                EXPECT_TRUE(colInfo.GetProperty() == nullptr) << "Column should not have a property";
                EXPECT_TRUE(colInfo.GetOriginProperty() == nullptr) << "MyInt should not have an origin property";
            }
        }

        {
        ECSqlStatement statement;
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT 100"));
        ASSERT_EQ(BE_SQLITE_ROW, statement.Step());

        ASSERT_EQ(1, statement.GetColumnCount());
            {
                auto& colInfo = statement.GetColumnInfo(1);
                EXPECT_FALSE(colInfo.IsGeneratedProperty()) << colInfo.GetPropertyPath().ToString();
                EXPECT_TRUE(colInfo.GetProperty() == nullptr) << "Column should not have a property";
                EXPECT_TRUE(colInfo.GetOriginProperty() == nullptr) << "Column should not have an origin property";
            }
        }

        {
        ECSqlStatement statement;
        auto status = statement.Prepare(m_ecdb, R"sqlstr(
        with recursive
         cte0 (a,b) as (select 100,200)
         select * from (select a from cte0 where a=100 and b=200)
         )sqlstr");
        ASSERT_EQ(ECSqlStatus::Success, status);
        ASSERT_EQ(BE_SQLITE_ROW, statement.Step());

        ASSERT_EQ(1, statement.GetColumnCount());
            {
                auto& colInfo = statement.GetColumnInfo(1);
                EXPECT_FALSE(colInfo.IsGeneratedProperty());
                EXPECT_TRUE(colInfo.GetProperty() == nullptr) << "Column should not have a property";
                EXPECT_TRUE(colInfo.GetOriginProperty() == nullptr) << "Column should not have an origin property";
            }
        }

        {
        ECSqlStatement statement;
        auto status = statement.Prepare(m_ecdb, R"sqlstr(
        with recursive
         cte0 (a,b) as (select 100,200)
         select a from cte0 where a=100 and b=200
         )sqlstr");
        ASSERT_EQ(ECSqlStatus::Success, status);
        ASSERT_EQ(BE_SQLITE_ROW, statement.Step());

        ASSERT_EQ(1, statement.GetColumnCount());
            {
                auto& colInfo = statement.GetColumnInfo(1);
                EXPECT_FALSE(colInfo.IsGeneratedProperty());
                EXPECT_TRUE(colInfo.GetProperty() == nullptr) << "Column should not have a property";
                EXPECT_TRUE(colInfo.GetOriginProperty() == nullptr) << "Column should not have an origin property";
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, ColumnInfoWithJoin)
    {
    const int perClassRowCount = 10;
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::Readonly)));
    ASSERT_EQ(SUCCESS, PopulateECDb( perClassRowCount));

    Utf8CP ecsql = "SELECT c1.ECInstanceId, c2.ECInstanceId, c1.ECClassId, c2.ECClassId FROM ecsql.PSA c1, ecsql.P c2 LIMIT 1";
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, ecsql)) << ecsql;

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    auto const& value1 = statement.GetValue(0);
    auto const& columnInfo1 = value1.GetColumnInfo();

    ASSERT_FALSE(value1.IsNull());
    ASSERT_FALSE(columnInfo1.IsGeneratedProperty());
    ASSERT_TRUE(columnInfo1.IsSystemProperty());
    ASSERT_STREQ("ClassECSqlSystemProperties", columnInfo1.GetProperty()->GetClass().GetName().c_str());
    ASSERT_STREQ("c1", columnInfo1.GetRootClass().GetAlias().c_str());
    ASSERT_STREQ("PSA", columnInfo1.GetRootClass().GetClass().GetName().c_str());

    auto const& value2 = statement.GetValue(1);
    auto const& columnInfo2 = value2.GetColumnInfo();

    ASSERT_FALSE(value2.IsNull());
    ASSERT_TRUE(columnInfo2.IsSystemProperty());
    ASSERT_FALSE(columnInfo2.IsGeneratedProperty());
    ASSERT_STREQ("ClassECSqlSystemProperties", columnInfo2.GetProperty()->GetClass().GetName().c_str());
    ASSERT_STREQ("c2", columnInfo2.GetRootClass().GetAlias().c_str());
    ASSERT_STREQ("P", columnInfo2.GetRootClass().GetClass().GetName().c_str());

    auto const& value3 = statement.GetValue(2);
    auto const& columnInfo3 = value3.GetColumnInfo();

    ASSERT_FALSE(value3.IsNull());
    ASSERT_TRUE(columnInfo3.IsSystemProperty());
    ASSERT_FALSE(columnInfo3.IsGeneratedProperty());
    ASSERT_STREQ("ClassECSqlSystemProperties", columnInfo3.GetProperty()->GetClass().GetName().c_str());
    ASSERT_STREQ("c1", columnInfo3.GetRootClass().GetAlias().c_str());
    ASSERT_STREQ("PSA", columnInfo3.GetRootClass().GetClass().GetName().c_str());

    auto const& value4 = statement.GetValue(3);
    auto const& columnInfo4 = value4.GetColumnInfo();

    ASSERT_FALSE(value4.IsNull());
    ASSERT_TRUE(columnInfo4.IsSystemProperty());
    ASSERT_FALSE(columnInfo4.IsGeneratedProperty());
    ASSERT_STREQ("ClassECSqlSystemProperties", columnInfo4.GetProperty()->GetClass().GetName().c_str());
    ASSERT_STREQ("c2", columnInfo4.GetRootClass().GetAlias().c_str());
    ASSERT_STREQ("P", columnInfo4.GetRootClass().GetClass().GetName().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, ColumnInfoAndNavigationAndPointProp)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::Readonly)));
    ASSERT_EQ(SUCCESS, PopulateECDb( 3));

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT MyPSA, P2D, P3D FROM ecsql.P LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());

    ASSERT_EQ(3, statement.GetColumnCount());
    for (int i = 0; i < 3; i++)
        {
        ECSqlColumnInfo const& colInfo = statement.GetColumnInfo(i);
        ASSERT_FALSE(colInfo.IsSystemProperty());
        ASSERT_FALSE(colInfo.IsGeneratedProperty());
        switch (i)
            {
                case 0:
                    ASSERT_TRUE(colInfo.GetDataType().IsNavigation());
                    break;
                case 1:
                    ASSERT_TRUE(colInfo.GetDataType().IsPrimitive());
                    ASSERT_EQ(PrimitiveType::PRIMITIVETYPE_Point2d, colInfo.GetDataType().GetPrimitiveType());
                    break;
                case 2:
                    ASSERT_TRUE(colInfo.GetDataType().IsPrimitive());
                    ASSERT_EQ(PrimitiveType::PRIMITIVETYPE_Point3d, colInfo.GetDataType().GetPrimitiveType());
                    break;
            }

        ASSERT_STREQ("P", colInfo.GetProperty()->GetClass().GetName().c_str());
        ASSERT_TRUE(colInfo.GetRootClass().GetAlias().empty());
        ASSERT_STREQ("P", colInfo.GetRootClass().GetClass().GetName().c_str());
        }

    }



//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, InvalidBindArrayCalls)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ecsql.PSA(I_Array, PStruct_Array) VALUES(?,?)"));

    IECSqlBinder& primArrayBinder = statement.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Error, primArrayBinder.BindInt(1)) << "Cannot call BindXXX before calling AddArrayElement on prim array parameter";
    ASSERT_EQ(ECSqlStatus::Error, primArrayBinder["i"].BindInt(1)) << "Cannot call [] before calling AddArrayElement on prim array parameter";
    IECSqlBinder& primArrayElementBinder = primArrayBinder.AddArrayElement();
    ASSERT_EQ(ECSqlStatus::Success, primArrayElementBinder.BindInt(1));
    ASSERT_EQ(ECSqlStatus::Error, primArrayBinder["i"].BindInt(1)) << "Cannot [] on prim array parameter";

    IECSqlBinder& structArrayBinder = statement.GetBinder(2);
    ASSERT_EQ(ECSqlStatus::Error, structArrayBinder.BindInt(1)) << "Cannot call BindXXX before calling AddArrayElement on struct array parameter";
    ASSERT_EQ(ECSqlStatus::Error, structArrayBinder["i"].BindInt(1)) << "Cannot call BindXXX before calling AddArrayElement on struct array parameter";

    IECSqlBinder& structArrayElementBinder = structArrayBinder.AddArrayElement();

    ASSERT_EQ(ECSqlStatus::Error, structArrayElementBinder.BindInt(1)) << "Cannot bind prim to struct";
    ASSERT_EQ(ECSqlStatus::Success, structArrayElementBinder["i"].BindInt(1));
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, StructArrayUpdate)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, PopulateECDb( 2));

    Utf8CP ecsql = "UPDATE  ONLY ecsql.PSA SET L = ?,  PStruct_Array = ? WHERE I = ?";
    ECSqlStatement statement;
    auto stat = statement.Prepare(m_ecdb, ecsql);
    ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation of '" << ecsql << "' failed";
    statement.BindInt(3, 123);
    statement.BindInt64(1, 1000);

    //add three array elements
    const uint32_t arraySize = 3;
    IECSqlBinder& arrayBinder = statement.GetBinder(2);
    for (int i = 0; i < arraySize; i++)
        {
        IECSqlBinder& arrayElementBinder = arrayBinder.AddArrayElement();
        ECSqlStatus stat = arrayElementBinder["d"].BindDouble(i * PI);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["i"].BindInt(i * 2);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["l"].BindInt64(i * 3);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["p2d"].BindPoint2d(DPoint2d::From(i, i + 1));
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["p3d"].BindPoint3d(DPoint3d::From(i, i + 1, i + 2));
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        }

    auto stepStatus = statement.Step();
    ASSERT_EQ(BE_SQLITE_DONE, stepStatus) << "Step for '" << ecsql << "' failed";
    }

void setProductsValues(StandaloneECInstancePtr instance, int ProductId, Utf8CP ProductName, double price)
    {
    instance->SetValue("ProductId", ECValue(ProductId));
    instance->SetValue("ProductName", ECValue(ProductName));
    instance->SetValue("Price", ECValue(price));
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, StructArrayDelete)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, PopulateECDb( 2));
    auto ecsql = "DELETE FROM  ONLY ecsql.PSA WHERE I = ?";
    ECSqlStatement statement;
    auto stat = statement.Prepare(m_ecdb, ecsql);
    ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation of '" << ecsql << "' failed";

    statement.BindInt(1, 123);

    auto stepStatus = statement.Step();
    ASSERT_EQ(BE_SQLITE_DONE, stepStatus) << "Step for '" << ecsql << "' failed";
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, BindToNumericProps)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("BindToNumericProps.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
          <ECEntityClass typeName="Parent" modifier="None" >
              <ECProperty propertyName="Name" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="Child" modifier="None" >
              <ECCustomAttributes>
                  <ClassMap xmlns="ECDbMap.02.00">
                      <MapStrategy>TablePerHierarchy</MapStrategy>
                  </ClassMap>
                  <ShareColumns xmlns="ECDbMap.02.00">
                      <MaxSharedColumnsBeforeOverflow>7</MaxSharedColumnsBeforeOverflow>
                      <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>
                  </ShareColumns>
              </ECCustomAttributes>
              <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward" />
              <ECProperty propertyName="IntProp" typeName="int" />
              <ECProperty propertyName="Int64Prop" typeName="long" />
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

    ECInstanceKey parentKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(parentKey, "INSERT INTO ts.Parent(Name) VALUES('Parent 01')"));
    ECInstanceId parentId = parentKey.GetInstanceId();
    Utf8String parentIdStr = parentId.ToString();
    // Literal strings to numeric props
    ECInstanceKey childKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(childKey, Utf8PrintfString("INSERT INTO ts.Child(IntProp,Int64Prop,Parent.Id) VALUES(100,100,%s)", parentIdStr.c_str()).c_str()));
    ECInstanceId childId = childKey.GetInstanceId();
    Utf8String childIdStr = childId.ToString();

    //literal strings
    EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s"}])json", childId.ToHexStr().c_str())), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Child WHERE IntProp=100 AND Int64Prop=100 AND Parent.Id=%s", parentIdStr.c_str()).c_str())) << "Ids as literal decimal strings";
    EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s"}])json", childId.ToHexStr().c_str())), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Child WHERE IntProp=100 AND Int64Prop=100 AND Parent.Id=%s", parentId.ToHexStr().c_str()).c_str())) << "Ids as literal hex strings";

    ECSqlStatement selStmt;
    ASSERT_EQ(ECSqlStatus::Success, selStmt.Prepare(m_ecdb, "SELECT 1 FROM ts.Child WHERE ECInstanceId=? AND Parent.Id=? AND IntProp=? AND Int64Prop=?"));

    //BindInt
    ASSERT_EQ(ECSqlStatus::Success, selStmt.BindInt(1, (int)childId.GetValue())) << selStmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, selStmt.BindInt(2, (int)parentId.GetValue())) << selStmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, selStmt.BindInt(3, 100)) << selStmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, selStmt.BindInt(4, 100)) << selStmt.GetECSql();
    EXPECT_EQ(BE_SQLITE_ROW, selStmt.Step()) << selStmt.GetECSql();
    selStmt.Reset();
    selStmt.ClearBindings();

    //BindInt64
    ASSERT_EQ(ECSqlStatus::Success, selStmt.BindInt64(1, childId.GetValue())) << selStmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, selStmt.BindInt64(2, parentId.GetValue())) << selStmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, selStmt.BindInt64(3, 100)) << selStmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, selStmt.BindInt64(4, 100)) << selStmt.GetECSql();
    EXPECT_EQ(BE_SQLITE_ROW, selStmt.Step()) << selStmt.GetECSql();
    selStmt.Reset();
    selStmt.ClearBindings();

    //BindText -- Expected to fail because the text is not converted to a number
    ASSERT_EQ(ECSqlStatus::Success, selStmt.BindText(1, childIdStr.c_str(), IECSqlBinder::MakeCopy::No)) << selStmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, selStmt.BindText(2, parentIdStr.c_str(), IECSqlBinder::MakeCopy::No)) << selStmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, selStmt.BindText(3, "100", IECSqlBinder::MakeCopy::No)) << selStmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, selStmt.BindText(4, "100", IECSqlBinder::MakeCopy::No)) << selStmt.GetECSql();
    EXPECT_EQ(BE_SQLITE_DONE, selStmt.Step()) << selStmt.GetECSql();
    selStmt.Reset();
    selStmt.ClearBindings();

    // Bind as hex
    // Note: SQLite cannot compare numbers to hex string literals:
    // Assume there is a row with ECInstanceId 0x111:
    // WHERE ECInstanceId='0x111' will not match
    // WHERE ECInstanceId=0x111 will match

    // Run a plain SQLite statement to verify that SQLite behaves like this:
    {
    Statement sqliteStmt;
    ASSERT_EQ(BE_SQLITE_OK, sqliteStmt.Prepare(m_ecdb, "select cast(0x2 as number), cast('0x2' as number)"));
    ASSERT_EQ(BE_SQLITE_ROW, sqliteStmt.Step());
    ASSERT_EQ(2, sqliteStmt.GetValueInt(0)) << "Hex decimal can be converted to number | " << sqliteStmt.GetSql();
    ASSERT_EQ(0, sqliteStmt.GetValueInt(1)) << "Hex string cannot be converted to number | " << sqliteStmt.GetSql();
    }

    //Binding hex strings works because ECDb converts them to int64 before handing over to SQLite
    selStmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, selStmt.Prepare(m_ecdb, "SELECT 1 FROM ts.Child WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, selStmt.BindText(1, childId.ToHexStr().c_str(), IECSqlBinder::MakeCopy::Yes)) << selStmt.GetECSql();
    EXPECT_EQ(BE_SQLITE_ROW, selStmt.Step()) << "Bind ECInstanceId as hex string | " << selStmt.GetECSql() << " | SQL: " << selStmt.GetNativeSql();

    selStmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, selStmt.Prepare(m_ecdb, "SELECT 1 FROM ts.Child WHERE Parent.Id=?"));
    ASSERT_EQ(ECSqlStatus::Success, selStmt.BindText(1, parentId.ToHexStr().c_str(), IECSqlBinder::MakeCopy::Yes)) << selStmt.GetECSql();
    EXPECT_EQ(BE_SQLITE_ROW, selStmt.Step()) << "Bind Parent.Id as hex string | " << selStmt.GetECSql() << " SQL: " << selStmt.GetNativeSql();

    selStmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, selStmt.Prepare(m_ecdb, "SELECT 1 FROM ts.Child WHERE IntProp=?"));
    ASSERT_EQ(ECSqlStatus::Success, selStmt.BindText(1,"0x64", IECSqlBinder::MakeCopy::No)) << selStmt.GetECSql();
    EXPECT_EQ(BE_SQLITE_DONE, selStmt.Step()) << "Bind hex string to IntProp | " << selStmt.GetECSql() << " SQL: " << selStmt.GetNativeSql();

    selStmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, selStmt.Prepare(m_ecdb, "SELECT 1 FROM ts.Child WHERE IntProp=?"));
    ASSERT_EQ(ECSqlStatus::Success, selStmt.BindInt(1, 0x64)) << selStmt.GetECSql();
    EXPECT_EQ(BE_SQLITE_ROW, selStmt.Step()) << "Bind hex dec to IntProp | " << selStmt.GetECSql() << " SQL: " << selStmt.GetNativeSql();

    selStmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, selStmt.Prepare(m_ecdb, "SELECT 1 FROM ts.Child WHERE Int64Prop=?"));
    ASSERT_EQ(ECSqlStatus::Success, selStmt.BindText(1, "0x64", IECSqlBinder::MakeCopy::No)) << selStmt.GetECSql();
    EXPECT_EQ(BE_SQLITE_DONE, selStmt.Step()) << "Bind hex string to Int64Prop | " << selStmt.GetECSql() << " SQL: " << selStmt.GetNativeSql();

    selStmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, selStmt.Prepare(m_ecdb, "SELECT 1 FROM ts.Child WHERE Int64Prop=?"));
    ASSERT_EQ(ECSqlStatus::Success, selStmt.BindInt64(1, 0x64)) << selStmt.GetECSql();
    EXPECT_EQ(BE_SQLITE_ROW, selStmt.Step()) << "Bind hex dec to Int64Prop | " << selStmt.GetECSql() << " SQL: " << selStmt.GetNativeSql();
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, InsertNullForECInstanceId)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecinstanceidbindnull.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    auto assertSequence = [] (ECDbCR ecdb, BeInt64Id expectedSequenceValue)
        {
        CachedStatementPtr stmt = ecdb.GetCachedStatement("SELECT Val FROM be_Local WHERE Name='ec_instanceidsequence'");
        ASSERT_TRUE(stmt != nullptr);
        ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
        ASSERT_EQ(expectedSequenceValue.GetValue(), stmt->GetValueUInt64(0));
        };

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.PSA(ECInstanceId) VALUES(?)"));
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    assertSequence(m_ecdb, key.GetInstanceId());

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNull(1));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    stmt.Finalize();
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    assertSequence(m_ecdb, key.GetInstanceId());

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.PSA(ECInstanceId) VALUES(NULL)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    assertSequence(m_ecdb, key.GetInstanceId());
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, BindEnum)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("BindEnum.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECEnumeration typeName="Status" backingTypeName="int" isStrict="true">
                    <ECEnumerator name="On" value="1" />
                    <ECEnumerator name="Off" value="2" />
                </ECEnumeration>
                <ECEnumeration typeName="Domain" backingTypeName="string" isStrict="true">
                    <ECEnumerator name="Org" value=".org" />
                    <ECEnumerator name="Com" value=".com" />
                </ECEnumeration>
                <ECEntityClass typeName="Foo" >
                    <ECProperty propertyName="Status" typeName="Status" />
                    <ECProperty propertyName="Domain" typeName="Domain" />
                    <ECArrayProperty propertyName="Statuses" typeName="Status" />
                    <ECArrayProperty propertyName="Domains" typeName="Domain" />
                </ECEntityClass>
              </ECSchema>)xml")));
    ECEnumerationCP statusEnum = m_ecdb.Schemas().GetEnumeration("TestSchema", "Status");
    ASSERT_TRUE(statusEnum != nullptr);
    ECEnumeratorCP statusOn = statusEnum->FindEnumeratorByName("On");
    ASSERT_TRUE(statusOn != nullptr);
    ECEnumeratorCP statusOff = statusEnum->FindEnumeratorByName("Off");
    ASSERT_TRUE(statusOff != nullptr);
    ECEnumerationCP domainEnum = m_ecdb.Schemas().GetEnumeration("TestSchema", "Domain");
    ASSERT_TRUE(domainEnum != nullptr);
    ECEnumeratorCP orgDomain = domainEnum->FindEnumeratorByName("Org");
    ASSERT_TRUE(orgDomain != nullptr);
    ECEnumeratorCP comDomain = domainEnum->FindEnumeratorByName("Com");
    ASSERT_TRUE(comDomain != nullptr);

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ts.Foo(Status,Statuses,Domain,Domains) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindEnum(1,*statusOn));

    ASSERT_EQ(ECSqlStatus::Success, statement.GetBinder(2).AddArrayElement().BindEnum(*statusOn));
    ASSERT_EQ(ECSqlStatus::Success, statement.GetBinder(2).AddArrayElement().BindEnum(*statusOff));

    ASSERT_EQ(ECSqlStatus::Success, statement.BindEnum(3, *comDomain));

    ASSERT_EQ(ECSqlStatus::Success, statement.GetBinder(4).AddArrayElement().BindEnum(*orgDomain));
    ASSERT_EQ(ECSqlStatus::Success, statement.GetBinder(4).AddArrayElement().BindEnum(*comDomain));

    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step(key));

    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT Status,Statuses,Domain,Domains FROM ts.Foo"));
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step()) << statement.GetECSql();
    ASSERT_EQ(statusOn->GetInteger(), statement.GetValueInt(0)) << statement.GetECSql();
    ECEnumeratorCP actualStatus = statement.GetValueEnum(0);
    ASSERT_TRUE(actualStatus != nullptr);
    ASSERT_STREQ(statusOn->GetName().c_str(), actualStatus->GetName().c_str()) << statement.GetECSql();
    ASSERT_EQ(statusOn->GetInteger(), actualStatus->GetInteger()) << statement.GetECSql();

    int i = 0;
    for (IECSqlValue const& arrayElement : statement.GetValue(1).GetArrayIterable())
        {
        ECEnumeratorCP actualStatus = arrayElement.GetEnum();
        ASSERT_TRUE(actualStatus != nullptr);
        switch (i)
            {
                case 0:
                {
                ASSERT_EQ(statusOn->GetInteger(), arrayElement.GetInt()) << statement.GetECSql();
                ASSERT_STREQ(statusOn->GetName().c_str(), actualStatus->GetName().c_str()) << statement.GetECSql();
                ASSERT_EQ(statusOn->GetInteger(), actualStatus->GetInteger()) << statement.GetECSql();
                break;
                }
                case 1:
                {
                ASSERT_EQ(statusOff->GetInteger(), arrayElement.GetInt()) << statement.GetECSql();
                ASSERT_STREQ(statusOff->GetName().c_str(), actualStatus->GetName().c_str()) << statement.GetECSql();
                ASSERT_EQ(statusOff->GetInteger(), actualStatus->GetInteger()) << statement.GetECSql();
                break;
                }
                default:
                    FAIL() << "Array is expected to only have two elements";
            }
        i++;
        }

    ASSERT_STREQ(comDomain->GetString().c_str(), statement.GetValueText(2)) << statement.GetECSql();
    ECEnumeratorCP actualDomain = statement.GetValueEnum(2);
    ASSERT_TRUE(actualDomain != nullptr);
    ASSERT_STREQ(actualDomain->GetName().c_str(), actualDomain->GetName().c_str()) << statement.GetECSql();
    ASSERT_STREQ(actualDomain->GetString().c_str(), actualDomain->GetString().c_str()) << statement.GetECSql();

    i = 0;
    for (IECSqlValue const& arrayElement : statement.GetValue(3).GetArrayIterable())
        {
        ECEnumeratorCP actualDomain = arrayElement.GetEnum();
        ASSERT_TRUE(actualStatus != nullptr);

        switch (i)
            {
                case 0:
                {
                ASSERT_STREQ(orgDomain->GetString().c_str(), arrayElement.GetText()) << statement.GetECSql();
                ASSERT_STREQ(orgDomain->GetName().c_str(), actualDomain->GetName().c_str()) << statement.GetECSql();
                ASSERT_EQ(orgDomain->GetString(), actualDomain->GetString()) << statement.GetECSql();
                break;
                }
                case 1:
                {
                ASSERT_STREQ(comDomain->GetString().c_str(), arrayElement.GetText()) << statement.GetECSql();
                ASSERT_STREQ(comDomain->GetName().c_str(), actualDomain->GetName().c_str()) << statement.GetECSql();
                ASSERT_EQ(comDomain->GetString(), actualDomain->GetString()) << statement.GetECSql();
                break;
                }
                default:
                    FAIL() << "Array is expected to only have two elements";
            }
        i++;
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, BindSourceAndTargetECInstanceId)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    ECSqlStatement stmt;

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.PSA(ECInstanceId) VALUES(111)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.PSA(ECInstanceId) VALUES(222)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.PSA(ECInstanceId) VALUES(1111)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.PSA(ECInstanceId) VALUES(2222)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.PSA(ECInstanceId) VALUES(11111)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.PSA(ECInstanceId) VALUES(22222)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.PSA(ECInstanceId) VALUES(1111111)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.PSA(ECInstanceId) VALUES(2222222)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.PSA(ECInstanceId) VALUES(0x111)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.PSA(ECInstanceId) VALUES(0x222)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ECSqlStatement statement;

    auto stat = statement.Prepare(m_ecdb, "INSERT INTO ecsql.PSAHasPSA (SourceECInstanceId, TargetECInstanceId) VALUES(?,?)");
    ASSERT_EQ(ECSqlStatus::Success, stat);

    {
    ASSERT_EQ(ECSqlStatus::Success, statement.BindId(1, ECInstanceId(UINT64_C(111))));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindId(2, ECInstanceId(UINT64_C(222))));

    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step(key));
    }

    {
    statement.Reset();
    statement.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt(1, 1111));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt(2, 2222));
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step(key));
    }

    {
    statement.Reset();
    statement.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt64(1, 11111LL));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt64(2, 22222LL));

    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step(key));
    }

    {
    statement.Reset();
    statement.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, statement.BindText(1, "1111111", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindText(2, "2222222", IECSqlBinder::MakeCopy::No));

    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step(key));
    }

    {
    statement.Reset();
    statement.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, statement.BindText(1, "0x111", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindText(2, "0x222", IECSqlBinder::MakeCopy::No));

    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step(key));
    }

    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, BindPrimitiveArray)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, PopulateECDb( 10));

    std::vector<int> expectedIntArray = {1, 2, 3, 4, 5, 6, 7, 8};
    std::vector<Utf8String> expectedStringArray = {"1", "2", "3", "4", "5", "6", "7", "8"};

    ECInstanceKey ecInstanceKey;
    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ecsql.PA (I_Array,S_Array) VALUES(:ia,:sa)"));

    IECSqlBinder& arrayBinderI = statement.GetBinder(1);
    for (int arrayElement : expectedIntArray)
        {
        ASSERT_EQ(ECSqlStatus::Success, arrayBinderI.AddArrayElement().BindInt(arrayElement));
        }

    IECSqlBinder& arrayBinderS = statement.GetBinder(2);
    for (Utf8StringCR arrayElement : expectedStringArray)
        {
        ASSERT_EQ(ECSqlStatus::Success, arrayBinderS.AddArrayElement().BindText(arrayElement.c_str(), IECSqlBinder::MakeCopy::No));
        }

    ASSERT_EQ(BE_SQLITE_DONE, statement.Step(ecInstanceKey));
    }

    {
    ECSqlStatement statement;
    auto stat = statement.Prepare(m_ecdb, "SELECT I_Array, S_Array FROM ONLY ecsql.PA WHERE ECInstanceId = ?");
    ASSERT_EQ(ECSqlStatus::Success, stat);
    statement.BindId(1, ecInstanceKey.GetInstanceId());

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());

    IECSqlValue const& intArray = statement.GetValue(0);
    size_t expectedIndex = 0;
    for (IECSqlValue const& arrayElement : intArray.GetArrayIterable())
        {
        int actualArrayElement = arrayElement.GetInt();
        ASSERT_EQ(expectedIntArray[expectedIndex], actualArrayElement);
        expectedIndex++;
        }

    IECSqlValue const& stringArray = statement.GetValue(1);
    expectedIndex = 0;
    for (IECSqlValue const& arrayElement : stringArray.GetArrayIterable())
        {
        auto actualArrayElement = arrayElement.GetText();
        ASSERT_STREQ(expectedStringArray[expectedIndex].c_str(), actualArrayElement);
        expectedIndex++;
        }
    }
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, Insert_BindDateTimeArray)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, PopulateECDb( 10));

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ecsql.PA (Dt_Array, DtUtc_Array) VALUES(:dt,:dtutc)"));

    std::vector<DateTime> testDates = {DateTime(DateTime::Kind::Utc, 2014, 07, 07, 12, 0),
        DateTime(DateTime::Kind::Unspecified, 2014, 07, 07, 12, 0),
        DateTime(DateTime::Kind::Local, 2014, 07, 07, 12, 0)};


    IECSqlBinder& arrayBinderDt = statement.GetBinder(1);

    for (DateTimeCR testDate : testDates)
        {
        IECSqlBinder& arrayElementBinder = arrayBinderDt.AddArrayElement();

        ECSqlStatus expectedStat = testDate.GetInfo().GetKind() == DateTime::Kind::Local ? ECSqlStatus::Error : ECSqlStatus::Success;
        ASSERT_EQ(expectedStat, arrayElementBinder.BindDateTime(testDate));
        }


    IECSqlBinder& arrayBinderDtUtc = statement.GetBinder(2);

    for (DateTimeCR testDate : testDates)
        {
        IECSqlBinder& arrayElementBinder = arrayBinderDtUtc.AddArrayElement();

        ECSqlStatus expectedStat = testDate.GetInfo().GetKind() == DateTime::Kind::Utc ? ECSqlStatus::Success : ECSqlStatus::Error;
        ASSERT_EQ(expectedStat, arrayElementBinder.BindDateTime(testDate)) << testDate.ToString().c_str();
        }

    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, BindPrimitiveArrayWithDifferentTypes)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("BindPrimitiveArrayWithDifferentTypes.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ecsql.PSA(B_Array,Bi_Array,D_Array,Dt_Array,I_Array,L_Array,P2D_Array,P3D_Array,S_Array) VALUES(?,?,?,?,?,?,?,?,?)"));

    const int64_t int64Val = INT64_C(123123123123);

    bmap<PrimitiveType, ECValue> paramValues;
    paramValues[PRIMITIVETYPE_Boolean] = ECValue(true);
    paramValues[PRIMITIVETYPE_Binary] = ECValue((Byte const*) &int64Val, sizeof(int64Val));
    paramValues[PRIMITIVETYPE_Double] = ECValue(3.0);
    paramValues[PRIMITIVETYPE_DateTime] = ECValue(DateTime(2018,2,12));
    paramValues[PRIMITIVETYPE_Integer] = ECValue(3);
    paramValues[PRIMITIVETYPE_Long] = ECValue(int64Val);
    paramValues[PRIMITIVETYPE_Point2d] = ECValue(DPoint2d::From(1.0, 1.0));
    paramValues[PRIMITIVETYPE_Point3d] = ECValue(DPoint3d::From(1.0, 1.0, 1.0));
    paramValues[PRIMITIVETYPE_String] = ECValue("Hello world", true);

    {
    //Boolean parameter
    IECSqlBinder& arrayBinder = statement.GetBinder(1);
    IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();

    EXPECT_EQ(ECSqlStatus::Success, elementBinder.BindBoolean(paramValues[PRIMITIVETYPE_Boolean].GetBoolean()));
    size_t blobSize = 0;
    void const* blob = paramValues[PRIMITIVETYPE_Binary].GetBinary(blobSize);
    EXPECT_EQ(ECSqlStatus::Success, elementBinder.BindBlob(blob, (int) blobSize, IECSqlBinder::MakeCopy::No));
    EXPECT_EQ(ECSqlStatus::Success, elementBinder.BindDouble(paramValues[PRIMITIVETYPE_Double].GetDouble()));
    EXPECT_EQ(ECSqlStatus::Error, elementBinder.BindDateTime(paramValues[PRIMITIVETYPE_DateTime].GetDateTime()));
    EXPECT_EQ(ECSqlStatus::Success, elementBinder.BindInt(paramValues[PRIMITIVETYPE_Integer].GetInteger()));
    EXPECT_EQ(ECSqlStatus::Success, elementBinder.BindInt64(paramValues[PRIMITIVETYPE_Long].GetLong()));
    EXPECT_EQ(ECSqlStatus::Error, elementBinder.BindPoint2d(paramValues[PRIMITIVETYPE_Point2d].GetPoint2d()));
    EXPECT_EQ(ECSqlStatus::Error, elementBinder.BindPoint3d(paramValues[PRIMITIVETYPE_Point3d].GetPoint3d()));
    EXPECT_EQ(ECSqlStatus::Success, elementBinder.BindText(paramValues[PRIMITIVETYPE_String].GetUtf8CP(), IECSqlBinder::MakeCopy::Yes));
    }

    {
    //Blob parameter
    IECSqlBinder& arrayBinder = statement.GetBinder(2);
    IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();

    EXPECT_EQ(ECSqlStatus::Success, elementBinder.BindBoolean(paramValues[PRIMITIVETYPE_Boolean].GetBoolean()));
    size_t blobSize = 0;
    void const* blob = paramValues[PRIMITIVETYPE_Binary].GetBinary(blobSize);
    EXPECT_EQ(ECSqlStatus::Success, elementBinder.BindBlob(blob, (int) blobSize, IECSqlBinder::MakeCopy::No));
    EXPECT_EQ(ECSqlStatus::Success, elementBinder.BindDouble(paramValues[PRIMITIVETYPE_Double].GetDouble()));
    EXPECT_EQ(ECSqlStatus::Error, elementBinder.BindDateTime(paramValues[PRIMITIVETYPE_DateTime].GetDateTime()));
    EXPECT_EQ(ECSqlStatus::Success, elementBinder.BindInt(paramValues[PRIMITIVETYPE_Integer].GetInteger()));
    EXPECT_EQ(ECSqlStatus::Success, elementBinder.BindInt64(paramValues[PRIMITIVETYPE_Long].GetLong()));
    EXPECT_EQ(ECSqlStatus::Error, elementBinder.BindPoint2d(paramValues[PRIMITIVETYPE_Point2d].GetPoint2d()));
    EXPECT_EQ(ECSqlStatus::Error, elementBinder.BindPoint3d(paramValues[PRIMITIVETYPE_Point3d].GetPoint3d()));
    EXPECT_EQ(ECSqlStatus::Success, elementBinder.BindText(paramValues[PRIMITIVETYPE_String].GetUtf8CP(), IECSqlBinder::MakeCopy::Yes));
    }

    {
    //Double parameter
    IECSqlBinder& arrayBinder = statement.GetBinder(3);
    IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();

    EXPECT_EQ(ECSqlStatus::Success, elementBinder.BindBoolean(paramValues[PRIMITIVETYPE_Boolean].GetBoolean()));
    size_t blobSize = 0;
    void const* blob = paramValues[PRIMITIVETYPE_Binary].GetBinary(blobSize);
    EXPECT_EQ(ECSqlStatus::Success, elementBinder.BindBlob(blob, (int) blobSize, IECSqlBinder::MakeCopy::No));
    EXPECT_EQ(ECSqlStatus::Success, elementBinder.BindDouble(paramValues[PRIMITIVETYPE_Double].GetDouble()));
    EXPECT_EQ(ECSqlStatus::Error, elementBinder.BindDateTime(paramValues[PRIMITIVETYPE_DateTime].GetDateTime()));
    EXPECT_EQ(ECSqlStatus::Success, elementBinder.BindInt(paramValues[PRIMITIVETYPE_Integer].GetInteger()));
    EXPECT_EQ(ECSqlStatus::Success, elementBinder.BindInt64(paramValues[PRIMITIVETYPE_Long].GetLong()));
    EXPECT_EQ(ECSqlStatus::Error, elementBinder.BindPoint2d(paramValues[PRIMITIVETYPE_Point2d].GetPoint2d()));
    EXPECT_EQ(ECSqlStatus::Error, elementBinder.BindPoint3d(paramValues[PRIMITIVETYPE_Point3d].GetPoint3d()));
    EXPECT_EQ(ECSqlStatus::Success, elementBinder.BindText(paramValues[PRIMITIVETYPE_String].GetUtf8CP(), IECSqlBinder::MakeCopy::Yes));
    }

    {
    //DateTime parameter
    IECSqlBinder& arrayBinder = statement.GetBinder(4);
    IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();

    EXPECT_EQ(ECSqlStatus::Error, elementBinder.BindBoolean(paramValues[PRIMITIVETYPE_Boolean].GetBoolean()));
    size_t blobSize = 0;
    void const* blob = paramValues[PRIMITIVETYPE_Binary].GetBinary(blobSize);
    EXPECT_EQ(ECSqlStatus::Error, elementBinder.BindBlob(blob, (int) blobSize, IECSqlBinder::MakeCopy::No));
    EXPECT_EQ(ECSqlStatus::Error, elementBinder.BindDouble(paramValues[PRIMITIVETYPE_Double].GetDouble()));
    EXPECT_EQ(ECSqlStatus::Success, elementBinder.BindDateTime(paramValues[PRIMITIVETYPE_DateTime].GetDateTime()));
    EXPECT_EQ(ECSqlStatus::Error, elementBinder.BindInt(paramValues[PRIMITIVETYPE_Integer].GetInteger()));
    EXPECT_EQ(ECSqlStatus::Error, elementBinder.BindInt64(paramValues[PRIMITIVETYPE_Long].GetLong()));
    EXPECT_EQ(ECSqlStatus::Error, elementBinder.BindPoint2d(paramValues[PRIMITIVETYPE_Point2d].GetPoint2d()));
    EXPECT_EQ(ECSqlStatus::Error, elementBinder.BindPoint3d(paramValues[PRIMITIVETYPE_Point3d].GetPoint3d()));
    EXPECT_EQ(ECSqlStatus::Success, elementBinder.BindText("2018-08-12", IECSqlBinder::MakeCopy::No));
    EXPECT_EQ(ECSqlStatus::Error, elementBinder.BindText("Hello", IECSqlBinder::MakeCopy::No));
    }

    {
    //Int parameter
    IECSqlBinder& arrayBinder = statement.GetBinder(5);
    IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();

    EXPECT_EQ(ECSqlStatus::Success, elementBinder.BindBoolean(paramValues[PRIMITIVETYPE_Boolean].GetBoolean()));
    size_t blobSize = 0;
    void const* blob = paramValues[PRIMITIVETYPE_Binary].GetBinary(blobSize);
    EXPECT_EQ(ECSqlStatus::Success, elementBinder.BindBlob(blob, (int) blobSize, IECSqlBinder::MakeCopy::No));
    EXPECT_EQ(ECSqlStatus::Success, elementBinder.BindDouble(paramValues[PRIMITIVETYPE_Double].GetDouble()));
    EXPECT_EQ(ECSqlStatus::Error, elementBinder.BindDateTime(paramValues[PRIMITIVETYPE_DateTime].GetDateTime()));
    EXPECT_EQ(ECSqlStatus::Success, elementBinder.BindInt(paramValues[PRIMITIVETYPE_Integer].GetInteger()));
    EXPECT_EQ(ECSqlStatus::Success, elementBinder.BindInt64(paramValues[PRIMITIVETYPE_Long].GetLong()));
    EXPECT_EQ(ECSqlStatus::Error, elementBinder.BindPoint2d(paramValues[PRIMITIVETYPE_Point2d].GetPoint2d()));
    EXPECT_EQ(ECSqlStatus::Error, elementBinder.BindPoint3d(paramValues[PRIMITIVETYPE_Point3d].GetPoint3d()));
    EXPECT_EQ(ECSqlStatus::Success, elementBinder.BindText(paramValues[PRIMITIVETYPE_String].GetUtf8CP(), IECSqlBinder::MakeCopy::Yes));
    }

    {
    //Int64 parameter
    IECSqlBinder& arrayBinder = statement.GetBinder(6);
    IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();

    EXPECT_EQ(ECSqlStatus::Success, elementBinder.BindBoolean(paramValues[PRIMITIVETYPE_Boolean].GetBoolean()));
    size_t blobSize = 0;
    void const* blob = paramValues[PRIMITIVETYPE_Binary].GetBinary(blobSize);
    EXPECT_EQ(ECSqlStatus::Success, elementBinder.BindBlob(blob, (int) blobSize, IECSqlBinder::MakeCopy::No));
    EXPECT_EQ(ECSqlStatus::Success, elementBinder.BindDouble(paramValues[PRIMITIVETYPE_Double].GetDouble()));
    EXPECT_EQ(ECSqlStatus::Error, elementBinder.BindDateTime(paramValues[PRIMITIVETYPE_DateTime].GetDateTime()));
    EXPECT_EQ(ECSqlStatus::Success, elementBinder.BindInt(paramValues[PRIMITIVETYPE_Integer].GetInteger()));
    EXPECT_EQ(ECSqlStatus::Success, elementBinder.BindInt64(paramValues[PRIMITIVETYPE_Long].GetLong()));
    EXPECT_EQ(ECSqlStatus::Error, elementBinder.BindPoint2d(paramValues[PRIMITIVETYPE_Point2d].GetPoint2d()));
    EXPECT_EQ(ECSqlStatus::Error, elementBinder.BindPoint3d(paramValues[PRIMITIVETYPE_Point3d].GetPoint3d()));
    EXPECT_EQ(ECSqlStatus::Success, elementBinder.BindText(paramValues[PRIMITIVETYPE_String].GetUtf8CP(), IECSqlBinder::MakeCopy::Yes));
    }

    {
    //Point2d parameter
    IECSqlBinder& arrayBinder = statement.GetBinder(7);
    IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();

    EXPECT_EQ(ECSqlStatus::Error, elementBinder.BindBoolean(paramValues[PRIMITIVETYPE_Boolean].GetBoolean()));
    size_t blobSize = 0;
    void const* blob = paramValues[PRIMITIVETYPE_Binary].GetBinary(blobSize);
    EXPECT_EQ(ECSqlStatus::Error, elementBinder.BindBlob(blob, (int) blobSize, IECSqlBinder::MakeCopy::No));
    EXPECT_EQ(ECSqlStatus::Error, elementBinder.BindDouble(paramValues[PRIMITIVETYPE_Double].GetDouble()));
    EXPECT_EQ(ECSqlStatus::Error, elementBinder.BindDateTime(paramValues[PRIMITIVETYPE_DateTime].GetDateTime()));
    EXPECT_EQ(ECSqlStatus::Error, elementBinder.BindInt(paramValues[PRIMITIVETYPE_Integer].GetInteger()));
    EXPECT_EQ(ECSqlStatus::Error, elementBinder.BindInt64(paramValues[PRIMITIVETYPE_Long].GetLong()));
    EXPECT_EQ(ECSqlStatus::Success, elementBinder.BindPoint2d(paramValues[PRIMITIVETYPE_Point2d].GetPoint2d()));
    EXPECT_EQ(ECSqlStatus::Error, elementBinder.BindPoint3d(paramValues[PRIMITIVETYPE_Point3d].GetPoint3d()));
    EXPECT_EQ(ECSqlStatus::Error, elementBinder.BindText(paramValues[PRIMITIVETYPE_String].GetUtf8CP(), IECSqlBinder::MakeCopy::Yes));
    }

    {
    //Point3d parameter
    IECSqlBinder& arrayBinder = statement.GetBinder(8);
    IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();

    EXPECT_EQ(ECSqlStatus::Error, elementBinder.BindBoolean(paramValues[PRIMITIVETYPE_Boolean].GetBoolean()));
    size_t blobSize = 0;
    void const* blob = paramValues[PRIMITIVETYPE_Binary].GetBinary(blobSize);
    EXPECT_EQ(ECSqlStatus::Error, elementBinder.BindBlob(blob, (int) blobSize, IECSqlBinder::MakeCopy::No));
    EXPECT_EQ(ECSqlStatus::Error, elementBinder.BindDouble(paramValues[PRIMITIVETYPE_Double].GetDouble()));
    EXPECT_EQ(ECSqlStatus::Error, elementBinder.BindDateTime(paramValues[PRIMITIVETYPE_DateTime].GetDateTime()));
    EXPECT_EQ(ECSqlStatus::Error, elementBinder.BindInt(paramValues[PRIMITIVETYPE_Integer].GetInteger()));
    EXPECT_EQ(ECSqlStatus::Error, elementBinder.BindInt64(paramValues[PRIMITIVETYPE_Long].GetLong()));
    EXPECT_EQ(ECSqlStatus::Error, elementBinder.BindPoint2d(paramValues[PRIMITIVETYPE_Point2d].GetPoint2d()));
    EXPECT_EQ(ECSqlStatus::Success, elementBinder.BindPoint3d(paramValues[PRIMITIVETYPE_Point3d].GetPoint3d()));
    EXPECT_EQ(ECSqlStatus::Error, elementBinder.BindText(paramValues[PRIMITIVETYPE_String].GetUtf8CP(), IECSqlBinder::MakeCopy::Yes));
    }

    {
    //String parameter
    IECSqlBinder& arrayBinder = statement.GetBinder(9);
    IECSqlBinder& elementBinder = arrayBinder.AddArrayElement();

    EXPECT_EQ(ECSqlStatus::Success, elementBinder.BindBoolean(paramValues[PRIMITIVETYPE_Boolean].GetBoolean()));
    size_t blobSize = 0;
    void const* blob = paramValues[PRIMITIVETYPE_Binary].GetBinary(blobSize);
    EXPECT_EQ(ECSqlStatus::Success, elementBinder.BindBlob(blob, (int) blobSize, IECSqlBinder::MakeCopy::No));
    EXPECT_EQ(ECSqlStatus::Success, elementBinder.BindDouble(paramValues[PRIMITIVETYPE_Double].GetDouble()));
    EXPECT_EQ(ECSqlStatus::Error, elementBinder.BindDateTime(paramValues[PRIMITIVETYPE_DateTime].GetDateTime()));
    EXPECT_EQ(ECSqlStatus::Success, elementBinder.BindInt(paramValues[PRIMITIVETYPE_Integer].GetInteger()));
    EXPECT_EQ(ECSqlStatus::Success, elementBinder.BindInt64(paramValues[PRIMITIVETYPE_Long].GetLong()));
    EXPECT_EQ(ECSqlStatus::Error, elementBinder.BindPoint2d(paramValues[PRIMITIVETYPE_Point2d].GetPoint2d()));
    EXPECT_EQ(ECSqlStatus::Error, elementBinder.BindPoint3d(paramValues[PRIMITIVETYPE_Point3d].GetPoint3d()));
    EXPECT_EQ(ECSqlStatus::Success, elementBinder.BindText(paramValues[PRIMITIVETYPE_String].GetUtf8CP(), IECSqlBinder::MakeCopy::Yes));
    }

    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, ConstrainedArrayProps)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ConstrainedArrayProps.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                                    <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                                       <ECEntityClass typeName="Foo">
                                                            <ECArrayProperty propertyName="PrimArray" typeName="int" minOccurs="5" maxOccurs="10" />
                                                            <ECStructArrayProperty propertyName="StructArray" typeName="MyStruct" minOccurs="5" maxOccurs="10"/>
                                                        </ECEntityClass>
                                                       <ECStructClass typeName="MyStruct">
                                                            <ECProperty propertyName="Code" typeName="int"/>
                                                        </ECStructClass>
                                                    </ECSchema>)xml")));


    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ts.Foo(PrimArray,StructArray) VALUES(?,?)"));

    auto bindArrayValues = [&statement] (ECInstanceKey& key, Nullable<int> const& count)
        {
        statement.Reset();
        statement.ClearBindings();

        IECSqlBinder& primArrayBinder = statement.GetBinder(1);
        IECSqlBinder& structArrayBinder = statement.GetBinder(2);
        if (count == nullptr)
            {
            if (ECSqlStatus::Success != primArrayBinder.BindNull())
                return BE_SQLITE_ERROR;

            if (ECSqlStatus::Success != structArrayBinder.BindNull())
                return BE_SQLITE_ERROR;
            }
        else
            {
            for (int i = 0; i < count.Value(); i++)
                {
                if (ECSqlStatus::Success != primArrayBinder.AddArrayElement().BindInt(i))
                    return BE_SQLITE_ERROR;

                if (ECSqlStatus::Success != structArrayBinder.AddArrayElement()["Code"].BindInt(i))
                    return BE_SQLITE_ERROR;
                }
            }

        return statement.Step(key);
        };

    //first: array size to bind. second: Expected to succeed
    const std::vector<std::pair<Nullable<int>, bool>> testArrayCounts = {
            {nullptr, true}, // -> binds null
            { 0, true }, // -> does not bind anything which amounts to binding null
            { 2, false }, { 5, true }, { 7, true }, { 10, true },
            { 20, true }}; //Bug in ECObjects: ignores maxoccurs and always interprets it as unbounded.

    for (std::pair<Nullable<int>, bool> const& testArrayCountItem : testArrayCounts)
        {
        Nullable<int> const& testArrayCount = testArrayCountItem.first;
        const bool expectedToSucceed = testArrayCountItem.second;
        ECInstanceKey key;
        const DbResult stepStat = bindArrayValues(key, testArrayCount);
        Utf8String assertMessage;
        if (testArrayCount == nullptr)
            assertMessage = "Binding null to array is expected to succeed for array parameter with minOccurs=5 and maxOccurs=10";
        else
            assertMessage.Sprintf("Binding array of length %d is expected to succceed for array parameter with minOccurs=5 and maxOccurs=10", testArrayCount.Value());

        if (expectedToSucceed)
            {
            ASSERT_EQ(BE_SQLITE_DONE, stepStat) << assertMessage;

            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT PrimArray,StructArray FROM ts.FOO WHERE ECInstanceId=?"));
            ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, key.GetInstanceId()));
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
            if (testArrayCount == nullptr || testArrayCount == 0)
                {
                ASSERT_TRUE(stmt.IsValueNull(0)) << assertMessage;
                ASSERT_EQ(0, stmt.GetValue(0).GetArrayLength()) << assertMessage;
                ASSERT_TRUE(stmt.IsValueNull(1)) << assertMessage;
                ASSERT_EQ(0, stmt.GetValue(1).GetArrayLength()) << assertMessage;
                }
            else
                {
                ASSERT_EQ(testArrayCount.Value(), stmt.GetValue(0).GetArrayLength()) << assertMessage;
                ASSERT_EQ(testArrayCount.Value(), stmt.GetValue(1).GetArrayLength()) << assertMessage;
                }
            }
        else
            ASSERT_EQ(BE_SQLITE_ERROR, stepStat) << assertMessage;
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, InsertWithStructBinding)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("InsertWithStructBinding.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, PopulateECDb(10));

    ECClassCP pStructClass = m_ecdb.Schemas().GetClass("ECSqlTest", "PStruct");
    ASSERT_TRUE(pStructClass != nullptr && pStructClass->IsStructClass());

    //**** Test 1 *****
    {
    BeJsDocument expectedJson;
    ASSERT_EQ(SUCCESS, TestUtilities::ParseJson(expectedJson,R"json(
         { "b" : true,
         "d" : 3.0,
         "dt" : "2014-03-27T12:00:00.000",
         "dtUtc" : "2014-03-27T12:00:00.000Z",
         "i" : 44444,
         "l" : 444444444,
         "s" : "Hello, world",
         "p2d" : { "x" : 3.0, "y" : 5.0 },
        "p3d" : { "x" : 3.0, "y" : 5.0, "z" : -6.0}
        })json"));

    ECSqlStatement insertStmt;
    ASSERT_EQ(ECSqlStatus::Success, insertStmt.Prepare(m_ecdb, "INSERT INTO ecsql.PSA (I, PStructProp) VALUES (?, ?)"));
    ASSERT_EQ(ECSqlStatus::Success, JsonECSqlBinder::BindStructValue(insertStmt.GetBinder(2), expectedJson, *pStructClass->GetStructClassCP())) << insertStmt.GetECSql();
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step(key)) << insertStmt.GetECSql();
    insertStmt.Finalize();

    ECSqlStatement selStmt;
    ASSERT_EQ(ECSqlStatus::Success, selStmt.Prepare(m_ecdb, "SELECT PStructProp FROM ecsql.PSA WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, selStmt.BindId(1, key.GetInstanceId())) << selStmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_ROW, selStmt.Step());
    JsonECSqlSelectAdapter jsonAdapter(selStmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::KeepOriginal, ECJsonInt64Format::AsNumber));
    BeJsDocument actualJson;
    ASSERT_EQ(SUCCESS, jsonAdapter.GetRow(actualJson)) << selStmt.GetECSql();
    ASSERT_TRUE(actualJson.isMember("PStructProp"));
    ASSERT_EQ(expectedJson, actualJson["PStructProp"]);
    }

    //**** Test 2 *****
    {
    BeJsDocument expectedJson;
    ASSERT_EQ(SUCCESS, TestUtilities::ParseJson(expectedJson, R"json(
        { "PStructProp" :
        { "b" : true,
         "d" : 3.0,
         "dt" : "2014-03-27T12:00:00.000",
         "dtUtc" : "2014-03-27T12:00:00.000Z",
         "i" : 44444,
         "l" : 444444444,
         "s" : "Hello, world",
         "p2d" : { "x" : 3.0, "y" : 5.0 },
        "p3d" : { "x" : 3.0, "y" : 5.0, "z" : -6.0}
        }})json"));
    ECClassCP saStructClass = m_ecdb.Schemas().GetClass("ECSqlTest", "SAStruct");
    ASSERT_TRUE(saStructClass != nullptr && saStructClass->IsStructClass());

    ECSqlStatement insertStmt;
    ASSERT_EQ(ECSqlStatus::Success, insertStmt.Prepare(m_ecdb, "INSERT INTO ecsql.SA(SAStructProp) VALUES(?)"));
    ASSERT_EQ(ECSqlStatus::Success, JsonECSqlBinder::BindStructValue(insertStmt.GetBinder(1), expectedJson, *saStructClass->GetStructClassCP())) << insertStmt.GetECSql();
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step(key)) << insertStmt.GetECSql();
    insertStmt.Finalize();

    ECSqlStatement selStmt;
    ASSERT_EQ(ECSqlStatus::Success, selStmt.Prepare(m_ecdb, "SELECT SAStructProp FROM ecsql.SA WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, selStmt.BindId(1, key.GetInstanceId())) << selStmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_ROW, selStmt.Step());
    JsonECSqlSelectAdapter jsonAdapter(selStmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::KeepOriginal, ECJsonInt64Format::AsNumber));
    Json::Value actualJson;
    ASSERT_EQ(SUCCESS, jsonAdapter.GetRow(actualJson)) << selStmt.GetECSql();
    ASSERT_TRUE(actualJson.isMember("SAStructProp"));
    ASSERT_EQ(expectedJson, actualJson["SAStructProp"]);
    }

    //Mismatching types
    {
    ECSqlStatement insertStmt;
    ASSERT_EQ(ECSqlStatus::Success, insertStmt.Prepare(m_ecdb, "INSERT INTO ecsql.PSA(PStructProp) VALUES(?)"));

    ECSqlStatement verifyStmt;
    ASSERT_EQ(ECSqlStatus::Success, verifyStmt.Prepare(m_ecdb, "SELECT PStructProp FROM ecsql.PSA WHERE ECInstanceId=?"));
    JsonECSqlSelectAdapter verifyAdapter(verifyStmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::KeepOriginal, ECJsonInt64Format::AsNumber));

    BeJsDocument expectedJson, actualJson;

    {
    //mismatching types which are convertible to each other
    ASSERT_EQ(SUCCESS, TestUtilities::ParseJson(expectedJson, R"json({ "i" : 3.1415 })json"));
    ASSERT_EQ(ECSqlStatus::Success, JsonECSqlBinder::BindStructValue(insertStmt.GetBinder(1), expectedJson, *pStructClass->GetStructClassCP())) << expectedJson.Stringify();
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step(key)) << insertStmt.GetECSql();
    insertStmt.Reset();
    insertStmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, verifyStmt.BindId(1, key.GetInstanceId())) << verifyStmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_ROW, verifyStmt.Step());
    ASSERT_EQ(SUCCESS, verifyAdapter.GetRow(actualJson)) << verifyStmt.GetECSql();
    verifyStmt.Reset();
    verifyStmt.ClearBindings();
    ASSERT_TRUE(actualJson.isMember("PStructProp"));
    ASSERT_EQ(SUCCESS, TestUtilities::ParseJson(expectedJson, R"json({ "i" : 3 })json")) << "Double value in original JSON is converted to int during binding";
    ASSERT_EQ(expectedJson, actualJson["PStructProp"]);
    }

    {
    //mismatching types which are convertible to each other
    ASSERT_EQ(SUCCESS, TestUtilities::ParseJson(expectedJson, R"json({ "l" : 3.1415 })json"));
    ASSERT_EQ(ECSqlStatus::Success, JsonECSqlBinder::BindStructValue(insertStmt.GetBinder(1), expectedJson, *pStructClass->GetStructClassCP())) << expectedJson.Stringify();
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step(key)) << insertStmt.GetECSql();
    insertStmt.Reset();
    insertStmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, verifyStmt.BindId(1, key.GetInstanceId())) << verifyStmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_ROW, verifyStmt.Step());
    ASSERT_EQ(SUCCESS, verifyAdapter.GetRow(actualJson)) << verifyStmt.GetECSql();
    verifyStmt.Reset();
    verifyStmt.ClearBindings();

    ASSERT_TRUE(actualJson.isMember("PStructProp"));
    ASSERT_EQ(SUCCESS, TestUtilities::ParseJson(expectedJson, R"json({ "l" : 3 })json")) << "Double value in original JSON is converted to int64 during binding";
    ASSERT_EQ(expectedJson, actualJson["PStructProp"]);
    }

    ASSERT_EQ(SUCCESS, TestUtilities::ParseJson(expectedJson, R"json({ "b" : 3.1415 })json"));
    ASSERT_EQ(ECSqlStatus::Error, JsonECSqlBinder::BindStructValue(insertStmt.GetBinder(1), expectedJson, *pStructClass->GetStructClassCP())) << expectedJson.Stringify();

    ASSERT_EQ(SUCCESS, TestUtilities::ParseJson(expectedJson, R"json({ "s" : 3.1415 })json"));
    ASSERT_EQ(ECSqlStatus::Error, JsonECSqlBinder::BindStructValue(insertStmt.GetBinder(1), expectedJson, *pStructClass->GetStructClassCP())) << expectedJson.Stringify();

    ASSERT_EQ(SUCCESS, TestUtilities::ParseJson(expectedJson, R"json({ "bi" : 3.1415 })json"));
    ASSERT_EQ(ECSqlStatus::Error, JsonECSqlBinder::BindStructValue(insertStmt.GetBinder(1), expectedJson, *pStructClass->GetStructClassCP())) << expectedJson.Stringify();

    ASSERT_EQ(SUCCESS, TestUtilities::ParseJson(expectedJson, R"json({ "p2d" : 3.1415 })json"));
    ASSERT_EQ(ECSqlStatus::Error, JsonECSqlBinder::BindStructValue(insertStmt.GetBinder(1), expectedJson, *pStructClass->GetStructClassCP())) << expectedJson.Stringify();

    ASSERT_EQ(SUCCESS, TestUtilities::ParseJson(expectedJson, R"json({ "p3d" : 3.1415 })json"));
    ASSERT_EQ(ECSqlStatus::Error, JsonECSqlBinder::BindStructValue(insertStmt.GetBinder(1), expectedJson, *pStructClass->GetStructClassCP())) << expectedJson.Stringify();

    ASSERT_EQ(SUCCESS, TestUtilities::ParseJson(expectedJson, R"json({ "dt" : 3.1415 })json"));
    ASSERT_EQ(ECSqlStatus::Error, JsonECSqlBinder::BindStructValue(insertStmt.GetBinder(1), expectedJson, *pStructClass->GetStructClassCP())) << expectedJson.Stringify();

    ASSERT_EQ(SUCCESS, TestUtilities::ParseJson(expectedJson, R"json({ "dtUtc" : 3.1415 })json"));
    ASSERT_EQ(ECSqlStatus::Error, JsonECSqlBinder::BindStructValue(insertStmt.GetBinder(1), expectedJson, *pStructClass->GetStructClassCP())) << expectedJson.Stringify();
    }
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, UpdateWithStructBinding)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("UpdateWithStructBinding.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, PopulateECDb(10));
    ECClassCP psaClass = m_ecdb.Schemas().GetClass("ECSqlTest", "PSA");
    ASSERT_TRUE(psaClass != nullptr);
    ECClassCP pStructClass = m_ecdb.Schemas().GetClass("ECSqlTest", "PStruct");
    ASSERT_TRUE(pStructClass != nullptr && pStructClass->IsStructClass());

    JsonInserter jsonInserter(m_ecdb, *psaClass, nullptr);
    ASSERT_TRUE(jsonInserter.IsValid());

    ECSqlStatement updateStmt;
    ASSERT_EQ(ECSqlStatus::Success, updateStmt.Prepare(m_ecdb, "UPDATE ecsql.PSA SET PStructProp=? WHERE ECInstanceId=?"));

    ECSqlStatement verifyStmt;
    ASSERT_EQ(ECSqlStatus::Success, verifyStmt.Prepare(m_ecdb, "SELECT PStructProp FROM ecsql.PSA WHERE ECInstanceId=?"));
    JsonECSqlSelectAdapter verifyAdapter(verifyStmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::KeepOriginal, ECJsonInt64Format::AsNumber));

    BeJsDocument initialJson;
    ASSERT_EQ(SUCCESS, TestUtilities::ParseJson(initialJson, R"json(
       { "PStructProp" : { "b" : true,
         "d" : 3.0,
         "dt" : "2014-03-27T12:00:00.000",
         "dtUtc" : "2014-03-27T12:00:00.000Z",
         "i" : 44444,
         "l" : 444444444,
         "s" : "Hello, world",
         "p2d" : { "x" : 3.0, "y" : 5.0 },
        "p3d" : { "x" : 3.0, "y" : 5.0, "z" : -6.0}
        }})json"));

    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_OK, jsonInserter.Insert(key, initialJson)) << initialJson.Stringify();

    BeJsDocument expectedUpdatedJson;
    ASSERT_EQ(SUCCESS, TestUtilities::ParseJson(expectedUpdatedJson, R"json(
       { "b" : false,
         "d" : 6.0,
         "dt" : "2014-03-27T12:00:00.000",
         "dtUtc" : "2017-03-27T12:00:00.000Z",
         "i" : 8888,
         "l" : 444444444,
         "s" : "Hello, world!",
         "p2d" : { "x" : 3.0, "y" : 5.0 },
        "p3d" : { "x" : 3.0, "y" : 5.0, "z" : 6.0}
        })json"));

    ASSERT_EQ(ECSqlStatus::Success, JsonECSqlBinder::BindStructValue(updateStmt.GetBinder(1), expectedUpdatedJson, *pStructClass->GetStructClassCP())) << expectedUpdatedJson.Stringify();
    ASSERT_EQ(ECSqlStatus::Success, updateStmt.BindId(2, key.GetInstanceId())) << expectedUpdatedJson.Stringify();
    ASSERT_EQ(BE_SQLITE_DONE, updateStmt.Step()) << expectedUpdatedJson.Stringify();
    updateStmt.Reset();
    updateStmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, verifyStmt.BindId(1, key.GetInstanceId())) << verifyStmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_ROW, verifyStmt.Step()) << "Id: " << key.GetInstanceId().ToString() << " " << verifyStmt.GetECSql();
    BeJsDocument actualJson;
    ASSERT_EQ(SUCCESS, verifyAdapter.GetRow(actualJson)) << key.GetInstanceId().ToString() << " " << verifyStmt.GetECSql();
    ASSERT_TRUE(actualJson.isMember("PStructProp"));
    ASSERT_EQ(expectedUpdatedJson, actualJson["PStructProp"]);
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, StructsInWhereClause)
    {
    SchemaItem schema("<?xml version='1.0' encoding='utf-8' ?>"
                      "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                      "    <ECStructClass typeName='Name' >"
                      "        <ECProperty propertyName='First' typeName='string' />"
                      "        <ECProperty propertyName='Last' typeName='string' />"
                      "    </ECStructClass>"
                      "    <ECEntityClass typeName='Person' >"
                      "        <ECStructProperty propertyName='FullName' typeName='Name' />"
                      "    </ECEntityClass>"
                      "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", schema));

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Person (FullName.[First], FullName.[Last]) VALUES (?,?)"));

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "John", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(2, "Smith", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "John", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(2, "Myer", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT count(*) FROM ts.Person WHERE FullName=?"));
    IECSqlBinder& binder = stmt.GetBinder(1);
    binder["First"].BindText("John", IECSqlBinder::MakeCopy::No);
    binder["Last"].BindText("Myer", IECSqlBinder::MakeCopy::No);

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(1, stmt.GetValueInt(0));
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT count(*) FROM ts.Person WHERE FullName<>?"));
    IECSqlBinder& binder = stmt.GetBinder(1);
    binder["First"].BindText("John", IECSqlBinder::MakeCopy::No);
    binder["Last"].BindText("Myer", IECSqlBinder::MakeCopy::No);

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(1, stmt.GetValueInt(0)) << stmt.GetNativeSql();
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT count(*) FROM ts.Person WHERE FullName IN (?,?,?)"));

    IECSqlBinder& binder1 = stmt.GetBinder(1);
    binder1["First"].BindText("John", IECSqlBinder::MakeCopy::No);
    binder1["Last"].BindText("Myer", IECSqlBinder::MakeCopy::No);

    IECSqlBinder& binder2 = stmt.GetBinder(2);
    binder2["First"].BindText("Rich", IECSqlBinder::MakeCopy::No);
    binder2["Last"].BindText("Myer", IECSqlBinder::MakeCopy::No);

    IECSqlBinder& binder3 = stmt.GetBinder(3);
    binder3["First"].BindText("John", IECSqlBinder::MakeCopy::No);
    binder3["Last"].BindText("Smith", IECSqlBinder::MakeCopy::No);

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(2, stmt.GetValueInt(0));
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE ts.Person SET FullName.[Last]='Meyer' WHERE FullName=?"));

    IECSqlBinder& binder = stmt.GetBinder(1);
    binder["First"].BindText("John", IECSqlBinder::MakeCopy::No);
    binder["Last"].BindText("Myer", IECSqlBinder::MakeCopy::No);
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ECSqlStatement verifyStmt;
    ASSERT_EQ(ECSqlStatus::Success, verifyStmt.Prepare(m_ecdb, "SELECT count(*) FROM ts.Person WHERE FullName.[Last]=?"));
    ASSERT_EQ(ECSqlStatus::Success, verifyStmt.BindText(1, "Myer", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_ROW, verifyStmt.Step());
    ASSERT_EQ(0, verifyStmt.GetValueInt(0));
    verifyStmt.Reset();
    verifyStmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, verifyStmt.BindText(1, "Meyer", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_ROW, verifyStmt.Step());
    ASSERT_EQ(1, verifyStmt.GetValueInt(0));
    }
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, ParameterInSelectClause)
    {
    const auto perClassRowCount = 10;
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::Readonly)));
    ASSERT_EQ(SUCCESS, PopulateECDb( perClassRowCount));

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT ?, S FROM ecsql.PSA LIMIT 1"));

    BeBriefcaseBasedId expectedId(BeBriefcaseId(3), 444);
    ASSERT_EQ(ECSqlStatus::Success, statement.BindId(1, expectedId));

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(expectedId.GetValue(), statement.GetValueId<ECInstanceId>(0).GetValueUnchecked());
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());

    statement.Reset();
    statement.ClearBindings();
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_TRUE(statement.IsValueNull(0));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT -?, S FROM ecsql.PSA LIMIT 1"));

    BeBriefcaseBasedId expectedId(BeBriefcaseId(3), 444);
    ASSERT_EQ(ECSqlStatus::Success, statement.BindId(1, expectedId));

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ((-1)*expectedId.GetValue(), statement.GetValueId<ECInstanceId>(0).GetValueUnchecked());
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());

    statement.Reset();
    statement.ClearBindings();
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_TRUE(statement.IsValueNull(0));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT -? AS MyId, S FROM ecsql.PSA LIMIT 1"));

    int64_t expectedId = -123456LL;
    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt64(1, expectedId));

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ((-1)*expectedId, statement.GetValueInt64(0));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());

    statement.Reset();
    statement.ClearBindings();
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_TRUE(statement.IsValueNull(0));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    }
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, GetParameterIndex)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ecsql.PSA(I,L,S) VALUES(123,123456789,'Sample string')"));

    {
    ECSqlStatement statement;
    auto stat = statement.Prepare(m_ecdb, "SELECT I, S FROM ecsql.PSA WHERE I = :i AND S = :s AND L = :i * 1000000 + 456789");
    ASSERT_EQ(ECSqlStatus::Success, stat);

    int actualParamIndex = statement.GetParameterIndex("i");
    EXPECT_EQ(1, actualParamIndex);
    EXPECT_EQ(1, statement.TryGetParameterIndex("i"));
    statement.BindInt(actualParamIndex, 123);

    actualParamIndex = statement.GetParameterIndex("s");
    EXPECT_EQ(2, actualParamIndex);
    EXPECT_EQ(2, statement.TryGetParameterIndex("s"));
    statement.BindText(actualParamIndex, "Sample string", IECSqlBinder::MakeCopy::Yes);

    actualParamIndex = statement.GetParameterIndex("garbage");
    EXPECT_EQ(-1, actualParamIndex);
    EXPECT_EQ(-1, statement.TryGetParameterIndex("garbage"));

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    }

    {
    ECSqlStatement statement;
    auto stat = statement.Prepare(m_ecdb, "SELECT I, S FROM ecsql.PSA WHERE I = ? AND S = :s AND L = :l");
    ASSERT_EQ(ECSqlStatus::Success, stat);

    statement.BindInt(1, 123);

    int actualParamIndex = statement.GetParameterIndex("s");
    EXPECT_EQ(2, actualParamIndex);
    EXPECT_EQ(2, statement.TryGetParameterIndex("s"));
    statement.BindText(actualParamIndex, "Sample string", IECSqlBinder::MakeCopy::Yes);

    actualParamIndex = statement.GetParameterIndex("l");
    EXPECT_EQ(3, actualParamIndex);
    EXPECT_EQ(3, statement.TryGetParameterIndex("l"));
    statement.BindInt64(actualParamIndex, 123456789);

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    }

    {
    ECSqlStatement statement;
    auto stat = statement.Prepare(m_ecdb, "SELECT I, S FROM ecsql.PSA WHERE I = ? AND S = :s AND L = ?");
    ASSERT_EQ(ECSqlStatus::Success, stat);

    statement.BindInt(1, 123);

    int actualParamIndex = statement.GetParameterIndex("s");
    EXPECT_EQ(2, actualParamIndex);
    EXPECT_EQ(2, statement.TryGetParameterIndex("s"));
    statement.BindText(actualParamIndex, "Sample string", IECSqlBinder::MakeCopy::Yes);

    statement.BindInt64(3, 123456789);

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, statement.Prepare(m_ecdb, "SELECT I, S FROM ecsql.PSA WHERE I = :value")) << "VALUE is a reserved word in the ECSQL grammar, so cannot be used without escaping, even in parameter names";
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ecsql.PSA (L,S,I) VALUES (?,?,:[value])"));

    int actualParamIndex = statement.GetParameterIndex("value");
    ASSERT_EQ(3, actualParamIndex);
    ASSERT_EQ(3, statement.TryGetParameterIndex("value"));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt(actualParamIndex, 300471));

    ECInstanceKey newKey;
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step(newKey));

    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT ECInstanceId FROM ecsql.PSA WHERE ECInstanceId = :[id]"));
    actualParamIndex = statement.GetParameterIndex("id");
    ASSERT_EQ(1, actualParamIndex);
    ASSERT_EQ(1, statement.TryGetParameterIndex("id"));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindId(actualParamIndex, newKey.GetInstanceId()));

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(newKey.GetInstanceId().GetValue(), statement.GetValueId<ECInstanceId>(0).GetValue());
    }

    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, NoECClassIdFilterOption)
    {
    const auto perClassRowCount = 10;
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, PopulateECDb( perClassRowCount));

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT ECInstanceId FROM ecsql.TH3 WHERE ECInstanceId=?"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_TRUE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT ECInstanceId FROM ecsql.TH3 WHERE ECInstanceId=? ECSQLOPTIONS NoECClassIdFilter"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_FALSE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT ECInstanceId FROM ecsql.TH3 WHERE ECInstanceId=? ECSQLOPTIONS NoECClassIdFilter=True"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_FALSE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT ECInstanceId FROM ecsql.TH3 WHERE ECInstanceId=? ECSQLOPTIONS NoECClassIdFilter=False"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_TRUE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT ECInstanceId FROM ecsql.TH3 WHERE ECInstanceId=? ECSQLOPTIONS NoECClassIdFilter=0"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_TRUE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT ECInstanceId FROM ecsql.TH3 WHERE ECInstanceId=? ECSQLOPTIONS NoECClassIdFilter=1"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_FALSE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT t.ECInstanceId FROM ecsql.TH3 t JOIN ecsql.PSA p USING ecsql.PSAHasTHBase_0N WHERE p.ECInstanceId=?"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_TRUE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT t.ECInstanceId FROM ecsql.TH3 t JOIN ecsql.PSA p USING ecsql.PSAHasTHBase_0N WHERE p.ECInstanceId=? ECSQLOPTIONS NoECClassIdFilter"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_FALSE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "UPDATE ecsql.TH3 SET S2='hh' WHERE ECInstanceId=?"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_TRUE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "UPDATE ecsql.TH3 SET S2='hh' WHERE ECInstanceId=? ECSQLOPTIONS NoECClassIdFilter"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_FALSE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "DELETE FROM ecsql.TH3 WHERE ECInstanceId=?"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_TRUE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "DELETE FROM ecsql.TH3 WHERE ECInstanceId=? ECSQLOPTIONS NoECClassIdFilter"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_FALSE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "DELETE FROM ecsql.TH3 WHERE ECInstanceId IN (SELECT t.ECInstanceId FROM ecsql.TH3 t JOIN ecsql.PSA USING ecsql.PSAHasTHBase_0N WHERE PSA.I=?)"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_TRUE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "DELETE FROM ecsql.TH3 WHERE ECInstanceId IN (SELECT t.ECInstanceId FROM ecsql.TH3 t JOIN ecsql.PSA USING ecsql.PSAHasTHBase_0N WHERE PSA.I=? ECSQLOPTIONS NoECClassIdFilter)"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_TRUE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "DELETE FROM ecsql.TH3 WHERE ECInstanceId IN (SELECT t.ECInstanceId FROM ecsql.TH3 t JOIN ecsql.PSA USING ecsql.PSAHasTHBase_0N WHERE PSA.I=?) ECSQLOPTIONS NoECClassIdFilter"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_TRUE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "DELETE FROM ecsql.TH3 WHERE ECInstanceId IN (SELECT t.ECInstanceId FROM ecsql.TH3 t JOIN ecsql.PSA USING ecsql.PSAHasTHBase_0N WHERE PSA.I=? ECSQLOPTIONS NoECClassIdFilter) ECSQLOPTIONS NoECClassIdFilter"));
    Utf8String nativeSql(statement.GetNativeSql());
    ASSERT_FALSE(nativeSql.ContainsI("ec_cache_ClassHierarchy")) << "Native SQL: " << nativeSql.c_str();
    }

    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, ReadonlyPropertiesAreUpdatable)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ReadonlyPropertiesAreUpdatable.ecdb",
                           SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                               "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                               "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                               "    <ECEntityClass typeName='Element'>"
                               "        <ECCustomAttributes>"
                               "            <ClassMap xmlns='ECDbMap.02.00'>"
                               "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                               "            </ClassMap>"
                               "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                               "        </ECCustomAttributes>"
                               "        <ECProperty propertyName='ReadonlyProp1' typeName='int' readOnly='True' />"
                               "    </ECEntityClass>"
                               "    <ECEntityClass typeName='SubElement'>"
                               "        <BaseClass>Element</BaseClass>"
                               "        <ECProperty propertyName='ReadonlyProp2' typeName='int' readOnly='True' />"
                               "    </ECEntityClass>"
                               "</ECSchema>")));

    ECInstanceKey key;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.SubElement(ReadonlyProp1,ReadonlyProp2) VALUES(1,2)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    }

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "UPDATE ONLY ts.SubElement SET ReadonlyProp1=10, ReadonlyProp2=20"));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE ONLY ts.SubElement SET ReadonlyProp1=10, ReadonlyProp2=20 ECSQLOPTIONS ReadonlyPropertiesAreUpdatable"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    //verify update worked
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ReadonlyProp1, ReadonlyProp2 FROM ts.SubElement WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(10, stmt.GetValueInt(0));
    ASSERT_EQ(20, stmt.GetValueInt(1));
    }



//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
struct PropertyPathEntry
    {
    Utf8String m_entry;
    bool m_isArrayIndex;

    PropertyPathEntry(Utf8CP entry, bool isArrayIndex) :m_entry(entry), m_isArrayIndex(isArrayIndex) {}
    };

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void AssertColumnInfo(Utf8CP expectedPropertyName, bool expectedIsSystem, bool expectedIsGenerated, Utf8CP expectedPropPathStr, Utf8CP expectedRootClassName, Utf8CP expectedRootClassAlias, ECSqlColumnInfoCR actualColumnInfo)
    {
    auto actualProperty = actualColumnInfo.GetProperty();
    if (expectedPropertyName == nullptr)
        {
        EXPECT_TRUE(actualProperty == nullptr);
        }
    else
        {
        ASSERT_TRUE(actualProperty != nullptr);
        EXPECT_STREQ(expectedPropertyName, actualProperty->GetName().c_str());
        }

    EXPECT_EQ(expectedIsSystem, actualColumnInfo.IsSystemProperty());
    EXPECT_EQ(expectedIsGenerated, actualColumnInfo.IsGeneratedProperty());

    ECSqlPropertyPath const& actualPropPath = actualColumnInfo.GetPropertyPath();
    Utf8String actualPropPathStr = actualPropPath.ToString();
    EXPECT_STREQ(expectedPropPathStr, actualPropPathStr.c_str());
    LOG.tracev("Property path: %s", actualPropPathStr.c_str());

    bvector<PropertyPathEntry> expectedPropPathEntries;
    bvector<Utf8String> expectedPropPathTokens;
    BeStringUtilities::Split(expectedPropPathStr, ".", expectedPropPathTokens);
    for (Utf8StringCR token : expectedPropPathTokens)
        {
        bvector<Utf8String> arrayTokens;
        BeStringUtilities::Split(token.c_str(), "[]", arrayTokens);

        if (arrayTokens.size() == 1)
            {
            expectedPropPathEntries.push_back(PropertyPathEntry(token.c_str(), false));
            continue;
            }

        ASSERT_EQ(2, arrayTokens.size());
        expectedPropPathEntries.push_back(PropertyPathEntry(arrayTokens[0].c_str(), false));
        expectedPropPathEntries.push_back(PropertyPathEntry(arrayTokens[1].c_str(), true));
        }

    ASSERT_EQ(expectedPropPathEntries.size(), actualPropPath.Size());

    size_t expectedPropPathEntryIx = 0;
    for (ECSqlPropertyPath::EntryCP propPathEntry : actualPropPath)
        {
        PropertyPathEntry const& expectedEntry = expectedPropPathEntries[expectedPropPathEntryIx];
        if (expectedEntry.m_isArrayIndex)
            {
            ASSERT_EQ(ECSqlPropertyPath::Entry::Kind::ArrayIndex, propPathEntry->GetKind());
            ASSERT_EQ(atoi(expectedEntry.m_entry.c_str()), propPathEntry->GetArrayIndex());
            }
        else
            {
            ASSERT_EQ(ECSqlPropertyPath::Entry::Kind::Property, propPathEntry->GetKind());
            ASSERT_STREQ(expectedEntry.m_entry.c_str(), propPathEntry->GetProperty()->GetName().c_str());
            }

        expectedPropPathEntryIx++;
        }

    EXPECT_STREQ(expectedRootClassName, actualColumnInfo.GetRootClass().GetClass().GetName().c_str());
    if (expectedRootClassAlias == nullptr)
        EXPECT_TRUE(actualColumnInfo.GetRootClass().GetAlias().empty());
    else
        EXPECT_STREQ(expectedRootClassAlias, actualColumnInfo.GetRootClass().GetAlias().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, ColumnInfoForPrimitiveArrays)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::Readonly)));
    ASSERT_EQ(SUCCESS, PopulateECDb( 10));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT c.Dt_Array FROM ecsql.PSA c LIMIT 1"));

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    //Top level column
    ECSqlColumnInfoCR topLevelColumnInfo = stmt.GetColumnInfo(0);
    AssertColumnInfo("Dt_Array", false, false, "Dt_Array", "PSA", "c", topLevelColumnInfo);
    IECSqlValue const& topLevelArrayValue = stmt.GetValue(0);

    //out of bounds test
    ASSERT_FALSE(stmt.GetColumnInfo(-1).IsValid()) << "ECSqlStatement::GetColumnInfo (-1) is expected to fail";
    ASSERT_FALSE(stmt.GetColumnInfo(2).IsValid()) << "ECSqlStatement::GetColumnInfo is expected to fail with too large index";

    //In array level
    int arrayIndex = 0;
    for (IECSqlValue const& arrayElement : topLevelArrayValue.GetArrayIterable())
        {
        ECSqlColumnInfoCR arrayElementColumnInfo = arrayElement.GetColumnInfo();
        Utf8String expectedPropPath;
        expectedPropPath.Sprintf("Dt_Array[%d]", arrayIndex);
        AssertColumnInfo(nullptr, false, false, expectedPropPath.c_str(), "PSA", "c", arrayElementColumnInfo);

        arrayIndex++;
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, ColumnInfoForStructs)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::Readonly)));
    ASSERT_EQ(SUCCESS, PopulateECDb( 10));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT SAStructProp FROM ecsql.SA LIMIT 1"));

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    //Top level column
    ECSqlColumnInfo const& topLevelColumnInfo = stmt.GetColumnInfo(0);
    AssertColumnInfo("SAStructProp", false, false, "SAStructProp", "SA", nullptr, topLevelColumnInfo);

    //out of bounds test
    {
    ASSERT_FALSE(stmt.GetColumnInfo(-1).IsValid()) << "Index out of range";
    ASSERT_FALSE(stmt.GetColumnInfo(2).IsValid()) << "Index out of range";
    }

    //SAStructProp.PStructProp level
    IECSqlValue const& topLevelStructValue = stmt.GetValue(0);
    AssertColumnInfo("PStructProp", false, false, "SAStructProp.PStructProp", "SA", nullptr, topLevelStructValue["PStructProp"].GetColumnInfo());

    //SAStructProp.PStructProp.XXX level
    IECSqlValue const& nestedStructValue = topLevelStructValue["PStructProp"];
    AssertColumnInfo("b", false, false, "SAStructProp.PStructProp.b", "SA", nullptr, nestedStructValue["b"].GetColumnInfo());
    AssertColumnInfo("bi", false, false, "SAStructProp.PStructProp.bi", "SA", nullptr, nestedStructValue["bi"].GetColumnInfo());
    AssertColumnInfo("p2d", false, false, "SAStructProp.PStructProp.p2d", "SA", nullptr, nestedStructValue["p2d"].GetColumnInfo());

    //invalid struct members
    ASSERT_FALSE(nestedStructValue[""].GetColumnInfo().IsValid());
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, ColumnInfoForStructArrays)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::Readonly)));
    ASSERT_EQ(SUCCESS, PopulateECDb( 10));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT SAStructProp FROM ecsql.SA LIMIT 1"));

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    //Top level column
    auto const& topLevelColumnInfo = stmt.GetColumnInfo(0);
    AssertColumnInfo("SAStructProp", false, false, "SAStructProp", "SA", nullptr, topLevelColumnInfo);
    auto const& topLevelStructValue = stmt.GetValue(0);

    ASSERT_FALSE(stmt.GetColumnInfo(-1).IsValid());

    //SAStructProp.PStruct_Array level
    auto const& pstructArrayValue = topLevelStructValue["PStruct_Array"];
    AssertColumnInfo("PStruct_Array", false, false, "SAStructProp.PStruct_Array", "SA", nullptr, pstructArrayValue.GetColumnInfo());

    //out of bounds test
    ASSERT_FALSE(topLevelStructValue[""].GetColumnInfo().IsValid()) << "GetValue ("").GetColumnInfo () for struct value";

    //SAStructProp.PStruct_Array[] level
    int arrayIndex = 0;
    Utf8String expectedPropPath;
    for (IECSqlValue const& arrayElement : pstructArrayValue.GetArrayIterable())
        {
        //first struct member
        auto const& arrayElementFirstColumnInfo = arrayElement["b"].GetColumnInfo();
        ASSERT_FALSE(arrayElementFirstColumnInfo.IsValid());

        expectedPropPath.Sprintf("SAStructProp.PStruct_Array[%d].b", arrayIndex);
        AssertColumnInfo("b", false, false, expectedPropPath.c_str(), "SA", nullptr, arrayElementFirstColumnInfo);

        //second struct member
        auto const& arrayElementSecondColumnInfo = arrayElement["bi"].GetColumnInfo();
        expectedPropPath.Sprintf("SAStructProp.PStruct_Array[%d].bi", arrayIndex);
        AssertColumnInfo("bi", false, false, expectedPropPath.c_str(), "SA", nullptr, arrayElementSecondColumnInfo);

        ASSERT_FALSE(arrayElement["foo"].GetColumnInfo().IsValid());

        arrayIndex++;
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, GetArrayValue)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("GetArrayValue.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    ECInstanceKey key;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.PSA(I_Array,L_Array,D_Array) VALUES(?,?,?)"));
    IECSqlBinder& arrayBinder = stmt.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindInt(1));
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindInt(2));
    IECSqlBinder& arrayBinder2 = stmt.GetBinder(2);
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder2.AddArrayElement().BindInt64(1001));
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder2.AddArrayElement().BindInt64(1002));
    IECSqlBinder& arrayBinder3 = stmt.GetBinder(3);
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder3.AddArrayElement().BindDouble(1.0));
    ASSERT_EQ(ECSqlStatus::Success, arrayBinder3.AddArrayElement().BindDouble(2.0));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    }

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT I_Array,L_Array,D_Array FROM ecsql.PSA WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    int arrayIndex = 0;
    for (IECSqlValue const& arrayElement : stmt.GetValue(0).GetArrayIterable())
        {
        ASSERT_EQ(arrayIndex + 1, arrayElement.GetInt()) << "Int array element must be callable with GetInt";
        ASSERT_EQ(arrayIndex + 1, arrayElement.GetInt64()) << "Int array element must be callable with GetInt64";
        ASSERT_EQ(arrayIndex + 1, arrayElement.GetDouble()) << "Int array element must be callable with GetDouble";
        arrayIndex++;
        }

    arrayIndex = 0;
    for (IECSqlValue const& arrayElement : stmt.GetValue(1).GetArrayIterable())
        {
        ASSERT_EQ(arrayIndex + 1001, arrayElement.GetInt64()) << "Int64 array element must be callable with GetInt64";
        ASSERT_DOUBLE_EQ(arrayIndex + 1001, arrayElement.GetDouble()) << "Int64 array element must be callable with GetDouble";
        ASSERT_EQ(0, arrayElement.GetInt()) << "Int64 array element cannot be called with GetInt";
        arrayIndex++;
        }

    arrayIndex = 0;
    for (IECSqlValue const& arrayElement : stmt.GetValue(2).GetArrayIterable())
        {
        ASSERT_DOUBLE_EQ(arrayIndex + 1, arrayElement.GetDouble()) << "Double array element must be callable with GetDouble";
        ASSERT_EQ(0, arrayElement.GetInt64()) << "Double array element cannot be called with GetInt64";
        ASSERT_EQ(0, arrayElement.GetInt()) << "Double array element cannot be called with GetInt";
        arrayIndex++;
        }

    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, Step)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, PopulateECDb( 10));

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT * FROM ecsql.P"));

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step()) << "Step() on ECSQL SELECT statement is expected to be allowed.";
    }

    {
    ECSqlStatement statement;
    ECInstanceKey ecInstanceKey;
    auto stepStat = statement.Step(ecInstanceKey);
    ASSERT_EQ(BE_SQLITE_ERROR, stepStat) << "Step(ECInstanceKey&) on ECSQL SELECT statement is expected to not be allowed.";
    }

    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ecsql.P (I, L) VALUES (100, 10203)"));

    auto stepStat = statement.Step();
    ASSERT_EQ(BE_SQLITE_DONE, stepStat) << "Step() on ECSQL INSERT statement is expected to be allowed.";

    statement.Reset();
    ECInstanceKey ecInstanceKey;
    stepStat = statement.Step(ecInstanceKey);
    ASSERT_EQ(BE_SQLITE_DONE, stepStat) << "Step(ECInstanceKey&) on ECSQL INSERT statement is expected to be allowed.";
    }
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, MultipleInsertsWithoutReprepare)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, PopulateECDb( 10));

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ecsql.PSA (I, S) VALUES (?, ?)"));

    int firstIntVal = 1;
    Utf8CP firstStringVal = "First insert";
    ECSqlStatus stat = statement.BindInt(1, firstIntVal);
    ASSERT_EQ(ECSqlStatus::Success, stat);
    stat = statement.BindText(2, firstStringVal, IECSqlBinder::MakeCopy::No);
    ASSERT_EQ(ECSqlStatus::Success, stat);

    ECInstanceKey firstECInstanceKey;
    auto stepStat = statement.Step(firstECInstanceKey);
    ASSERT_EQ(BE_SQLITE_DONE, stepStat);

    stat = statement.Reset();
    ASSERT_EQ(ECSqlStatus::Success, stat);
    stat = statement.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stat);

    //second insert with same statement

    int secondIntVal = 2;
    Utf8CP secondStringVal = "Second insert";
    stat = statement.BindInt(1, secondIntVal);
    ASSERT_EQ(ECSqlStatus::Success, stat);
    stat = statement.BindText(2, secondStringVal, IECSqlBinder::MakeCopy::No);
    ASSERT_EQ(ECSqlStatus::Success, stat);

    ECInstanceKey secondECInstanceKey;
    stepStat = statement.Step(secondECInstanceKey);
    ASSERT_EQ(BE_SQLITE_DONE, stepStat);

    //check the inserts
    ASSERT_GT(secondECInstanceKey.GetInstanceId().GetValue(), firstECInstanceKey.GetInstanceId().GetValue());

    statement.Finalize();
    stat = statement.Prepare(m_ecdb, "SELECT ECInstanceId, I, S FROM ecsql.PSA WHERE ECInstanceId = ?");
    ASSERT_EQ(ECSqlStatus::Success, stat);

    //check first insert
    stat = statement.BindId(1, firstECInstanceKey.GetInstanceId());
    ASSERT_EQ(ECSqlStatus::Success, stat);

    stepStat = statement.Step();
    ASSERT_EQ(BE_SQLITE_ROW, stepStat);
    ASSERT_EQ(firstECInstanceKey.GetInstanceId().GetValue(), statement.GetValueId<ECInstanceId>(0).GetValue());
    ASSERT_EQ(firstIntVal, statement.GetValueInt(1));
    ASSERT_STREQ(firstStringVal, statement.GetValueText(2));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());

    //check second insert
    stat = statement.Reset();
    ASSERT_EQ(ECSqlStatus::Success, stat);
    stat = statement.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stat);

    stat = statement.BindId(1, secondECInstanceKey.GetInstanceId());
    ASSERT_EQ(ECSqlStatus::Success, stat);

    stepStat = statement.Step();
    ASSERT_EQ(BE_SQLITE_ROW, stepStat);
    ASSERT_EQ(secondECInstanceKey.GetInstanceId().GetValue(), statement.GetValueId<ECInstanceId>(0).GetValue());
    ASSERT_EQ(secondIntVal, statement.GetValueInt(1));
    ASSERT_STREQ(secondStringVal, statement.GetValueText(2));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, Reset)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::Readonly)));
    ASSERT_EQ(SUCCESS, PopulateECDb( 10));

    {
    ECSqlStatement stmt;
    auto stat = stmt.Prepare(m_ecdb, "SELECT * FROM ecsql.P LIMIT 2");
    ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation for a valid ECSQL failed.";
    int expectedRowCount = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        expectedRowCount++;
        }

    stat = stmt.Reset();
    ASSERT_EQ(ECSqlStatus::Success, stat) << "After ECSqlStatement::Reset";
    int actualRowCount = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        actualRowCount++;
        }
    ASSERT_EQ(expectedRowCount, actualRowCount) << "After resetting ECSqlStatement is expected to return same number of returns as after preparation.";
    }
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, Finalize)
    {
    const int perClassRowCount = 10;
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::Readonly)));
    ASSERT_EQ(SUCCESS, PopulateECDb( perClassRowCount));

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT * FROM blablabla")) << "Preparation for an invalid ECSQL succeeded unexpectedly.";

    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * FROM ecsql.P")) << "Preparation for a valid ECSQL failed.";
    int actualRowCount = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        actualRowCount++;
        }

    ASSERT_EQ(perClassRowCount, actualRowCount);
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * FROM ecsql.P")) << "Preparation for a valid ECSQL failed.";

    int actualRowCount = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        actualRowCount++;
        }
    ASSERT_EQ(perClassRowCount, actualRowCount);

    //now finalize and do the exact same stuff. In particular this tests that the cursor is reset so that we get all results
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * FROM ecsql.P")) << "Preparation for a valid ECSQL failed.";
    actualRowCount = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        actualRowCount++;
        }

    ASSERT_EQ(perClassRowCount, actualRowCount);
    }
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, IssueListener)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, PopulateECDb( 10));

    {
    ECIssueListener issueListener(m_ecdb);
    ECSqlStatement stmt;
    ASSERT_FALSE(issueListener.GetIssue().has_value()) << "new ECSqlStatement";

    auto stat = stmt.Prepare(m_ecdb, "SELECT * FROM ecsql.P WHERE I = ?");
    ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation for a valid ECSQL failed.";
    ASSERT_FALSE(issueListener.GetIssue().has_value()) << "After successful call to Prepare.";
    }

    {
    ECIssueListener issueListener(m_ecdb);

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT * FROM blablabla")) << "Preparation for an invalid ECSQL succeeded unexpectedly.";

    ECDbIssue lastIssue = issueListener.GetIssue();
    ASSERT_TRUE(lastIssue.has_value()) << "After preparing invalid ECSQL.";
    ASSERT_STREQ("Invalid ECSQL class expression 'blablabla': Valid syntax: [<table space>.]<schema name or alias>.<class name>[.function call]", lastIssue.message.c_str());

    stmt.Finalize();
    ASSERT_FALSE(issueListener.GetIssue().has_value()) << "After successful call to Finalize";

    //now reprepare with valid ECSQL
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * FROM ecsql.P")) << "Preparation for a valid ECSQL failed.";
    ASSERT_FALSE(issueListener.GetIssue().has_value()) << "After successful call to Prepare";
    }

    }


//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, GetValueWithPartialPoints)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("jsonreaderpartialpoints.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    ECClassCP testClass = m_ecdb.Schemas().GetClass("ECSqlTest", "PSA");
    ASSERT_TRUE(testClass != nullptr);

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.PSA(P2D.X,P3D.Y,PStructProp.p2d.y,PStructProp.p3d.z) VALUES(1.0, 2.0, 3.0, 4.0)"));
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << stmt.GetECSql();
    stmt.Finalize();

    ECSqlStatement selStmt;
    ASSERT_EQ(ECSqlStatus::Success, selStmt.Prepare(m_ecdb, "SELECT P2D,P3D,PStructProp.p2d,PStructProp.p3d FROM ecsql.PSA WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, selStmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, selStmt.Step());

    //The coordinates of partial points being NULL in the ECDb file will be returned as 0 as this is what SQLite
    //implicitly returns when calling GetValueDouble on a NULL column.
    ASSERT_TRUE(DPoint2d::From(1.0, 0.0).AlmostEqual(selStmt.GetValuePoint2d(0))) << selStmt.GetECSql();
    ASSERT_TRUE(DPoint3d::From(0.0, 2.0, 0.0).AlmostEqual(selStmt.GetValuePoint3d(1))) << selStmt.GetECSql();
    ASSERT_TRUE(DPoint2d::From(0.0, 3.0).AlmostEqual(selStmt.GetValuePoint2d(2))) << selStmt.GetECSql();
    ASSERT_TRUE(DPoint3d::From(0.0, 0.0, 4.0).AlmostEqual(selStmt.GetValuePoint3d(3))) << selStmt.GetECSql();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void AssertGeometry(IGeometryCR expected, IGeometryCR actual, Utf8CP assertMessage)
    {
    ASSERT_TRUE(actual.IsSameStructureAndGeometry(expected)) << assertMessage;
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, Geometry)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    std::vector<IGeometryPtr> expectedGeoms;

    IGeometryPtr line1 = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, 1.0, 1.0, 1.0)));
    IGeometryPtr line2 = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(1.0, 1.0, 1.0, 2.0, 2.0, 2.0)));
    IGeometryPtr line3 = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(2.0, 2.0, 2.0, 3.0, 3.0, 3.0)));

    expectedGeoms.push_back(line1);
    expectedGeoms.push_back(line2);
    expectedGeoms.push_back(line3);
    IGeometryPtr expectedGeomSingle = expectedGeoms[0];

    // insert geometries in various variations
    {
    auto ecsql = "INSERT INTO ecsql.PASpatial (Geometry, B, Geometry_Array) VALUES(?, True, ?)";

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, ecsql)) << "Preparation of '" << ecsql << "' failed";

    ASSERT_EQ(ECSqlStatus::Success, statement.BindGeometry(1, *expectedGeomSingle));

    IECSqlBinder& arrayBinder = statement.GetBinder(2);
    for (auto& geom : expectedGeoms)
        {
        ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindGeometry(*geom));
        }

    ASSERT_EQ((int) BE_SQLITE_DONE, (int) statement.Step());
    }

    {
    auto ecsql = "INSERT INTO ecsql.SSpatial (SpatialStructProp.Geometry, SpatialStructProp.Geometry_Array) VALUES(?,?)";

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, ecsql)) << "Preparation of '" << ecsql << "' failed";

    ASSERT_EQ(ECSqlStatus::Success, statement.BindGeometry(1, *expectedGeomSingle));

    IECSqlBinder& arrayBinder = statement.GetBinder(2);
    for (auto& geom : expectedGeoms)
        {
        ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindGeometry(*geom));
        }

    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    }

    {
    auto ecsql = "INSERT INTO ecsql.SSpatial (SpatialStructProp) VALUES(?)";

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, ecsql)) << "Preparation of '" << ecsql << "' failed";

    IECSqlBinder& structBinder = statement.GetBinder(1);

    ASSERT_EQ(ECSqlStatus::Success, structBinder["Geometry"].BindGeometry(*expectedGeomSingle));

    IECSqlBinder& arrayBinder = structBinder["Geometry_Array"];
    for (auto& geom : expectedGeoms)
        {
        ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindGeometry(*geom));
        }

    ASSERT_EQ((int) BE_SQLITE_DONE, (int) statement.Step());
    }

    m_ecdb.SaveChanges();

    //now verify the inserts
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT B, Geometry_Array, Geometry FROM ecsql.PASpatial")) << "Preparation failed";
    int rowCount = 0;
    while (statement.Step() == BE_SQLITE_ROW)
        {
        rowCount++;

        ASSERT_TRUE(statement.GetValueBoolean(0)) << "First column value is expected to be true";

        IECSqlValue const& arrayVal = statement.GetValue(1);
        int i = 0;
        for (IECSqlValue const& arrayElem : arrayVal.GetArrayIterable())
            {
            IGeometryPtr actualGeom = arrayElem.GetGeometry();
            ASSERT_TRUE(actualGeom != nullptr);

            AssertGeometry(*expectedGeoms[i], *actualGeom, "PASpatial.Geometry_Array");
            i++;
            }
        ASSERT_EQ((int) expectedGeoms.size(), i);

        IGeometryPtr actualGeom = statement.GetValueGeometry(2);
        ASSERT_TRUE(actualGeom != nullptr);
        AssertGeometry(*expectedGeomSingle, *actualGeom, "PASpatial.Geometry");
        }
    ASSERT_EQ(1, rowCount);

    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT SpatialStructProp.Geometry_Array, SpatialStructProp.Geometry FROM ecsql.SSpatial")) << "Preparation failed";
    rowCount = 0;
    while (statement.Step() == BE_SQLITE_ROW)
        {
        rowCount++;

        IECSqlValue const& arrayVal = statement.GetValue(0);
        int i = 0;
        for (IECSqlValue const& arrayElem : arrayVal.GetArrayIterable())
            {
            IGeometryPtr actualGeom = arrayElem.GetGeometry();
            ASSERT_TRUE(actualGeom != nullptr);

            AssertGeometry(*expectedGeoms[i], *actualGeom, "SSpatial.SpatialStructProp.Geometry_Array");
            i++;
            }
        ASSERT_EQ((int) expectedGeoms.size(), i);

        IGeometryPtr actualGeom = statement.GetValueGeometry(1);
        ASSERT_TRUE(actualGeom != nullptr);
        AssertGeometry(*expectedGeomSingle, *actualGeom, "SSpatial.SpatialStructProp.Geometry");
        }
    ASSERT_EQ(2, rowCount);

    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT SpatialStructProp FROM ecsql.SSpatial")) << "Preparation failed";
    rowCount = 0;
    while (statement.Step() == BE_SQLITE_ROW)
        {
        rowCount++;

        IECSqlValue const& structVal = statement.GetValue(0);
        for (IECSqlValue const& structMemberVal : structVal.GetStructIterable())
            {
            Utf8StringCR structMemberName = structMemberVal.GetColumnInfo().GetProperty()->GetName();
            if (structMemberName.Equals("Geometry"))
                {
                IGeometryPtr actualGeom = structMemberVal.GetGeometry();
                AssertGeometry(*expectedGeomSingle, *actualGeom, "SSpatial.SpatialStructProp > Geometry");
                }
            else if (structMemberName.Equals("Geometry_Array"))
                {
                int i = 0;
                for (IECSqlValue const& arrayElem : structMemberVal.GetArrayIterable())
                    {
                    IGeometryPtr actualGeom = arrayElem.GetGeometry();
                    ASSERT_TRUE(actualGeom != nullptr);

                    AssertGeometry(*expectedGeoms[i], *actualGeom, "SSpatial.SpatialStructProp > Geometry_Array");
                    i++;
                    }
                ASSERT_EQ((int) expectedGeoms.size(), i);
                }
            }
        }
    ASSERT_EQ(2, rowCount);
    }


//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, GeometryAndOverflow)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsql_geometry.ecdb", SchemaItem(R"xml(
                            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                             <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                              <ECEntityClass typeName="Element">
                                 <ECCustomAttributes>
                                    <ClassMap xmlns='ECDbMap.02.00'>
                                       <MapStrategy>TablePerHierarchy</MapStrategy>
                                    </ClassMap>
                                    <ShareColumns xmlns='ECDbMap.02.00'>
                                        <MaxSharedColumnsBeforeOverflow>2</MaxSharedColumnsBeforeOverflow>
                                    </ShareColumns>
                                 </ECCustomAttributes>
                                <ECProperty propertyName="Geom" typeName="Bentley.Geometry.Common.IGeometry" />
                                <ECArrayProperty propertyName="GeomArray" typeName="Bentley.Geometry.Common.IGeometry" />
                                <ECProperty propertyName="Geom_Overflow" typeName="Bentley.Geometry.Common.IGeometry" />
                                <ECArrayProperty propertyName="GeomArray_Overflow" typeName="Bentley.Geometry.Common.IGeometry" />
                              </ECEntityClass>
                            </ECSchema>)xml")));

    std::vector<IGeometryPtr> expectedGeoms {IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, 1.0, 1.0, 1.0))),
        IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(1.0, 1.0, 1.0, 2.0, 2.0, 2.0))),
        IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(2.0, 2.0, 2.0, 3.0, 3.0, 3.0)))};

    IGeometryPtr expectedGeomSingle = expectedGeoms[0];

    ECInstanceKey key;
    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ts.Element(Geom,GeomArray,Geom_Overflow,GeomArray_Overflow) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindGeometry(1, *expectedGeomSingle));
    IECSqlBinder& arrayBinder = statement.GetBinder(2);
    for (IGeometryPtr& geom : expectedGeoms)
        {
        ASSERT_EQ(ECSqlStatus::Success, arrayBinder.AddArrayElement().BindGeometry(*geom));
        }

    ASSERT_EQ(ECSqlStatus::Success, statement.BindGeometry(3, *expectedGeomSingle));
    IECSqlBinder& arrayBinder2 = statement.GetBinder(4);
    for (IGeometryPtr& geom : expectedGeoms)
        {
        ASSERT_EQ(ECSqlStatus::Success, arrayBinder2.AddArrayElement().BindGeometry(*geom));

        }

    ASSERT_EQ(BE_SQLITE_DONE, statement.Step(key)) << statement.GetECSql() << " " << statement.GetNativeSql();
    }

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT Geom,GeomArray,Geom_Overflow,GeomArray_Overflow FROM ts.Element WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step()) << statement.GetECSql();

    AssertGeometry(*expectedGeomSingle, *statement.GetValueGeometry(0), "Geometry property");
    size_t arrayIndex = 0;
    for (IECSqlValue const& arrayElementVal : statement.GetValue(1).GetArrayIterable())
        {
        AssertGeometry(*expectedGeoms[arrayIndex], *arrayElementVal.GetGeometry(), "Geometry array property");
        arrayIndex++;
        }
    AssertGeometry(*expectedGeomSingle, *statement.GetValueGeometry(2), "Geometry property overflow");
    arrayIndex = 0;
    for (IECSqlValue const& arrayElementVal : statement.GetValue(3).GetArrayIterable())
        {
        AssertGeometry(*expectedGeoms[arrayIndex], *arrayElementVal.GetGeometry(), "Geometry array property overflow");
        arrayIndex++;
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, GetGeometryWithInvalidBlobFormat)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    // insert invalid geom blob
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "INSERT INTO ecsqltest_PASpatial(Id, Geometry) VALUES (1,?)"));
    double dummyValue = 3.141516;
    ASSERT_EQ(BE_SQLITE_OK, stmt.BindBlob(1, &dummyValue, (int) sizeof(dummyValue), Statement::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ECSqlStatement ecsqlStmt;
    ASSERT_EQ(ECSqlStatus::Success, ecsqlStmt.Prepare(m_ecdb, "SELECT Geometry FROM ecsql.PASpatial"));
    int rowCount = 0;
    while (ecsqlStmt.Step() == BE_SQLITE_ROW)
        {
        rowCount++;
        ASSERT_FALSE(ecsqlStmt.IsValueNull(0)) << "Geometry column is not expected to be null.";

        ASSERT_TRUE(ecsqlStmt.GetValueGeometry(0) == nullptr) << "Invalid geom blob format expected so that nullptr is returned.";
        }

    ASSERT_EQ(1, rowCount);
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, ClassWithStructHavingStructArrayInsert)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ecsql.SA (SAStructProp) VALUES(?)"));

    IECSqlBinder& saStructBinder = statement.GetBinder(1); //SAStructProp
    ASSERT_EQ(ECSqlStatus::Success, saStructBinder["PStructProp"]["i"].BindInt(99));

    //add three array elements
    const int count = 3;
    IECSqlBinder& arrayBinder = saStructBinder["PStruct_Array"];
    for (int i = 0; i < count; i++)
        {
        IECSqlBinder& arrayElementBinder = arrayBinder.AddArrayElement();
        ASSERT_EQ(ECSqlStatus::Success, arrayElementBinder["d"].BindDouble(i * PI));
        ASSERT_EQ(ECSqlStatus::Success, arrayElementBinder["i"].BindInt(i * 2));
        ASSERT_EQ(ECSqlStatus::Success, arrayElementBinder["l"].BindInt64(i * 3));
        ASSERT_EQ(ECSqlStatus::Success, arrayElementBinder["p2d"].BindPoint2d(DPoint2d::From(i, i + 1)));
        ASSERT_EQ(ECSqlStatus::Success, arrayElementBinder["p3d"].BindPoint3d(DPoint3d::From(i, i + 1, i + 2)));
        }

    ASSERT_EQ(BE_SQLITE_DONE, statement.Step()) << statement.GetECSql();
    statement.Finalize();

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT SAStructProp.PStruct_Array FROM ecsql.SA"));
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        auto const& pStructArray = stmt.GetValue(0);
        ASSERT_EQ(3, pStructArray.GetArrayLength());
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, StructArrayInsertWithParametersLongAndArray)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    auto ecsql = "INSERT INTO ecsql.PSA (L,PStruct_Array) VALUES(123, ?)";
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, ecsql)) << "Preparation of '" << ecsql << "' failed";

    //add three array elements
    const int count = 3;
    IECSqlBinder& arrayBinder = statement.GetBinder(1);
    for (int i = 0; i < count; i++)
        {
        IECSqlBinder& arrayElementBinder = arrayBinder.AddArrayElement();
        auto stat = arrayElementBinder["d"].BindDouble(i * PI);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["i"].BindInt(i * 2);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["l"].BindInt64(i * 3);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["p2d"].BindPoint2d(DPoint2d::From(i, i + 1));
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["p3d"].BindPoint3d(DPoint3d::From(i, i + 1, i + 2));
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        }

    auto stepStatus = statement.Step();
    ASSERT_EQ((int) BE_SQLITE_DONE, (int) stepStatus) << "Step for '" << ecsql << "' failed";
    statement.Finalize();
    ECSqlStatement stmt;
    auto prepareStatus = stmt.Prepare(m_ecdb, "SELECT * FROM ecsql.PSA");
    ASSERT_TRUE(prepareStatus == ECSqlStatus::Success);
    ECInstanceECSqlSelectAdapter classPReader(stmt);
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        auto inst = classPReader.GetInstance();
        ECValue v;
        inst->GetValue(v, "L");
        ASSERT_EQ(123, v.GetLong());
        inst->GetValue(v, "PStruct_Array");
        ASSERT_EQ(count, v.GetArrayInfo().GetCount());
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, SystemProperties) {
    ASSERT_EQ(SUCCESS, SetupECDb("system.ecdb"));

    auto sysClass = m_ecdb.Schemas().GetClass("ECDbSystem", "ClassECSqlSystemProperties");
    ASSERT_NE(sysClass, nullptr);

    auto sysRelationship = m_ecdb.Schemas().GetClass("ECDbSystem","RelationshipECSqlSystemProperties");
    ASSERT_NE(sysRelationship, nullptr);

    auto sysNavigation = m_ecdb.Schemas().GetClass("ECDbSystem","NavigationECSqlSystemProperties");
    ASSERT_NE(sysNavigation, nullptr);

    auto propClassId = sysClass->GetPropertyP("ECClassId");
    ASSERT_NE(propClassId, nullptr);
    ASSERT_STREQ(propClassId->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "ClassId");

    auto propSourceClassId = sysRelationship->GetPropertyP("SourceECClassId");
    ASSERT_NE(propSourceClassId, nullptr);
    ASSERT_STREQ(propSourceClassId->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "SourceClassId");

    auto propTargetClassId = sysRelationship->GetPropertyP("TargetECClassId");
    ASSERT_NE(propTargetClassId, nullptr);
    ASSERT_STREQ(propTargetClassId->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "TargetClassId");

    auto propRelClassId = sysNavigation->GetPropertyP("RelECClassId");
    ASSERT_NE(propRelClassId, nullptr);
    ASSERT_STREQ(propRelClassId->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "NavRelClassId");

    auto propInstanceId = sysClass->GetPropertyP("ECInstanceId");
    ASSERT_NE(propInstanceId, nullptr);
    ASSERT_STREQ(propInstanceId->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "Id");

    auto propSourceInstanceId = sysRelationship->GetPropertyP("SourceECInstanceId");
    ASSERT_NE(propSourceInstanceId, nullptr);
    ASSERT_STREQ(propSourceInstanceId->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "SourceId");

    auto propTargetInstanceId = sysRelationship->GetPropertyP("TargetECInstanceId");
    ASSERT_NE(propTargetInstanceId, nullptr);
    ASSERT_STREQ(propTargetInstanceId->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "TargetId");

    auto propId = sysNavigation->GetPropertyP("Id");
    ASSERT_NE(propId, nullptr);
    ASSERT_STREQ(propId->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "NavId");

    // meta.ClassHasBaseClasses

    if ("sub query relationship") {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success,
        stmt.Prepare(m_ecdb, R"(
            SELECT * FROM (SELECT
                D.ECInstanceId         ,
                D.ECInstanceId       a1,
                D.ECClassId            ,
                D.ECClassId          a2,
                D.SourceECInstanceId   ,
                D.SourceECInstanceId a3,
                D.SourceECClassId      ,
                D.SourceECClassId    a4,
                D.TargetECInstanceId   ,
                D.TargetECInstanceId a5,
                D.TargetECClassId      ,
                D.TargetECClassId    a6
            FROM meta.ClassHasBaseClasses D))"));

        ASSERT_FALSE(stmt.GetColumnInfo( 0).IsGeneratedProperty());// D.ECInstanceId            CORRECT
        ASSERT_TRUE (stmt.GetColumnInfo( 1).IsGeneratedProperty());// D.ECInstanceId       a1   CORRECT
        ASSERT_FALSE(stmt.GetColumnInfo( 2).IsGeneratedProperty());// D.ECClassId               CORRECT
        ASSERT_TRUE (stmt.GetColumnInfo( 3).IsGeneratedProperty());// D.ECClassId          a2   CORRECT
        ASSERT_FALSE(stmt.GetColumnInfo( 4).IsGeneratedProperty());// D.SourceECInstanceId      CORRECT
        ASSERT_TRUE (stmt.GetColumnInfo( 5).IsGeneratedProperty());// D.SourceECInstanceId a3   CORRECT
        ASSERT_FALSE(stmt.GetColumnInfo( 6).IsGeneratedProperty());// D.SourceECClassId         CORRECT
        ASSERT_TRUE (stmt.GetColumnInfo( 7).IsGeneratedProperty());// D.SourceECClassId    a4   CORRECT
        ASSERT_FALSE(stmt.GetColumnInfo( 8).IsGeneratedProperty());// D.TargetECInstanceId      CORRECT
        ASSERT_TRUE (stmt.GetColumnInfo( 9).IsGeneratedProperty());// D.TargetECInstanceId a5   CORRECT
        ASSERT_FALSE(stmt.GetColumnInfo(10).IsGeneratedProperty());// D.TargetECClassId         CORRECT
        ASSERT_TRUE (stmt.GetColumnInfo(11).IsGeneratedProperty());// D.TargetECClassId    a6   CORRECT

        ASSERT_TRUE (stmt.GetColumnInfo( 0).IsSystemProperty());// D.ECInstanceId               CORRECT
        ASSERT_TRUE (stmt.GetColumnInfo( 1).IsSystemProperty());// D.ECInstanceId       a1      CORRECT
        ASSERT_TRUE (stmt.GetColumnInfo( 2).IsSystemProperty());// D.ECClassId                  CORRECT
        ASSERT_TRUE (stmt.GetColumnInfo( 3).IsSystemProperty());// D.ECClassId          a2      CORRECT
        ASSERT_TRUE (stmt.GetColumnInfo( 4).IsSystemProperty());// D.SourceECInstanceId         CORRECT
        ASSERT_TRUE (stmt.GetColumnInfo( 5).IsSystemProperty());// D.SourceECInstanceId a3      CORRECT
        ASSERT_TRUE (stmt.GetColumnInfo( 6).IsSystemProperty());// D.SourceECClassId            CORRECT
        ASSERT_TRUE (stmt.GetColumnInfo( 7).IsSystemProperty());// D.SourceECClassId    a4      CORRECT
        ASSERT_TRUE (stmt.GetColumnInfo( 8).IsSystemProperty());// D.TargetECInstanceId         CORRECT
        ASSERT_TRUE (stmt.GetColumnInfo( 9).IsSystemProperty());// D.TargetECInstanceId a5      CORRECT
        ASSERT_TRUE (stmt.GetColumnInfo(10).IsSystemProperty());// D.TargetECClassId            CORRECT
        ASSERT_TRUE (stmt.GetColumnInfo(11).IsSystemProperty());// D.TargetECClassId    a6      CORRECT

        EXPECT_STREQ(stmt.GetColumnInfo( 0).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "Id"            );// D.ECInstanceId               CORRECT
        EXPECT_STREQ(stmt.GetColumnInfo( 1).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "Id"            );// D.ECInstanceId       a1      CORRECT
        EXPECT_STREQ(stmt.GetColumnInfo( 2).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "ClassId"       );// D.ECClassId                  CORRECT
        EXPECT_STREQ(stmt.GetColumnInfo( 3).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "Id"            );// D.ECClassId          a2      WRONG?
        EXPECT_STREQ(stmt.GetColumnInfo( 4).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "SourceId"      );// D.SourceECInstanceId         CORRECT
        EXPECT_STREQ(stmt.GetColumnInfo( 5).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "Id"            );// D.SourceECInstanceId a3      WRONG?
        EXPECT_STREQ(stmt.GetColumnInfo( 6).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "SourceClassId" );// D.SourceECClassId            CORRECT
        EXPECT_STREQ(stmt.GetColumnInfo( 7).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "Id"            );// D.SourceECClassId    a4      WRONG?
        EXPECT_STREQ(stmt.GetColumnInfo( 8).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "TargetId"      );// D.TargetECInstanceId         CORRECT
        EXPECT_STREQ(stmt.GetColumnInfo( 9).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "Id"            );// D.TargetECInstanceId a5      WRONG?
        EXPECT_STREQ(stmt.GetColumnInfo(10).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "TargetClassId" );// D.TargetECClassId            CORRECT
        EXPECT_STREQ(stmt.GetColumnInfo(11).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "Id"            );// D.TargetECClassId    a6      WRONG?
    }

    if ("top level subquery relationship") {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success,
        stmt.Prepare(m_ecdb, R"(
            SELECT
                D.ECInstanceId         ,
                D.ECInstanceId       a1,
                D.ECClassId            ,
                D.ECClassId          a2,
                D.SourceECInstanceId   ,
                D.SourceECInstanceId a3,
                D.SourceECClassId      ,
                D.SourceECClassId    a4,
                D.TargetECInstanceId   ,
                D.TargetECInstanceId a5,
                D.TargetECClassId      ,
                D.TargetECClassId    a6
            FROM meta.ClassHasBaseClasses D)"));

        ASSERT_FALSE(stmt.GetColumnInfo( 0).IsGeneratedProperty());// D.ECInstanceId               CORRECT
        ASSERT_TRUE (stmt.GetColumnInfo( 1).IsGeneratedProperty());// D.ECInstanceId       a1      CORRECT
        ASSERT_FALSE(stmt.GetColumnInfo( 2).IsGeneratedProperty());// D.ECClassId                  CORRECT
        ASSERT_TRUE (stmt.GetColumnInfo( 3).IsGeneratedProperty());// D.ECClassId          a2      CORRECT
        ASSERT_FALSE(stmt.GetColumnInfo( 4).IsGeneratedProperty());// D.SourceECInstanceId         CORRECT
        ASSERT_TRUE (stmt.GetColumnInfo( 5).IsGeneratedProperty());// D.SourceECInstanceId a3      CORRECT
        ASSERT_FALSE(stmt.GetColumnInfo( 6).IsGeneratedProperty());// D.SourceECClassId            CORRECT
        ASSERT_TRUE (stmt.GetColumnInfo( 7).IsGeneratedProperty());// D.SourceECClassId    a4      CORRECT
        ASSERT_FALSE(stmt.GetColumnInfo( 8).IsGeneratedProperty());// D.TargetECInstanceId         CORRECT
        ASSERT_TRUE (stmt.GetColumnInfo( 9).IsGeneratedProperty());// D.TargetECInstanceId a5      CORRECT
        ASSERT_FALSE(stmt.GetColumnInfo(10).IsGeneratedProperty());// D.TargetECClassId            CORRECT
        ASSERT_TRUE (stmt.GetColumnInfo(11).IsGeneratedProperty());// D.TargetECClassId    a6      CORRECT

        ASSERT_TRUE (stmt.GetColumnInfo( 0).IsSystemProperty());    // D.ECInstanceId               CORRECT
        ASSERT_FALSE(stmt.GetColumnInfo( 1).IsSystemProperty());    // D.ECInstanceId       a1      WRONG
        ASSERT_TRUE (stmt.GetColumnInfo( 2).IsSystemProperty());    // D.ECClassId                  CORRECT
        ASSERT_FALSE(stmt.GetColumnInfo( 3).IsSystemProperty());    // D.ECClassId          a2      WRONG
        ASSERT_TRUE (stmt.GetColumnInfo( 4).IsSystemProperty());    // D.SourceECInstanceId         CORRECT
        ASSERT_FALSE(stmt.GetColumnInfo( 5).IsSystemProperty());    // D.SourceECInstanceId a3      WRONG
        ASSERT_TRUE (stmt.GetColumnInfo( 6).IsSystemProperty());    // D.SourceECClassId            CORRECT
        ASSERT_FALSE(stmt.GetColumnInfo( 7).IsSystemProperty());    // D.SourceECClassId    a4      WRONG
        ASSERT_TRUE (stmt.GetColumnInfo( 8).IsSystemProperty());    // D.TargetECInstanceId         CORRECT
        ASSERT_FALSE(stmt.GetColumnInfo( 9).IsSystemProperty());    // D.TargetECInstanceId a5      WRONG
        ASSERT_TRUE (stmt.GetColumnInfo(10).IsSystemProperty());    // D.TargetECClassId            CORRECT
        ASSERT_FALSE(stmt.GetColumnInfo(11).IsSystemProperty());    // D.TargetECClassId    a6      WRONG

        EXPECT_STREQ(stmt.GetColumnInfo( 0).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "Id"        );// D.ECInstanceId               CORRECT
        EXPECT_STREQ(stmt.GetColumnInfo( 1).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "Id"        );// D.ECInstanceId       a1      CORRECT
        EXPECT_STREQ(stmt.GetColumnInfo( 2).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "ClassId"   );// D.ECClassId                  CORRECT
        EXPECT_STREQ(stmt.GetColumnInfo( 3).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "ClassId"   );// D.ECClassId          a2      CORRECT
        EXPECT_STREQ(stmt.GetColumnInfo( 4).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "SourceId"        );// D.SourceECInstanceId         CORRECT
        EXPECT_STREQ(stmt.GetColumnInfo( 5).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "SourceId"        );// D.SourceECInstanceId a3      CORRECT
        EXPECT_STREQ(stmt.GetColumnInfo( 6).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "SourceClassId"   );// D.SourceECClassId            CORRECT
        EXPECT_STREQ(stmt.GetColumnInfo( 7).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "SourceClassId"   );// D.SourceECClassId    a4      CORRECT
        EXPECT_STREQ(stmt.GetColumnInfo( 8).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "TargetId"        );// D.TargetECInstanceId         CORRECT
        EXPECT_STREQ(stmt.GetColumnInfo( 9).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "TargetId"        );// D.TargetECInstanceId a5      CORRECT
        EXPECT_STREQ(stmt.GetColumnInfo(10).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "TargetClassId"   );// D.TargetECClassId            CORRECT
        EXPECT_STREQ(stmt.GetColumnInfo(11).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "TargetClassId"   );// D.TargetECClassId    a6      CORRECT
    }

    if ("top level eclass") {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success,
        stmt.Prepare(m_ecdb, R"(
            SELECT
                D.ECInstanceId,
                D.ECInstanceId Goo,
                D.ECClassId,
                D.ECClassId Boo,
                D.Schema,
                D.Schema.Id,
                D.Schema.RelECClassId,
                D.Schema S,
                D.Schema.Id SID,
                D.Schema.RelECClassId SRID
            FROM meta.ECClassDef D)"));

        ASSERT_FALSE(stmt.GetColumnInfo(0).IsGeneratedProperty());// D.ECInstanceId,
        ASSERT_TRUE (stmt.GetColumnInfo(1).IsGeneratedProperty());// D.ECInstanceId Goo,
        ASSERT_FALSE(stmt.GetColumnInfo(2).IsGeneratedProperty());// D.ECClassId,
        ASSERT_TRUE (stmt.GetColumnInfo(3).IsGeneratedProperty());// D.ECClassId Boo,
        ASSERT_FALSE(stmt.GetColumnInfo(4).IsGeneratedProperty());// D.Schema,
        ASSERT_FALSE(stmt.GetColumnInfo(5).IsGeneratedProperty());// D.Schema.Id,
        ASSERT_FALSE(stmt.GetColumnInfo(6).IsGeneratedProperty());// D.Schema.RelECClassId,
        ASSERT_TRUE (stmt.GetColumnInfo(7).IsGeneratedProperty());// D.Schema S,
        ASSERT_TRUE (stmt.GetColumnInfo(8).IsGeneratedProperty());// D.Schema.Id SID,
        ASSERT_TRUE (stmt.GetColumnInfo(9).IsGeneratedProperty());// D.Schema.RelECClassId SRID

        ASSERT_TRUE (stmt.GetColumnInfo(0).IsSystemProperty());// D.ECInstanceId,
        ASSERT_FALSE(stmt.GetColumnInfo(1).IsSystemProperty());// D.ECInstanceId Goo,
        ASSERT_TRUE (stmt.GetColumnInfo(2).IsSystemProperty());// D.ECClassId,
        ASSERT_FALSE(stmt.GetColumnInfo(3).IsSystemProperty());// D.ECClassId Boo,
        ASSERT_FALSE(stmt.GetColumnInfo(4).IsSystemProperty());// D.Schema,
        ASSERT_TRUE (stmt.GetColumnInfo(5).IsSystemProperty());// D.Schema.Id,
        ASSERT_TRUE (stmt.GetColumnInfo(6).IsSystemProperty());// D.Schema.RelECClassId,
        ASSERT_FALSE(stmt.GetColumnInfo(7).IsSystemProperty());// D.Schema S,
        ASSERT_FALSE(stmt.GetColumnInfo(8).IsSystemProperty());// D.Schema.Id SID,
        ASSERT_FALSE(stmt.GetColumnInfo(9).IsSystemProperty());// D.Schema.RelECClassId SRID

        EXPECT_STREQ(stmt.GetColumnInfo(0).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "Id");           // D.ECInstanceId,
        EXPECT_STREQ(stmt.GetColumnInfo(1).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "Id");           // D.ECInstanceId Goo,
        EXPECT_STREQ(stmt.GetColumnInfo(2).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "ClassId");      // D.ECClassId,
        EXPECT_STREQ(stmt.GetColumnInfo(3).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "ClassId");      // D.ECClassId Boo,
        ASSERT_TRUE (stmt.GetColumnInfo(4).GetProperty()->GetIsNavigation());                                                       // D.Schema,
        EXPECT_STREQ(stmt.GetColumnInfo(5).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "NavId");        // D.Schema.Id,
        EXPECT_STREQ(stmt.GetColumnInfo(6).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "NavRelClassId");// D.Schema.RelECClassId,
        ASSERT_TRUE (stmt.GetColumnInfo(7).GetProperty()->GetIsNavigation());                                                       // D.Schema S,
        EXPECT_STREQ(stmt.GetColumnInfo(8).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "NavId");        // D.Schema.Id SID,
        EXPECT_STREQ(stmt.GetColumnInfo(9).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "NavRelClassId");// D.Schema.RelECClassId SRID

    }

    if ("sub query class") {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success,
        stmt.Prepare(m_ecdb, R"(
            SELECT * FROM (SELECT
                D.ECInstanceId,
                D.ECInstanceId Goo,
                D.ECClassId,
                D.ECClassId Boo,
                D.Schema,
                D.Schema.Id,
                D.Schema.RelECClassId,
                D.Schema S,
                D.Schema.Id SID,
                D.Schema.RelECClassId SRID
            FROM meta.ECClassDef D))"));

        ASSERT_FALSE(stmt.GetColumnInfo(0).IsGeneratedProperty());// D.ECInstanceId,
        ASSERT_TRUE (stmt.GetColumnInfo(1).IsGeneratedProperty());// D.ECInstanceId Goo,
        ASSERT_FALSE(stmt.GetColumnInfo(2).IsGeneratedProperty());// D.ECClassId,
        ASSERT_TRUE (stmt.GetColumnInfo(3).IsGeneratedProperty());// D.ECClassId Boo,
        ASSERT_FALSE(stmt.GetColumnInfo(4).IsGeneratedProperty());// D.Schema,
        ASSERT_FALSE(stmt.GetColumnInfo(5).IsGeneratedProperty());// D.Schema.Id,
        ASSERT_FALSE(stmt.GetColumnInfo(6).IsGeneratedProperty());// D.Schema.RelECClassId,
        ASSERT_TRUE (stmt.GetColumnInfo(7).IsGeneratedProperty());// D.Schema S,
        ASSERT_TRUE (stmt.GetColumnInfo(8).IsGeneratedProperty());// D.Schema.Id SID,
        ASSERT_TRUE (stmt.GetColumnInfo(9).IsGeneratedProperty());// D.Schema.RelECClassId SRID

        ASSERT_TRUE (stmt.GetColumnInfo (0).IsSystemProperty());// D.ECInstanceId,
        ASSERT_TRUE (stmt.GetColumnInfo (1).IsSystemProperty());// D.ECInstanceId Goo,
        ASSERT_TRUE (stmt.GetColumnInfo (2).IsSystemProperty());// D.ECClassId,
        ASSERT_TRUE (stmt.GetColumnInfo (3).IsSystemProperty());// D.ECClassId Boo,
        ASSERT_FALSE(stmt.GetColumnInfo (4).IsSystemProperty());// D.Schema,
        ASSERT_TRUE (stmt.GetColumnInfo (5).IsSystemProperty());// D.Schema.Id,
        ASSERT_TRUE (stmt.GetColumnInfo (6).IsSystemProperty());// D.Schema.RelECClassId,
        ASSERT_FALSE(stmt.GetColumnInfo (7).IsSystemProperty());// D.Schema S,
        ASSERT_TRUE (stmt.GetColumnInfo (8).IsSystemProperty());// D.Schema.Id SID,
        ASSERT_TRUE (stmt.GetColumnInfo (9).IsSystemProperty());// D.Schema.RelECClassId SRID

        EXPECT_STREQ(stmt.GetColumnInfo(0).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "Id");           // D.ECInstanceId,
        EXPECT_STREQ(stmt.GetColumnInfo(1).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "Id");           // D.ECInstanceId Goo,
        EXPECT_STREQ(stmt.GetColumnInfo(2).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "ClassId");      // D.ECClassId,
        EXPECT_STREQ(stmt.GetColumnInfo(3).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "Id");           // D.ECClassId Boo,             WRONG?
        ASSERT_TRUE (stmt.GetColumnInfo(4).GetProperty()->GetIsNavigation());                                                       // D.Schema,
        EXPECT_STREQ(stmt.GetColumnInfo(5).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "NavId");        // D.Schema.Id,
        EXPECT_STREQ(stmt.GetColumnInfo(6).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "NavRelClassId");// D.Schema.RelECClassId,
        ASSERT_TRUE (stmt.GetColumnInfo(7).GetProperty()->GetIsNavigation());                                                       // D.Schema S,
        EXPECT_STREQ(stmt.GetColumnInfo(8).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "Id");           // D.Schema.Id SID,             WRONG?
        EXPECT_STREQ(stmt.GetColumnInfo(9).GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str(), "Id");           // D.Schema.RelECClassId SRID   WRONG?
    }
}
//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, InsertWithMixParametersIntAndInt)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    auto ecsql = "INSERT INTO ecsql.Sub1 (I,Sub1I) VALUES(123, ?)";
    ECSqlStatement statement;
    auto stat = statement.Prepare(m_ecdb, ecsql);
    ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation of '" << ecsql << "' failed";

    statement.BindInt(1, 333);

    auto stepStatus = statement.Step();
    ASSERT_EQ((int) BE_SQLITE_DONE, (int) stepStatus) << "Step for '" << ecsql << "' failed";
    statement.Finalize();
    ECSqlStatement stmt;
    auto prepareStatus = stmt.Prepare(m_ecdb, "SELECT * FROM ecsql.PSA");
    ASSERT_TRUE(prepareStatus == ECSqlStatus::Success);
    ECInstanceECSqlSelectAdapter classPReader(stmt);
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        auto inst = classPReader.GetInstance();
        ECValue v;
        inst->GetValue(v, "I");
        ASSERT_EQ(123, v.GetInteger());
        inst->GetValue(v, "Sub1I");
        ASSERT_EQ(123, v.GetInteger());
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, InsertWithMixParameters)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    auto ecsql = "INSERT INTO ecsql.P (B,D,I,L,S) VALUES(1, ?,?,123,?)";
    ECSqlStatement statement;
    auto stat = statement.Prepare(m_ecdb, ecsql);
    ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation of '" << ecsql << "' failed";

    statement.BindDouble(1, 2.22);
    statement.BindInt(2, 123);
    statement.BindText(3, "Test Test", IECSqlBinder::MakeCopy::Yes);
    auto stepStatus = statement.Step();
    ASSERT_EQ((int) BE_SQLITE_DONE, (int) stepStatus) << "Step for '" << ecsql << "' failed";
    statement.Finalize();
    ECSqlStatement stmt;
    auto prepareStatus = stmt.Prepare(m_ecdb, "SELECT * FROM ecsql.PSA");
    ASSERT_TRUE(prepareStatus == ECSqlStatus::Success);
    ECInstanceECSqlSelectAdapter classPReader(stmt);
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        auto inst = classPReader.GetInstance();
        ECValue v;
        inst->GetValue(v, "B");
        ASSERT_EQ(true, v.GetBoolean());
        inst->GetValue(v, "D");
        ASSERT_EQ(2.22, v.GetDouble());
        inst->GetValue(v, "I");
        ASSERT_EQ(123, v.GetInteger());
        inst->GetValue(v, "L");
        ASSERT_EQ(123, v.GetLong());
        inst->GetValue(v, "S");
        ASSERT_STREQ("Test Test", v.GetUtf8CP());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, ClassWithStructHavingStructArrayInsertWithDotOperator)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    auto ecsql = "INSERT INTO ecsql.SA (SAStructProp.PStruct_Array) VALUES(?)";
    ECSqlStatement statement;
    ECSqlStatus stat = statement.Prepare(m_ecdb, ecsql);
    ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation of '" << ecsql << "' failed";

    //add three array elements
    const int count = 3;
    auto& arrayBinder = statement.GetBinder(1);
    for (int i = 0; i < count; i++)
        {
        IECSqlBinder& arrayElementBinder = arrayBinder.AddArrayElement();
        stat = arrayElementBinder["d"].BindDouble(i * PI);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["i"].BindInt(i * 2);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["l"].BindInt64(i * 3);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["p2d"].BindPoint2d(DPoint2d::From(i, i + 1));
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["p3d"].BindPoint3d(DPoint3d::From(i, i + 1, i + 2));
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        }

    auto stepStatus = statement.Step();
    ASSERT_EQ((int) BE_SQLITE_DONE, (int) stepStatus) << "Step for '" << ecsql << "' failed";
    statement.Finalize();
    ECSqlStatement stmt;
    auto prepareStatus = stmt.Prepare(m_ecdb, "SELECT SAStructProp.PStruct_Array FROM ecsql.SA");
    ASSERT_TRUE(prepareStatus == ECSqlStatus::Success);
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        auto &pStructArray = stmt.GetValue(0);
        ASSERT_EQ(3, pStructArray.GetArrayLength());
        //need to Verify all values
        /*  for (auto const & arrayItem : pStructArray)
        {
        IECSqlValue const & value= arrayItem->operator[](0);
        value.GetDouble();
        }

        // ASSERT_TRUE(ECObjectsStatus::Success == inst->GetValue(v, L"SAStructProp.PStruct_Array",0));
        // IECInstancePtr structInstance = v.GetStruct();
        // structInstance->GetValue(v, L"PStruct_Array");
        //ASSERT_TRUE(v.IsArray());
        ASSERT_TRUE(pStructArray == pStructArray);*/
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, StructUpdateWithDotOperator)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    auto ecsql = "INSERT INTO ecsql.SA (SAStructProp.PStructProp.i) VALUES(2)";

    ECSqlStatement statement;
    auto stat = statement.Prepare(m_ecdb, ecsql);
    ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation of '" << ecsql << "' failed";
    auto stepStatus = statement.Step();
    ASSERT_EQ((int) BE_SQLITE_DONE, (int) stepStatus) << "Step for '" << ecsql << "' failed";
    statement.Finalize();
    {
    auto prepareStatus = statement.Prepare(m_ecdb, "SELECT * FROM ecsql.SA");
    ASSERT_TRUE(prepareStatus == ECSqlStatus::Success);
    ECInstanceECSqlSelectAdapter classPReader(statement);
    while (statement.Step() == BE_SQLITE_ROW)
        {
        auto inst = classPReader.GetInstance();
        ECValue v;
        inst->GetValue(v, "SAStructProp.PStructProp.i");
        ASSERT_EQ(2, v.GetInteger());
        }
    }
    statement.Finalize();
    ecsql = "UPDATE ONLY ecsql.SA SET SAStructProp.PStructProp.i = 3 ";
    stat = statement.Prepare(m_ecdb, ecsql);
    ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation of '" << ecsql << "' failed";
    stepStatus = statement.Step();
    ASSERT_EQ((int) BE_SQLITE_DONE, (int) stepStatus) << "Step for '" << ecsql << "' failed";
    statement.Finalize();

    auto prepareStatus = statement.Prepare(m_ecdb, "SELECT * FROM ecsql.SA");
    ASSERT_TRUE(prepareStatus == ECSqlStatus::Success);
    ECInstanceECSqlSelectAdapter classPReader(statement);
    while (statement.Step() == BE_SQLITE_ROW)
        {
        auto inst = classPReader.GetInstance();
        ECValue v;
        inst->GetValue(v, "SAStructProp.PStructProp.i");
        ASSERT_EQ(3, v.GetInteger());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, ClassWithStructHavingStructArrayUpdateWithDotOperator)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    auto ecsql = "INSERT INTO ecsql.SA (SAStructProp.PStruct_Array) VALUES(?)";
    ECSqlStatement insertStatement;
    auto stat = insertStatement.Prepare(m_ecdb, ecsql);
    ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation of '" << ecsql << "' failed";

    //add three array elements
    int count = 3;
    IECSqlBinder& arrayBinder = insertStatement.GetBinder(1);
    for (int i = 0; i < count; i++)
        {
        IECSqlBinder& arrayElementBinder = arrayBinder.AddArrayElement();
        stat = arrayElementBinder["d"].BindDouble(i * PI);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["i"].BindInt(i * 2);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["l"].BindInt64(i * 3);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["p2d"].BindPoint2d(DPoint2d::From(i, i + 1));
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["p3d"].BindPoint3d(DPoint3d::From(i, i + 1, i + 2));
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        }

    auto stepStatus = insertStatement.Step();
    ASSERT_EQ(BE_SQLITE_DONE, stepStatus) << "Step for '" << ecsql << "' failed";
    ECSqlStatement selectStatement;
    auto prepareStatus = selectStatement.Prepare(m_ecdb, "SELECT SAStructProp.PStruct_Array FROM ecsql.SA");
    ASSERT_TRUE(prepareStatus == ECSqlStatus::Success);
    while (selectStatement.Step() == BE_SQLITE_ROW)
        {
        auto const& pStructArray = selectStatement.GetValue(0);
        ASSERT_EQ(count, pStructArray.GetArrayLength());
        }

    ECSqlStatement updateStatement;
    ecsql = "UPDATE ONLY ecsql.SA SET SAStructProp.PStruct_Array=?";
    ASSERT_EQ(ECSqlStatus::Success, updateStatement.Prepare(m_ecdb, ecsql)) << ecsql;
    count = 3;
    IECSqlBinder& updateArrayBinder = updateStatement.GetBinder(1);
    for (int i = 0; i < count; i++)
        {
        IECSqlBinder& arrayElementBinder = updateArrayBinder.AddArrayElement();
        stat = arrayElementBinder["d"].BindDouble(-count);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["i"].BindInt(-count);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["l"].BindInt64(-count);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["p2d"].BindPoint2d(DPoint2d::From(i, i + 1));
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        stat = arrayElementBinder["p3d"].BindPoint3d(DPoint3d::From(i, i + 1, i + 2));
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Bind to struct member failed";
        }

    stepStatus = updateStatement.Step();
    ASSERT_EQ(BE_SQLITE_DONE, stepStatus) << "Step for '" << ecsql << "' failed";

    ECSqlStatement statement;
    prepareStatus = statement.Prepare(m_ecdb, "SELECT SAStructProp.PStruct_Array FROM ecsql.SA");
    ASSERT_TRUE(prepareStatus == ECSqlStatus::Success);
    while (statement.Step() == BE_SQLITE_ROW)
        {
        auto const& pStructArray = statement.GetValue(0);
        ASSERT_EQ(count, pStructArray.GetArrayLength());
        }
    statement.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, AmbiguousQuery)
    {
    SchemaItem schemaXml("<?xml version='1.0' encoding='utf-8'?>"
                         "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                         "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                         "    <ECStructClass typeName='Struct' >"
                         "        <ECProperty propertyName='P1' typeName='string' />"
                         "        <ECProperty propertyName='P2' typeName='int' />"
                         "    </ECStructClass>"
                         "    <ECEntityClass typeName='TestClass' >"
                         "        <ECProperty propertyName='P1' typeName='string'/>"
                         "         <ECStructProperty propertyName = 'TestClass' typeName = 'Struct'/>"
                         "    </ECEntityClass>"
                         "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("AmbiguousQuery.ecdb", schemaXml));

    ECN::ECSchemaCP schema = m_ecdb.Schemas().GetSchema("TestSchema");

    ECClassCP TestClass = schema->GetClassCP("TestClass");
    ASSERT_TRUE(TestClass != nullptr);

    ECN::StandaloneECInstancePtr Instance1 = TestClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ECN::StandaloneECInstancePtr Instance2 = TestClass->GetDefaultStandaloneEnabler()->CreateInstance();

    Instance1->SetValue("P1", ECValue("Harvey"));
    Instance1->SetValue("TestClass.P1", ECValue("val1"));
    Instance1->SetValue("TestClass.P2", ECValue(123));

    Instance2->SetValue("P1", ECValue("Mike"));
    Instance2->SetValue("TestClass.P1", ECValue("val2"));
    Instance2->SetValue("TestClass.P2", ECValue(345));

    //Inserting values of TestClass
    ECInstanceInserter inserter(m_ecdb, *TestClass, nullptr);
    ASSERT_TRUE(inserter.IsValid());

    ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(*Instance1, true));
    ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(*Instance2, true));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT P1 FROM ts.TestClass"));
    Utf8String ExpectedValueOfP1 = "Harvey-Mike-";
    Utf8String ActualValueOfP1;

    while (stmt.Step() == BE_SQLITE_ROW)
        {
        ActualValueOfP1.append(stmt.GetValueText(0));
        ActualValueOfP1.append("-");
        }

    ASSERT_EQ(ExpectedValueOfP1, ActualValueOfP1);
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT TestClass.TestClass.P1 FROM ts.TestClass"));
    Utf8String ExpectedValueOfStructP1 = "val1-val2-";
    Utf8String ActualValueOfStructP1;

    while (stmt.Step() == BE_SQLITE_ROW)
        {
        ActualValueOfStructP1.append(stmt.GetValueText(0));
        ActualValueOfStructP1.append("-");
        }

    ASSERT_EQ(ExpectedValueOfStructP1, ActualValueOfStructP1);
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT TestClass.P2 FROM ts.TestClass"));
    int ActualValueOfStructP2 = 468;
    int ExpectedValueOfStructP2 = 0;

    while (stmt.Step() == BE_SQLITE_ROW)
        {
        ExpectedValueOfStructP2 += stmt.GetValueInt(0);
        }

    ASSERT_EQ(ExpectedValueOfStructP2, ActualValueOfStructP2);
    stmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, BindNegECInstanceId)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("BindNegECInstanceId.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    ECSqlStatement stmt;

    //Inserting Values for a negative ECInstanceId.
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.P (ECInstanceId,B,D,S) VALUES(?,?,?,?)"));

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(1, (int64_t) (-1)));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindBoolean(2, true));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(3, 100.54));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(4, "Foo", IECSqlBinder::MakeCopy::No));

    ASSERT_EQ(DbResult::BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    //Retrieving Values.
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "Select B,D,S FROM ecsql.P WHERE ECInstanceId=-1"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(true, stmt.GetValueBoolean(0));
    ASSERT_EQ(100.54, stmt.GetValueDouble(1));
    ASSERT_STREQ("Foo", stmt.GetValueText(2));
    stmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, InstanceInsertionInArray)
    {
    SchemaItem schema("<?xml version='1.0' encoding='utf-8'?>"
                      "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                      "    <ECEntityClass typeName='TestClass' >"
                      "        <ECProperty propertyName='P1' typeName='string'/>"
                      "        <ECArrayProperty propertyName = 'P2_Array' typeName = 'string' minOccurs='1' maxOccurs='2'/>"
                      "        <ECArrayProperty propertyName = 'P3_Array' typeName = 'int' minOccurs='1' maxOccurs='2'/>"
                      "    </ECEntityClass>"
                      "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("InstanceInsertionInArray.ecdb", schema));

    std::vector<Utf8String> expectedStringArray = {"val1", "val2", "val3"};
    std::vector<int> expectedIntArray = {200, 400, 600};

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.TestClass (P1, P2_Array, P3_Array) VALUES(?, ?, ?)"));

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Foo", IECSqlBinder::MakeCopy::No));

    IECSqlBinder& arrayBinderS = stmt.GetBinder(2);
    for (Utf8StringCR arrayElement : expectedStringArray)
        {
        ASSERT_EQ(ECSqlStatus::Success, arrayBinderS.AddArrayElement().BindText(arrayElement.c_str(), IECSqlBinder::MakeCopy::No));
        }

    IECSqlBinder& arrayBinderI = stmt.GetBinder(3);
    for (int arrayElement : expectedIntArray)
        {
        ASSERT_EQ(ECSqlStatus::Success, arrayBinderI.AddArrayElement().BindInt(arrayElement));
        }
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT P1, P2_Array, P3_Array FROM ts.TestClass"));
    while (stmt.Step() == DbResult::BE_SQLITE_ROW)
        {
        ASSERT_STREQ("Foo", stmt.GetValueText(0));

        IECSqlValue const& StringArray = stmt.GetValue(1);
        size_t expectedIndex = 0;

        for (IECSqlValue const& arrayElement : StringArray.GetArrayIterable())
            {
            Utf8CP actualArrayElement = arrayElement.GetText();
            ASSERT_STREQ(expectedStringArray[expectedIndex].c_str(), actualArrayElement);
            expectedIndex++;
            }

        IECSqlValue const& IntArray = stmt.GetValue(2);
        expectedIndex = 0;
        for (IECSqlValue const& arrayElement : IntArray.GetArrayIterable())
            {
            int actualArrayElement = arrayElement.GetInt();
            ASSERT_EQ(expectedIntArray[expectedIndex], actualArrayElement);
            expectedIndex++;
            }
        }
    stmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, SelectAfterImport)
    {
    auto importSchema = [] (ECDbCR db, ECN::ECSchemaCR schemaIn)
        {
        ECN::ECSchemaCP existing = db.Schemas().GetSchema(schemaIn.GetName());
        if (nullptr != existing)
            return;

        ECN::ECSchemaPtr imported = nullptr;
        ASSERT_EQ(ECN::ECObjectsStatus::Success, schemaIn.CopySchema(imported));

        ASSERT_EQ(ECN::ECObjectsStatus::Success, imported->SetName(schemaIn.GetName()));

        ASSERT_EQ(ECN::ECObjectsStatus::Success, imported->SetAlias(schemaIn.GetAlias()));

        ECN::ECSchemaReadContextPtr contextPtr = ECN::ECSchemaReadContext::CreateContext();
        ASSERT_EQ(ECN::ECObjectsStatus::Success, contextPtr->AddSchema(*imported));

        ASSERT_EQ(SUCCESS, db.Schemas().ImportSchemas(contextPtr->GetCache().GetSchemas()));
        };

    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("ImportTwoInARow.ecdb"));

    {
    ECN::ECSchemaPtr schema;
    ASSERT_EQ(ECN::ECObjectsStatus::Success, ECN::ECSchema::CreateSchema(schema, "ImportTwoInARow", "tir", 0, 0, 0));

    ECN::ECEntityClassP ecclass;
    ASSERT_EQ(ECN::ECObjectsStatus::Success, schema->CreateEntityClass(ecclass, "C1"));

    ECN::PrimitiveECPropertyP ecprop;
    ASSERT_EQ(ECN::ECObjectsStatus::Success, ecclass->CreatePrimitiveProperty(ecprop, "X", PRIMITIVETYPE_String));

    ecprop->SetType(ECN::PRIMITIVETYPE_Double);

    importSchema(m_ecdb, *schema);
    }

    EC::ECSqlStatement selectC1;
    selectC1.Prepare(m_ecdb, "SELECT ECInstanceId FROM tir.C1");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ECSqlStatementTestFixture, SelectCountPolymorphic)
    {
    auto getCount = [](ECDbCR db, ECClassCR ecClass, bool isPolymorphic)
        {
        Utf8String ecsql("SELECT count(*) FROM ");

        if (!isPolymorphic)
            ecsql.append("ONLY ");

        ecsql.append(ecClass.GetECSqlName());

        ECSqlStatement statement;
        ECSqlStatus stat = statement.Prepare(db, ecsql.c_str());
        if (stat != ECSqlStatus::Success)
            return -1;

        if (statement.Step() != BE_SQLITE_ROW)
            return -1;

        return statement.GetValueInt(0);
        };

    ASSERT_EQ(SUCCESS, SetupECDb("StartupCompany.ecdb", SchemaItem::CreateForFile("StartupCompany.02.00.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, PopulateECDb( 3));

    /*
    * Test retrieval when parent and children are all in the same table (TablePerHierarchy)
    */
    ECClassCP furniture = m_ecdb.Schemas().GetClass("StartupCompany", "Furniture");
    ASSERT_TRUE(furniture != nullptr);
    int count = getCount(m_ecdb, *furniture, false);
    ASSERT_EQ(3, count);
    count = getCount(m_ecdb, *furniture, true);
    ASSERT_EQ(9, count);

    /*
    * Test retrieval when parent and children are all in different tables (TablePerClass)
    */
    ECClassCP hardware = m_ecdb.Schemas().GetClass("StartupCompany", "Hardware");
    ASSERT_TRUE(hardware != nullptr);
    count = getCount(m_ecdb, *hardware, false);
    ASSERT_EQ(3, count);
    count = getCount(m_ecdb, *hardware, true);
    ASSERT_EQ(9, count);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ECSqlStatementTestFixture, SelectClause)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("StartupCompany.ecdb", SchemaItem::CreateForFile("StartupCompany.02.00.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, PopulateECDb( 3));
    ECClassCP employee = m_ecdb.Schemas().GetClass("StartupCompany", "Employee");
    ASSERT_TRUE(employee != nullptr);

    ECSqlStatement ecStatement;

    Utf8String jobTitle1;
    int managerId1;
    {
    // ECSQL should honor the order of the ecColumns from the select clause
    ASSERT_EQ(ECSqlStatus::Success, ecStatement.Prepare(m_ecdb, "SELECT JobTitle, ManagerID FROM [StartupCompany].[Employee]"));
    ASSERT_EQ(BE_SQLITE_ROW, ecStatement.Step());
    jobTitle1 = ecStatement.GetValueText(0);
    managerId1 = ecStatement.GetValueInt(1);
    EXPECT_TRUE(ecStatement.GetColumnInfo(0).GetProperty()->GetName().Equals("JobTitle"));
    EXPECT_TRUE(ecStatement.GetColumnInfo(1).GetProperty()->GetName().Equals("ManagerID"));
    ecStatement.Finalize();
    }

    {
    ASSERT_EQ(ECSqlStatus::Success, ecStatement.Prepare(m_ecdb, "SELECT JobTitle, ManagerID FROM [StartupCompany].[Employee]"));
    ASSERT_EQ(BE_SQLITE_ROW, ecStatement.Step());
    Utf8String jobTitle2 = ecStatement.GetValueText(0);
    int        managerId2 = ecStatement.GetValueInt(1);
    EXPECT_TRUE(ecStatement.GetColumnInfo(0).GetProperty()->GetName().Equals("JobTitle"));
    EXPECT_TRUE(ecStatement.GetColumnInfo(1).GetProperty()->GetName().Equals("ManagerID"));
    ecStatement.Finalize();

    ASSERT_EQ(managerId1, managerId2);
    ASSERT_TRUE(jobTitle1.Equals(jobTitle2));
    }

    {
    // ECSQL SelectAll (aka '*') should select in same order as ECProperties appear in the ECSchema
    ASSERT_TRUE(ECSqlStatus::Success == ecStatement.Prepare(m_ecdb, "SELECT * FROM [StartupCompany].[Employee]"));
    ASSERT_TRUE(BE_SQLITE_ROW == ecStatement.Step());
    EXPECT_TRUE(ecStatement.GetColumnInfo(0).GetProperty()->GetName().Equals("ECInstanceId"));
    EXPECT_TRUE(ecStatement.GetColumnInfo(1).GetProperty()->GetName().Equals("ECClassId"));
    EXPECT_TRUE(ecStatement.GetColumnInfo(2).GetProperty()->GetName().Equals("EmployeeID"));
    EXPECT_TRUE(ecStatement.GetColumnInfo(3).GetProperty()->GetName().Equals("FirstName"));
    EXPECT_TRUE(ecStatement.GetColumnInfo(4).GetProperty()->GetName().Equals("JobTitle"));
    EXPECT_TRUE(ecStatement.GetColumnInfo(5).GetProperty()->GetName().Equals("LastName"));
    EXPECT_TRUE(ecStatement.GetColumnInfo(6).GetProperty()->GetName().Equals("ManagerID"));
    EXPECT_TRUE(ecStatement.GetColumnInfo(7).GetProperty()->GetName().Equals("Room"));
    EXPECT_TRUE(ecStatement.GetColumnInfo(8).GetProperty()->GetName().Equals("SSN"));
    EXPECT_TRUE(ecStatement.GetColumnInfo(9).GetProperty()->GetName().Equals("Project"));
    EXPECT_TRUE(ecStatement.GetColumnInfo(10).GetProperty()->GetName().Equals("WorkPhone"));
    EXPECT_TRUE(ecStatement.GetColumnInfo(11).GetProperty()->GetName().Equals("MobilePhone"));
    EXPECT_TRUE(ecStatement.GetColumnInfo(12).GetProperty()->GetName().Equals("FullName"));
    EXPECT_TRUE(ecStatement.GetColumnInfo(13).GetProperty()->GetName().Equals("Certifications"));
    EXPECT_TRUE(ecStatement.GetColumnInfo(14).GetProperty()->GetName().Equals("Location"));
    EXPECT_TRUE(ecStatement.GetValue(14)["Coordinate"].GetColumnInfo().GetProperty()->GetName().Equals(/*Location.*/"Coordinate"));
    EXPECT_TRUE(ecStatement.GetValue(14)["Street"].GetColumnInfo().GetProperty()->GetName().Equals(/*Location.*/"Street"));
    EXPECT_TRUE(ecStatement.GetColumnInfo(15).GetProperty()->GetName().Equals("EmployeeType"));
    EXPECT_TRUE(ecStatement.GetColumnInfo(16).GetProperty()->GetName().Equals("Address"));
    EXPECT_TRUE(ecStatement.GetValue(16)["Coordinate"].GetColumnInfo().GetProperty()->GetName().Equals(/*Location.*/"Coordinate"));
    EXPECT_TRUE(ecStatement.GetValue(16)["Street"].GetColumnInfo().GetProperty()->GetName().Equals(/*Location.*/"Street"));
    EXPECT_TRUE(ecStatement.GetColumnInfo(17).GetProperty()->GetName().Equals("EmployeeRecordKey"));
    }
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ECSqlStatementTestFixture, OrderBy)
    {
    // Create StartupCompany
    ASSERT_EQ(SUCCESS, SetupECDb("StartupCompany.ecdb", SchemaItem::CreateForFile("StartupCompany.02.00.00.ecschema.xml")));

    auto insertPerson = [](ECDbCR ecdb, ECClassCR ecClass, Utf8CP firstName, Utf8CP lastName)
        {
        IECInstancePtr ecInstance = ecClass.GetDefaultStandaloneEnabler()->CreateInstance(0);
        ECValue val;
        val.SetUtf8CP(firstName, false);
        if (ECObjectsStatus::Success != ecInstance->SetValue("FirstName", val))
            return BE_SQLITE_ERROR;

        val.Clear();
        val.SetUtf8CP(lastName, false);
        if (ECObjectsStatus::Success != ecInstance->SetValue("LastName", val))
            return BE_SQLITE_ERROR;

        ECInstanceInserter inserter(ecdb, ecClass, nullptr);
        if (!inserter.IsValid())
            return BE_SQLITE_ERROR;

        return inserter.Insert(*ecInstance);
        };

    // Add some employees
    ECClassCP employeeClass = m_ecdb.Schemas().GetClass("StartupCompany", "Employee");
    ASSERT_EQ(BE_SQLITE_OK, insertPerson(m_ecdb, *employeeClass, "Leonardo", "Da Vinci"));
    ASSERT_EQ(BE_SQLITE_OK, insertPerson(m_ecdb, *employeeClass, "Galileo", "Galilei"));
    ASSERT_EQ(BE_SQLITE_OK, insertPerson(m_ecdb, *employeeClass, "Nikola", "Tesla"));
    ASSERT_EQ(BE_SQLITE_OK, insertPerson(m_ecdb, *employeeClass, "Niels", "Bohr"));
    ASSERT_EQ(BE_SQLITE_OK, insertPerson(m_ecdb, *employeeClass, "Albert", "Einstein"));
    ASSERT_EQ(BE_SQLITE_OK, insertPerson(m_ecdb, *employeeClass, "Albert", "Einstein"));
    ASSERT_EQ(BE_SQLITE_OK, insertPerson(m_ecdb, *employeeClass, "Srinivasa", "Ramanujan"));
    m_ecdb.SaveChanges();

    // Retrieve them in alphabetical order
    Utf8String ecsql("SELECT FirstName,LastName FROM ");
    ecsql.append(employeeClass->GetECSqlName()).append(" ORDER BY LastName, FirstName");
    ECSqlStatement statement;
    ECSqlStatus stat = statement.Prepare(m_ecdb, ecsql.c_str());
    ASSERT_EQ(ECSqlStatus::Success, stat);

    // Just log for a manual check
    Utf8CP firstName, lastName;
    while (statement.Step() == BE_SQLITE_ROW)
        {
        firstName = statement.GetValueText(0);
        lastName = statement.GetValueText(1);
        LOG.debugv("%s, %s", lastName, firstName);
        }

    // Validate the first few entries
    statement.Reset();
    DbResult stepStat = statement.Step();
    ASSERT_EQ(BE_SQLITE_ROW, stepStat);
    lastName = statement.GetValueText(1);
    ASSERT_EQ(0, strcmp(lastName, "Bohr"));

    stepStat = statement.Step();
    ASSERT_EQ(BE_SQLITE_ROW, stepStat);
    lastName = statement.GetValueText(1);
    ASSERT_EQ(0, strcmp(lastName, "Da Vinci"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ECSqlStatementTestFixture, LimitOffset)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("StartupCompany.ecdb", SchemaItem::CreateForFile("StartupCompany.02.00.00.ecschema.xml")));

    // Populate 100 instances
    ECClassCP ecClass = m_ecdb.Schemas().GetClass("StartupCompany", "ClassWithPrimitiveProperties");
    IECInstancePtr ecInstance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance(0);
    ECInstanceInserter inserter(m_ecdb, *ecClass, nullptr);
    ASSERT_TRUE(inserter.IsValid());
    for (int ii = 0; ii < 100; ii++)
        {
        ECValue ecValue(ii);
        ASSERT_EQ(ECObjectsStatus::Success, ecInstance->SetValue("intProp", ecValue));
        ECInstanceKey instanceKey;
        ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(instanceKey, *ecInstance));
        }
    m_ecdb.SaveChanges();

    // Setup query for a page of instances
    Utf8String ecsql("SELECT intProp FROM ");
    ecsql.append(ecClass->GetECSqlName()).append(" LIMIT :pageSize OFFSET :pageSize * (:pageNumber - 1)");
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str();
    int pageSizeIndex = statement.GetParameterIndex("pageSize");
    ASSERT_TRUE(pageSizeIndex >= 0);
    int pageNumberIndex = statement.GetParameterIndex("pageNumber");
    ASSERT_TRUE(pageNumberIndex >= 0);

    // Start getting the 5th page, where each page is 10 instances
    int pageSize = 10;
    int pageNumber = 5;
    statement.BindInt(pageSizeIndex, pageSize);
    statement.BindInt(pageNumberIndex, pageNumber);

    // Verify the first result
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    int actualValue = statement.GetValueInt(0);
    int expectedValue = pageSize * (pageNumber - 1);
    ASSERT_EQ(actualValue, expectedValue);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, RowConstructor)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("emptydb.ecdb"));

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "VALUES(1) UNION VALUES(1)"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(1, stmt.GetValueInt(0));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "VALUES(0xfffff)"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        EXPECT_STREQ("__x0030__xfffff", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        EXPECT_STREQ("0xfffff", stmt.GetColumnInfo(0).GetProperty()->GetDisplayLabel().c_str());
        EXPECT_EQ(PRIMITIVETYPE_Long, stmt.GetColumnInfo(0).GetProperty()->GetAsPrimitiveProperty()->GetType());
        EXPECT_EQ(0xfffff, stmt.GetValueInt(0));
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "VALUES(-1222)"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        EXPECT_STREQ("__x002D__1222", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        EXPECT_STREQ("-1222", stmt.GetColumnInfo(0).GetProperty()->GetDisplayLabel().c_str());
        EXPECT_EQ(PRIMITIVETYPE_Double, stmt.GetColumnInfo(0).GetProperty()->GetAsPrimitiveProperty()->GetType());
        EXPECT_EQ(-1222, stmt.GetValueInt(0));
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "VALUES(12334234234)"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_STREQ("__x0031__2334234234", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        ASSERT_STREQ("12334234234", stmt.GetColumnInfo(0).GetProperty()->GetDisplayLabel().c_str());
        ASSERT_EQ(PRIMITIVETYPE_Long, stmt.GetColumnInfo(0).GetProperty()->GetAsPrimitiveProperty()->GetType());
        ASSERT_EQ(12334234234, stmt.GetValueInt64(0));
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "VALUES(-12334234234)"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        EXPECT_STREQ("__x002D__12334234234", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        EXPECT_STREQ("-12334234234", stmt.GetColumnInfo(0).GetProperty()->GetDisplayLabel().c_str());
        EXPECT_EQ(PRIMITIVETYPE_Double, stmt.GetColumnInfo(0).GetProperty()->GetAsPrimitiveProperty()->GetType());
        EXPECT_EQ(-12334234234, stmt.GetValueInt64(0));
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "VALUES('Sample Test')"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        EXPECT_STREQ("__x0027__Sample__x0020__Test__x0027__", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        EXPECT_STREQ("'Sample Test'", stmt.GetColumnInfo(0).GetProperty()->GetDisplayLabel().c_str());
        EXPECT_EQ(PRIMITIVETYPE_String, stmt.GetColumnInfo(0).GetProperty()->GetAsPrimitiveProperty()->GetType());
        EXPECT_STREQ("Sample Test", stmt.GetValueText(0));
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "VALUES(true)"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        EXPECT_STREQ("TRUE", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        EXPECT_STREQ("TRUE", stmt.GetColumnInfo(0).GetProperty()->GetDisplayLabel().c_str());
        EXPECT_EQ(PRIMITIVETYPE_Boolean, stmt.GetColumnInfo(0).GetProperty()->GetAsPrimitiveProperty()->GetType());
        EXPECT_EQ(true, stmt.GetValueBoolean(0));
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "VALUES(False)"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        EXPECT_STREQ("FALSE", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        EXPECT_STREQ("FALSE", stmt.GetColumnInfo(0).GetProperty()->GetDisplayLabel().c_str());
        EXPECT_EQ(PRIMITIVETYPE_Boolean, stmt.GetColumnInfo(0).GetProperty()->GetAsPrimitiveProperty()->GetType());
        EXPECT_EQ(false, stmt.GetValueBoolean(0));
        }


        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "VALUES(TIMESTAMP '2013-02-09T12:00:00')"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        EXPECT_STREQ("TIMESTAMP__x0020____x0027__2013__x002D__02__x002D__09T12__x003A__00__x003A__00__x0027__", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        EXPECT_STREQ("TIMESTAMP '2013-02-09T12:00:00'", stmt.GetColumnInfo(0).GetProperty()->GetDisplayLabel().c_str());
        EXPECT_EQ(PRIMITIVETYPE_DateTime, stmt.GetColumnInfo(0).GetProperty()->GetAsPrimitiveProperty()->GetType());
        EXPECT_EQ(DateTime(DateTime::Kind::Unspecified, 2013, 2, 9, 12, 0), stmt.GetValueDateTime(0));
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "VALUES(2333.1212)"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        EXPECT_STREQ("__x0032__333__x002E__1212", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        EXPECT_STREQ("2333.1212", stmt.GetColumnInfo(0).GetProperty()->GetDisplayLabel().c_str());
        EXPECT_EQ(PRIMITIVETYPE_Double, stmt.GetColumnInfo(0).GetProperty()->GetAsPrimitiveProperty()->GetType());
        EXPECT_EQ(2333.1212, stmt.GetValueDouble(0));
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "VALUES(-2333.1212)"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        EXPECT_STREQ("__x002D__2333__x002E__1212", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        EXPECT_STREQ("-2333.1212", stmt.GetColumnInfo(0).GetProperty()->GetDisplayLabel().c_str());
        EXPECT_EQ(PRIMITIVETYPE_Double, stmt.GetColumnInfo(0).GetProperty()->GetAsPrimitiveProperty()->GetType());
        EXPECT_EQ(-2333.1212, stmt.GetValueDouble(0));
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "VALUES(0xfffff, -1222, 12334234234, -12334234234, 'Sample Test', true , false, TIMESTAMP '2013-02-09T12:00:00', 2333.1212,-2333.1212)"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        EXPECT_EQ(10, stmt.GetColumnCount());
        EXPECT_EQ(0xfffff, stmt.GetValueInt(0));
        EXPECT_EQ(-1222, stmt.GetValueInt(1));
        EXPECT_EQ(12334234234, stmt.GetValueInt64(2));
        EXPECT_EQ(-12334234234, stmt.GetValueInt64(3));
        EXPECT_STREQ("Sample Test", stmt.GetValueText(4));
        EXPECT_EQ(true, stmt.GetValueBoolean(5));
        EXPECT_EQ(false, stmt.GetValueBoolean(6));
        EXPECT_EQ(DateTime(DateTime::Kind::Unspecified, 2013, 2, 9, 12, 0), stmt.GetValueDateTime(7));
        EXPECT_EQ(2333.1212, stmt.GetValueDouble(8));
        EXPECT_EQ(-2333.1212, stmt.GetValueDouble(9));
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "VALUES(1) UNION VALUES(1)"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(1, stmt.GetValueInt(0));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "VALUES(1) UNION ALL VALUES(1)"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(1, stmt.GetValueInt(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(1, stmt.GetValueInt(0));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "VALUES(1 + 1, 11.2 + 1.8, 'A' || 'B', TIMESTAMP '2013-02-09T12:00:00',true) UNION VALUES(2, 13.0, 'AB', TIMESTAMP '2013-02-09T12:00:00',true)"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(2, stmt.GetValueInt(0));
        ASSERT_EQ(13.0, stmt.GetValueDouble(1));
        ASSERT_STREQ("AB", stmt.GetValueText(2));
        ASSERT_EQ(DateTime(DateTime::Kind::Unspecified, 2013, 2, 9, 12, 0), stmt.GetValueDateTime(3));
        ASSERT_EQ(true, stmt.GetValueBoolean(4));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "VALUES(1 + 1, 11.2 + 1.8, 'A' || 'B', TIMESTAMP '2013-02-09T12:00:00',true) UNION ALL VALUES(2, 13.0, 'AB', TIMESTAMP '2013-02-09T12:00:00',true)"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(2, stmt.GetValueInt(0));
        ASSERT_EQ(13.0, stmt.GetValueDouble(1));
        ASSERT_STREQ("AB", stmt.GetValueText(2));
        ASSERT_EQ(DateTime(DateTime::Kind::Unspecified, 2013, 2, 9, 12, 0), stmt.GetValueDateTime(3));
        ASSERT_EQ(true, stmt.GetValueBoolean(4));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(2, stmt.GetValueInt(0));
        ASSERT_EQ(13.0, stmt.GetValueDouble(1));
        ASSERT_STREQ("AB", stmt.GetValueText(2));
        ASSERT_EQ(DateTime(DateTime::Kind::Unspecified, 2013, 2, 9, 12, 0), stmt.GetValueDateTime(3));
        ASSERT_EQ(true, stmt.GetValueBoolean(4));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "VALUES(?+10000)"));
        stmt.BindInt(1, 10000);
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(20000, stmt.GetValueInt(0));
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "VALUES(?) UNION VALUES(?)"));
        stmt.BindInt(1, 10000);
        stmt.BindInt(2, 10000);
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(10000, stmt.GetValueInt(0));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "VALUES(?) UNION ALL VALUES(?)"));
        stmt.BindInt(1, 10000);
        stmt.BindInt(2, 10000);
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(10000, stmt.GetValueInt(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(10000, stmt.GetValueInt(0));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "VALUES(?) UNION VALUES(?)"));
        stmt.BindInt(1, 10000);
        stmt.BindInt(2, 20000);
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(10000, stmt.GetValueInt(0));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(20000, stmt.GetValueInt(0));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ECSqlStatementTestFixture, WriteCalculatedECProperty)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("SimpleCompany.ecdb", SchemaItem::CreateForFile("SimpleCompany.01.00.00.ecschema.xml")));
    Utf8String instanceXml = "<Manager xmlns = \"SimpleCompany.01.00\" >"
        "<FirstName>StRiNg - 10002</FirstName>"
        "<LastName>StRiNg - 10002</LastName>"
        "<EmployeeId>10002</EmployeeId>"
        "<LatestEducation>"
        "<Degree>StRiNg - 10007</Degree>"
        "<Year>10010</Year>"
        "</LatestEducation>"
        "<EducationHistory>"
        "<EducationStruct>"
        "<Degree>StRiNg - 10008</Degree>"
        "<GPA>10004.5</GPA>"
        "</EducationStruct>"
        "<EducationStruct>"
        "<Degree>StRiNg - 10009</Degree>"
        "<Year>10011</Year>"
        "<GPA>10005</GPA>"
        "</EducationStruct>"
        "<EducationStruct>"
        "<Year>10012</Year>"
        "<GPA>10005.5</GPA>"
        "</EducationStruct>"
        "</EducationHistory>"
        "<FullName>StRiNg - 10002 StRiNg - 10002</FullName>"
        "</Manager>";
    ECSchemaReadContextPtr schemaReadContext = ECSchemaReadContext::CreateContext();
    schemaReadContext->AddSchemaLocater(m_ecdb.GetSchemaLocater());
    ECSchemaPtr schema;
    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext(*schemaReadContext, *schema, &schema);
    IECInstancePtr instance;
    InstanceReadStatus status = IECInstance::ReadFromXmlString(instance, instanceXml.c_str(), *instanceContext);
    ASSERT_TRUE(InstanceReadStatus::Success == status);

    ECClassCR ecClass = instance->GetClass();
    ECInstanceInserter inserter(m_ecdb, ecClass, nullptr);
    ASSERT_TRUE(inserter.IsValid());
    ECInstanceKey id;
    ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(id, *instance));
    ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(id, *instance));
    ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(id, *instance));
    m_ecdb.SaveChanges();

    SqlPrintfString ecSql("SELECT * FROM ONLY [%s].[%s]", ecClass.GetSchema().GetName().c_str(), ecClass.GetName().c_str());
    ECSqlStatement ecStatement;
    ECSqlStatus prepareStatus = ecStatement.Prepare(m_ecdb, ecSql.GetUtf8CP());
    ASSERT_TRUE(ECSqlStatus::Success == prepareStatus);
    bvector<IECInstancePtr> instances;
    ECInstanceECSqlSelectAdapter adapter(ecStatement);

    while (BE_SQLITE_ROW == ecStatement.Step())
        {
        IECInstancePtr newInstance = adapter.GetInstance();

        BeJsDocument newInstanceJson, instanceJson;
        ASSERT_EQ(SUCCESS, JsonEcInstanceWriter::WriteInstanceToJson(newInstanceJson, *newInstance, nullptr, false));
        ASSERT_EQ(SUCCESS, JsonEcInstanceWriter::WriteInstanceToJson(instanceJson, *instance, nullptr, false));
        ASSERT_EQ(JsonValue(newInstanceJson), JsonValue(instanceJson));
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, PointsMappedToSharedColumns)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("pointsmappedtosharedcolumns.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Base'>"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Prop1' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1'>"
        "        <ECCustomAttributes>"
        "           <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>"
        "           </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='Prop2' typeName='double' />"
        "        <ECProperty propertyName='Center' typeName='point3d' />"
        "    </ECEntityClass>"
        "</ECSchema>")));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Sub1(Prop1,Center) VALUES(1.1,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint3d(1, DPoint3d::From(1.0, 2.0, 3.0)));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    ECClassId sub1ClassId = m_ecdb.Schemas().GetSchema("TestSchema")->GetClassCP("Sub1")->GetId();
    Utf8String expectedNativeSql;
    expectedNativeSql.Sprintf("INSERT INTO [ts_Base] ([Id],[Prop1],ECClassId) VALUES (:_ecdb_ecsqlparam_id_col1,1.1,%s);INSERT INTO [ts_Sub1] ([BaseId],[js2],[js3],[js4],ECClassId) VALUES (:_ecdb_ecsqlparam_id_col1,:_ecdb_ecsqlparam_ix1_col1,:_ecdb_ecsqlparam_ix1_col2,:_ecdb_ecsqlparam_ix1_col3,%s)", sub1ClassId.ToString().c_str(), sub1ClassId.ToString().c_str());
    ASSERT_STREQ(expectedNativeSql.c_str(), stmt.GetNativeSql());

    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT Center.X, Center.Y, Center.Z FROM ts.Sub1 LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(1.0, stmt.GetValueDouble(0));
    ASSERT_EQ(2.0, stmt.GetValueDouble(1));
    ASSERT_EQ(3.0, stmt.GetValueDouble(2));

    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT count(*) FROM ts.Sub1 WHERE Center.X > 0 AND Center.Y > 0 AND Center.Z > 0"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(1, stmt.GetValueInt(0));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, BindZeroBlob)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("bindzeroblob.ecdb", SchemaItem(R"xml(
                            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                             <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                              <ECEntityClass typeName="Element">
                                 <ECCustomAttributes>
                                    <ClassMap xmlns='ECDbMap.02.00'>
                                       <MapStrategy>TablePerHierarchy</MapStrategy>
                                    </ClassMap>
                                    <ShareColumns xmlns='ECDbMap.02.00'>
                                        <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>
                                    </ShareColumns>
                                 </ECCustomAttributes>
                                <ECProperty propertyName="Prop1" typeName="Binary" />
                                <ECProperty propertyName="Prop2" typeName="Bentley.Geometry.Common.IGeometry" />
                                <ECProperty propertyName="Prop3" typeName="String" />
                                <ECArrayProperty propertyName="Prop4" typeName="Binary" />
                                <ECStructArrayProperty propertyName="Prop5" typeName="ecdbmap:DbIndex" />
                                <ECProperty propertyName="Prop1_Overflow" typeName="Binary" />
                                <ECProperty propertyName="Prop2_Overflow" typeName="Bentley.Geometry.Common.IGeometry" />
                              </ECEntityClass>
                            </ECSchema>)xml")));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Element(Prop1,Prop2,Prop3,Prop4,Prop5,Prop1_Overflow,Prop2_Overflow) VALUES(?,?,?,?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindZeroBlob(1, 10)) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindZeroBlob(2, 10)) << stmt.GetECSql();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindZeroBlob(3, 10)) << stmt.GetECSql() << "BindZeroBlob against string prop is ok as blob and strings are compatible in SQLite";
    ASSERT_EQ(ECSqlStatus::Error, stmt.BindZeroBlob(4, 10)) << stmt.GetECSql() << "BindZeroBlob against prim array is never allowed";
    ASSERT_EQ(ECSqlStatus::Error, stmt.BindZeroBlob(5, 10)) << stmt.GetECSql() << "BindZeroBlob against struct array is never allowed";
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindZeroBlob(6, 10)) << stmt.GetECSql() << "BindZeroBlob against property mapped to overflow col is never allowed";
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindZeroBlob(7, 10)) << stmt.GetECSql() << "BindZeroBlob against property mapped to overflow col is never allowed";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, BlobIO)
    {
    auto assertBlobIO = [] (ECDbCR ecdb, Utf8CP className, Utf8CP accessString)
        {
        ECClassCP ecClass = ecdb.Schemas().GetClass("ECSqlTest", className);
        ASSERT_TRUE(ecClass != nullptr) << className;

        std::vector<Utf8String> expectedBlobChunks {"Hello",", ", "world"};
        const int expectedBlobSize = 13;

        Utf8String ecsql;
        ecsql.Sprintf("INSERT INTO %s(%s) VALUES(?)", ecClass->GetECSqlName().c_str(), accessString);
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str())) << ecsql.c_str();
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindZeroBlob(1, expectedBlobSize)) << stmt.GetECSql();
        ECInstanceKey key;
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << stmt.GetECSql();
        stmt.Finalize();

        BlobIO io;
        ASSERT_EQ(SUCCESS, ecdb.OpenBlobIO(io, *ecClass, accessString, key.GetInstanceId(), true));
        ASSERT_TRUE(io.IsValid());
        ASSERT_EQ(expectedBlobSize, io.GetNumBytes());

        int offset = 0;
        for (Utf8StringCR blobChunk : expectedBlobChunks)
            {
            ASSERT_EQ(BE_SQLITE_OK, io.Write(blobChunk.c_str(), (int) blobChunk.size(), offset));
            offset += (int) blobChunk.size();
            }
        //add trailing \0
        Utf8Char nullChar('\0');
        ASSERT_EQ(BE_SQLITE_OK, io.Write(&nullChar, 1, offset));
        ASSERT_EQ(BE_SQLITE_OK, io.Close());
        ASSERT_FALSE(io.IsValid());

        //validate result
        ecsql.Sprintf("SELECT %s FROM %s WHERE ECInstanceId=%s", accessString,
                      ecClass->GetECSqlName().c_str(), key.GetInstanceId().ToString().c_str());

        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str())) << ecsql.c_str();
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
        int actualBlobSize = -1;
        ASSERT_STREQ("Hello, world", (Utf8CP) stmt.GetValueBlob(0, &actualBlobSize)) << stmt.GetECSql();
        ASSERT_EQ(expectedBlobSize, actualBlobSize) << stmt.GetECSql();

        //validate result via blobio
        ASSERT_EQ(SUCCESS, ecdb.OpenBlobIO(io, *ecClass, accessString, key.GetInstanceId(), false));
        ASSERT_TRUE(io.IsValid());
        ASSERT_EQ(expectedBlobSize, io.GetNumBytes());
        Utf8String actualBlobBuffer;
        actualBlobBuffer.reserve((size_t) expectedBlobSize);
        ASSERT_EQ(BE_SQLITE_OK, io.Read(const_cast<Utf8P> (actualBlobBuffer.data()), expectedBlobSize, 0));
        ASSERT_STREQ("Hello, world", actualBlobBuffer.c_str()) << "BlobIO::Read";
        };


    ASSERT_EQ(SUCCESS, SetupECDb("blobio.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    assertBlobIO(m_ecdb, "P", "Bi");
    assertBlobIO(m_ecdb, "PSA", "PStructProp.bi");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, BlobIOForInvalidProperties)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("blobioinvalidcases.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    {
    ECClassCP ecClass = m_ecdb.Schemas().GetClass("ECSqlTest", "PSA");
    ASSERT_TRUE(ecClass != nullptr);
    for (ECPropertyCP prop : ecClass->GetProperties(true))
        {
        const bool expectedToSucceed = prop->GetIsPrimitive() && (prop->GetAsPrimitiveProperty()->GetType() == PRIMITIVETYPE_Binary || prop->GetAsPrimitiveProperty()->GetType() == PRIMITIVETYPE_IGeometry);
        if (!expectedToSucceed)
            {
            BlobIO io;
            ASSERT_EQ(ERROR, m_ecdb.OpenBlobIO(io, *ecClass, prop->GetName().c_str(), ECInstanceId((uint64_t) 1), true)) << "Not a binary/geometry property";
            ASSERT_FALSE(io.IsValid());
            }
        }
    }

    {
    ECClassCP ecClass = m_ecdb.Schemas().GetClass("ECSqlTest", "PStruct");
    ASSERT_TRUE(ecClass != nullptr);
    for (ECPropertyCP prop : ecClass->GetProperties(true))
        {
        BlobIO io;
        ASSERT_EQ(ERROR, m_ecdb.OpenBlobIO(io, *ecClass, prop->GetName().c_str(), ECInstanceId((uint64_t) 1), true)) << "Cannot use BlobIO on ECStructs";
        ASSERT_FALSE(io.IsValid());
        }
    }

    {
    ECClassCP ecClass = m_ecdb.Schemas().GetClass("ECDbMap", "ClassMap");
    ASSERT_TRUE(ecClass != nullptr);
    for (ECPropertyCP prop : ecClass->GetProperties(true))
        {
        BlobIO io;
        ASSERT_EQ(ERROR, m_ecdb.OpenBlobIO(io, *ecClass, prop->GetName().c_str(), ECInstanceId((uint64_t) 1), true)) << "Cannot use BlobIO on custom attribute classes";
        ASSERT_FALSE(io.IsValid());
        }
    }

    m_ecdb.CloseDb();

    {
    ASSERT_EQ(SUCCESS, SetupECDb("blobioinvalidcases2.ecdb", SchemaItem(R"xml(
                            <ECSchema schemaName="TestSchema" alias="ts1" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                             <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                              <ECEntityClass typeName="Element">
                                 <ECCustomAttributes>
                                    <ClassMap xmlns='ECDbMap.02.00'>
                                       <MapStrategy>TablePerHierarchy</MapStrategy>
                                    </ClassMap>
                                    <ShareColumns xmlns='ECDbMap.02.00'>
                                        <MaxSharedColumnsBeforeOverflow>2</MaxSharedColumnsBeforeOverflow>
                                    </ShareColumns>
                                 </ECCustomAttributes>
                                <ECProperty propertyName="Prop1" typeName="Binary" />
                                <ECProperty propertyName="Prop2" typeName="Bentley.Geometry.Common.IGeometry" />
                                <ECProperty propertyName="Prop1_Overflow" typeName="Binary" />
                                <ECProperty propertyName="Prop2_Overflow" typeName="Bentley.Geometry.Common.IGeometry" />
                              </ECEntityClass>
                            </ECSchema>)xml")));

    ECInstanceKey key;
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO TestSchema.Element(Prop1,Prop2,Prop1_Overflow,Prop2_Overflow) VALUES(zeroblob(10),?,?,?)"));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindZeroBlob(1, 10)) << stmt.GetECSql();
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindZeroBlob(2, 10)) << stmt.GetECSql();
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindZeroBlob(3, 10)) << stmt.GetECSql();
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << stmt.GetECSql();
        }

    ECClassCP testClass = m_ecdb.Schemas().GetClass("TestSchema", "Element");
    ASSERT_TRUE(testClass != nullptr);

        {
        BlobIO io;
        ASSERT_EQ(SUCCESS, m_ecdb.OpenBlobIO(io, *testClass, "Prop1", key.GetInstanceId(), true)) << "Binary property not mapped to overflow table";
        }
        {
        BlobIO io;
        ASSERT_EQ(SUCCESS, m_ecdb.OpenBlobIO(io, *testClass, "Prop2", key.GetInstanceId(), true)) << "IGeometry property not mapped to overflow table";
        }
        {
        BlobIO io;
        ASSERT_EQ(SUCCESS, m_ecdb.OpenBlobIO(io, *testClass, "Prop1_Overflow", key.GetInstanceId(), true)) << "Binary property mapped to overflow table";
        }
        {
        BlobIO io;
        ASSERT_EQ(SUCCESS, m_ecdb.OpenBlobIO(io, *testClass, "Prop2_Overflow", key.GetInstanceId(), true)) << "IGeometry property mapped to overflow table";
        }
    }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, ECSqlOnAbstractClassWithOrderbyClause)
    {
    auto assertECSql = [] (Utf8CP ecsql, ECDbCR ecdb, ECSqlStatus sqlStatus = ECSqlStatus::Success, DbResult dbResult = DbResult::BE_SQLITE_DONE)
        {
        ECSqlStatement statement;
        ASSERT_EQ(sqlStatus, statement.Prepare(ecdb, ecsql));
        ASSERT_EQ(dbResult, statement.Step());
        };

    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlonabstractclasswithorderbyclause.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                                                       "<ECSchema schemaName='TestSchema' alias='rs' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                                                       "    <ECEntityClass typeName='Element' modifier='Abstract'>"
                                                                       "        <ECProperty propertyName='P0' typeName='string' />"
                                                                       "        <ECProperty propertyName='P1' typeName='int' />"
                                                                       "    </ECEntityClass>"
                                                                       "    <ECEntityClass typeName='GeometricElement' >"
                                                                       "         <BaseClass>Element</BaseClass>"
                                                                       "    </ECEntityClass>"
                                                                       "</ECSchema>")));

    assertECSql("INSERT INTO TestSchema.GeometricElement(P0, P1) VALUES ('World ', 2)", m_ecdb);
    assertECSql("INSERT INTO TestSchema.GeometricElement(P0, P1) VALUES ('Hello ', 1)", m_ecdb);
    assertECSql("INSERT INTO TestSchema.GeometricElement(P0, P1) VALUES ('!!!', 3)", m_ecdb);

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT P0 FROM TestSchema.Element ORDER BY P1"));
    int rowCount = 0;
    Utf8CP expectedValues = "Hello World !!!";
    Utf8String actualValues;
    while (statement.Step() == BE_SQLITE_ROW)
        {
        actualValues.append(statement.GetValueText(0));
        rowCount++;
        }
    ASSERT_STREQ(expectedValues, actualValues.c_str()) << statement.GetECSql();
    ASSERT_EQ(3, rowCount);
    statement.Finalize();
    }



//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, OrderByAgainstMixin)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("orderbyonmixin.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA"/>
            <ECEntityClass typeName="Base" modifier="Abstract">
                <ECProperty propertyName="BaseProp1" typeName="string" />
                <ECProperty propertyName="BaseProp2" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="IOriginSource" modifier="Abstract">
              <ECCustomAttributes>
                  <IsMixin xmlns="CoreCustomAttributes.01.00.00">
                    <AppliesToEntityClass>Base</AppliesToEntityClass>
                  </IsMixin>
                </ECCustomAttributes>
                <ECProperty propertyName="Code" typeName="int" />
                <ECProperty propertyName="Origin" typeName="Point3d" />
            </ECEntityClass>
            <ECEntityClass typeName="Geometric2dElement">
                 <BaseClass>Base</BaseClass>
                 <BaseClass>IOriginSource</BaseClass>
                <ECProperty propertyName="TwoDType" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="Geometric3dElement">
                 <BaseClass>Base</BaseClass>
                 <BaseClass>IOriginSource</BaseClass>
                <ECProperty propertyName="ThreeDType" typeName="string" />
            </ECEntityClass>
        </ECSchema>)xml")));

    {
    int id = 1;
    ECSqlStatement statement1;
    ASSERT_EQ(ECSqlStatus::Success, statement1.Prepare(m_ecdb, "INSERT INTO ts.Geometric2dElement(TwoDType, Code, Origin, BaseProp2) VALUES('Line', ?, ?, ?)"));

    ECSqlStatement statement2;
    ASSERT_EQ(ECSqlStatus::Success, statement2.Prepare(m_ecdb, "INSERT INTO ts.Geometric3dElement(ThreeDType, Code, Origin, BaseProp2) VALUES('Volume', ?, ?, ?)"));

    ASSERT_EQ(ECSqlStatus::Success, statement1.BindInt(1, 10));
    ASSERT_EQ(ECSqlStatus::Success, statement1.BindPoint3d(2, DPoint3d::From(10.0,10.0,10.0)));
    ASSERT_EQ(ECSqlStatus::Success, statement1.BindInt(3, id));
    ASSERT_EQ(BE_SQLITE_DONE, statement1.Step());
    statement1.Reset();
    statement1.ClearBindings();

    id++;
    ASSERT_EQ(ECSqlStatus::Success, statement2.BindInt(1, 5));
    ASSERT_EQ(ECSqlStatus::Success, statement2.BindPoint3d(2, DPoint3d::From(5.0, 5.0, 5.0)));
    ASSERT_EQ(ECSqlStatus::Success, statement2.BindInt(3, id));
    ASSERT_EQ(BE_SQLITE_DONE, statement2.Step());
    statement2.Reset();
    statement2.ClearBindings();

    id++;
    ASSERT_EQ(ECSqlStatus::Success, statement1.BindInt(1, 2));
    ASSERT_EQ(ECSqlStatus::Success, statement1.BindPoint3d(2, DPoint3d::From(2.0, 2.0, 2.0)));
    ASSERT_EQ(ECSqlStatus::Success, statement1.BindInt(3, id));
    ASSERT_EQ(BE_SQLITE_DONE, statement1.Step());
    statement1.Reset();
    statement1.ClearBindings();

    id++;
    ASSERT_EQ(ECSqlStatus::Success, statement2.BindInt(1, 1));
    ASSERT_EQ(ECSqlStatus::Success, statement2.BindPoint3d(2, DPoint3d::From(1.0, 1.0, 1.0)));
    ASSERT_EQ(ECSqlStatus::Success, statement2.BindInt(3, id));
    ASSERT_EQ(BE_SQLITE_DONE, statement2.Step());
    statement2.Reset();
    statement2.ClearBindings();
    }

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT Origin.X, Origin.Y, Origin.Z, Code FROM ts.IOriginSource ORDER BY Code"));
    std::vector<int> expectedCodes {1, 2, 5, 10};
    int rowCount = 0;
    while (BE_SQLITE_ROW == statement.Step())
        {
        int ix = rowCount;
        rowCount++;
        const int actualCode = statement.GetValueInt(3);
        ASSERT_EQ(expectedCodes[(size_t) ix], actualCode);
        ASSERT_EQ(actualCode, statement.GetValueInt(0));
        ASSERT_EQ(actualCode, statement.GetValueInt(1));
        ASSERT_EQ(actualCode, statement.GetValueInt(2));
        }

    ASSERT_EQ((int) expectedCodes.size(), rowCount);
    }


//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, UpdateAndDeleteAgainstMixin)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("updatedeletemixin.ecdb",
                           SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                                          <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                          <ECSchemaReference name="CoreCustomAttributes" version="01.00" alias="CoreCA" />
                                          <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                                              <ECEntityClass typeName="Model">
                                                 <ECCustomAttributes>
                                                   <ClassMap xmlns="ECDbMap.02.00">
                                                          <MapStrategy>TablePerHierarchy</MapStrategy>
                                                   </ClassMap>
                                                   <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00"/>
                                                 </ECCustomAttributes>
                                                  <ECProperty propertyName="Name" typeName="string" />
                                              </ECEntityClass>
                                              <ECEntityClass typeName="IIsGeometricModel" modifier="Abstract">
                                                 <ECCustomAttributes>
                                                    <IsMixin xmlns="CoreCustomAttributes.01.00">
                                                        <AppliesToEntityClass>Model</AppliesToEntityClass>
                                                    </IsMixin>"
                                                 </ECCustomAttributes>
                                                  <ECProperty propertyName="Is2d" typeName="boolean"/>
                                                  <ECProperty propertyName="SupportedGeometryType" typeName="string" />
                                              </ECEntityClass>
                                              <ECEntityClass typeName="Model2d">
                                                    <BaseClass>Model</BaseClass>
                                                    <BaseClass>IIsGeometricModel</BaseClass>
                                                  <ECProperty propertyName="Origin2d" typeName="Point2d" />
                                              </ECEntityClass>
                                              <ECEntityClass typeName="Model3d">
                                                    <BaseClass>Model</BaseClass>
                                                    <BaseClass>IIsGeometricModel</BaseClass>
                                                  <ECProperty propertyName="Origin3d" typeName="Point3d" />
                                              </ECEntityClass>
                                          </ECSchema>)xml")));

    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().CreateClassViewsInDb());
    m_ecdb.SaveChanges();

    std::vector<ECInstanceKey> model2dKeys, model3dKeys, element2dKeys, element3dKeys;
    //INSERT
    {
    ECInstanceKey key;
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Model2d(Name,Is2d,SupportedGeometryType,Origin2d) VALUES(?,true,'Line',?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Main2d", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint2d(2, DPoint2d::From(1.0, 1.0)));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    ASSERT_TRUE(key.IsValid());
    model2dKeys.push_back(key);
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Detail2d", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint2d(2, DPoint2d::From(1.5, 1.5)));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    ASSERT_TRUE(key.IsValid());
    model2dKeys.push_back(key);
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Model3d(Name,Is2d,SupportedGeometryType,Origin3d) VALUES(?,false,'Solid',?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Main3d", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint3d(2, DPoint3d::From(2.0, 2.0, 2.0)));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    ASSERT_TRUE(key.IsValid());
    model3dKeys.push_back(key);
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Detail3d", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint3d(2, DPoint3d::From(2.5, 2.5, 2.5)));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    ASSERT_TRUE(key.IsValid());
    model3dKeys.push_back(key);
    }
    m_ecdb.SaveChanges();
    {
    //Select models via mixin
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId, ECClassId, Is2d, SupportedGeometryType FROM ts.IIsGeometricModel"));
    int rowCount2d = 0;
    int rowCount3d = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        ECClassId actualClassId = stmt.GetValueId<ECClassId>(1);
        if (actualClassId == model2dKeys[0].GetClassId())
            {
            rowCount2d++;
            ASSERT_TRUE(stmt.GetValueBoolean(2)) << stmt.GetECSql();
            ASSERT_STREQ("Line", stmt.GetValueText(3)) << stmt.GetECSql();
            }
        else if (actualClassId == model3dKeys[0].GetClassId())
            {
            rowCount3d++;
            ASSERT_FALSE(stmt.GetValueBoolean(2)) << stmt.GetECSql();
            ASSERT_STREQ("Solid", stmt.GetValueText(3)) << stmt.GetECSql();
            }
        }

    ASSERT_EQ(2, rowCount2d) << stmt.GetECSql();
    ASSERT_EQ(2, rowCount3d) << stmt.GetECSql();
    stmt.Finalize();

    //Select concrete models
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId, Name, SupportedGeometryType, Origin2d FROM ts.Model2d"));
    int rowCount = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        rowCount++;
        ASSERT_STREQ("Line", stmt.GetValueText(2));

        const ECInstanceId actualModelId = stmt.GetValueId<ECInstanceId>(0);
        Utf8CP actualModelName = stmt.GetValueText(1);
        if (actualModelId == model2dKeys[0].GetInstanceId())
            ASSERT_STREQ("Main2d", actualModelName) << stmt.GetECSql();
        else if (actualModelId == model2dKeys[1].GetInstanceId())
            ASSERT_STREQ("Detail2d", actualModelName) << stmt.GetECSql();
        }

    ASSERT_EQ(2, rowCount) << stmt.GetECSql();

    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId, Name, SupportedGeometryType, Origin3d FROM ts.Model3d"));
    rowCount = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        rowCount++;
        ASSERT_STREQ("Solid", stmt.GetValueText(2));

        const ECInstanceId actualModelId = stmt.GetValueId<ECInstanceId>(0);
        Utf8CP actualModelName = stmt.GetValueText(1);
        if (actualModelId == model3dKeys[0].GetInstanceId())
            ASSERT_STREQ("Main3d", actualModelName) << stmt.GetECSql();
        else if (actualModelId == model3dKeys[1].GetInstanceId())
            ASSERT_STREQ("Detail3d", actualModelName) << stmt.GetECSql();
        }

    ASSERT_EQ(2, rowCount) << stmt.GetECSql();
    }

    //UPDATE Mixin non-polymorphically (not allowed)
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "UPDATE ONLY ts.IIsGeometricModel SET SupportedGeometryType='Surface' WHERE Is2d"));
    }

    //UPDATE Mixin polymorphically (not allowed)
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "UPDATE ts.IIsGeometricModel SET SupportedGeometryType='Surface' WHERE Is2d"));
    }

    //DELETE Mixin non-polymorphically (not allowed)
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "DELETE FROM ONLY ts.IIsGeometricModel WHERE Is2d=False"));
    }

    //DELETE Mixin polymorphically (not allowed)
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "DELETE FROM ts.IIsGeometricModel WHERE Is2d=False"));
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, PrepareECSqlStatmentUsingSecondaryReadonlyConnection)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("econn1.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECEntityClass typeName="A">
                        <ECProperty propertyName="Name" typeName="string" />
                        <ECProperty propertyName="Size" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="B">
                        <ECProperty propertyName="Name" typeName="string" />
                        <ECProperty propertyName="A" typeName="int" />
                     </ECEntityClass>
                </ECSchema>)xml")));

    BeFileName seedFileA(m_ecdb.GetDbFileName(), true);
    m_ecdb.CloseDb();

    ASSERT_EQ(SUCCESS, SetupECDb("econn2.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECEntityClass typeName="A">
                        <ECProperty propertyName="Name" typeName="string" />
                        <ECProperty propertyName="Size" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="B">
                        <ECProperty propertyName="Name" typeName="string" />
                        <ECProperty propertyName="A" typeName="int" />
                     </ECEntityClass>
                </ECSchema>)xml")));

    BeFileName seedFileB(m_ecdb.GetDbFileName(), true);
    m_ecdb.CloseDb();


    ECDb ecdbA, ecdbB;
    Db dbA, dbB;

    ASSERT_EQ(BE_SQLITE_OK, ecdbA.OpenBeSQLiteDb(seedFileA, Db::OpenParams(Db::OpenMode::Readonly)));
    ASSERT_EQ(BE_SQLITE_OK, dbA.OpenBeSQLiteDb(seedFileA, Db::OpenParams(Db::OpenMode::Readonly)));
    ASSERT_EQ(BE_SQLITE_OK, ecdbB.OpenBeSQLiteDb(seedFileB, Db::OpenParams(Db::OpenMode::Readonly)));
    ASSERT_EQ(BE_SQLITE_OK, dbB.OpenBeSQLiteDb(seedFileB, Db::OpenParams(Db::OpenMode::Readonly)));

    { //ECdb and Db must match. They must point to exact same file on disk
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdbA.Schemas(), dbA, "SELECT* FROM ts.A"));
    }

    { //ECdb and Db must match. They must point to exact same file on disk
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Error, stmt.Prepare(ecdbA.Schemas(), dbB, "SELECT* FROM ts.A"));
    }

    { //ECdb and Db must match. They must point to exact same file on disk
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Error, stmt.Prepare(ecdbB.Schemas(), dbA, "SELECT* FROM ts.A"));
    }

    { //ECdb and Db must match. They must point to exact same file on disk
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdbB.Schemas(), dbB, "SELECT* FROM ts.A"));
    }

    { //Only metaData could be read/write or readonly. Data connection must be readonly
    Db wDbA;
    ASSERT_EQ(BE_SQLITE_OK, wDbA.OpenBeSQLiteDb(seedFileA, Db::OpenParams(Db::OpenMode::ReadWrite)));
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Error, stmt.Prepare(ecdbA.Schemas(), wDbA, "SELECT* FROM ts.A"));
    }

    { //Only metaData could be read/write or readonly. Data connection must be readonly
    ECDb wECDb;
    ASSERT_EQ(BE_SQLITE_OK, wECDb.OpenBeSQLiteDb(seedFileA, Db::OpenParams(Db::OpenMode::ReadWrite)));
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(wECDb.Schemas(), dbA, "SELECT* FROM ts.A"));
    }

    { //Only SELECT is supported
    ECDb wECDb;
    ASSERT_EQ(BE_SQLITE_OK, wECDb.OpenBeSQLiteDb(seedFileA, Db::OpenParams(Db::OpenMode::ReadWrite)));
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Error, stmt.Prepare(wECDb.Schemas(), dbA, "insert into [ts].[A]([Name],[Size])values('foo', 2222)"));
    }

    { //Only SELECT is supported
    ECDb wECDb;
    ASSERT_EQ(BE_SQLITE_OK, wECDb.OpenBeSQLiteDb(seedFileA, Db::OpenParams(Db::OpenMode::ReadWrite)));
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Error, stmt.Prepare(wECDb.Schemas(), dbA, "update [ts].[A] set [Name]='foo',[Size]=233223"));
    }

    { //Only SELECT is supported
    ECDb wECDb;
    ASSERT_EQ(BE_SQLITE_OK, wECDb.OpenBeSQLiteDb(seedFileA, Db::OpenParams(Db::OpenMode::ReadWrite)));
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Error, stmt.Prepare(wECDb.Schemas(), dbA, "delete from [ts].[A]"));
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, InterruptStep)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("InterruptStep.ecdb"));

    int totalRowCount = -1;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT count(*) FROM meta.ECPropertyDef"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    totalRowCount = stmt.GetValueInt(0);
    }

    const int interruptAfterIterations = 5;
    ASSERT_GE(totalRowCount, interruptAfterIterations + 100) << "There should be many rows after the interruption to make sure SQLite does not finish the stepping";
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * FROM meta.ECPropertyDef"));

    for (int i = 0; i < interruptAfterIterations; i++)
        {
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        }

    m_ecdb.Interrupt();
    ASSERT_EQ(BE_SQLITE_INTERRUPT, stmt.Step());
    }

//---------------------------------------------------------------------------------
// Verifies correct ECSQL parsing on Android
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, ECSqlParseTreeFormatter_ParseAndFormatECSqlParseNodeTree)
    {
    auto AssertParseECSql = [] (ECDbCR ecdb, Utf8CP ecsql)
        {
        Utf8String parseTree;
        ASSERT_EQ(SUCCESS, ECSqlParseTreeFormatter::ParseAndFormatECSqlParseNodeTree(parseTree, ecdb, ecsql)) << "Failed to parse ECSQL";
        };

    ECDb ecdb; // only needed for issue listener, doesn't need to represent a file on disk
    AssertParseECSql(ecdb, "SELECT '''' FROM stco.Hardware");
    AssertParseECSql(ecdb, "SELECT 'aa', '''', b FROM stco.Hardware WHERE Name = 'a''b'");
    AssertParseECSql(ecdb, "SELECT _Aa, _bC, _123, Abc, a123, a_123, a_b, _a_b_c FROM stco.Hardware WHERE Name = 'Fusion'");
    AssertParseECSql(ecdb, "SELECT * FROM stco.Hardware WHERE Name = 'Fusion'");
    AssertParseECSql(ecdb, "SELECT [Foo].[Name] FROM stco.[Hardware] [Foo]");
    AssertParseECSql(ecdb, "SELECT [Foo].[Name] FROM stco.[Hardware] [Foo] WHERE [Name] = 'HelloWorld'");
    AssertParseECSql(ecdb, "Select EQUIP_NO From only appdw.Equipment where EQUIP_NO = '50E-101A' ");
    AssertParseECSql(ecdb, "INSERT INTO [V8TagsetDefinitions].[STRUCTURE_IL1] ([VarFixedStartZ], [DeviceID1], [ObjectType], [PlaceMethod], [CopyConstrDrwToProj]) VALUES ('?', '-E1-1', 'SGL', '1', 'Y')");
    AssertParseECSql(ecdb, "INSERT INTO [V8TagsetDefinitions].[grid__x0024__0__x0024__CB_1] ([CB_1_8457], [CB_1_8456], [CB_1_8455], [CB_1_8454], [CB_1_8457], [CB_1_8456], [CB_1_8455], [CB_1_8454], [CB_1_8457], [CB_1_8456], [CB_1_8455], [CB_1_8454], [CB_1_8457], [CB_1_8456], [CB_1_8455], [CB_1_8454], [CB_1_8457], [CB_1_8456], [CB_1_8455], [CB_1_8454], [CB_1_8457], [CB_1_8456], [CB_1_8455], [CB_1_8454], [CB_1_8457],[CB_1_8456], [CB_1_8455], [CB_1_8454], [CB_1_8457], [CB_1_8456], [CB_1_8455], [CB_1_8454]) VALUES ('', '1.1', '', '', '', '2.2', '', '', '', '2.5', '', '', '', '2.5', '', '', '', '2.1', '', '', '', 'E.3', '', '', '', 'B.4', '', '', '', 'D.4', '', '')");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementTestFixture, OptimizeECSqlForSealedAndClassWithNotDerviedClasses)
    {
    if (true)
        {
        ASSERT_EQ(SUCCESS, SetupECDb("ecsqlOpt00.ecdb", SchemaItem(
            R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECEntityClass typeName="Foo">
                        <ECProperty propertyName="Name" typeName="string" />
                        <ECProperty propertyName="Size" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Goo" modifier="sealed">
                        <ECProperty propertyName="Name" typeName="string" />
                        <ECProperty propertyName="A" typeName="int" />
                     </ECEntityClass>
                </ECSchema>)xml")));


        ECClassId fooId = m_ecdb.Schemas().GetClass("TestSchema", "Foo")->GetId();
        ECClassId gooId = m_ecdb.Schemas().GetClass("TestSchema", "Goo")->GetId();

        if (true)
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ts.Foo"));
            Utf8String acutal = stmt.GetNativeSql();
            Utf8String expected = Utf8PrintfString("SELECT [Foo].[ECInstanceId] FROM (SELECT [Id] ECInstanceId,%" PRIu64 " ECClassId FROM [main].[ts_Foo]) [Foo]", fooId.GetValue());
            ASSERT_STREQ(expected.c_str(), acutal.c_str());
            }


        if (true)
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ONLY ts.Foo"));
            Utf8String acutal = stmt.GetNativeSql();
            Utf8String expected = Utf8PrintfString("SELECT [Foo].[ECInstanceId] FROM (SELECT [Id] ECInstanceId,%" PRIu64 " ECClassId FROM [main].[ts_Foo]) [Foo]", fooId.GetValue());
            ASSERT_STREQ(expected.c_str(), acutal.c_str());
            }

        if (true)
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ts.Goo"));
            Utf8String acutal = stmt.GetNativeSql();
            Utf8String expected = Utf8PrintfString("SELECT [Goo].[ECInstanceId] FROM (SELECT [Id] ECInstanceId,%" PRIu64 " ECClassId FROM [main].[ts_Goo]) [Goo]", gooId.GetValue());
            ASSERT_STREQ(expected.c_str(), acutal.c_str());
            }

        if (true)
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ONLY ts.Goo"));
            Utf8String acutal = stmt.GetNativeSql();
            Utf8String expected = Utf8PrintfString("SELECT [Goo].[ECInstanceId] FROM (SELECT [Id] ECInstanceId,%" PRIu64 " ECClassId FROM [main].[ts_Goo]) [Goo]", gooId.GetValue());
            ASSERT_STREQ(expected.c_str(), acutal.c_str());
            }

        m_ecdb.CloseDb();
        }

    if (true)
        {
        ASSERT_EQ(SUCCESS, SetupECDb("ecsqlOpt01.ecdb", SchemaItem(
            R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Boo">
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                 <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00'/>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName="Foo">
                       <BaseClass>Boo</BaseClass>
                        <ECProperty propertyName="Name" typeName="string" />
                        <ECProperty propertyName="Size" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Goo" modifier="sealed">
\                       <BaseClass>Boo</BaseClass>
                        <ECProperty propertyName="Name" typeName="string" />
                        <ECProperty propertyName="A" typeName="int" />
                     </ECEntityClass>
                </ECSchema>)xml")));


        ECClassId fooId = m_ecdb.Schemas().GetClass("TestSchema", "Foo")->GetId();
        ECClassId gooId = m_ecdb.Schemas().GetClass("TestSchema", "Goo")->GetId();

        if (true)
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ts.Foo"));
            Utf8String acutal = stmt.GetNativeSql();
            Utf8String expected = Utf8PrintfString("SELECT [Foo].[ECInstanceId] FROM (SELECT [Id] ECInstanceId,[ECClassId] FROM [main].[ts_Boo] WHERE [ts_Boo].ECClassId=%" PRIu64 ") [Foo]", fooId.GetValue());
            ASSERT_STREQ(expected.c_str(), acutal.c_str());
            }


        if (true)
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ONLY ts.Foo"));
            Utf8String acutal = stmt.GetNativeSql();
            Utf8String expected = Utf8PrintfString("SELECT [Foo].[ECInstanceId] FROM (SELECT [Id] ECInstanceId,[ECClassId] FROM [main].[ts_Boo] WHERE [ts_Boo].ECClassId=%" PRIu64 ") [Foo]", fooId.GetValue());
            ASSERT_STREQ(expected.c_str(), acutal.c_str());
            }

        if (true)
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ts.Goo"));
            Utf8String acutal = stmt.GetNativeSql();
            Utf8String expected = Utf8PrintfString("SELECT [Goo].[ECInstanceId] FROM (SELECT [Id] ECInstanceId,[ECClassId] FROM [main].[ts_Boo] WHERE [ts_Boo].ECClassId=%" PRIu64 ") [Goo]", gooId.GetValue());
            ASSERT_STREQ(expected.c_str(), acutal.c_str());
            }

        if (true)
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ONLY ts.Goo"));
            Utf8String acutal = stmt.GetNativeSql();
            Utf8String expected = Utf8PrintfString("SELECT [Goo].[ECInstanceId] FROM (SELECT [Id] ECInstanceId,[ECClassId] FROM [main].[ts_Boo] WHERE [ts_Boo].ECClassId=%" PRIu64 ") [Goo]", gooId.GetValue());
            ASSERT_STREQ(expected.c_str(), acutal.c_str());
            }

        m_ecdb.CloseDb();
        }


    if (true)
        {
        ASSERT_EQ(SUCCESS, SetupECDb("ecsqlOpt02.ecdb", SchemaItem(
            R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Boo">
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                 <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00'/>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName="Foo">
                       <BaseClass>Boo</BaseClass>
                        <ECProperty propertyName="Name" typeName="string" />
                        <ECProperty propertyName="Size" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Coo">
                       <BaseClass>Foo</BaseClass>
                        <ECProperty propertyName="N1" typeName="string" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Voo">
                       <BaseClass>Foo</BaseClass>
                        <ECProperty propertyName="N2" typeName="string" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Goo" modifier="sealed">
                       <BaseClass>Boo</BaseClass>
                        <ECProperty propertyName="Name" typeName="string" />
                        <ECProperty propertyName="A" typeName="int" />
                     </ECEntityClass>
                </ECSchema>)xml")));


        ECClassId fooId = m_ecdb.Schemas().GetClass("TestSchema", "Foo")->GetId();
        ECClassId gooId = m_ecdb.Schemas().GetClass("TestSchema", "Goo")->GetId();
        ECClassId cooId = m_ecdb.Schemas().GetClass("TestSchema", "Coo")->GetId();
        ECClassId vooId = m_ecdb.Schemas().GetClass("TestSchema", "Voo")->GetId();

        if (true)
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ts.Foo"));
            Utf8String acutal = stmt.GetNativeSql();
            Utf8String expected = Utf8PrintfString("SELECT [Foo].[ECInstanceId] FROM (SELECT [ts_Boo].[Id] ECInstanceId,[ts_Boo].[ECClassId] FROM [main].[ts_Boo] INNER JOIN [main].ec_cache_ClassHierarchy [CHC_ts_Boo] ON [CHC_ts_Boo].[ClassId]=[ts_Boo].ECClassId AND [CHC_ts_Boo].[BaseClassId]=%" PRIu64 ") [Foo]", fooId.GetValue());
            ASSERT_STREQ(expected.c_str(), acutal.c_str());
            }


        if (true)
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ONLY ts.Foo"));
            Utf8String acutal = stmt.GetNativeSql();
            Utf8String expected = Utf8PrintfString("SELECT [Foo].[ECInstanceId] FROM (SELECT [Id] ECInstanceId,[ECClassId] FROM [main].[ts_Boo] WHERE [ts_Boo].ECClassId=%" PRIu64 ") [Foo]", fooId.GetValue());
            ASSERT_STREQ(expected.c_str(), acutal.c_str());
            }


        if (true)
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ts.Coo"));
            Utf8String acutal = stmt.GetNativeSql();
            Utf8String expected = Utf8PrintfString("SELECT [Coo].[ECInstanceId] FROM (SELECT [Id] ECInstanceId,[ECClassId] FROM [main].[ts_Boo] WHERE [ts_Boo].ECClassId=%" PRIu64 ") [Coo]", cooId.GetValue());
            ASSERT_STREQ(expected.c_str(), acutal.c_str());
            }


        if (true)
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ONLY ts.Coo"));
            Utf8String acutal = stmt.GetNativeSql();
            Utf8String expected = Utf8PrintfString("SELECT [Coo].[ECInstanceId] FROM (SELECT [Id] ECInstanceId,[ECClassId] FROM [main].[ts_Boo] WHERE [ts_Boo].ECClassId=%" PRIu64 ") [Coo]", cooId.GetValue());
            ASSERT_STREQ(expected.c_str(), acutal.c_str());
            }

        if (true)
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ts.Voo"));
            Utf8String acutal = stmt.GetNativeSql();
            Utf8String expected = Utf8PrintfString("SELECT [Voo].[ECInstanceId] FROM (SELECT [Id] ECInstanceId,[ECClassId] FROM [main].[ts_Boo] WHERE [ts_Boo].ECClassId=%" PRIu64 ") [Voo]", vooId.GetValue());
            ASSERT_STREQ(expected.c_str(), acutal.c_str());
            }


        if (true)
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ONLY ts.Voo"));
            Utf8String acutal = stmt.GetNativeSql();
            Utf8String expected = Utf8PrintfString("SELECT [Voo].[ECInstanceId] FROM (SELECT [Id] ECInstanceId,[ECClassId] FROM [main].[ts_Boo] WHERE [ts_Boo].ECClassId=%" PRIu64 ") [Voo]", vooId.GetValue());
            ASSERT_STREQ(expected.c_str(), acutal.c_str());
            }


        if (true)
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ts.Goo"));
            Utf8String acutal = stmt.GetNativeSql();
            Utf8String expected = Utf8PrintfString("SELECT [Goo].[ECInstanceId] FROM (SELECT [Id] ECInstanceId,[ECClassId] FROM [main].[ts_Boo] WHERE [ts_Boo].ECClassId=%" PRIu64 ") [Goo]", gooId.GetValue());
            ASSERT_STREQ(expected.c_str(), acutal.c_str());
            }

        if (true)
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ONLY ts.Goo"));
            Utf8String acutal = stmt.GetNativeSql();
            Utf8String expected = Utf8PrintfString("SELECT [Goo].[ECInstanceId] FROM (SELECT [Id] ECInstanceId,[ECClassId] FROM [main].[ts_Boo] WHERE [ts_Boo].ECClassId=%" PRIu64 ") [Goo]", gooId.GetValue());
            ASSERT_STREQ(expected.c_str(), acutal.c_str());
            }

        m_ecdb.CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, AliasedEnumProps)
    {
    for (Utf8CP schemaXml : {R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECEnumeration typeName="Status" backingTypeName="int" isStrict="true">
                    <ECEnumerator value="1" displayLabel="On" />
                    <ECEnumerator value="2" displayLabel="Off"/>
                </ECEnumeration>
                <ECEnumeration typeName="Domains" backingTypeName="string" isStrict="true">
                    <ECEnumerator value="Org" displayLabel="Org" />
                    <ECEnumerator value="Com" displayLabel="Com"/>
                </ECEnumeration>
                <ECEntityClass typeName="Foo" >
                    <ECProperty propertyName="Status" typeName="Status" />
                    <ECArrayProperty propertyName="Statuses" typeName="Status" />
                    <ECProperty propertyName="Domain" typeName="Domain" />
                    <ECArrayProperty propertyName="Domains" typeName="Domain" />
                </ECEntityClass>
              </ECSchema>)xml",
                R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECEnumeration typeName="Status" backingTypeName="int" isStrict="true">
                    <ECEnumerator name="On" value="1" />
                    <ECEnumerator name="Off" value="2" />
                </ECEnumeration>
                <ECEnumeration typeName="Domains" backingTypeName="string" isStrict="true">
                    <ECEnumerator name="Org" value="Org" displayLabel="Org" />
                    <ECEnumerator name="Com" value="Com" displayLabel="Com"/>
                </ECEnumeration>
                <ECEntityClass typeName="Foo" >
                    <ECProperty propertyName="Status" typeName="Status" />
                    <ECArrayProperty propertyName="Statuses" typeName="Status" />
                    <ECProperty propertyName="Domain" typeName="Domain" />
                    <ECArrayProperty propertyName="Domains" typeName="Domain" />
                </ECEntityClass>
              </ECSchema>)xml"})
        {
        ASSERT_EQ(SUCCESS, SetupECDb("AliasedEnumProps.ecdb", SchemaItem(schemaXml)));
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Foo(Status,Statuses,Domain,Domains) VALUES (1,?,'Org',?)"));
        ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(1).AddArrayElement().BindInt(1)) << stmt.GetECSql();
        ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(1).AddArrayElement().BindInt(2)) << stmt.GetECSql();
        ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(2).AddArrayElement().BindText("Org", IECSqlBinder::MakeCopy::No)) << stmt.GetECSql();
        ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(2).AddArrayElement().BindText("Com", IECSqlBinder::MakeCopy::No)) << stmt.GetECSql();
        ECInstanceKey key;
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << stmt.GetECSql();
        stmt.Finalize();

        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT Status as MyStatus,Statuses as MyStatuses,Domain as MyDomain, Domains as MyDomains FROM ts.Foo"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
        ECSqlColumnInfoCR colInfo0 = stmt.GetColumnInfo(0);
        ASSERT_TRUE(colInfo0.IsGeneratedProperty()) << stmt.GetECSql();
        ASSERT_FALSE(colInfo0.GetDataType().IsArray()) << stmt.GetECSql();
        ASSERT_EQ(PRIMITIVETYPE_Integer, colInfo0.GetDataType().GetPrimitiveType()) << stmt.GetECSql();

        ECSqlColumnInfoCR colInfo1 = stmt.GetColumnInfo(1);
        ASSERT_TRUE(colInfo1.IsGeneratedProperty()) << stmt.GetECSql();
        ASSERT_TRUE(colInfo1.GetDataType().IsArray()) << stmt.GetECSql();
        ASSERT_EQ(PRIMITIVETYPE_Integer, colInfo1.GetDataType().GetPrimitiveType()) << stmt.GetECSql();

        ECSqlColumnInfoCR colInfo2 = stmt.GetColumnInfo(2);
        ASSERT_TRUE(colInfo2.IsGeneratedProperty()) << stmt.GetECSql();
        ASSERT_FALSE(colInfo2.GetDataType().IsArray()) << stmt.GetECSql();
        ASSERT_EQ(PRIMITIVETYPE_String, colInfo2.GetDataType().GetPrimitiveType()) << stmt.GetECSql();

        ECSqlColumnInfoCR colInfo3 = stmt.GetColumnInfo(3);
        ASSERT_TRUE(colInfo3.IsGeneratedProperty()) << stmt.GetECSql();
        ASSERT_TRUE(colInfo3.GetDataType().IsArray()) << stmt.GetECSql();
        ASSERT_EQ(PRIMITIVETYPE_String, colInfo3.GetDataType().GetPrimitiveType()) << stmt.GetECSql();

        stmt.Finalize();
        CloseECDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, EnumeratorNamesForEC31Enums)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("EnumeratorNamesForEC31Enums.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8' ?>"
        "<ECSchema schemaName='TestSchema' displayLabel='Test Schema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECEnumeration typeName='Colors' displayLabel='Color' backingTypeName='string' isStrict='True'>"
        "        <ECEnumerator value='Red'/>"
        "        <ECEnumerator value='Blue'/>"
        "        <ECEnumerator value='Green'/>"
        "        <ECEnumerator value='Yellow'/>"
        "        <ECEnumerator value='Black'/>"
        "    </ECEnumeration>"
        "    <ECEnumeration typeName='Domains' displayLabel='Domain' backingTypeName='int' isStrict='True'>"
        "        <ECEnumerator value='1' DisplayLabel='com'/>"
        "        <ECEnumerator value='2' DisplayLabel='org'/>"
        "        <ECEnumerator value='3' DisplayLabel='edu'/>"
        "        <ECEnumerator value='4' DisplayLabel='net'/>"
        "        <ECEnumerator value='5' DisplayLabel='int'/>"
        "    </ECEnumeration>"
        "    <ECEntityClass typeName='TestClass'>"
        "        <ECProperty propertyName='Color' typeName='Colors'/>"
        "        <ECProperty propertyName='Domain' typeName='Domains' />"
        "    </ECEntityClass>"
        "</ECSchema>")));

    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.TestClass(Color,Domain) VALUES (ts.Colors.Red, ts.Domains.Domains1)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.TestClass(Color,Domain) VALUES ('Red', 1)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.TestClass(Color,Domain) VALUES (ts.Colors.Blue, ts.Domains.Domains2)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.TestClass(Color,Domain) VALUES ('Blue', 2)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.TestClass(Color,Domain) VALUES (ts.Colors.Green, ts.Domains.Domains3)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.TestClass(Color,Domain) VALUES ('Green', 3)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.TestClass(Color,Domain) VALUES (ts.Colors.Yellow, ts.Domains.Domains4)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.TestClass(Color,Domain) VALUES ('Yellow', 4)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.TestClass(Color,Domain) VALUES (ts.Colors.Black, ts.Domains.Domains5)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.TestClass(Color,Domain) VALUES ('Black', 5)"));
    EXPECT_EQ(JsonValue("[{\"cnt\": 2}]"), GetHelper().ExecuteSelectECSql("SELECT COUNT(*) cnt FROM ts.TestClass WHERE Color = ts.Colors.Red"));
    EXPECT_EQ(JsonValue("[{\"cnt\": 2}]"), GetHelper().ExecuteSelectECSql("SELECT COUNT(*) cnt FROM ts.TestClass WHERE Color = 'Red'"));
    EXPECT_EQ(JsonValue("[{\"cnt\": 4}]"), GetHelper().ExecuteSelectECSql("SELECT COUNT(*) cnt FROM ts.TestClass WHERE Color IN (ts.Colors.Red, ts.Colors.Blue)"));
    EXPECT_EQ(JsonValue("[{\"cnt\": 4}]"), GetHelper().ExecuteSelectECSql("SELECT COUNT(*) cnt FROM ts.TestClass WHERE Color IN (ts.Colors.Red, 'Blue')"));
    EXPECT_EQ(JsonValue("[{\"cnt\": 0}]"), GetHelper().ExecuteSelectECSql("SELECT COUNT(*) cnt FROM ts.TestClass WHERE Color BETWEEN ts.Colors.Red AND ts.Colors.Black"));
    EXPECT_EQ(JsonValue("[{\"cnt\": 2}]"), GetHelper().ExecuteSelectECSql("SELECT COUNT(*) cnt FROM ts.TestClass WHERE Domain = ts.Domains.Domains1"));
    EXPECT_EQ(JsonValue("[{\"cnt\": 2}]"), GetHelper().ExecuteSelectECSql("SELECT COUNT(*) cnt FROM ts.TestClass WHERE Domain = 1"));
    EXPECT_EQ(JsonValue("[{\"cnt\": 6}]"), GetHelper().ExecuteSelectECSql("SELECT COUNT(*) cnt FROM ts.TestClass WHERE Domain IN (ts.Domains.Domains1, ts.Domains.Domains2, ts.Domains.Domains3)"));
    EXPECT_EQ(JsonValue("[{\"cnt\": 6}]"), GetHelper().ExecuteSelectECSql("SELECT COUNT(*) cnt FROM ts.TestClass WHERE Domain IN (1, ts.Domains.Domains2, 3)"));
    EXPECT_EQ(JsonValue("[{\"cnt\": 10}]"), GetHelper().ExecuteSelectECSql("SELECT COUNT(*) cnt FROM ts.TestClass WHERE Domain BETWEEN ts.Domains.Domains1 AND ts.Domains.Domains5"));
    EXPECT_EQ(JsonValue("[{\"cnt\": 10}]"), GetHelper().ExecuteSelectECSql("SELECT COUNT(*) cnt FROM ts.TestClass WHERE Domain BETWEEN ts.Domains.Domains1 AND 5"));

    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("UPDATE ts.TestClass SET Color = ts.Colors.Red WHERE Color = ts.Colors.Yellow"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("UPDATE ts.TestClass SET Domain = ts.Domains.Domains5 WHERE Domain = ts.Domains.Domains3"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("DELETE FROM ts.TestClass WHERE Color = ts.Colors.Yellow"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("DELETE FROM ts.TestClass WHERE Domain = ts.Domains.Domains4"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, EnumeratorNames)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("EnumeratorNames.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8' ?>"
        "<ECSchema schemaName='TestSchema' displayLabel='Test Schema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>"
        "    <ECEnumeration typeName='Colors' displayLabel='Color' backingTypeName='string' isStrict='True'>"
        "        <ECEnumerator name='Red' value='Red' />"
        "        <ECEnumerator name='Blue' value='Blue' />"
        "        <ECEnumerator name='Green' value='Green' />"
        "        <ECEnumerator name='Yellow' value='Yellow' />"
        "        <ECEnumerator name='Black' value='Black' />"
        "    </ECEnumeration>"
        "    <ECEnumeration typeName='Domains' displayLabel='Domain' backingTypeName='int' isStrict='True'>"
        "        <ECEnumerator name='Com' value='1' DisplayLabel='com' />"
        "        <ECEnumerator name='Org' value='2' DisplayLabel='org' />"
        "        <ECEnumerator name='Edu' value='3' DisplayLabel='edu' />"
        "        <ECEnumerator name='Net' value='4' DisplayLabel='net' />"
        "        <ECEnumerator name='Int' value='5' DisplayLabel='int' />"
        "    </ECEnumeration>"
        "    <ECEntityClass typeName='TestClass'>"
        "        <ECProperty propertyName='Color' typeName='Colors' />"
        "        <ECProperty propertyName='Domain' typeName='Domains' />"
        "    </ECEntityClass>"
        "</ECSchema>")));

    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.TestClass(Color,Domain) VALUES (ts.Colors.Red, ts.Domains.Com)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.TestClass(Color,Domain) VALUES ('Red', 1)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.TestClass(Color,Domain) VALUES (ts.Colors.Blue, ts.Domains.Org)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.TestClass(Color,Domain) VALUES ('Blue', 2)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.TestClass(Color,Domain) VALUES (ts.Colors.Green, ts.Domains.Edu)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.TestClass(Color,Domain) VALUES ('Green', 3)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.TestClass(Color,Domain) VALUES (ts.Colors.Yellow, ts.Domains.Net)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.TestClass(Color,Domain) VALUES ('Yellow', 4)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.TestClass(Color,Domain) VALUES (ts.Colors.Black, ts.Domains.[Int])"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.TestClass(Color,Domain) VALUES ('Black', 5)"));
    ASSERT_EQ(JsonValue("[{\"cnt\": 2}]"), GetHelper().ExecuteSelectECSql("SELECT COUNT(*) cnt FROM ts.TestClass WHERE Color = ts.Colors.Red"));
    ASSERT_EQ(JsonValue("[{\"cnt\": 2}]"), GetHelper().ExecuteSelectECSql("SELECT COUNT(*) cnt FROM ts.TestClass WHERE Color = 'Red'"));
    ASSERT_EQ(JsonValue("[{\"cnt\": 4}]"), GetHelper().ExecuteSelectECSql("SELECT COUNT(*) cnt FROM ts.TestClass WHERE Color IN (ts.Colors.Red, ts.Colors.Blue)"));
    ASSERT_EQ(JsonValue("[{\"cnt\": 4}]"), GetHelper().ExecuteSelectECSql("SELECT COUNT(*) cnt FROM ts.TestClass WHERE Color IN ('Red', ts.Colors.Blue)"));
    ASSERT_EQ(JsonValue("[{\"cnt\": 0}]"), GetHelper().ExecuteSelectECSql("SELECT COUNT(*) cnt FROM ts.TestClass WHERE Color BETWEEN ts.Colors.Red AND ts.Colors.Black"));
    ASSERT_EQ(JsonValue("[{\"cnt\": 0}]"), GetHelper().ExecuteSelectECSql("SELECT COUNT(*) cnt FROM ts.TestClass WHERE Color BETWEEN 'Red' AND 'Black'"));
    ASSERT_EQ(JsonValue("[{\"cnt\": 2}]"), GetHelper().ExecuteSelectECSql("SELECT COUNT(*) cnt FROM ts.TestClass WHERE Domain = ts.Domains.Com"));
    ASSERT_EQ(JsonValue("[{\"cnt\": 2}]"), GetHelper().ExecuteSelectECSql("SELECT COUNT(*) cnt FROM ts.TestClass WHERE Domain = 1"));
    ASSERT_EQ(JsonValue("[{\"cnt\": 6}]"), GetHelper().ExecuteSelectECSql("SELECT COUNT(*) cnt FROM ts.TestClass WHERE Domain IN (ts.Domains.Com, ts.Domains.Org, ts.Domains.Edu)"));
    ASSERT_EQ(JsonValue("[{\"cnt\": 6}]"), GetHelper().ExecuteSelectECSql("SELECT COUNT(*) cnt FROM ts.TestClass WHERE Domain IN (1, ts.Domains.Org, 3)"));
    ASSERT_EQ(JsonValue("[{\"cnt\": 10}]"), GetHelper().ExecuteSelectECSql("SELECT COUNT(*) cnt FROM ts.TestClass WHERE Domain BETWEEN ts.Domains.Com AND ts.Domains.[Int]"));
    ASSERT_EQ(JsonValue("[{\"cnt\": 10}]"), GetHelper().ExecuteSelectECSql("SELECT COUNT(*) cnt FROM ts.TestClass WHERE Domain BETWEEN 1 AND ts.Domains.[Int]"));

    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("UPDATE ts.TestClass SET Color = ts.Colors.Red WHERE Color = ts.Colors.Yellow"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("UPDATE ts.TestClass SET Domain = ts.Domains.Net WHERE Domain = ts.Domains.Edu"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("DELETE FROM ts.TestClass WHERE Color = ts.Colors.Yellow"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("DELETE FROM ts.TestClass WHERE Domain = ts.Domains.Net"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, ORedEnumerators)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ORedEnumerators.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECEnumeration typeName="Status" backingTypeName="int" isStrict="false">
                    <ECEnumerator name="On" value="1" />
                    <ECEnumerator name="Off" value="2" />
                </ECEnumeration>
                <ECEnumeration typeName="Domain" backingTypeName="string" isStrict="false">
                    <ECEnumerator name="Org" value=".org" />
                    <ECEnumerator name="Com" value=".com" />
                </ECEnumeration>
                <ECEntityClass typeName="Foo" >
                    <ECProperty propertyName="Status" typeName="Status" />
                    <ECProperty propertyName="Domain" typeName="Domain" />
                </ECEntityClass>
              </ECSchema>)xml")));
    m_ecdb.SaveChanges();
    ECInstanceKey unoredKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(unoredKey, "INSERT INTO ts.Foo(Status, Domain) VALUES (ts.Status.[On], ts.Domain.Com)"));
    ECInstanceKey oredKey;
    ASSERT_EQ(BE_SQLITE_ERROR, GetHelper().ExecuteInsertECSql(oredKey, "INSERT INTO ts.Foo(Status, Domain) VALUES (ts.Status.[On] | ts.Status.Off, ts.Domain.Com | ts.Domain.Org)")) << "Bitwise OR not supported yet";

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ts.Foo WHERE Status = ts.Status.[On]"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    ASSERT_EQ(unoredKey.GetInstanceId(), stmt.GetValueId<ECInstanceId>(0)) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ts.Foo WHERE Status & ts.Status.[On] <> 0")) << "Bitwise AND not supported yet";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, TryGetContainedEnumerators)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("TryGetContainedEnumerators.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECEnumeration typeName="Color" backingTypeName="int" isStrict="false">
                    <ECEnumerator name="Red" value="1" />
                    <ECEnumerator name="Yellow" value="2" />
                    <ECEnumerator name="Blue" value="4" />
                </ECEnumeration>
                <ECEnumeration typeName="Domain" backingTypeName="string" isStrict="false">
                    <ECEnumerator name="Org" value=".org" />
                    <ECEnumerator name="Com" value=".com" />
                    <ECEnumerator name="Gov" value=".gov" />
                </ECEnumeration>
                <ECEntityClass typeName="Foo" >
                    <ECProperty propertyName="AssetColor" typeName="Color" />
                    <ECProperty propertyName="AssetDomain" typeName="Domain" />
                </ECEntityClass>
              </ECSchema>)xml")));
    ECInstanceKey unoredKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(unoredKey, "INSERT INTO ts.Foo(AssetColor, AssetDomain) VALUES (ts.Color.Red, ts.Domain.Com)"));
    ECInstanceKey oredKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(oredKey, "INSERT INTO ts.Foo(AssetColor, AssetDomain) VALUES (3, '.org;.gov')"));
    ECInstanceKey unmatchingKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(unmatchingKey, "INSERT INTO ts.Foo(AssetColor, AssetDomain) VALUES (9, '.org;.de')"));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT AssetColor,AssetDomain FROM ts.Foo WHERE ECInstanceId=?"));

    {
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, unoredKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    IECSqlValue const& colorValue = stmt.GetValue(0);
    ASSERT_TRUE(colorValue.GetColumnInfo().GetEnumType() != nullptr);
    ECEnumeratorCP e = colorValue.GetEnum();
    ASSERT_TRUE(e != nullptr);
    ASSERT_STREQ("Red", e->GetName().c_str());

    bvector<ECEnumeratorCP> flags;
    ASSERT_EQ(SUCCESS, colorValue.TryGetContainedEnumerators(flags));
    ASSERT_EQ(1, flags.size());
    ASSERT_STREQ("Red", flags[0]->GetName().c_str());

    IECSqlValue const& domainValue = stmt.GetValue(1);
    ASSERT_TRUE(domainValue.GetColumnInfo().GetEnumType() != nullptr);
    e = domainValue.GetEnum();
    ASSERT_TRUE(e != nullptr);
    ASSERT_STREQ("Com", e->GetName().c_str());

    flags.clear();
    ASSERT_EQ(SUCCESS, domainValue.TryGetContainedEnumerators(flags));
    ASSERT_EQ(1, flags.size());
    ASSERT_STREQ("Com", flags[0]->GetName().c_str());

    stmt.Reset();
    stmt.ClearBindings();
    }

    {
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, oredKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    IECSqlValue const& colorValue = stmt.GetValue(0);
    ASSERT_TRUE(colorValue.GetColumnInfo().GetEnumType() != nullptr);
    ASSERT_TRUE(colorValue.GetEnum() == nullptr);

    bvector<ECEnumeratorCP> flags;
    ASSERT_EQ(SUCCESS, colorValue.TryGetContainedEnumerators(flags));
    ASSERT_EQ(2, flags.size());
    ASSERT_STREQ("Red", flags[0]->GetName().c_str());
    ASSERT_STREQ("Yellow", flags[1]->GetName().c_str());

    IECSqlValue const& domainValue = stmt.GetValue(1);
    ASSERT_TRUE(domainValue.GetColumnInfo().GetEnumType() != nullptr);

    flags.clear();
    ASSERT_EQ(SUCCESS, domainValue.TryGetContainedEnumerators(flags));
    ASSERT_EQ(2, flags.size());
    ASSERT_STREQ("Org", flags[0]->GetName().c_str());
    ASSERT_STREQ("Gov", flags[1]->GetName().c_str());

    stmt.Reset();
    stmt.ClearBindings();
    }

    {
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, unmatchingKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    IECSqlValue const& colorValue = stmt.GetValue(0);
    ASSERT_TRUE(colorValue.GetColumnInfo().GetEnumType() != nullptr);
    ASSERT_TRUE(colorValue.GetEnum() == nullptr);

    bvector<ECEnumeratorCP> flags;
    ASSERT_EQ(ERROR, colorValue.TryGetContainedEnumerators(flags));

    IECSqlValue const& domainValue = stmt.GetValue(1);
    ASSERT_TRUE(domainValue.GetColumnInfo().GetEnumType() != nullptr);

    flags.clear();
    ASSERT_EQ(ERROR, domainValue.TryGetContainedEnumerators(flags));

    stmt.Reset();
    stmt.ClearBindings();
    }
    }
//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, ConfuseRealNumberWithClassName) {
    ASSERT_EQ(SUCCESS, SetupECDb("ECSqlStatementTests.ecdb", SchemaItem(
            R"xml(<ECSchema schemaName="NwdDynamic" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECEntityClass typeName="E000005521485VFW120710002235" >
                        <ECProperty propertyName="P" typeName="string" />
                    </ECEntityClass>
                    <ECEntityClass typeName="E0" >
                        <ECProperty propertyName="P" typeName="string" />
                    </ECEntityClass>
                 </ECSchema>)xml")));

    if ("prod test") {
        ECSqlStatement insertStmt;
        ASSERT_EQ(ECSqlStatus::Success, insertStmt.Prepare(m_ecdb, "INSERT INTO NwdDynamic.E000005521485VFW120710002235(P) VALUES(?)"));
    }

    if ("min test") {
        ECSqlStatement insertStmt;
        ASSERT_EQ(ECSqlStatus::Success, insertStmt.Prepare(m_ecdb, "INSERT INTO NwdDynamic.E0(P) VALUES(?)"));
    }

    }
END_ECDBUNITTESTS_NAMESPACE
