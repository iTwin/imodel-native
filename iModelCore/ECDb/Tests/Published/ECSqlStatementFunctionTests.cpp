/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECSqlStatementFunctionTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECSqlStatementTestFixture.h"
#include <Bentley/Base64Utilities.h>

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
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlbuiltinfunctiontest.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::Readonly)));
    ASSERT_EQ(SUCCESS, Populate(m_ecdb, 5));
    std::vector<std::pair<Utf8CP, ExpectedResult>> testDataset {
            {"SELECT ABS(I) FROM ecsql.P LIMIT 1", ExpectedResult (ECN::PRIMITIVETYPE_Double)},
            {"SELECT ANY(B) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_Boolean)},
            {"SELECT AVG(I) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_Double)},
            {"SELECT COALESCE(I,L) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Double)},
            {"SELECT COUNT(*) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_Long)},
            {"SELECT COUNT(ECInstanceId) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_Long)},
            {"SELECT COUNT(I) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_Long)},
            {"SELECT EVERY(B) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_Boolean)},
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
            {"SELECT NULLIF(I,123) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_Double)},
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
            {"SELECT 10 FROM ecsql.P WHERE NOT InVirtualSet(?,123)", ExpectedResult(ECN::PRIMITIVETYPE_Long)},
            //ECDb built-in functions
            {"SELECT Base64ToBlob('Rm9vMTIzIQ==') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Binary)},
            {"SELECT BlobToBase64(RANDOMBLOB(5)) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
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

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

        ECN::ECTypeDescriptor const& actualColumnType = stmt.GetColumnInfo(0).GetDataType();

        ASSERT_TRUE(actualColumnType.IsPrimitive()) << ecsql;
        ASSERT_EQ(expectedResult.m_returnType, actualColumnType.GetPrimitiveType()) << ecsql;
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 09/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, CoalesceAndNullIf)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("builtinfunctiontests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::ReadWrite)));

    
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
TEST_F(ECSqlStatementTestFixture, FunctionCallWithDistinct)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlbuiltinfunctiontest.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::ReadWrite)));

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
// @bsiclass                                     Krischan.Eberle                 11/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, Base64Functions)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("base64functions.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::ReadWrite)));
    ASSERT_EQ(SUCCESS, Populate(m_ecdb, 3));

    BeGuid guid(true);
    void const* expectedBlob = &guid;
    Utf8String expectedBase64Str;
    Base64Utilities::Encode(expectedBase64Str, static_cast<Byte const*> (expectedBlob), sizeof(BeGuid));

    ECInstanceKey key;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.P(Bi, S) VALUES(:blobparam,BlobToBase64(:blobparam))"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindBlob(stmt.GetParameterIndex("blobparam"), expectedBlob, sizeof(BeGuid), IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    }

    //ECSQL
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT S, Bi, BlobToBase64(Bi), Base64ToBlob(S) FROM ecsql.P WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    //select clause item 0
    ASSERT_STREQ(expectedBase64Str.c_str(), stmt.GetValueText(0)) << stmt.GetECSql();


    //select clause item 1
    int actualBlobSize = -1;
    void const* actualBlob = stmt.GetValueBlob(1, &actualBlobSize);
    ASSERT_EQ(sizeof(BeGuid), (size_t) actualBlobSize) << "Second select clause item in ECSQL " << stmt.GetECSql();
    ASSERT_EQ(0, memcmp(expectedBlob, actualBlob, sizeof(BeGuid))) << "Second select clause item in ECSQL " << stmt.GetECSql();

    //select clause item 2
    ASSERT_STREQ(expectedBase64Str.c_str(), stmt.GetValueText(2)) << stmt.GetECSql();

    //select clause item 3
    actualBlobSize = -1;
    actualBlob = stmt.GetValueBlob(3, &actualBlobSize);
    ASSERT_EQ(sizeof(BeGuid), (size_t) actualBlobSize) << "Fourth select clause item in ECSQL " << stmt.GetECSql();
    ASSERT_EQ(0, memcmp(expectedBlob, actualBlob, sizeof(BeGuid))) << "Fourth select clause item in ECSQL " << stmt.GetECSql();
    }

    {
    Utf8String ecsql;
    ecsql.Sprintf("SELECT Base64ToBlob('%s') FROM ecsql.P WHERE ECInstanceId=?", expectedBase64Str.c_str());

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    //select clause item 3
    int actualBlobSize = -1;
    ASSERT_EQ(0, memcmp(expectedBlob, stmt.GetValueBlob(0, &actualBlobSize), sizeof(BeGuid))) << stmt.GetECSql();
    ASSERT_EQ(sizeof(BeGuid), (size_t) actualBlobSize) << stmt.GetECSql();
    }

    //BeSQLite
    {
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT S, Bi, BlobToBase64(Bi), Base64ToBlob(S) FROM ecsqltest_P WHERE Id=?"));
    ASSERT_EQ(BE_SQLITE_OK, stmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    //select clause item 0
    ASSERT_STREQ(expectedBase64Str.c_str(), stmt.GetValueText(0)) << "SQL: " << stmt.GetSql();


    //select clause item 1
    ASSERT_EQ(sizeof(BeGuid), (size_t) stmt.GetColumnBytes(1)) << "SQL: " << stmt.GetSql();
    ASSERT_EQ(0, memcmp(expectedBlob, stmt.GetValueBlob(1), sizeof(BeGuid))) << "SQL: " << stmt.GetSql();

    //select clause item 2
    ASSERT_STREQ(expectedBase64Str.c_str(), stmt.GetValueText(2)) << "SQL: " << stmt.GetSql();

    //select clause item 3
    ASSERT_EQ(sizeof(BeGuid), (size_t) stmt.GetColumnBytes(3)) << "SQL: " << stmt.GetSql();
    ASSERT_EQ(0, memcmp(expectedBlob, stmt.GetValueBlob(3), sizeof(BeGuid))) << "SQL: " << stmt.GetSql();
    }

    {
    Utf8String sql;
    sql.Sprintf("SELECT Base64ToBlob('%s') FROM ecsqltest_P WHERE Id=?", expectedBase64Str.c_str());

    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, sql.c_str()));
    ASSERT_EQ(BE_SQLITE_OK, stmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    ASSERT_EQ(sizeof(BeGuid), (size_t) stmt.GetColumnBytes(0)) << "SQL: " << stmt.GetSql();
    ASSERT_EQ(0, memcmp(expectedBlob, stmt.GetValueBlob(0), sizeof(BeGuid))) << "SQL: " << stmt.GetSql();
    }


    //NULL input
    {
    ECSqlStatement ecsqlStmt;
    ASSERT_EQ(ECSqlStatus::Success, ecsqlStmt.Prepare(m_ecdb, "SELECT BlobToBase64(NULL), BlobToBase64(?), Base64ToBlob(NULL), Base64ToBlob(?) FROM ecsql.P LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, ecsqlStmt.Step());
    ASSERT_TRUE(ecsqlStmt.IsValueNull(0)) << ecsqlStmt.GetECSql();
    ASSERT_TRUE(ecsqlStmt.IsValueNull(1)) << ecsqlStmt.GetECSql();
    ASSERT_TRUE(ecsqlStmt.IsValueNull(2)) << ecsqlStmt.GetECSql();
    ASSERT_TRUE(ecsqlStmt.IsValueNull(3)) << ecsqlStmt.GetECSql();

    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT BlobToBase64(NULL), BlobToBase64(?), Base64ToBlob(NULL), Base64ToBlob(?) FROM ecsqltest_P LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_TRUE(stmt.IsColumnNull(0)) << stmt.GetSql();
    ASSERT_TRUE(stmt.IsColumnNull(1)) << stmt.GetSql();
    ASSERT_TRUE(stmt.IsColumnNull(2)) << stmt.GetSql();
    ASSERT_TRUE(stmt.IsColumnNull(3)) << stmt.GetSql();
    }

    //invalid input
    {
    ECSqlStatement ecsqlStmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, ecsqlStmt.Prepare(m_ecdb, "SELECT BlobToBase64() FROM ecsql.P"));
    ecsqlStmt.Finalize();
    ASSERT_EQ(ECSqlStatus::InvalidECSql, ecsqlStmt.Prepare(m_ecdb, "SELECT Base64ToBlob() FROM ecsql.P"));
    ecsqlStmt.Finalize();
    ASSERT_EQ(ECSqlStatus::InvalidECSql, ecsqlStmt.Prepare(m_ecdb, "SELECT BlobToBase64(randomblob(10), 1000) FROM ecsql.P"));
    ecsqlStmt.Finalize();
    ASSERT_EQ(ECSqlStatus::InvalidECSql, ecsqlStmt.Prepare(m_ecdb, "SELECT Base64ToBlob('ddd',12312) FROM ecsql.P"));
    ecsqlStmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, ecsqlStmt.Prepare(m_ecdb, "SELECT Base64ToBlob('a_d') FROM ecsql.P LIMIT 1")) << "invalid base64 string";
    ASSERT_EQ(BE_SQLITE_ERROR, ecsqlStmt.Step()) << "invalid base64 string in ECSQL " << ecsqlStmt.GetECSql();
    ecsqlStmt.Finalize();
    BeTest::SetFailOnAssert(false);
    {
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_ERROR, stmt.Prepare(m_ecdb, "SELECT BlobToBase64() FROM ecsqltest_P"));
    stmt.Finalize();
    ASSERT_EQ(BE_SQLITE_ERROR, stmt.Prepare(m_ecdb, "SELECT Base64ToBlob() FROM ecsqltest_P"));
    stmt.Finalize();
    ASSERT_EQ(BE_SQLITE_ERROR, stmt.Prepare(m_ecdb, "SELECT BlobToBase64(randomblob(10), 133) FROM ecsqltest_P"));
    stmt.Finalize();
    ASSERT_EQ(BE_SQLITE_ERROR, stmt.Prepare(m_ecdb, "SELECT Base64ToBlob('ddd',12312) FROM ecsqltest_P"));
    stmt.Finalize();

    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT Base64ToBlob('a_d') FROM ecsqltest_P LIMIT 1")) << "invalid base64 string";
    ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step()) << "invalid base64 string in ECSQL " << stmt.GetSql();
    stmt.Finalize();

    }
    BeTest::SetFailOnAssert(true);
    }

    }
//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementTestFixture, InVirtualSetFunction)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml")));
    const int perClassRowCount = 10;
    ASSERT_EQ(SUCCESS, Populate(m_ecdb, perClassRowCount));

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
