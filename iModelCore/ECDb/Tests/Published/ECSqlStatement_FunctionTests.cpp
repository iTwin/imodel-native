/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECSqlStatement_FunctionTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECSqlStatementTestFixture.h"

BEGIN_ECDBUNITTESTS_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/15
//+---------------+---------------+---------------+---------------+---------------+------
struct ExpectedResult
    {
    bool m_isPrepareSupported;
    bool m_isStepSupported;
    ECN::PrimitiveType m_returnType;

    ExpectedResult() : m_isPrepareSupported(false), m_isStepSupported (false) {}
    explicit ExpectedResult(ECN::PrimitiveType returnType, bool isStepSupported = true) : m_isPrepareSupported(true), m_isStepSupported(isStepSupported), m_returnType(returnType) {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, BuiltinFunctions)
    {
    ECDbR ecdb = SetupECDb("ecsqlbuiltinfunctiontest.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), 5, ECDb::OpenParams(Db::OpenMode::Readonly));

    std::vector<std::pair<Utf8CP, ExpectedResult>> testDataset {
            {"SELECT ABS(I) FROM ecsql.P LIMIT 1", ExpectedResult (ECN::PRIMITIVETYPE_Double)},
            {"SELECT ANY(B) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_Boolean)},
            {"SELECT AVG(I) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_Double)},
            {"SELECT COUNT(*) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_Long)},
            {"SELECT COUNT(ECInstanceId) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_Long)},
            {"SELECT COUNT(I) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_Long)},
            {"SELECT EVERY(B) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_Boolean)},
            {"SELECT GETECCLASSID() FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Long)},
            {"SELECT GLOB(S,'Sample') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Boolean)},
            {"SELECT GROUP_CONCAT(S) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT GROUP_CONCAT(S,'|') FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT GROUP_CONCAT(DISTINCT S) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT HEX(Bi) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT INSTR(S,'str') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Integer)},
            {"SELECT LENGTH(S) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Long)},
            {"SELECT LOWER(S) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT LIKE(S,'Sample') FROM ecsql.P LIMIT 1", ExpectedResult()},//Not supported as LIKE is a reserved token, and the ECSQL grammar only has one rule yet for LIKe which is the standard X LIKE Y syntax
            {"SELECT LIKE(S,'Sample','/') FROM ecsql.P LIMIT 1", ExpectedResult()},//Not supported as LIKE is a reserved token, and the ECSQL grammar only has one rule yet for LIKe which is the standard X LIKE Y syntax
            {"SELECT LTRIM(S) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT LTRIM(S, '$') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT MATCH(S, 'str') FROM ecsql.P LIMIT 1", ExpectedResult()},//Not supported as MATCH is a reserved token, and the ECSQL grammar only has one rule yet for MATCH which is the X MATCH function() syntax
            {"SELECT MAX(I) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_Double)},
            {"SELECT MAX(123, 125, 512) FROM ecsql.P LIMIT 1", ExpectedResult()},//Not supported as MAX(arg) is a dedicated ECSQL grammar rule
            {"SELECT MIN(I) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_Double)},
            {"SELECT MIN(123, 125, 512) FROM ecsql.P LIMIT 1", ExpectedResult()},//Not supported as MIN(arg) is a dedicated ECSQL grammar rule
            {"SELECT QUOTE(S) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT RANDOM() FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Double)},
            {"SELECT RANDOMBLOB(5) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Binary)},
            {"SELECT REGEXP(S, 'str') FROM ecsql.P LIMIT 1", ExpectedResult()},//Not supported as by default SQLite doesn't come with REGEXP impl
            {"SELECT REPLACE(S,'Sample','Simple') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT RTRIM(S) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT RTRIM(S, '$') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT SOME(B) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_Boolean)},
            {"SELECT SOUNDEX(S) FROM ecsql.P LIMIT 1", ExpectedResult()}, //Only available if SQLite is compiled with SQLITE_SOUNDEX compile option
            {"SELECT SUBSTR(S,'a',3) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT SUM(I) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_Double)},
            {"SELECT TOTAL(I) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_Double)},
            {"SELECT TRIM(S) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT TRIM(S, '$') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT UNICODE('K') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Long)},
            {"SELECT UPPER(S) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT ZEROBLOB(5) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Binary)},
            {"SELECT ZEROBLOB(5) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Binary)},
            //BeSQLite built-in functions
            {"SELECT NULL FROM ecsql.P WHERE InVirtualSet(?,123)", ExpectedResult(ECN::PRIMITIVETYPE_Boolean, false)},
        };


    for (std::pair<Utf8CP, ExpectedResult> const& kvPair : testDataset)
        {
        Utf8CP ecsql = kvPair.first;
        ExpectedResult const& expectedResult = kvPair.second;
        const ECSqlStatus expectedPrepareStat = expectedResult.m_isPrepareSupported ? ECSqlStatus::Success : ECSqlStatus::InvalidECSql;

        ECSqlStatement stmt;
        ASSERT_EQ(expectedPrepareStat, stmt.Prepare(ecdb, ecsql)) << ecsql;

        if (!expectedResult.m_isStepSupported)
            continue;

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

        ECN::ECTypeDescriptor const& actualColumnType = stmt.GetColumnInfo(0).GetDataType();

        ASSERT_TRUE(actualColumnType.IsPrimitive()) << ecsql;
        ASSERT_EQ(expectedResult.m_returnType, actualColumnType.GetPrimitiveType()) << ecsql;
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, FunctionCallWithDistinct)
    {
    ECDbR ecdb = SetupECDb("ecsqlbuiltinfunctiontest.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), 0, ECDb::OpenParams(Db::OpenMode::ReadWrite));

    auto getIntScalar = [] (ECDbCR ecdb, Utf8CP ecsql)
        {
        ECSqlStatement stmt;
        if (ECSqlStatus::Success != stmt.Prepare(ecdb, ecsql))
            return -1;

        if (BE_SQLITE_ROW != stmt.Step())
            return -1;

        return stmt.GetValueInt(0);
        };

    auto getStringScalar = [] (ECDbCR ecdb, Utf8CP ecsql)
        {
        ECSqlStatement stmt;
        if (ECSqlStatus::Success != stmt.Prepare(ecdb, ecsql))
            return Utf8String();

        if (BE_SQLITE_ROW != stmt.Step())
            return Utf8String();

        return Utf8String(stmt.GetValueText(0));
        };


    {
    //create the following data
    //I|S
    //---
    //1|'1'
    //2|'1'
    //1|'2'
    //2|'3'
    //2|'3'
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ecsql.P(I,S) VALUES(?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, 1));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(2, "1", IECSqlBinder::MakeCopy::Yes));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, 2));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(2, "1", IECSqlBinder::MakeCopy::Yes));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, 1));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(2, "2", IECSqlBinder::MakeCopy::Yes));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, 2));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(2, "3", IECSqlBinder::MakeCopy::Yes));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, 2));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(2, "3", IECSqlBinder::MakeCopy::Yes));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    ASSERT_EQ(5, getIntScalar(ecdb, "SELECT count(*) from ecsql.P"));
    ASSERT_EQ(5, getIntScalar(ecdb, "SELECT count(distinct ECInstanceId) from ecsql.P"));
    ASSERT_EQ(2, getIntScalar(ecdb, "SELECT count(distinct I) from ecsql.P"));
    ASSERT_EQ(3, getIntScalar(ecdb, "SELECT count(distinct S) from ecsql.P"));

    ASSERT_STREQ("1,1,2,3,3", getStringScalar(ecdb, "SELECT group_concat(S) from ecsql.P").c_str());
    ASSERT_STREQ("1&1&2&3&3", getStringScalar(ecdb, "SELECT group_concat(S,'&') from ecsql.P").c_str());
    ASSERT_STREQ("11233", getStringScalar(ecdb, "SELECT group_concat(S,'') from ecsql.P").c_str());
    ASSERT_STREQ("1,2,3", getStringScalar(ecdb, "SELECT group_concat(DISTINCT S) from ecsql.P").c_str());
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, InVirtualSetFunction)
    {
    const int perClassRowCount = 10;
    ECDbCR ecdb = SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), perClassRowCount);

    bvector<ECInstanceId> allIds;
    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT ECInstanceId FROM ecsql.PSA"));
    while (BE_SQLITE_ROW == statement.Step())
        {
        allIds.push_back(statement.GetValueId<ECInstanceId>(0));
        }
    ASSERT_EQ(perClassRowCount, (int) allIds.size());
    }

    ECInstanceIdSet idSet;
    ASSERT_TRUE(0 < perClassRowCount);
    idSet.insert(allIds[0]);
    ASSERT_TRUE(4 < perClassRowCount);
    idSet.insert(allIds[4]);
    ASSERT_TRUE(6 < perClassRowCount);
    idSet.insert(allIds[6]);

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT count(*) FROM ecsql.PSA WHERE I<>? AND InVirtualSet(?, ECInstanceId)"));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt(1, 0));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindVirtualSet(2, idSet));

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ((int) idSet.size(), statement.GetValueInt(0));

    statement.Reset();
    statement.ClearBindings();

    idSet.clear();
    ASSERT_TRUE(1 < perClassRowCount);
    idSet.insert(allIds[1]);
    ASSERT_TRUE(3 < perClassRowCount);
    idSet.insert(allIds[3]);

    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt(1, 0));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindVirtualSet(2, idSet));

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ((int) idSet.size(), statement.GetValueInt(0));

    //now same statement but with InVirtualSet in parentheses
    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT count(*) FROM ecsql.PSA WHERE (InVirtualSet(?, ECInstanceId))"));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindVirtualSet(1, idSet));

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step()) << "Step with binding a virtual set in parentheses";
    ASSERT_EQ((int) idSet.size(), statement.GetValueInt(0)) << "Step with binding a virtual set in parentheses";

    statement.Reset();
    statement.ClearBindings();

    idSet.clear();
    ASSERT_TRUE(1 < perClassRowCount);
    idSet.insert(allIds[1]);
    ASSERT_TRUE(3 < perClassRowCount);
    idSet.insert(allIds[3]);

    ASSERT_EQ(ECSqlStatus::Success, statement.BindVirtualSet(1, idSet));

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step()) << "Step with binding a virtual set in parentheses";
    ASSERT_EQ((int) idSet.size(), statement.GetValueInt(0)) << "Step with binding a virtual set in parentheses";
    }

END_ECDBUNITTESTS_NAMESPACE
