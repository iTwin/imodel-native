/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <Bentley/Base64Utilities.h>

BEGIN_ECDBUNITTESTS_NAMESPACE

struct ECSqlStatementFunctionTestFixture : ECDbTestFixture
    {
    //---------------------------------------------------------------------------------------
    // @bsiclass                                     Krischan.Eberle                 03/15
    //+---------------+---------------+---------------+---------------+---------------+------
    struct ExpectedResult final
        {
        bool m_isPrepareSupported = false;
        bool m_isStepSupported = false;
        ECN::PrimitiveType m_returnType;

        ExpectedResult() {}
        explicit ExpectedResult(ECN::PrimitiveType returnType, bool isStepSupported = true) : m_isPrepareSupported(true), m_isStepSupported(isStepSupported), m_returnType(returnType) {}
        };
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementFunctionTestFixture, BuiltinFunctions)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlbuiltinfunctiontest.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::Readonly)));
    ASSERT_EQ(SUCCESS, PopulateECDb(5));
    ASSERT_EQ(SUCCESS, m_ecdb.AttachChangeCache(ECDb::GetDefaultChangeCachePath(m_ecdb.GetDbFileName())));

    std::vector<std::pair<Utf8CP, ExpectedResult>> testDataset {
            {"SELECT ABS(I) FROM ecsql.P LIMIT 1", ExpectedResult (ECN::PRIMITIVETYPE_Double)},
            {"SELECT ANY(B) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_Boolean)},
            {"SELECT AVG(I) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_Double)},
            {"SELECT CHANGES() FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Long)},
            {"SELECT CHAR(123,124) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT COALESCE(I,L) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Double)},
            {"SELECT COUNT(*) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_Long)},
            {"SELECT COUNT(ECInstanceId) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_Long)},
            {"SELECT COUNT(I) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_Long)},
            {"SELECT DATE('now') FROM ecsql.P LIMIT 1", ExpectedResult()}, //Not supported as DATE is a reserved token, and the ECSQL grammar only has one rule yet for DATE which is the DATE literal exp DATE '2000-01-01'
            {"SELECT DATETIME('now') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT EVERY(B) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_Boolean)},
            {"SELECT GLOB(S,'Sample') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Boolean)},
            {"SELECT GROUP_CONCAT(S) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT GROUP_CONCAT(S,'|') FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT GROUP_CONCAT(DISTINCT S) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT HEX(Bi) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT INSTR(S,'str') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Integer)},
            {"SELECT JSON(12.3) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT JSON_ARRAY(12.3, 1.3, 3) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT JSON_ARRAY_LENGTH('[1,2,3]') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Long)},
            {"SELECT JSON_EXTRACT('{\"a\" : 12}','$.a') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT JSON_GROUP_ARRAY('[1,2,3]') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT JSON_GROUP_OBJECT('a','[1,2,3]') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT JSON_INSERT('{\"a\" : 12}','$.e',10) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT JSON_OBJECT('a',1,'b',2) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT JSON_QUOTE(123) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT JSON_REMOVE('{\"a\" : 12}','$.a') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT JSON_REPLACE('{\"a\" : 12}','$.a',10) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT JSON_SET('{\"a\" : 12}','$.a',10) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT JSON_TYPE('{\"a\" : 12}') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT JSON_VALID('{\"a\" : 12}') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Boolean)},
            {"SELECT JULIANDAY('now') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Double)},
            {"SELECT LAST_INSERT_ROWID() FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Long)},
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
            {"SELECT NULLIF(I,123) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_Double)},
            {"SELECT QUOTE(S) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT PRINTF('%d',I) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT RANDOM() FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Long)},
            {"SELECT RANDOMBLOB(5) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Binary)},
            {"SELECT REGEXP(S, 'str') FROM ecsql.P LIMIT 1", ExpectedResult()},//Not supported as by default SQLite doesn't come with REGEXP impl
            {"SELECT REPLACE(S,'Sample','Simple') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT RTRIM(S) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT RTRIM(S, '$') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT SOME(B) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_Boolean)},
            {"SELECT SOUNDEX(S) FROM ecsql.P LIMIT 1", ExpectedResult()}, //Only available if SQLite is compiled with SQLITE_SOUNDEX compile option
            {"SELECT SQLITE_COMPILEOPTION_GET(0) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT SQLITE_COMPILEOPTION_USED(0) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Boolean)},
            {"SELECT SQLITE_SOURCE_ID() FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT SQLITE_VERSION() FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT STRFTIME('%J','now') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT SUBSTR(S,'a',3) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT SUM(I) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_Double)},
            {"SELECT TIME('now') FROM ecsql.P LIMIT 1", ExpectedResult()}, //Not supported as TIME is a reserved token, and the ECSQL grammar only has one rule yet for TIME which is the TIME literal exp TIME '23:44:14'
            {"SELECT TOTAL(I) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_Double)},
            {"SELECT TOTAL_CHANGES() FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_Long)},
            {"SELECT TRIM(S) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT TRIM(S, '$') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT TYPEOF(I) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT UNICODE('K') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Long)},
            {"SELECT UPPER(S) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT ZEROBLOB(5) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Binary)},
            {"SELECT ZEROBLOB(5) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Binary)},
            //BeSQLite built-in functions
            {"SELECT 10 FROM ecsql.P WHERE NOT InVirtualSet(?,123)", ExpectedResult(ECN::PRIMITIVETYPE_Long)},
            //ECDb built-in functions 
            {"SELECT ChangedValue(1,'S','AfterInsert',S) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Binary)}, //only for ECDb internal use
            {"SELECT ChangedValueStateToOpCode('BeforeUpdate') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Long)}, //only for ECDb internal use
            {"SELECT ChangedValueStateToOpCode(2) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Long)}, //only for ECDb internal use
        };


    for (std::pair<Utf8CP, ExpectedResult> const& kvPair : testDataset)
        {
        Utf8CP ecsql = kvPair.first;
        ExpectedResult const& expectedResult = kvPair.second;
        const ECSqlStatus expectedPrepareStat = expectedResult.m_isPrepareSupported ? ECSqlStatus::Success : ECSqlStatus::InvalidECSql;

        ECSqlStatement stmt;
        ASSERT_EQ(expectedPrepareStat, stmt.Prepare(m_ecdb, ecsql)) << ecsql;

        if (!expectedResult.m_isStepSupported)
            continue;

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << ecsql;

        ECN::ECTypeDescriptor const& actualColumnType = stmt.GetColumnInfo(0).GetDataType();

        ASSERT_TRUE(actualColumnType.IsPrimitive()) << ecsql;
        EXPECT_EQ(expectedResult.m_returnType, actualColumnType.GetPrimitiveType()) << ecsql;
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 09/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementFunctionTestFixture, CoalesceAndNullIf)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("builtinfunctiontests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::ReadWrite)));

    {
    //insert two test rows
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.P(I,L) VALUES(?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, 123));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(2, 124));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());


    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT I,L,COALESCE(I,L),COALESCE(L,I) FROM ecsql.P"));
    while (BE_SQLITE_ROW == stmt.Step())
        {
        if (stmt.IsValueNull(0))
            {
            ASSERT_FALSE(stmt.IsValueNull(1)) << stmt.GetECSql();
            ASSERT_EQ(stmt.GetValueInt(1), stmt.GetValueInt(2)) << "First coalesce " << stmt.GetECSql();
            ASSERT_EQ(stmt.GetValueInt(1), stmt.GetValueInt(3)) << "Second coalesce " << stmt.GetECSql();
            }
        else
            {
            ASSERT_TRUE(stmt.IsValueNull(1)) << stmt.GetECSql();
            ASSERT_EQ(stmt.GetValueInt(0), stmt.GetValueInt(2)) << "First coalesce " << stmt.GetECSql();
            ASSERT_EQ(stmt.GetValueInt(0), stmt.GetValueInt(3)) << "Second coalesce " << stmt.GetECSql();
            }
        }

    stmt.Finalize();
    m_ecdb.AbandonChanges();
    }

    {
    //insert a test row
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.P(I,L) VALUES(123,124)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT I,L, NULLIF(I,123), NULLIF(I,124), NULLIF(L,123), NULLIF(L,124) FROM ecsql.P"));

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();


    ASSERT_TRUE(stmt.IsValueNull(2)) << "first nullif " << stmt.GetECSql();
    ASSERT_FALSE(stmt.IsValueNull(3)) << "second nullif " << stmt.GetECSql();
    ASSERT_EQ(stmt.GetValueInt(0), stmt.GetValueInt(3)) << "second nullif " << stmt.GetECSql();

    ASSERT_FALSE(stmt.IsValueNull(4)) << "third nullif " << stmt.GetECSql();
    ASSERT_EQ(stmt.GetValueInt(1), stmt.GetValueInt(4)) << "third nullif " << stmt.GetECSql();
    ASSERT_TRUE(stmt.IsValueNull(5)) << "fourth nullif " << stmt.GetECSql();
    }

    }


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementFunctionTestFixture, FunctionCallWithDistinct)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlbuiltinfunctiontest.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::ReadWrite)));

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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.P(I,S) VALUES(?,?)"));
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

    ASSERT_EQ(5, getIntScalar(m_ecdb, "SELECT count(*) from ecsql.P"));
    ASSERT_EQ(5, getIntScalar(m_ecdb, "SELECT count(distinct ECInstanceId) from ecsql.P"));
    ASSERT_EQ(2, getIntScalar(m_ecdb, "SELECT count(distinct I) from ecsql.P"));
    ASSERT_EQ(3, getIntScalar(m_ecdb, "SELECT count(distinct S) from ecsql.P"));

    ASSERT_STREQ("1,1,2,3,3", getStringScalar(m_ecdb, "SELECT group_concat(S) from ecsql.P").c_str());
    ASSERT_STREQ("1&1&2&3&3", getStringScalar(m_ecdb, "SELECT group_concat(S,'&') from ecsql.P").c_str());
    ASSERT_STREQ("11233", getStringScalar(m_ecdb, "SELECT group_concat(S,'') from ecsql.P").c_str());
    ASSERT_STREQ("1,2,3", getStringScalar(m_ecdb, "SELECT group_concat(DISTINCT S) from ecsql.P").c_str());
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementFunctionTestFixture, InVirtualSetFunction)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));
    const int perClassRowCount = 10;
    ASSERT_EQ(SUCCESS, PopulateECDb( perClassRowCount));

    bvector<ECInstanceId> allIds;
    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT ECInstanceId FROM ecsql.PSA"));
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
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT count(*) FROM ecsql.PSA WHERE I<>? AND InVirtualSet(?, ECInstanceId)"));
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

    statement.Reset();
    statement.ClearBindings();

    //now binding a NULL virtual set
    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt(1, 0));

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(0, statement.GetValueInt(0)) << statement.GetECSql() << " with NULL bound for the virtual set";

    statement.Reset();
    statement.ClearBindings();

    //now binding an empty virtual set
    idSet.clear();
    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt(1, 0));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindVirtualSet(2, idSet));

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(0, statement.GetValueInt(0)) << statement.GetECSql() << " with empty virtual set";

    //now same statement but with InVirtualSet in parentheses
    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT count(*) FROM ecsql.PSA WHERE (InVirtualSet(?, ECInstanceId))"));
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
