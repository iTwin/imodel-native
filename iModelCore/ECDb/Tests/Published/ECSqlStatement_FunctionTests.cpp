/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECSqlStatement_FunctionTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECSqlTestFixture.h"

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
    ECDbR ecdb = SetupECDb("ecsqlbuiltinfunctiontest.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::Readonly), 5);

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
END_ECDBUNITTESTS_NAMESPACE
