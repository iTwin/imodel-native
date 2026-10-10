/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#pragma push_macro("LOG")
#undef LOG
#include "../../ECDb/ECSqlRowRenderer.h"
#include "../../ECDb/ConcurrentQueryManagerImpl.h"
#pragma pop_macro("LOG")

USING_NAMESPACE_BENTLEY_EC
#include <ECDb/ConcurrentQueryManager.h>
#include "../../../BeSQLite/SQLite/sqlite3.h"
#include <future>
#include <chrono>
#include <queue>
#include <thread>
#include <memory>
#include <mutex>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <random>
BEGIN_ECDBUNITTESTS_NAMESPACE
using namespace std::chrono_literals;

struct ConcurrentQueryFixture : ECDbTestFixture {
    void SetUp() override {
         // ConsoleLogger::SetSeverity("ECDb.ConcurrentQuery", BentleyApi::NativeLogging::LOG_TRACE);
        ECDbTestFixture::SetUp();
        ConcurrentQueryMgr::Config::Reset(std::nullopt);
    }
    BentleyStatus SetupRenderingPayload(Utf8CP fileName, std::string const& text, double number) {
        if (SUCCESS != SetupECDb(fileName, SchemaItem(
            R"xml(<ECSchema schemaName="RenderPayload" alias="rp" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECEntityClass typeName="Payload">
                    <ECProperty propertyName="TextValue" typeName="string"/>
                    <ECProperty propertyName="DoubleValue" typeName="double"/>
                </ECEntityClass>
            </ECSchema>)xml")))
            return ERROR;
        ECSqlStatement insert;
        if (ECSqlStatus::Success != insert.Prepare(m_ecdb, "INSERT INTO rp.Payload(TextValue,DoubleValue) VALUES(?,?)") ||
            ECSqlStatus::Success != insert.BindText(1, text.c_str(), IECSqlBinder::MakeCopy::Yes) ||
            ECSqlStatus::Success != insert.BindDouble(2, number) ||
            BE_SQLITE_DONE != insert.Step() ||
            BE_SQLITE_OK != m_ecdb.SaveChanges())
            return ERROR;
        return SUCCESS;
    }
};
TEST_F(ConcurrentQueryFixture, HexIdRenderingMatchesBeId) {
    auto check = [](uint64_t rawId) {
        BeInt64Id id(rawId);
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        ASSERT_TRUE(ECSqlLongRenderer::RenderHexId(writer, id));
        EXPECT_EQ(std::string("\"") + id.ToHexStr().c_str() + "\"", buffer.GetString());
    };
    for (uint64_t id : {UINT64_C(0), UINT64_C(1), UINT64_C(15), UINT64_C(16),
            UINT64_C(255), UINT64_C(256), UINT64_C(0x8000000000000000), UINT64_MAX})
        check(id);
    std::mt19937_64 random(42);
    for (int i = 0; i < 10000; ++i)
        check(random());
}

TEST_F(ConcurrentQueryFixture, PointCoordinatesRefreshAcrossStepAndReset) {
    ASSERT_EQ(SUCCESS, SetupECDb("PointCoordinatesRefreshAcrossStepAndReset.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName="PointCache" alias="pc" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="PointRow">
                <ECProperty propertyName="Ordinal" typeName="int"/>
                <ECProperty propertyName="Pair" typeName="Point2d"/>
                <ECProperty propertyName="Triple" typeName="Point3d"/>
            </ECEntityClass>
        </ECSchema>)xml")));
    ECSqlStatement insert;
    ASSERT_EQ(ECSqlStatus::Success, insert.Prepare(m_ecdb, "INSERT INTO pc.PointRow(Ordinal,Pair,Triple) VALUES(?,?,?)"));
    for (int i = 0; i < 5; ++i) {
        ASSERT_EQ(ECSqlStatus::Success, insert.BindInt(1, i));
        if (i != 2) {
            double x = i == 3 ? std::numeric_limits<double>::infinity() :
                i == 4 ? std::numeric_limits<double>::quiet_NaN() : i + 0.5;
            ASSERT_EQ(ECSqlStatus::Success, insert.BindPoint2d(2, DPoint2d::From(x, -2.0 - i)));
            ASSERT_EQ(ECSqlStatus::Success, insert.BindPoint3d(3, DPoint3d::From(x, -2.0 - i, 3.0 + i)));
        }
        ASSERT_EQ(BE_SQLITE_DONE, insert.Step());
        ASSERT_EQ(ECSqlStatus::Success, insert.Reset());
        ASSERT_EQ(ECSqlStatus::Success, insert.ClearBindings());
    }
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT Pair,Triple FROM pc.PointRow ORDER BY Ordinal"));
    for (bool readBeforeNullCheck : {false, true}) {
        for (int i = 0; i < 5; ++i) {
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
            auto const& pair = stmt.GetValue(0);
            auto const& triple = stmt.GetValue(1);
            if (readBeforeNullCheck) {
                pair.GetPoint2d();
                triple.GetPoint3d();
            }
            EXPECT_EQ(i >= 2, pair.IsNull());
            EXPECT_EQ(i >= 2, triple.IsNull());
            if (i < 2) {
                EXPECT_DOUBLE_EQ(i + 0.5, pair.GetPoint2d().x);
                EXPECT_DOUBLE_EQ(-2.0 - i, pair.GetPoint2d().y);
                EXPECT_DOUBLE_EQ(i + 0.5, triple.GetPoint3d().x);
                EXPECT_DOUBLE_EQ(-2.0 - i, triple.GetPoint3d().y);
                EXPECT_DOUBLE_EQ(3.0 + i, triple.GetPoint3d().z);
            }
        }
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
        ASSERT_EQ(ECSqlStatus::Success, stmt.Reset());
    }
}

TEST_F(ConcurrentQueryFixture, JsonSerializationMatchesRowAdaptor) {
    const std::string largeText = std::string(16384, 'x') + "\"\\\n\t" + "\xc3\xa9";
    const double number = 9.9999999999999995e-21;
    ASSERT_EQ(SUCCESS, SetupRenderingPayload("JsonSerializationMatchesRowAdaptor.ecdb", largeText, number));
    Utf8CP sql =
        "WITH sequence(n) AS (SELECT 1 UNION ALL SELECT n+1 FROM sequence WHERE n<4) "
        "SELECT n, CASE WHEN n%2=0 THEN p.TextValue ELSE 'small' END, NULL, p.DoubleValue, NULL "
        "FROM sequence CROSS JOIN rp.Payload p WHERE p.TextValue=? AND p.DoubleValue=? ORDER BY n";
    ECSqlParams args;
    args.BindString(1, largeText);
    args.BindDouble(2, number);
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, sql));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, largeText.c_str(), IECSqlBinder::MakeCopy::Yes));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(2, number));
    ECSqlRowAdaptor adaptor(m_ecdb);
    std::string expected = "[";
    DbResult rc;
    while ((rc = stmt.Step()) == BE_SQLITE_ROW) {
        BeJsDocument row;
        ASSERT_EQ(SUCCESS, adaptor.RenderRowAsArray(row, ECSqlStatementRow(stmt)));
        if (expected.size() > 1)
            expected.push_back(',');
        expected.append(row.Stringify());
    }
    ASSERT_EQ(BE_SQLITE_DONE, rc);
    expected.push_back(']');
    stmt.Finalize();

    for (uint32_t threadCount : {2u, 4u, 8u}) {
        auto config = ConcurrentQueryMgr::Config::GetDefault();
        config.SetWorkerThreadCount(threadCount);
        ConcurrentQueryMgr::Config::Reset(config);
        ConcurrentQueryMgr mgr(m_ecdb);
        auto check = [&](Utf8CP query, ECSqlParams params, std::string const& expectedJson, uint32_t rowCount) {
            auto response = mgr.Enqueue(ECSqlRequest::MakeRequest(query, params)).Get();
            ASSERT_EQ(QueryResponse::Status::Done, response->GetStatus()) << response->GetError();
            auto const& page = response->GetAsConst<ECSqlResponse>();
            EXPECT_EQ(rowCount, page.GetRowCount());
            EXPECT_EQ(expectedJson, page.asJsonString());
        };
        check(sql, args, expected, 4);
        check("SELECT 1 FROM meta.ECClassDef WHERE 1=0", ECSqlParams(), "[]", 0);
        check("SELECT NULL", ECSqlParams(), "[[]]", 1);
        check("SELECT NULL, 1, NULL", ECSqlParams(), "[[null,1.0]]", 1);
        check("SELECT NULL, CAST(1 AS INT), NULL", ECSqlParams(), "[[null,1]]", 1);
    }
}

TEST_F(ConcurrentQueryFixture, ScalarRenderingMatchesAcrossPagesAndOptions) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("ScalarRenderingMatchesAcrossPagesAndOptions.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.EnableWalMode(true));
    auto config = ConcurrentQueryMgr::Config::GetDefault();
    config.SetWorkerThreadCount(1).SetQuota(QueryQuota(60s, 128));
    ConcurrentQueryMgr::Config::Reset(config);
    ConcurrentQueryMgr mgr(m_ecdb);
    std::vector<std::string> queries {
        "SELECT ECInstanceId, ECClassId, Schema.Id, Name, NULL FROM meta.ECClassDef ORDER BY ECInstanceId",
        "SELECT ECClassId AS AliasedClass, Schema.RelECClassId, NULL, Name, NULL FROM meta.ECClassDef ORDER BY ECInstanceId",
        "SELECT ECClassId, Schema, NULL, Name, NULL FROM meta.ECClassDef ORDER BY ECInstanceId",
    };
    for (auto const& query : queries) {
        for (bool primary : {false, true}) {
            for (auto format : {ECSqlRequest::ECSqlValueFormat::ECSqlNames, ECSqlRequest::ECSqlValueFormat::JsNames}) {
                for (bool convert : {true, false, true}) {
                    ECSqlStatement stmt;
                    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query.c_str()));
                    JsReadOptions options;
                    options.SetAbbreviateBlobs(false).SetUseJsNames(format == ECSqlRequest::ECSqlValueFormat::JsNames)
                        .SetConvertClassIdsToClassNames(convert).SetDoNotConvertClassIdsToClassNamesWhenAliased(true);
                    ECSqlRowAdaptor adaptor(m_ecdb, options);
                    PreparedECSqlRowRenderer renderer;
                    renderer.BeginPage(stmt, adaptor);
                    EXPECT_TRUE(renderer.CanRender(adaptor));
                    std::string expected = "[";
                    uint32_t expectedRows = 0;
                    DbResult rc;
                    while ((rc = stmt.Step()) == BE_SQLITE_ROW) {
                        BeJsDocument row;
                        ASSERT_EQ(SUCCESS, adaptor.RenderRowAsArray(row, ECSqlStatementRow(stmt)));
                        if (expectedRows++ > 0)
                            expected.push_back(',');
                        expected.append(row.Stringify());
                    }
                    ASSERT_EQ(BE_SQLITE_DONE, rc);
                    expected.push_back(']');
                    stmt.Finalize();

                    std::string actual = "[";
                    uint32_t rows = 0, pages = 0, resumed = 0;
                    for (;;) {
                        auto req = ECSqlRequest::MakeRequest(query);
                        req->SetUsePrimaryConnection(primary);
                        req->SetValueFmt(format).SetConvertClassIdsToClassNames(convert)
                            .SetCursorId("scalar-rendering").SetLimit(QueryLimit(-1, rows));
                        auto response = mgr.Enqueue(std::move(req)).Get();
                        ASSERT_TRUE(response->IsSuccess()) << response->GetError();
                        auto const& page = response->GetAsConst<ECSqlResponse>();
                        auto const& json = page.asJsonString();
                        ASSERT_GE(json.size(), 2u);
                        if (page.GetRowCount() > 0) {
                            if (rows > 0)
                                actual.push_back(',');
                            actual.append(json, 1, json.size() - 2);
                        }
                        rows += page.GetRowCount();
                        ++pages;
                        resumed += response->GetStats().Resumed() ? 1 : 0;
                        if (response->IsDone())
                            break;
                        ASSERT_GT(page.GetRowCount(), 0u);
                        ASSERT_LT(pages, expectedRows + 2);
                    }
                    actual.push_back(']');
                    EXPECT_EQ(expectedRows, rows);
                    EXPECT_EQ(expected, actual) << query;
                    if (primary)
                        EXPECT_EQ(0u, resumed);
                    else {
                        EXPECT_GT(pages, 1u);
                        EXPECT_EQ(pages - 1, resumed);
                    }
                }
            }
        }
    }
}

TEST_F(ConcurrentQueryFixture, ScalarAndDynamicRenderingPreserveValues) {
    ASSERT_EQ(SUCCESS, SetupECDb("ScalarAndDynamicRenderingPreserveValues.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName="RenderTest" alias="rt" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="Base"/>
            <ECEntityClass typeName="TextRow"><BaseClass>Base</BaseClass>
                <ECProperty propertyName="Value" typeName="string"/>
            </ECEntityClass>
            <ECEntityClass typeName="NumberRow"><BaseClass>Base</BaseClass>
                <ECProperty propertyName="Value" typeName="int"/>
            </ECEntityClass>
            <ECEntityClass typeName="ScalarRow">
                <ECProperty propertyName="Text" typeName="string"/>
                <ECProperty propertyName="LongNumber" typeName="long"/>
                <ECProperty propertyName="IntegerNumber" typeName="int"/>
                <ECProperty propertyName="DoubleNumber" typeName="double"/>
                <ECProperty propertyName="Flag" typeName="boolean"/>
            </ECEntityClass>
        </ECSchema>)xml")));
    const std::string text = std::string(8192, 'x') + "\"\\\n\t" + "\xc3\xa9";
    ECSqlStatement insert;
    ASSERT_EQ(ECSqlStatus::Success, insert.Prepare(m_ecdb, "INSERT INTO rt.TextRow([Value]) VALUES(?)"));
    ASSERT_EQ(ECSqlStatus::Success, insert.BindText(1, text.c_str(), IECSqlBinder::MakeCopy::Yes));
    ASSERT_EQ(BE_SQLITE_DONE, insert.Step());
    insert.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, insert.Prepare(m_ecdb, "INSERT INTO rt.NumberRow([Value]) VALUES(-2147483648)"));
    ASSERT_EQ(BE_SQLITE_DONE, insert.Step());
    insert.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, insert.Prepare(m_ecdb,
        "INSERT INTO rt.ScalarRow(Text,LongNumber,IntegerNumber,DoubleNumber,Flag) VALUES(?,?,?,?,?)"));
    for (double number : {-0.0, 9.9999999999999995e-21, std::numeric_limits<double>::max(),
                          std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()}) {
        ASSERT_EQ(ECSqlStatus::Success, insert.BindText(1, text.c_str(), IECSqlBinder::MakeCopy::Yes));
        ASSERT_EQ(ECSqlStatus::Success, insert.BindInt64(2, std::numeric_limits<int64_t>::max()));
        ASSERT_EQ(ECSqlStatus::Success, insert.BindInt(3, std::numeric_limits<int32_t>::min()));
        ASSERT_EQ(ECSqlStatus::Success, insert.BindDouble(4, number));
        ASSERT_EQ(ECSqlStatus::Success, insert.BindBoolean(5, true));
        ASSERT_EQ(BE_SQLITE_DONE, insert.Step());
        ASSERT_EQ(ECSqlStatus::Success, insert.Reset());
        ASSERT_EQ(ECSqlStatus::Success, insert.ClearBindings());
    }
    insert.Finalize();
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    auto config = ConcurrentQueryMgr::Config::GetDefault();
    config.SetWorkerThreadCount(2).SetQuota(QueryQuota(60s, 1024 * 1024));
    ConcurrentQueryMgr::Config::Reset(config);
    ConcurrentQueryMgr mgr(m_ecdb);
    for (Utf8CP query : {
        "SELECT NULL,Text,LongNumber,IntegerNumber,DoubleNumber,Flag,NULL FROM rt.ScalarRow ORDER BY ECInstanceId",
        "SELECT ECInstanceId,$->[Value],NULL FROM rt.Base ORDER BY ECInstanceId",
        "SELECT ECClassId,Schema,NULL FROM meta.ECClassDef ORDER BY ECInstanceId LIMIT 10",
    }) {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        ECSqlRowAdaptor adaptor(m_ecdb);
        std::string expected = "[";
        uint32_t rows = 0;
        DbResult rc;
        while ((rc = stmt.Step()) == BE_SQLITE_ROW) {
            BeJsDocument row;
            ASSERT_EQ(SUCCESS, adaptor.RenderRowAsArray(row, ECSqlStatementRow(stmt)));
            if (rows++ > 0)
                expected.push_back(',');
            expected.append(row.Stringify());
        }
        ASSERT_EQ(BE_SQLITE_DONE, rc);
        expected.push_back(']');
        stmt.Finalize();
        for (bool primary : {false, true}) {
            auto req = ECSqlRequest::MakeRequest(query);
            req->SetUsePrimaryConnection(primary);
            auto response = mgr.Enqueue(std::move(req)).Get();
            ASSERT_EQ(QueryResponse::Status::Done, response->GetStatus()) << response->GetError();
            auto const& page = response->GetAsConst<ECSqlResponse>();
            EXPECT_EQ(rows, page.GetRowCount());
            EXPECT_EQ(expected, page.asJsonString()) << query;
        }
    }
}

TEST_F(ConcurrentQueryFixture, CompositeRenderingMatchesRowAdaptor) {
    ASSERT_EQ(SUCCESS, SetupECDb("CompositeRenderingMatchesRowAdaptor.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName="CompositeRender" alias="cr" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECStructClass typeName="BaseLeaf">
                <ECProperty propertyName="Inherited" typeName="int" readOnly="true"/>
            </ECStructClass>
            <ECStructClass typeName="Leaf"><BaseClass>BaseLeaf</BaseClass>
                <ECProperty propertyName="Zeta" typeName="string"/>
                <ECProperty propertyName="Pair" typeName="Point2d"/>
            </ECStructClass>
            <ECStructClass typeName="Container">
                <ECStructProperty propertyName="Nested" typeName="Leaf"/>
                <ECArrayProperty propertyName="Numbers" typeName="long" extendedTypeName="Id"/>
                <ECStructArrayProperty propertyName="Items" typeName="Leaf"/>
            </ECStructClass>
            <ECStructClass typeName="Opaque">
                <ECProperty propertyName="Blob" typeName="binary"/>
            </ECStructClass>
            <ECEntityClass typeName="CompositeRow">
                <ECProperty propertyName="Description" typeName="string"/>
                <ECProperty propertyName="Point2" typeName="Point2d"/>
                <ECProperty propertyName="Point3" typeName="Point3d"/>
                <ECStructProperty propertyName="Payload" typeName="Container"/>
                <ECStructArrayProperty propertyName="Leaves" typeName="Leaf"/>
                <ECArrayProperty propertyName="Integers" typeName="int"/>
                <ECArrayProperty propertyName="Longs" typeName="long" extendedTypeName="Id"/>
                <ECArrayProperty propertyName="Doubles" typeName="double"/>
                <ECArrayProperty propertyName="Strings" typeName="string"/>
                <ECArrayProperty propertyName="Flags" typeName="boolean"/>
                <ECArrayProperty propertyName="Points2" typeName="Point2d"/>
                <ECArrayProperty propertyName="Points3" typeName="Point3d"/>
                <ECStructProperty propertyName="Fallback" typeName="Opaque"/>
            </ECEntityClass>
        </ECSchema>)xml")));
    ECSqlStatement insert;
    ASSERT_EQ(ECSqlStatus::Success, insert.Prepare(m_ecdb,
        "INSERT INTO cr.CompositeRow(Description,Point2,Point3,Payload,Leaves,Integers,Longs,Doubles,Strings,Flags,Points2,Points3,Fallback) "
        "VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?)"));
    const auto point2 = DPoint2d::From(-0.0, 9.9999999999999995e-21);
    const auto point3 = DPoint3d::From(1.5, -2.0, 3.0);
    const std::string text = std::string(8192, 'x') + "\"\\\n\t" + "\xc3\xa9";
    ASSERT_EQ(ECSqlStatus::Success, insert.BindText(1, "full", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, insert.BindPoint2d(2, point2));
    ASSERT_EQ(ECSqlStatus::Success, insert.BindPoint3d(3, point3));
    auto& payload = insert.GetBinder(4);
    ASSERT_EQ(ECSqlStatus::Success, payload["Nested"]["Zeta"].BindText(text.c_str(), IECSqlBinder::MakeCopy::Yes));
    ASSERT_EQ(ECSqlStatus::Success, payload["Nested"]["Inherited"].BindInt(7));
    ASSERT_EQ(ECSqlStatus::Success, payload["Nested"]["Pair"].BindPoint2d(point2));
    ASSERT_EQ(ECSqlStatus::Success, payload["Numbers"].AddArrayElement().BindNull());
    ASSERT_EQ(ECSqlStatus::Success, payload["Numbers"].AddArrayElement().BindInt64(std::numeric_limits<int64_t>::max()));
    ASSERT_EQ(ECSqlStatus::Success, payload["Items"].AddArrayElement().BindNull());
    auto& nestedItem = payload["Items"].AddArrayElement();
    ASSERT_EQ(ECSqlStatus::Success, nestedItem["Inherited"].BindInt(9));
    ASSERT_EQ(ECSqlStatus::Success, nestedItem["Zeta"].BindText("nested", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, insert.GetBinder(5).AddArrayElement().BindNull());
    ASSERT_EQ(ECSqlStatus::Success, insert.GetBinder(5).AddArrayElement()["Zeta"].BindText("array", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, insert.GetBinder(6).AddArrayElement().BindInt(1));
    ASSERT_EQ(ECSqlStatus::Success, insert.GetBinder(6).AddArrayElement().BindNull());
    ASSERT_EQ(ECSqlStatus::Success, insert.GetBinder(6).AddArrayElement().BindInt(2));
    ASSERT_EQ(ECSqlStatus::Success, insert.GetBinder(6).AddArrayElement().BindNull());
    ASSERT_EQ(ECSqlStatus::Success, insert.GetBinder(7).AddArrayElement().BindInt64(std::numeric_limits<int64_t>::max()));
    ASSERT_EQ(ECSqlStatus::Success, insert.GetBinder(7).AddArrayElement().BindNull());
    ASSERT_EQ(ECSqlStatus::Success, insert.GetBinder(8).AddArrayElement().BindDouble(-0.0));
    ASSERT_EQ(ECSqlStatus::Success, insert.GetBinder(8).AddArrayElement().BindDouble(9.9999999999999995e-21));
    ASSERT_EQ(ECSqlStatus::Success, insert.GetBinder(9).AddArrayElement().BindText(text.c_str(), IECSqlBinder::MakeCopy::Yes));
    ASSERT_EQ(ECSqlStatus::Success, insert.GetBinder(9).AddArrayElement().BindNull());
    ASSERT_EQ(ECSqlStatus::Success, insert.GetBinder(10).AddArrayElement().BindBoolean(true));
    ASSERT_EQ(ECSqlStatus::Success, insert.GetBinder(10).AddArrayElement().BindBoolean(false));
    ASSERT_EQ(ECSqlStatus::Success, insert.GetBinder(11).AddArrayElement().BindPoint2d(point2));
    ASSERT_EQ(ECSqlStatus::Success, insert.GetBinder(11).AddArrayElement().BindNull());
    ASSERT_EQ(ECSqlStatus::Success, insert.GetBinder(12).AddArrayElement().BindPoint3d(point3));
    const Byte blob[] = {0, 1, 255};
    ASSERT_EQ(ECSqlStatus::Success, insert.GetBinder(13)["Blob"].BindBlob(blob, sizeof(blob), IECSqlBinder::MakeCopy::Yes));
    ASSERT_EQ(BE_SQLITE_DONE, insert.Step());
    insert.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, insert.Prepare(m_ecdb, "INSERT INTO cr.CompositeRow(Description) VALUES('empty')"));
    ASSERT_EQ(BE_SQLITE_DONE, insert.Step());
    insert.Finalize();
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.ExecuteSql(
        "UPDATE cr_CompositeRow SET Leaves='[]',Integers='[]',Longs='[]',Doubles='[]',Strings='[]',Flags='[]',Points2='[]',Points3='[]' "
        "WHERE Description='empty'"));
    ASSERT_EQ(ECSqlStatus::Success, insert.Prepare(m_ecdb, "INSERT INTO cr.CompositeRow(Description) VALUES(NULL)"));
    ASSERT_EQ(BE_SQLITE_DONE, insert.Step());
    insert.Finalize();
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.EnableWalMode(true));
    auto config = ConcurrentQueryMgr::Config::GetDefault();
    config.SetWorkerThreadCount(1).SetQuota(QueryQuota(60s, 128));
    ConcurrentQueryMgr::Config::Reset(config);
    ConcurrentQueryMgr mgr(m_ecdb);
    std::vector<std::pair<std::string, bool>> queries {
        {"SELECT NULL,Description,Point2,Point3,Payload,Leaves,Integers,Longs,Doubles,Strings,Flags,Points2,Points3,NULL "
         "FROM cr.CompositeRow ORDER BY ECInstanceId", true},
        {"SELECT Payload.Nested AS AliasedStruct,Payload.Items,Payload.Numbers,NULL FROM cr.CompositeRow ORDER BY ECInstanceId", true},
        {"SELECT ECClassId,Schema,NULL FROM meta.ECClassDef ORDER BY ECInstanceId LIMIT 10", true},
        {"SELECT ECInstanceId,Fallback,NULL FROM cr.CompositeRow ORDER BY ECInstanceId", false},
        {"SELECT ECInstanceId,$->Payload,NULL FROM cr.CompositeRow ORDER BY ECInstanceId", false},
    };
    for (auto const& query : queries) {
        for (auto format : {ECSqlRequest::ECSqlValueFormat::ECSqlNames, ECSqlRequest::ECSqlValueFormat::JsNames}) {
            for (bool convert : {true, false, true}) {
                ECSqlStatement stmt;
                ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query.first.c_str()));
                JsReadOptions options;
                options.SetAbbreviateBlobs(false).SetUseJsNames(format == ECSqlRequest::ECSqlValueFormat::JsNames)
                    .SetConvertClassIdsToClassNames(convert).SetDoNotConvertClassIdsToClassNamesWhenAliased(true);
                ECSqlRowAdaptor adaptor(m_ecdb, options);
                PreparedECSqlRowRenderer renderer;
                renderer.BeginPage(stmt, adaptor);
                EXPECT_EQ(query.second, renderer.CanRender(adaptor)) << query.first;
                std::string expected = "[";
                uint32_t expectedRows = 0;
                DbResult rc;
                while ((rc = stmt.Step()) == BE_SQLITE_ROW) {
                    BeJsDocument row;
                    ASSERT_EQ(SUCCESS, adaptor.RenderRowAsArray(row, ECSqlStatementRow(stmt)));
                    if (query.second) {
                        rapidjson::StringBuffer buffer;
                        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                        ASSERT_TRUE(renderer.WriteRow(writer, stmt, adaptor));
                        EXPECT_EQ(row.Stringify(), std::string(buffer.GetString(), buffer.GetSize()));
                    }
                    if (expectedRows++ > 0)
                        expected.push_back(',');
                    expected.append(row.Stringify());
                }
                ASSERT_EQ(BE_SQLITE_DONE, rc);
                expected.push_back(']');
                stmt.Finalize();
                for (bool primary : {false, true}) {
                    std::string actual = "[";
                    uint32_t rows = 0, pages = 0;
                    for (;;) {
                        auto req = ECSqlRequest::MakeRequest(query.first);
                        req->SetUsePrimaryConnection(primary);
                        req->SetValueFmt(format).SetConvertClassIdsToClassNames(convert).SetAbbreviateBlobs(false)
                            .SetCursorId("composite-rendering").SetLimit(QueryLimit(-1, rows));
                        auto response = mgr.Enqueue(std::move(req)).Get();
                        ASSERT_TRUE(response->IsSuccess()) << response->GetError();
                        auto const& page = response->GetAsConst<ECSqlResponse>();
                        auto const& json = page.asJsonString();
                        ASSERT_GE(json.size(), 2u);
                        if (page.GetRowCount() > 0) {
                            if (rows > 0)
                                actual.push_back(',');
                            actual.append(json, 1, json.size() - 2);
                        }
                        rows += page.GetRowCount();
                        ++pages;
                        if (response->IsDone())
                            break;
                        ASSERT_GT(page.GetRowCount(), 0u);
                        ASSERT_LT(pages, expectedRows + 2);
                    }
                    actual.push_back(']');
                    EXPECT_EQ(expectedRows, rows);
                    EXPECT_EQ(expected, actual) << query.first;
                }
            }
        }
    }
    for (Utf8CP query : {
        "SELECT Payload.Nested.Inherited,Payload,Leaves FROM cr.CompositeRow ORDER BY ECInstanceId",
        "SELECT ECClassId,Schema FROM meta.ECClassDef ORDER BY ECInstanceId LIMIT 10"}) {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, query));
        ECSqlRowAdaptor adaptor(m_ecdb);
        PreparedECSqlRowRenderer renderer;
        for (bool jsNames : {false, true}) {
            for (bool fullName : {false, true, false}) {
                for (bool skipReadOnly : {false, true}) {
                    adaptor.GetOptions().SetUseJsNames(jsNames).SetUseClassFullNameInsteadofClassName(fullName)
                        .SetSkipReadOnlyProperties(skipReadOnly);
                    renderer.BeginPage(stmt, adaptor);
                    ASSERT_TRUE(renderer.CanRender(adaptor));
                    DbResult rc;
                    while ((rc = stmt.Step()) == BE_SQLITE_ROW) {
                        BeJsDocument expected;
                        ASSERT_EQ(SUCCESS, adaptor.RenderRowAsArray(expected, ECSqlStatementRow(stmt)));
                        rapidjson::StringBuffer buffer;
                        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                        ASSERT_TRUE(renderer.WriteRow(writer, stmt, adaptor));
                        EXPECT_EQ(expected.Stringify(), std::string(buffer.GetString(), buffer.GetSize()));
                    }
                    ASSERT_EQ(BE_SQLITE_DONE, rc);
                    ASSERT_EQ(ECSqlStatus::Success, stmt.Reset());
                }
            }
        }
    }
    ECSqlStatement fixed;
    ASSERT_EQ(ECSqlStatus::Success, fixed.Prepare(m_ecdb, "SELECT Integers,Leaves FROM cr.CompositeRow WHERE Description='full'"));
    ASSERT_EQ(BE_SQLITE_ROW, fixed.Step());
    ECSqlRowAdaptor adaptor(m_ecdb);
    PreparedECSqlRowRenderer renderer;
    renderer.BeginPage(fixed, adaptor);
    ASSERT_TRUE(renderer.CanRender(adaptor));
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    ASSERT_TRUE(renderer.WriteRow(writer, fixed, adaptor));
    EXPECT_STREQ("[[1,2],[{},{\"Zeta\":\"array\"}]]", buffer.GetString());
}

struct SleepFunc : BeSQLite::ScalarFunction {
    SleepFunc() : ScalarFunction("imodel_sleep", -1){}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override {
        std::chrono::milliseconds t = 10ms;
        if (nArgs > 0) {
            t = std::chrono::milliseconds(args[0].GetValueInt());
        }
        std::this_thread::sleep_for(t);
        ctx.SetResultInt(1);
    }
    static SleepFunc& Instance() {static SleepFunc sleepFunc; return sleepFunc;}
};

struct CountFunc : BeSQLite::ScalarFunction {
    int _rowCount;
    CountFunc() : ScalarFunction("count_row", -1), _rowCount(0){}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override { _rowCount++; }
    void Reset() { _rowCount = 0; }
    static CountFunc& Instance() {static CountFunc countFunc; return countFunc;}
};

// Records the id of the thread that executes it. The "imodel_" name prefix is required so that
// the function is propagated to concurrent query worker connections (see
// CachedConnection::GetPrimaryDbSqlFunctions).
struct ThreadIdFunc : BeSQLite::ScalarFunction {
    std::mutex m_mutex;
    std::thread::id m_lastThreadId;
    bool m_invoked = false;
    ThreadIdFunc() : ScalarFunction("imodel_thread_id", 0){}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override {
        {
            std::lock_guard<std::mutex> lk(m_mutex);
            m_lastThreadId = std::this_thread::get_id();
            m_invoked = true;
        }
        ctx.SetResultInt(1);
    }
    void Reset() { std::lock_guard<std::mutex> lk(m_mutex); m_invoked = false; m_lastThreadId = std::thread::id(); }
    std::thread::id LastThreadId() { std::lock_guard<std::mutex> lk(m_mutex); return m_lastThreadId; }
    bool Invoked() { std::lock_guard<std::mutex> lk(m_mutex); return m_invoked; }
    static ThreadIdFunc& Instance() { static ThreadIdFunc f; return f; }
};

struct CursorCountFunc : BeSQLite::ScalarFunction {
    std::atomic<uint32_t> m_calls{0};
    CursorCountFunc() : ScalarFunction("imodel_cursor_count", 1) {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int, BeSQLite::DbValue* args) override {
        ++m_calls;
        ctx.SetResultInt(args[0].GetValueInt());
    }
};

struct ConnectionPragmaFunc : BeSQLite::ScalarFunction {
    std::string m_sql;
    ConnectionPragmaFunc(Utf8CP name, Utf8CP sql) : ScalarFunction(name, 0), m_sql(sql) {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int, BeSQLite::DbValue*) override {
        // BeSQLite passes sqlite3_context directly as its Context wrapper.
        auto db = sqlite3_context_db_handle(reinterpret_cast<sqlite3_context*>(&ctx));
        sqlite3_stmt* stmt = nullptr;
        auto rc = sqlite3_prepare_v2(db, m_sql.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            ctx.SetResultError_code(rc);
            return;
        }
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW)
            ctx.SetResultInt64(sqlite3_column_int64(stmt, 0));
        else
            ctx.SetResultError("connection PRAGMA returned no row");
        rc = sqlite3_finalize(stmt);
        if (rc != SQLITE_OK)
            ctx.SetResultError_code(rc);
    }
};

TEST_F(ConcurrentQueryFixture, PendingSelectSurvivesSavepointCommit) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("PendingSelectSurvivesSavepointCommit.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    ECDb worker;
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.OpenSecondaryConnection(worker, ECDb::OpenParams(Db::OpenMode::Readonly, DefaultTxn::No)));
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(worker,
        "WITH sequence(n) AS (SELECT 1 UNION ALL SELECT n+1 FROM sequence WHERE n<5) SELECT n FROM sequence LIMIT 4 OFFSET 1"));
    for (int expected = 2; expected <= 5; ++expected) {
        Savepoint txn(worker, "cursor_spike", false);
        ASSERT_EQ(BE_SQLITE_OK, txn.Begin());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        EXPECT_EQ(expected, stmt.GetValueInt(0));
        ASSERT_EQ(BE_SQLITE_OK, txn.Commit());
    }
    EXPECT_EQ(BE_SQLITE_DONE, stmt.Step());
}

TEST_F(ConcurrentQueryFixture, MemoryMapFileSizeAppliesToWorkersOnly) {
    ConnectionPragmaFunc mmapSize("imodel_mmap_size", "PRAGMA mmap_size");
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("MemoryMapFileSizeAppliesToWorkersOnly.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.AddFunction(mmapSize));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.TryExecuteSql("PRAGMA mmap_size=1048576"));
    ECSqlStatement primarySize;
    ASSERT_EQ(ECSqlStatus::Success, primarySize.Prepare(m_ecdb, "SELECT imodel_mmap_size()"));
    ASSERT_EQ(BE_SQLITE_ROW, primarySize.Step());
    const auto effectiveSize = primarySize.GetValueInt64(0);
    primarySize.Finalize();
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.TryExecuteSql("PRAGMA mmap_size=0"));

    for (uint32_t size : {0u, 1048576u}) {
        auto config = ConcurrentQueryMgr::Config::GetDefault();
        config.SetWorkerThreadCount(1).SetMemoryMapFileSize(size);
        ConcurrentQueryMgr::Config::Reset(config);
        ConcurrentQueryMgr mgr(m_ecdb);
        auto worker = mgr.Enqueue(ECSqlRequest::MakeRequest("SELECT imodel_mmap_size()")).Get();
        ASSERT_EQ(QueryResponse::Status::Done, worker->GetStatus()) << worker->GetError();
        EXPECT_EQ(size == 0 ? 0 : effectiveSize,
            BeJsDocument(worker->GetAsConst<ECSqlResponse>().asJsonString())[0][0].asInt64());
        auto req = ECSqlRequest::MakeRequest("SELECT imodel_mmap_size()");
        req->SetUsePrimaryConnection(true);
        auto primary = mgr.Enqueue(std::move(req)).Get();
        ASSERT_EQ(QueryResponse::Status::Done, primary->GetStatus()) << primary->GetError();
        EXPECT_EQ(0, BeJsDocument(primary->GetAsConst<ECSqlResponse>().asJsonString())[0][0].asInt64());
    }
}

TEST_F(ConcurrentQueryFixture, CacheSizeInKBAppliesToWorkersOnly) {
    ConnectionPragmaFunc cacheSize("imodel_cache_size", "PRAGMA cache_size");
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("CacheSizeInKBAppliesToWorkersOnly.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.AddFunction(cacheSize));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.TryExecuteSql("PRAGMA cache_size=123"));
    int64_t defaultCacheSize;
    {
        ECDb baseline;
        ASSERT_EQ(BE_SQLITE_OK, m_ecdb.OpenSecondaryConnection(baseline, ECDb::OpenParams(Db::OpenMode::Readonly, DefaultTxn::No)));
        Savepoint txn(baseline, "cache_size_baseline");
        Statement stmt;
        ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(baseline, "PRAGMA cache_size"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        defaultCacheSize = stmt.GetValueInt64(0);
    }
    for (int32_t size : {-1, 0, 8192}) {
        auto config = ConcurrentQueryMgr::Config::GetDefault();
        config.SetWorkerThreadCount(1);
        if (size >= 0)
            config.SetCacheSizeInKB(static_cast<uint32_t>(size));
        ConcurrentQueryMgr::Config::Reset(config);
        ConcurrentQueryMgr mgr(m_ecdb);
        auto worker = mgr.Enqueue(ECSqlRequest::MakeRequest("SELECT imodel_cache_size()")).Get();
        ASSERT_EQ(QueryResponse::Status::Done, worker->GetStatus()) << worker->GetError();
        EXPECT_EQ(size < 0 ? defaultCacheSize : -static_cast<int64_t>(size),
            BeJsDocument(worker->GetAsConst<ECSqlResponse>().asJsonString())[0][0].asInt64());
        auto req = ECSqlRequest::MakeRequest("SELECT imodel_cache_size()");
        req->SetUsePrimaryConnection(true);
        auto primary = mgr.Enqueue(std::move(req)).Get();
        ASSERT_EQ(QueryResponse::Status::Done, primary->GetStatus()) << primary->GetError();
        EXPECT_EQ(123, BeJsDocument(primary->GetAsConst<ECSqlResponse>().asJsonString())[0][0].asInt64());
    }
}

TEST_F(ConcurrentQueryFixture, CursorResumesSorterWithoutReexecution) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("CursorResumesSorterWithoutReexecution.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.EnableWalMode(true));
    CursorCountFunc count;
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.AddFunction(count));
    for (bool enabled : {false, true}) {
        auto config = ConcurrentQueryMgr::Config::GetDefault();
        config.SetWorkerThreadCount(1).SetEnableCursors(enabled).SetQuota(QueryQuota(60s, 1));
        ConcurrentQueryMgr::Config::Reset(config);
        count.m_calls = 0;
        {
            ConcurrentQueryMgr mgr(m_ecdb);
            for (int64_t i = 0; i < 10; ++i) {
                auto req = ECSqlRequest::MakeRequest(
                    "WITH sequence(n) AS (SELECT 1 UNION ALL SELECT n+1 FROM sequence WHERE n<?) "
                    "SELECT n FROM sequence ORDER BY imodel_cursor_count(n) DESC", ECSqlParams().BindInt(1, 100));
                req->SetCursorId("sorter").SetLimit(QueryLimit(10 - i, 5 + i));
                auto resp = mgr.Enqueue(std::move(req)).Get();
                ASSERT_EQ(QueryResponse::Status::Partial, resp->GetStatus()) << resp->GetError();
                EXPECT_EQ(enabled && i > 0, resp->GetStats().Resumed());
                BeJsDocument rows(resp->GetAsConst<ECSqlResponse>().asJsonString());
                ASSERT_EQ(1, rows.size());
                EXPECT_EQ(95 - i, rows[0][0].asInt64());
                EXPECT_EQ(enabled ? 100u : 100u * (i + 1), count.m_calls.load());
            }
        }
    }
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.RemoveFunction(count));
}

TEST_F(ConcurrentQueryFixture, CursorMatchingEvictionAndFallback) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("CursorMatchingEvictionAndFallback.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.EnableWalMode(true));
    auto config = ConcurrentQueryMgr::Config::GetDefault();
    config.SetWorkerThreadCount(1).SetStatementCacheSizePerWorker(2).SetMaxCursorsPerWorker(1).SetQuota(QueryQuota(60s, 1));
    ConcurrentQueryMgr::Config::Reset(config);
    ConcurrentQueryMgr mgr(m_ecdb);
    auto page = [&](std::string const& id, int64_t offset, int value, int64_t count = -1) {
        auto req = ECSqlRequest::MakeRequest(
            "WITH sequence(n) AS (SELECT 1 UNION ALL SELECT n+1 FROM sequence WHERE n<10) SELECT n+? FROM sequence ORDER BY n",
            ECSqlParams().BindInt(1, value));
        req->SetCursorId(id).SetLimit(QueryLimit(count, offset));
        return mgr.Enqueue(std::move(req)).Get();
    };
    auto check = [](QueryResponse::Ptr const& resp, int row, bool resumed) {
        ASSERT_EQ(QueryResponse::Status::Partial, resp->GetStatus()) << resp->GetError();
        EXPECT_EQ(resumed, resp->GetStats().Resumed());
        BeJsDocument rows(resp->GetAsConst<ECSqlResponse>().asJsonString());
        ASSERT_EQ(1, rows.size());
        EXPECT_EQ(row, rows[0][0].asInt());
    };
    check(page("a", 0, 0), 1, false);
    check(page("a", 1, 0), 2, true);
    check(page("b", 0, 100), 101, false);
    check(page("a", 2, 0), 3, false); // a was evicted when b was parked
    check(page("a", 3, 100), 104, false); // changed bindings
    check(page("a", 4, 100, 2), 105, false); // changed count
    check(page("a", 5, 100, 1), 106, true);
    check(page("a", 6, 100), 107, false); // previous finite LIMIT was consumed
    check(page("", 0, 0), 1, false);
    check(page("", 1, 0), 2, true); // legacy caller without a cursor hint
    check(page("", 5, 0), 6, false); // noncontiguous page
}

TEST_F(ConcurrentQueryFixture, CursorExpiresAndPrimaryRequestsNeverResume) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("CursorExpiresAndPrimaryRequestsNeverResume.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.EnableWalMode(true));
    auto config = ConcurrentQueryMgr::Config::GetDefault();
    config.SetWorkerThreadCount(1).SetCursorIdleTimeout(1s).SetQuota(QueryQuota(60s, 1));
    ConcurrentQueryMgr::Config::Reset(config);
    ConcurrentQueryMgr mgr(m_ecdb);
    auto page = [&](int offset, bool primary) {
        auto req = ECSqlRequest::MakeRequest(
            "WITH sequence(n) AS (SELECT 1 UNION ALL SELECT n+1 FROM sequence WHERE n<10) SELECT n FROM sequence");
        req->SetCursorId("expires").SetLimit(QueryLimit(-1, offset));
        req->SetUsePrimaryConnection(primary);
        return mgr.Enqueue(std::move(req)).Get();
    };
    EXPECT_FALSE(page(0, false)->GetStats().Resumed());
    std::this_thread::sleep_for(1100ms);
    auto expired = page(1, false);
    ASSERT_EQ(QueryResponse::Status::Partial, expired->GetStatus());
    EXPECT_FALSE(expired->GetStats().Resumed());
    EXPECT_EQ(2, BeJsDocument(expired->GetAsConst<ECSqlResponse>().asJsonString())[0][0].asInt());
    EXPECT_FALSE(page(0, true)->GetStats().Resumed());
    EXPECT_FALSE(page(1, true)->GetStats().Resumed());
}

TEST_F(ConcurrentQueryFixture, CursorInvalidatesOnCommittedDataChange) {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("CursorInvalidatesOnCommittedDataChange.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName="CursorTest" alias="ct" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="Row"><ECProperty propertyName="n" typeName="int"/></ECEntityClass>
        </ECSchema>)xml")));
    for (int n = 1; n <= 5; ++n) {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ct.[Row](n) VALUES(?)"));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, n));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.EnableWalMode(true));
    auto config = ConcurrentQueryMgr::Config::GetDefault();
    config.SetWorkerThreadCount(1).SetQuota(QueryQuota(60s, 1));
    ConcurrentQueryMgr::Config::Reset(config);
    ConcurrentQueryMgr mgr(m_ecdb);
    auto page = [&](int offset) {
        auto req = ECSqlRequest::MakeRequest("SELECT n FROM ct.[Row] ORDER BY n");
        req->SetCursorId("data-change").SetLimit(QueryLimit(-1, offset));
        return mgr.Enqueue(std::move(req)).Get();
    };
    ASSERT_EQ(QueryResponse::Status::Partial, page(0)->GetStatus());
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE ct.[Row] SET n=n+100"));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    auto resp = page(1);
    ASSERT_EQ(QueryResponse::Status::Partial, resp->GetStatus()) << resp->GetError();
    EXPECT_FALSE(resp->GetStats().Resumed());
    EXPECT_EQ(102, BeJsDocument(resp->GetAsConst<ECSqlResponse>().asJsonString())[0][0].asInt());
}

TEST_F(ConcurrentQueryFixture, CursorConfigRoundTrips) {
    BeJsDocument val(R"json({"enableCursors":false,"maxCursorsPerWorker":7,"cursorIdleTimeout":12})json");
    auto config = ConcurrentQueryMgr::Config::From(val);
    EXPECT_FALSE(config.GetEnableCursors());
    EXPECT_EQ(7, config.GetMaxCursorsPerWorker());
    EXPECT_EQ(12s, config.GetCursorIdleTimeout());
    BeJsDocument serialized;
    config.To(serialized);
    EXPECT_TRUE(config.Equals(ConcurrentQueryMgr::Config::From(serialized)));
    EXPECT_FALSE(config.Equals(ConcurrentQueryMgr::Config::GetDefault()));
}

TEST_F(ConcurrentQueryFixture, CacheSizeInKBConfigRoundTrips) {
    auto const& defaults = ConcurrentQueryMgr::Config::GetDefault();
    EXPECT_FALSE(defaults.GetCacheSizeInKB().has_value());
    for (uint32_t size : {0u, 8192u, 2147483647u}) {
        auto config = defaults;
        config.SetCacheSizeInKB(size);
        EXPECT_FALSE(config.Equals(defaults));
        BeJsDocument serialized;
        config.To(serialized);
        EXPECT_EQ(size, serialized["cacheSizeInKB"].asUInt());
        EXPECT_TRUE(config.Equals(ConcurrentQueryMgr::Config::From(serialized)));
        defaults.To(serialized);
        EXPECT_FALSE(serialized.isMember("cacheSizeInKB"));
    }
    for (Utf8CP json : {
        R"json({"cacheSizeInKB":-1})json", R"json({"cacheSizeInKB":1.5})json",
        R"json({"cacheSizeInKB":2147483648})json", R"json({"cacheSizeInKB":"8192"})json"}) {
        BeJsDocument input(json);
        EXPECT_EQ(ConcurrentQueryMgr::Config::GetFromEnv().GetCacheSizeInKB(),
            ConcurrentQueryMgr::Config::From(input).GetCacheSizeInKB());
    }
    auto config = defaults;
    bool rejected = false;
    try {
        config.SetCacheSizeInKB(2147483648u);
    } catch (std::invalid_argument const&) {
        rejected = true;
    }
    EXPECT_TRUE(rejected);
}

TEST_F(ConcurrentQueryFixture, CursorSettingsApplyToExistingManager) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("CursorSettingsApplyToExistingManager.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.EnableWalMode(true));
    auto config = ConcurrentQueryMgr::Config::GetDefault();
    config.SetWorkerThreadCount(1).SetQuota(QueryQuota(60s, 1));
    ConcurrentQueryMgr::Config::Reset(config);
    ConcurrentQueryMgr mgr(m_ecdb);
    auto page = [&](int offset) {
        auto req = ECSqlRequest::MakeRequest(
            "WITH sequence(n) AS (SELECT 1 UNION ALL SELECT n+1 FROM sequence WHERE n<10) SELECT n FROM sequence");
        req->SetCursorId("config").SetLimit(QueryLimit(-1, offset));
        return mgr.Enqueue(std::move(req)).Get();
    };
    ASSERT_EQ(QueryResponse::Status::Partial, page(0)->GetStatus());
    EXPECT_TRUE(page(1)->GetStats().Resumed());
    ConcurrentQueryMgr::Config::Reset(config.SetEnableCursors(false));
    EXPECT_FALSE(page(2)->GetStats().Resumed());
    ConcurrentQueryMgr::Config::Reset(config.SetEnableCursors(true));
    EXPECT_FALSE(page(3)->GetStats().Resumed());
    EXPECT_TRUE(page(4)->GetStats().Resumed());
    ConcurrentQueryMgr::Config::Reset(config.SetMaxCursorsPerWorker(0));
    EXPECT_FALSE(page(5)->GetStats().Resumed());
}

TEST_F(ConcurrentQueryFixture, CursorAutoBudgetAndZeroCapacity) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("CursorAutoBudgetAndZeroCapacity.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.EnableWalMode(true));
    for (bool readonly : {false, true}) {
        if (readonly)
            ASSERT_EQ(BE_SQLITE_OK, ReopenECDb(ECDb::OpenParams(Db::OpenMode::Readonly)));
        for (uint32_t cacheSize : {0u, 8u}) {
            auto config = ConcurrentQueryMgr::Config::GetDefault();
            config.SetWorkerThreadCount(1).SetStatementCacheSizePerWorker(cacheSize).SetQuota(QueryQuota(60s, 1));
            ConcurrentQueryMgr::Config::Reset(config);
            ConcurrentQueryMgr mgr(m_ecdb);
            auto page = [&](std::string const& id, int offset) {
                auto req = ECSqlRequest::MakeRequest(
                    "WITH sequence(n) AS (SELECT 1 UNION ALL SELECT n+1 FROM sequence WHERE n<10) SELECT n FROM sequence");
                req->SetCursorId(id).SetLimit(QueryLimit(-1, offset));
                return mgr.Enqueue(std::move(req)).Get();
            };
            for (int i = 0; i < 6; ++i)
                ASSERT_EQ(QueryResponse::Status::Partial, page(std::to_string(i), 0)->GetStatus());
            auto resp = page("0", 1);
            ASSERT_EQ(QueryResponse::Status::Partial, resp->GetStatus());
            EXPECT_EQ(readonly && cacheSize > 0, resp->GetStats().Resumed());
            EXPECT_EQ(2, BeJsDocument(resp->GetAsConst<ECSqlResponse>().asJsonString())[0][0].asInt());
        }
    }
}

TEST_F(ConcurrentQueryFixture, CursorRestartAndShutdown) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("CursorRestartAndShutdown.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.EnableWalMode(true));
    auto config = ConcurrentQueryMgr::Config::GetDefault();
    config.SetWorkerThreadCount(1).SetQuota(QueryQuota(60s, 1));
    ConcurrentQueryMgr::Config::Reset(config);
    auto page = [&](ConcurrentQueryMgr& mgr, std::string const& id, int offset) {
        auto req = ECSqlRequest::MakeRequest(
            "WITH sequence(n) AS (SELECT 1 UNION ALL SELECT n+1 FROM sequence WHERE n<10) SELECT n FROM sequence");
        req->SetCursorId(id).SetLimit(QueryLimit(-1, offset));
        req->SetRestartToken("restart");
        return mgr.Enqueue(std::move(req)).Get();
    };
    {
        ConcurrentQueryMgr mgr(m_ecdb);
        EXPECT_FALSE(page(mgr, "old", 0)->GetStats().Resumed());
        EXPECT_TRUE(page(mgr, "old", 1)->GetStats().Resumed());
        EXPECT_FALSE(page(mgr, "new", 0)->GetStats().Resumed());
        EXPECT_FALSE(page(mgr, "old", 2)->GetStats().Resumed());
    }
    {
        ConcurrentQueryMgr mgr(m_ecdb);
        EXPECT_FALSE(page(mgr, "old", 3)->GetStats().Resumed());
    }
}

TEST_F(ConcurrentQueryFixture, CompletedResponseRestartPreservesCursor) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("CompletedResponseRestartPreservesCursor.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.EnableWalMode(true));
    auto config = ConcurrentQueryMgr::Config::GetDefault();
    config.SetWorkerThreadCount(1).SetQuota(QueryQuota(60s, 1));
    ConcurrentQueryMgr::Config::Reset(config);
    RunnableRequestQueue queue(m_ecdb);
    ConnectionCache conns(m_ecdb, 1);
    auto makeRequest = [](int offset) {
        auto request = ECSqlRequest::MakeRequest(
            "WITH sequence(n) AS (SELECT 1 UNION ALL SELECT n+1 FROM sequence WHERE n<10) SELECT n FROM sequence");
        request->SetCursorId("completed").SetLimit(QueryLimit(-1, offset)).SetRestartToken("restart");
        return request;
    };
    RunnableRequestBase* publishingRequest = nullptr;
    bool published = false;
    ConcurrentQueryMgr::OnCompletion callback = [&](auto response) {
        published = true;
        EXPECT_EQ(QueryResponse::Status::Partial, response->GetStatus());
        EXPECT_TRUE(publishingRequest->IsCompleted());
        conns.InterruptIf([](RunnableRequestBase const& request) {
            return request.GetRequest().GetRestartToken() == "restart";
        }, true);
        EXPECT_FALSE(publishingRequest->IsCancelled());
        EXPECT_FALSE(publishingRequest->IsInterrupted());
    };
    auto first = std::make_unique<RunnableRequestWithCallback>(queue, makeRequest(0), config.GetQuota(), 1, callback);
    publishingRequest = first.get();
    auto owner = conns.GetConnection(*first);
    ASSERT_NE(nullptr, owner);
    owner->Execute([](QueryAdaptorCache& cache, RunnableRequestBase& request) {
        QueryHelper::Execute(cache, request);
    }, std::move(first));
    ASSERT_TRUE(published);
    owner.reset();

    auto next = std::make_unique<RunnableRequestWithPromise>(queue, makeRequest(1), config.GetQuota(), 2);
    auto result = next->GetFuture();
    auto connection = conns.GetConnection(*next);
    ASSERT_NE(nullptr, connection);
    connection->Execute([](QueryAdaptorCache& cache, RunnableRequestBase& request) {
        QueryHelper::Execute(cache, request);
    }, std::move(next));
    auto response = result.Get();
    EXPECT_EQ(QueryResponse::Status::Partial, response->GetStatus());
    EXPECT_TRUE(response->GetStats().Resumed());
    EXPECT_EQ(2, BeJsDocument(response->GetAsConst<ECSqlResponse>().asJsonString())[0][0].asInt());
}

TEST_F(ConcurrentQueryFixture, CursorReadOnlyPrimaryObservesExternalCommit) {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("CursorReadOnlyPrimaryObservesExternalCommit.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName="CursorExternal" alias="ce" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="Row"><ECProperty propertyName="n" typeName="int"/></ECEntityClass>
        </ECSchema>)xml")));
    for (int n = 1; n <= 5; ++n) {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ce.[Row](n) VALUES(?)"));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, n));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.EnableWalMode(true));
    ECDb writer;
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.OpenSecondaryConnection(writer, ECDb::OpenParams(Db::OpenMode::ReadWrite, DefaultTxn::No)));
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb(ECDb::OpenParams(Db::OpenMode::Readonly)));
    auto config = ConcurrentQueryMgr::Config::GetDefault();
    config.SetWorkerThreadCount(1).SetQuota(QueryQuota(60s, 1));
    ConcurrentQueryMgr::Config::Reset(config);
    ConcurrentQueryMgr mgr(m_ecdb);
    auto page = [&](int offset) {
        auto req = ECSqlRequest::MakeRequest("SELECT n FROM ce.[Row] ORDER BY n");
        req->SetCursorId("external").SetLimit(QueryLimit(-1, offset));
        return mgr.Enqueue(std::move(req)).Get();
    };
    ASSERT_EQ(QueryResponse::Status::Partial, page(0)->GetStatus());
    {
        Savepoint txn(writer, "external_commit");
        {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(writer, "UPDATE ce.[Row] SET n=n+100"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
        }
        ASSERT_EQ(BE_SQLITE_OK, txn.Commit());
    }
    auto resp = page(1);
    ASSERT_EQ(QueryResponse::Status::Partial, resp->GetStatus()) << resp->GetError();
    EXPECT_FALSE(resp->GetStats().Resumed());
    EXPECT_EQ(102, BeJsDocument(resp->GetAsConst<ECSqlResponse>().asJsonString())[0][0].asInt());
}

TEST_F(ConcurrentQueryFixture, CursorNonWalFallsBack) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("CursorNonWalFallsBack.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.EnableWalMode(false));
    auto config = ConcurrentQueryMgr::Config::GetDefault();
    config.SetWorkerThreadCount(1).SetQuota(QueryQuota(60s, 1));
    ConcurrentQueryMgr::Config::Reset(config);
    for (bool readonly : {false, true}) {
        if (readonly)
            ASSERT_EQ(BE_SQLITE_OK, ReopenECDb(ECDb::OpenParams(Db::OpenMode::Readonly)));
        ConcurrentQueryMgr mgr(m_ecdb);
        for (int offset = 0; offset < 2; ++offset) {
            auto req = ECSqlRequest::MakeRequest("SELECT ECInstanceId FROM meta.ECClassDef ORDER BY ECInstanceId");
            req->SetCursorId("non-wal").SetLimit(QueryLimit(-1, offset));
            auto resp = mgr.Enqueue(std::move(req)).Get();
            ASSERT_EQ(QueryResponse::Status::Partial, resp->GetStatus());
            EXPECT_FALSE(resp->GetStats().Resumed());
        }
    }
}

TEST_F(ConcurrentQueryFixture, NativeReaderCrossesPageBoundaries) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("NativeReaderCrossesPageBoundaries.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.EnableWalMode(true));
    auto config = ConcurrentQueryMgr::Config::GetDefault();
    config.SetWorkerThreadCount(1).SetQuota(QueryQuota(60s, 1));
    ConcurrentQueryMgr::Config::Reset(config);
    ConcurrentQueryMgr mgr(m_ecdb);
    ECSqlReader reader(mgr,
        "WITH sequence(n) AS (SELECT 1 UNION ALL SELECT n+1 FROM sequence WHERE n<10) SELECT n FROM sequence ORDER BY n");
    int expected = 1;
    while (reader.Next())
        EXPECT_EQ(expected++, reader.GetRow()[0].asInt());
    EXPECT_EQ(11, expected);
    EXPECT_FALSE(reader.Next());
}

TEST_F(ConcurrentQueryFixture, NativeReaderReplacesBatchDocuments) {
    const std::string largeText(16384, 'x');
    const double number = 9.9999999999999995e-21;
    ASSERT_EQ(SUCCESS, SetupRenderingPayload("NativeReaderReplacesBatchDocuments.ecdb", largeText, number));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.EnableWalMode(true));
    auto config = ConcurrentQueryMgr::Config::GetDefault();
    config.SetWorkerThreadCount(1).SetQuota(QueryQuota(60s, 1));
    ConcurrentQueryMgr::Config::Reset(config);
    ECSqlParams args;
    args.BindString(1, largeText);
    args.BindDouble(2, number);
    ConcurrentQueryMgr mgr(m_ecdb);
    ECSqlReader reader(mgr,
        "WITH sequence(n) AS (SELECT 1 UNION ALL SELECT n+1 FROM sequence WHERE n<100) "
        "SELECT n, CASE WHEN n%2=0 THEN p.TextValue ELSE 'small' END, p.DoubleValue, NULL "
        "FROM sequence CROSS JOIN rp.Payload p WHERE p.TextValue=? AND p.DoubleValue=? ORDER BY n",
        args);
    int expected = 1;
    while (reader.Next()) {
        auto row = reader.GetRow();
        EXPECT_EQ(expected, row[0].asInt());
        EXPECT_STREQ(expected % 2 == 0 ? largeText.c_str() : "small", row[1].asCString());
        EXPECT_DOUBLE_EQ(number, row[2].asDouble());
        EXPECT_TRUE(row[3].isNull());
        ++expected;
    }
    EXPECT_EQ(101, expected);
    EXPECT_FALSE(reader.Next());
}

TEST_F(ConcurrentQueryFixture, CursorBusyOwnerFallsBack) {
    struct GateFunc : BeSQLite::ScalarFunction {
        std::promise<void> m_started;
        std::promise<void> m_release;
        std::shared_future<void> m_released;
        std::atomic_bool m_completed{false};
        GateFunc() : ScalarFunction("imodel_cursor_gate", 0), m_released(m_release.get_future().share()) {}
        void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int, BeSQLite::DbValue*) override {
            m_started.set_value();
            m_released.wait_for(10s);
            m_completed = true;
            ctx.SetResultInt(1);
        }
    } gate;
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("CursorBusyOwnerFallsBack.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.EnableWalMode(true));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.AddFunction(gate));
    auto config = ConcurrentQueryMgr::Config::GetDefault();
    config.SetWorkerThreadCount(2).SetQuota(QueryQuota(60s, 1));
    ConcurrentQueryMgr::Config::Reset(config);
    {
        ConcurrentQueryMgr mgr(m_ecdb);
        auto page = [&](int offset) {
            auto req = ECSqlRequest::MakeRequest(
                "WITH sequence(n) AS (SELECT 1 UNION ALL SELECT n+1 FROM sequence WHERE n<10) SELECT n FROM sequence");
            req->SetCursorId("busy").SetLimit(QueryLimit(-1, offset));
            return mgr.Enqueue(std::move(req)).Get();
        };
        EXPECT_EQ(QueryResponse::Status::Partial, page(0)->GetStatus());
        auto started = gate.m_started.get_future();
        auto busy = mgr.Enqueue(ECSqlRequest::MakeRequest("SELECT imodel_cursor_gate()"));
        EXPECT_EQ(std::future_status::ready, started.wait_for(5s));
        auto resp = page(1);
        EXPECT_EQ(QueryResponse::Status::Partial, resp->GetStatus()) << resp->GetError();
        EXPECT_FALSE(resp->GetStats().Resumed());
        if (resp->IsSuccess())
            EXPECT_EQ(2, BeJsDocument(resp->GetAsConst<ECSqlResponse>().asJsonString())[0][0].asInt());
        EXPECT_FALSE(gate.m_completed.load());
        auto unrelated = mgr.Enqueue(ECSqlRequest::MakeRequest("SELECT 42")).Get();
        EXPECT_TRUE(unrelated->IsSuccess()) << unrelated->GetError();
        if (unrelated->IsSuccess()) {
            EXPECT_EQ(1u, unrelated->GetAsConst<ECSqlResponse>().GetRowCount());
            EXPECT_EQ(42, BeJsDocument(unrelated->GetAsConst<ECSqlResponse>().asJsonString())[0][0].asInt());
        }
        EXPECT_FALSE(gate.m_completed.load());
        gate.m_release.set_value();
        EXPECT_TRUE(busy.Get()->IsSuccess());
    }
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.RemoveFunction(gate));
}

TEST_F(ConcurrentQueryFixture, CursorAffinityWaitIsBoundedAndReusesOwner) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("CursorAffinityWaitIsBoundedAndReusesOwner.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.EnableWalMode(true));
    auto config = ConcurrentQueryMgr::Config::GetDefault();
    config.SetWorkerThreadCount(2).SetQuota(QueryQuota(60s, 1));
    ConcurrentQueryMgr::Config::Reset(config);
    RunnableRequestQueue queue(m_ecdb);
    ConnectionCache conns(m_ecdb, 2);
    auto makeRequest = [&](int offset, Utf8CP cursor = "affinity") {
        auto request = ECSqlRequest::MakeRequest(
            "WITH sequence(n) AS (SELECT 1 UNION ALL SELECT n+1 FROM sequence WHERE n<10) SELECT n FROM sequence");
        request->SetCursorId(cursor).SetLimit(QueryLimit(-1, offset));
        return std::make_unique<RunnableRequestWithPromise>(queue, std::move(request), config.GetQuota(), 1);
    };
    auto first = makeRequest(0);
    auto firstResult = first->GetFuture();
    auto owner = conns.GetConnection(*first);
    ASSERT_NE(nullptr, owner);
    const auto ownerId = owner->Id();
    owner->Execute([](QueryAdaptorCache& cache, RunnableRequestBase& request) {
        QueryHelper::Execute(cache, request);
    }, std::move(first));
    ASSERT_EQ(QueryResponse::Status::Partial, firstResult.Get()->GetStatus());

    auto waiting = makeRequest(1);
    EXPECT_EQ(nullptr, conns.GetConnection(*waiting));
    auto unrelated = makeRequest(1, "unrelated");
    auto otherConnection = conns.GetConnection(*unrelated);
    ASSERT_NE(nullptr, otherConnection);
    EXPECT_NE(ownerId, otherConnection->Id());
    otherConnection.reset();

    auto expired = makeRequest(1);
    auto start = std::chrono::steady_clock::now() - RunnableRequestBase::kCursorAffinityWait;
    EXPECT_TRUE(expired->ShouldWaitForCursor(start));
    EXPECT_FALSE(expired->ShouldWaitForCursor(start + RunnableRequestBase::kCursorAffinityWait));
    auto fallback = conns.GetConnection(*expired);
    ASSERT_NE(nullptr, fallback);
    EXPECT_NE(ownerId, fallback->Id());
    fallback.reset();

    owner.reset();
    auto resumedConnection = conns.GetConnection(*waiting);
    ASSERT_NE(nullptr, resumedConnection);
    EXPECT_EQ(ownerId, resumedConnection->Id());
    auto result = waiting->GetFuture();
    resumedConnection->Execute([](QueryAdaptorCache& cache, RunnableRequestBase& request) {
        QueryHelper::Execute(cache, request);
    }, std::move(waiting));
    auto response = result.Get();
    EXPECT_EQ(QueryResponse::Status::Partial, response->GetStatus());
    EXPECT_TRUE(response->GetStats().Resumed());
    EXPECT_EQ(2, BeJsDocument(response->GetAsConst<ECSqlResponse>().asJsonString())[0][0].asInt());
}

TEST_F(ConcurrentQueryFixture, CursorPagingBenchmark) {
    if (std::getenv("RUN_CURSOR_PAGING_BENCHMARK") == nullptr) {
#if defined(USE_GTEST)
        GTEST_SKIP() << "Set RUN_CURSOR_PAGING_BENCHMARK to benchmark cursor paging";
#else
        printf("Skipping cursor paging benchmark: set RUN_CURSOR_PAGING_BENCHMARK to run it.\n");
        return;
#endif
    }
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("CursorPagingBenchmark.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.EnableWalMode(true));
    std::vector<std::string> queries = {
        "WITH sequence(n) AS (SELECT 1 UNION ALL SELECT n+1 FROM sequence WHERE n<50000) SELECT n FROM sequence",
        "WITH sequence(n) AS (SELECT 1 UNION ALL SELECT n+1 FROM sequence WHERE n<50000) SELECT n FROM sequence ORDER BY n DESC",
        "WITH sequence(n) AS (SELECT 1 UNION ALL SELECT n+1 FROM sequence WHERE n<50000) SELECT n,COUNT(*) FROM sequence GROUP BY n",
        "SELECT ECInstanceId FROM meta.ECClassDef ORDER BY ECInstanceId LIMIT 5",
    };
    for (bool readonly : {false, true}) {
        if (readonly)
            ASSERT_EQ(BE_SQLITE_OK, ReopenECDb(ECDb::OpenParams(Db::OpenMode::Readonly)));
        for (auto const& query : queries) {
            std::string baselineRows;
            for (bool enabled : {false, true}) {
                auto config = ConcurrentQueryMgr::Config::GetDefault();
                config.SetWorkerThreadCount(1).SetEnableCursors(enabled).SetQuota(QueryQuota(60s, 4096));
                ConcurrentQueryMgr::Config::Reset(config);
                ConcurrentQueryMgr mgr(m_ecdb);
                int64_t offset = 0;
                uint32_t pages = 0, resumes = 0;
                int64_t prepareMs = 0;
                std::vector<int64_t> pageTimes;
                std::string rows;
                auto start = std::chrono::steady_clock::now();
                do {
                    auto pageStart = std::chrono::steady_clock::now();
                    auto req = ECSqlRequest::MakeRequest(query);
                    req->SetCursorId("benchmark").SetLimit(QueryLimit(-1, offset));
                    auto resp = mgr.Enqueue(std::move(req)).Get();
                    ASSERT_TRUE(resp->IsSuccess()) << resp->GetError();
                    auto const& page = resp->GetAsConst<ECSqlResponse>();
                    pageTimes.push_back(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - pageStart).count());
                    ++pages;
                    resumes += resp->GetStats().Resumed() ? 1 : 0;
                    prepareMs += resp->GetStats().PrepareTime().count();
                    rows += page.asJsonString();
                    offset += page.GetRowCount();
                    if (resp->IsDone())
                        break;
                    ASSERT_GT(page.GetRowCount(), 0);
                } while (true);
                auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start).count();
                if (!enabled)
                    baselineRows = rows;
                else
                    EXPECT_EQ(baselineRows, rows);
                std::printf("cursor benchmark: readonly=%d enabled=%d rows=%" PRId64 " pages=%u resumes=%u elapsed_us=%" PRId64 " prepare_ms=%" PRId64 " query=%s\n",
                    readonly, enabled, offset, pages, resumes, elapsed, prepareMs, query.c_str());
                for (size_t i = 0; i < pageTimes.size(); ++i)
                    std::printf("  page=%zu elapsed_us=%" PRId64 "\n", i, pageTimes[i]);
            }
        }
    }
}

struct StressTest {
    using query_request_t = ECSqlRequest::Ptr;
    using futures_t= std::vector<QueryResponse::Future>;
    using responses_t = std::queue<QueryResponse::Ptr>;
    struct QueryParam final {
        float _percentage_of_cacheable_queries;
        float _percentage_of_restartable_queries ;
        int _number_of_restartable_tokens;
        int _min_rows_to_generate ;
        int _max_rows_to_generate;
        int _concurrent_queries;
        std::chrono::milliseconds _sleep_per_request;

        static QueryParam Default() {
            QueryParam _;
            _._percentage_of_cacheable_queries = 0.5f;
            _._percentage_of_restartable_queries = 0.5f;
            _._number_of_restartable_tokens = 1;
            _._min_rows_to_generate = 100;
            _._max_rows_to_generate = 1000;
            _._concurrent_queries = 10;
            _._sleep_per_request = std::chrono::milliseconds(10);
            return _;
        }

        static QueryParam Aggressive() {
            auto _ = Default();
            _._percentage_of_cacheable_queries = 0.5f;
            _._percentage_of_restartable_queries = 0.6f;
            _._sleep_per_request = std::chrono::milliseconds(0);
            return _;
        }
        static QueryParam Slow() {
            QueryParam _ = Aggressive();
            _._sleep_per_request = std::chrono::milliseconds(100);
            return _;
        }
    };
    struct Client {
        static bool IsProb(float per = 0.5) { return ((float)rand() / RAND_MAX) < per; }
        static int Rand(int start = 0, int stop = 0) { return (int)((float)rand() / RAND_MAX)*(stop - start) + stop; }
        private:
            virtual query_request_t _MakeRequest(QueryParam const& param) const {
                const auto isCacheableQuery = IsProb(param._percentage_of_cacheable_queries);
                const auto isRestartableQuery = IsProb(param._percentage_of_restartable_queries);
                const auto restartTokenId = Rand(0, param._number_of_restartable_tokens);
                const auto maxRows = Rand(param._min_rows_to_generate, _param._max_rows_to_generate);
                const auto randomNoneZeroInt = isCacheableQuery ? rand() + 1 : 1;
                const std::string restartToken = SqlPrintfString("restart_token_%d", restartTokenId).GetUtf8CP();
                const std::string query = SqlPrintfString("with cnt(x) as (values(100000) union select x+1 from cnt where x < (? + 100000)) select x,x,x,x,x,x,x,x,x,x from cnt where %d",
                    randomNoneZeroInt).GetUtf8CP();
                auto req = ECSqlRequest::MakeRequest(query, ECSqlParams().BindInt(1, maxRows));
                if (isRestartableQuery) { req->SetRestartToken(restartToken); }
                if (param._sleep_per_request.count() > 0) {
                    std::this_thread::sleep_for(param._sleep_per_request);
                }
                return std::move(req);
            }
        private:
            std::thread _thread;
            std::atomic_bool _running;
            uint64_t _total_time;
            uint64_t _total_bytes;
            uint32_t _requestMade;
            uint32_t _maxRequest;
            QueryParam _param;
            ConcurrentQueryMgr& _mgr;
            query_request_t MakeRequest(QueryParam const& param) const { return _MakeRequest(param); }
            std::map<QueryResponse::Status, int> _statuses;

        public:
            std::map<QueryResponse::Status, int> const& GetStatusCount() const { return _statuses; }
            QueryParam const& GetQueryParam() const { return _param; }
            uint64_t GetTotalTime() const { return _total_time;  }
            uint64_t GetTotalResultSize() const { return _total_bytes;  }
            bool IsRunning() const { return _running.load(); }
            uint32_t GetRequestMade() const { return _requestMade; }
            Client (ConcurrentQueryMgr& mgr, uint32_t maxRequest= 1000, QueryParam param = QueryParam::Default())
                :_running(true),_total_time(0),_total_bytes(0),_mgr(mgr),_requestMade(0),_maxRequest(maxRequest),_param(param)  {
                _thread = std::thread([&]() {
                    while(_running.load()) {
                        futures_t futures;
                        for (int i = 0; i < _param._concurrent_queries; ++i) {
                            futures.push_back(_mgr.Enqueue(MakeRequest(_param)));
                            ++_requestMade;
                            if (_maxRequest > 0) {
                                if (_requestMade > _maxRequest) {
                                    _running = false;
                                }
                            }
                        }

                        for (auto& future : futures) {
                            auto resp = future.Get();
                            _total_time += resp->GetStats().TotalTime().count();
                            _total_bytes += resp->GetStats().MemUsed();
                            if (_statuses.find(resp->GetStatus()) == _statuses.end()) {
                                _statuses[resp->GetStatus()] = 1;
                            } else {
                                ++_statuses[resp->GetStatus()];
                            }
                            if ((int)resp->GetStatus() >= (int)QueryResponse::Status::Error){
                                BeAssert(false);
                                LOG.errorv("%s: %s", QueryResponse::StatusToString(resp->GetStatus()), resp->GetError().c_str());
                            }
                            if (_maxRequest > 0) {
                                if (_requestMade > _maxRequest) {
                                    _running = false;
                                }
                            }
                            std::this_thread::yield();
                        }
                    }
                });
            }
            void AppendStatus(std::map<QueryResponse::Status, int>& statuses) {
                for (auto& entry : _statuses) {
                    if (statuses.find(entry.first) == statuses.end()) {
                        statuses[entry.first] = entry.second;
                    } else {
                        statuses[entry.first] += entry.second;
                    }
                }
            }
            void Stop() { _running.store(false); }
            ~Client() { _thread.join(); }
            static std::unique_ptr<Client> Make(ConcurrentQueryMgr& mgr, uint32_t maxRequest= 1000, QueryParam param = QueryParam::Default()) { return std::make_unique<Client>(mgr, maxRequest, param); }
    };
private:
    std::vector<std::unique_ptr<Client>> m_clients;

public:
    StressTest() {}
    static std::vector<std::unique_ptr<Client>> Make(ConcurrentQueryMgr& mgr, uint32_t maxRequest= 1000, int aggressiveClients = 10, int normalClients = 10, int slowClients = 10) {
        std::vector<std::unique_ptr<Client>> clients;
        for (int i = 0; i < slowClients; ++i) {
            clients.push_back(Client::Make(mgr, maxRequest, QueryParam::Slow()));
        }
        for (int i = 0; i < normalClients; ++i) {
            clients.push_back(Client::Make(mgr, maxRequest, QueryParam::Default()));
        }
        for (int i = 0; i < aggressiveClients; ++i) {
            clients.push_back(Client::Make(mgr, maxRequest, QueryParam::Aggressive()));
        }
        return std::move(clients);
    }
    static void WaitOrStop(std::vector<std::unique_ptr<Client>>& clients,  uint32_t maxRequest= 1000) {
        for (auto& client : clients) {
            if (client->GetRequestMade() >= maxRequest) {
                if (client->IsRunning()) {
                    client->Stop();
                    while(client->IsRunning()) {
                        std::this_thread::yield();
                    }
                }
            }
        }
    }
};

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
#if 0
TEST_F(ConcurrentQueryFixture, StressTest) {
    GTEST_SKIP() << "This test take 15 sec and is meant for stress testing concurrent query for failures under load";

    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("conn_query.ecdb"));
    m_ecdb.AddFunction(SleepFunc::Instance());
    ConcurrentQueryMgr::Config::GetInstance().SetQuota(QueryQuota(std::chrono::seconds(10),1024*1025*1));
    ConcurrentQueryMgr::Config::GetInstance().SetWorkerThreadCount(std::thread::hardware_concurrency());

    auto& mgr = ConcurrentQueryMgr::GetInstance(m_ecdb);
    const uint32_t maxRequest = 100;
    const int aggressiveClients = 10;
    const int normalClients = 10;
    const int slowClients = 10;

    auto clients = StressTest::Make(mgr, maxRequest, aggressiveClients, normalClients, slowClients);
    StressTest::WaitOrStop(clients, maxRequest);
}
#endif

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConcurrentQueryFixture, Blob_Metadata) {
    const uint8_t bin[] = {0x22, 0xfa, 0x33, 0x1a, 0x33, 0xe2, 0x39, 0xef, 0xcf, 0xd4};

    BeJsDocument expectedMetadataDoc;
    expectedMetadataDoc.Parse(R"json({
        "className":"TestSchema:testEntity",
        "accessString":"bin",
        "generated":false,
        "index":0,
        "jsonName":"bin",
        "name":"bin",
        "extendedType":"",
        "typeName":"binary"
    })json");

    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("Blob_Metadata.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECEntityClass typeName="testEntity"
                    description="Cover all primitive, primitive array, struct of primitive, array of struct">
                    <ECProperty propertyName="bin" typeName="binary" />
                </ECEntityClass>
            </ECSchema>)xml")));
    ECSqlStatement stmt;
    auto rc = stmt.Prepare(m_ecdb, R"sql(
        insert into ts.testEntity(bin)
        values(?)
    )sql");
    stmt.BindBlob(1, (void const*)bin, (int)sizeof(bin), IECSqlBinder::MakeCopy::No);
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);
    m_ecdb.SaveChanges();

    ConcurrentQueryMgr::WithInstance(m_ecdb, [&](auto& mgr){
        auto req = ECSqlRequest::MakeRequest("SELECT bin FROM ts.testEntity");
        req->SetIncludeMetaData(true);  // Metadata must be included for this test
        auto r = mgr.Enqueue(std::move(req)).Get();
        EXPECT_EQ(r->GetStatus(), QueryResponse::Status::Done);
        auto res = ((ECSqlResponse*) r.get());

        BeJsDocument resJson;
        res->ToJs(resJson, true);
        auto metadataJson = resJson["meta"][0];

        EXPECT_STREQ(metadataJson["accessString"].asCString(), "bin");
        EXPECT_STREQ(metadataJson["typeName"].asCString(), "binary");
        EXPECT_STREQ(metadataJson["extendedType"].asCString(), "");
        EXPECT_TRUE(metadataJson.isExactEqual(expectedMetadataDoc))
            << "\n  actual:   " << metadataJson.Stringify(StringifyFormat::Indented)
            << "\n  expected: " << expectedMetadataDoc.Stringify(StringifyFormat::Indented);
    });
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConcurrentQueryFixture, Blob_NotAbbreviatedByDefault) {
    const uint8_t bin[] = {0x1, 0x1, 0x1};

    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("Blob_NotAbbreviatedByDefault.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECEntityClass typeName="testEntity"
                    description="Cover all primitive, primitive array, struct of primitive, array of struct">
                    <ECProperty propertyName="bin" typeName="binary" />
                </ECEntityClass>
            </ECSchema>)xml")));
    ECSqlStatement stmt;
    auto rc = stmt.Prepare(m_ecdb, R"sql(
        insert into ts.testEntity(bin)
        values(?)
    )sql");
    stmt.BindBlob(1, (void const*)bin, (int)sizeof(bin), IECSqlBinder::MakeCopy::No);
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);
    m_ecdb.SaveChanges();

    ConcurrentQueryMgr::WithInstance(m_ecdb, [&](auto& mgr){
        auto req = ECSqlRequest::MakeRequest("SELECT bin FROM ts.testEntity");
        auto r = mgr.Enqueue(std::move(req)).Get();
        ASSERT_EQ(r->GetStatus(), QueryResponse::Status::Done);
        auto res = ((ECSqlResponse*) r.get());

        BeJsDocument resJson;
        res->ToJs(resJson, true);
        BeJsDocument resData;
        resData.Parse(resJson["data"].asString());

        EXPECT_STREQ(resData[0][0].asString().c_str(), "encoding=base64;AQEB");
    });
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConcurrentQueryFixture, Blob_NotAbbreviated) {
    const uint8_t bin[] = {0x22, 0xfa, 0x33, 0x1a, 0x33, 0xe2, 0x39, 0xef, 0xcf, 0xd4};

    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("Blob_NotAbbreviated.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECEntityClass typeName="testEntity"
                    description="Cover all primitive, primitive array, struct of primitive, array of struct">
                    <ECProperty propertyName="bin" typeName="binary" />
                </ECEntityClass>
            </ECSchema>)xml")));
    ECSqlStatement stmt;
    auto rc = stmt.Prepare(m_ecdb, R"sql(
        insert into ts.testEntity(bin)
        values(?)
    )sql");
    stmt.BindBlob(1, (void const*)bin, (int)sizeof(bin), IECSqlBinder::MakeCopy::No);
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);
    m_ecdb.SaveChanges();

    ConcurrentQueryMgr::WithInstance(m_ecdb, [&](auto& mgr) {
        auto req = ECSqlRequest::MakeRequest("SELECT bin FROM ts.testEntity");
        req->SetAbbreviateBlobs(false);
        auto r = mgr.Enqueue(std::move(req)).Get();
        ASSERT_EQ(r->GetStatus(), QueryResponse::Status::Done);
        auto res = ((ECSqlResponse*) r.get());

        BeJsDocument resJson;
        res->ToJs(resJson, true);
        BeJsDocument resData;
        resData.Parse(resJson["data"].asString());

        ASSERT_STREQ(resData[0][0].asString().c_str(), "encoding=base64;IvozGjPiOe/P1A==");
    });
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConcurrentQueryFixture, CTEWithAComment) {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("CTEWithAComment.ecdb"));

    const auto sqlTemplate = R"(
        WITH ce(TestColumn1, TestColumn2) AS (
            %s
        ) SELECT * FROM ce)";

    const auto testCases = {
        std::make_tuple(1, "TestColumn",
            R"(
                -- comment
                SELECT 1, 'TestColumn' FROM meta.ECClassDef LIMIT 1
            )"),
        std::make_tuple(2, "TestColumn",
            R"(
                -- multi
                -- line
                -- comment
                SELECT 1, 'TestColumn' FROM meta.ECClassDef LIMIT 1
            )"),
        std::make_tuple(3, "TestColumn",
            R"(
                -- multi
                -- line
                -- comment()
                SELECT 1, 'TestColumn' FROM meta.ECClassDef LIMIT 1
            )"),
        std::make_tuple(4, "TestColumn",
            R"(
                /* comment) */
                SELECT 1, 'TestColumn' FROM meta.ECClassDef LIMIT 1
            )"),
        std::make_tuple(5, "TestColumn",
            R"(
                // calling function()
                SELECT 1, 'TestColumn' FROM meta.ECClassDef LIMIT 1
            )"),
        std::make_tuple(6, "TestColumn",
            R"(
                -- calling function()
                SELECT 1, 'TestColumn' FROM meta.ECClassDef LIMIT 1
            )"),
        std::make_tuple(7, "TestColumn",
            R"(
                /* comment */
                SELECT 1, 'TestColumn' FROM meta.ECClassDef LIMIT 1
            )"),
        std::make_tuple(8, "TestColumn",
            R"(
                SELECT 1, 'TestColumn' FROM meta.ECClassDef LIMIT 1 -- comment)
            )"),
        std::make_tuple(9, "TestColumn",
            R"(
                SELECT 1, 'TestColumn' FROM meta.ECClassDef LIMIT 1 /* comment) */
            )"),
        std::make_tuple(10, "invalid -- column",
            R"(
                SELECT 1, 'invalid -- column' AS TestColumn FROM meta.ECClassDef LIMIT 1
            )"),
        std::make_tuple(11, "text with ) parenthesis",
            R"(
                SELECT 1, 'text with ) parenthesis' AS TestColumn FROM meta.ECClassDef LIMIT 1 -- real comment
            )"),
        std::make_tuple(12, "text /* not a comment */",
            R"(
                SELECT 1, 'text /* not a comment */' AS TestColumn FROM meta.ECClassDef LIMIT 1
            )"),
        std::make_tuple(13, "comment)",
            R"(
                SELECT 1, 'comment)' AS TestColumn FROM meta.ECClassDef LIMIT 1 -- called from function XYZ()
            )"),
        std::make_tuple(14, "TestColumn",
            R"(
                SELECT /* Primary Key (class Id) */ 1, /* Class Name */ 'TestColumn' FROM meta.ECClassDef LIMIT 1
            )"),
        std::make_tuple(15, "TestColumn",
            R"(
                SELECT 1,
                'TestColumn' /* multiline
                comment */ FROM meta.ECClassDef LIMIT 1
            )"),
    };

    ConcurrentQueryMgr::WithInstance(m_ecdb, [&](auto& mgr) {
        for (const auto& [testCaseNumber, expectedSecondColumnValue, ecSqlQuery] : testCases) {
            const auto errorMessage = Utf8PrintfString("Test case number: %d failed.", testCaseNumber);
            
            auto req = ECSqlRequest::MakeRequest(SqlPrintfString(sqlTemplate, ecSqlQuery).GetUtf8CP());
            auto queryResponse = mgr.Enqueue(std::move(req)).Get();
            EXPECT_EQ(queryResponse->GetStatus(), QueryResponse::Status::Done) << errorMessage;

            auto ecsqlResponse = static_cast<ECSqlResponse*>(queryResponse.get());
            BeJsDocument responseJson;
            ecsqlResponse->ToJs(responseJson, true);
            EXPECT_EQ(responseJson["rowCount"].asInt(), 1) << errorMessage;

            BeJsDocument row;
            row.Parse(responseJson["data"].asString());

            ASSERT_TRUE(row.isArray()) << errorMessage;
            ASSERT_TRUE(row[0].isArray()) << errorMessage;
            ASSERT_EQ(row[0].size(), 2) << errorMessage;

            EXPECT_EQ(row[0][0].asDouble(), 1.0) << errorMessage;
            EXPECT_STREQ(row[0][1].asString().c_str(), expectedSecondColumnValue) << errorMessage;
        }
    });
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConcurrentQueryFixture, Blob_Abbreviated) {
    const uint8_t bin[] = {0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21};

    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("Blob_Abbreviated.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECEntityClass typeName="testEntity"
                    description="Cover all primitive, primitive array, struct of primitive, array of struct">
                    <ECProperty propertyName="bin" typeName="binary" />
                </ECEntityClass>
            </ECSchema>)xml")));
    ECSqlStatement stmt;
    auto rc = stmt.Prepare(m_ecdb, R"sql(
        insert into ts.testEntity(bin)
        values(?)
    )sql");
    stmt.BindBlob(1, (void const*)bin, (int)sizeof(bin), IECSqlBinder::MakeCopy::No);
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);
    m_ecdb.SaveChanges();

    ConcurrentQueryMgr::WithInstance(m_ecdb, [&](auto& mgr) {
        auto req = ECSqlRequest::MakeRequest("SELECT bin FROM ts.testEntity");
        req->SetAbbreviateBlobs(true);
        auto r = mgr.Enqueue(std::move(req)).Get();
        EXPECT_EQ(r->GetStatus(), QueryResponse::Status::Done);
        auto res = ((ECSqlResponse*)r.get());

        BeJsDocument resJson;
        res->ToJs(resJson, true);
        BeJsDocument resData;
        resData.Parse(resJson["data"].asString());

        EXPECT_STREQ(resData[0][0].asString().c_str(), "{\"bytes\":13}");
    });
}


//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConcurrentQueryFixture, BlobColumnInfoRepeated) {
    // There was a bug where the abbreviateBlobs flag was not populated properly, this test checks that the flag behaves correctly
    // when running the same query repeatedly with different values of the flag.
    const uint8_t bin[] = {0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21};

    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("BlobColumnInfoRepeated.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECEntityClass typeName="testEntity"
                    description="Cover all primitive, primitive array, struct of primitive, array of struct">
                    <ECProperty propertyName="bin" typeName="binary" />
                </ECEntityClass>
            </ECSchema>)xml")));
    ECSqlStatement stmt;
    auto rc = stmt.Prepare(m_ecdb, R"sql(
        insert into ts.testEntity(bin)
        values(?)
    )sql");
    stmt.BindBlob(1, (void const*)bin, (int)sizeof(bin), IECSqlBinder::MakeCopy::No);
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);
    m_ecdb.SaveChanges();

    auto checkBlobColumnInfo = [&](const std::string& expectedExtendedType, const std::string& expectedTypeName, std::optional<bool> abbreviateBlobs = std::nullopt) {
        ConcurrentQueryMgr::WithInstance(m_ecdb, [&](auto& mgr) {
            auto req = ECSqlRequest::MakeRequest("SELECT bin FROM ts.testEntity");
            if (abbreviateBlobs.has_value()) {
                req->SetAbbreviateBlobs(abbreviateBlobs.value());
            }
            auto r = mgr.Enqueue(std::move(req)).Get();
            EXPECT_EQ(r->GetStatus(), QueryResponse::Status::Done);
            auto res = ((ECSqlResponse*) r.get());
            auto& rowProps = res->GetProperties();
            EXPECT_EQ(rowProps.size(), 1);
            auto& prop = rowProps[0];
            EXPECT_STREQ(prop.GetExtendedType().c_str(), expectedExtendedType.c_str());
            EXPECT_STREQ(prop.GetTypeName().c_str(), expectedTypeName.c_str());
        });
    };

    checkBlobColumnInfo("", "binary");
    checkBlobColumnInfo("", "binary", false);
    checkBlobColumnInfo("Json", "string", true);
    checkBlobColumnInfo("", "binary");
    checkBlobColumnInfo("", "binary", false);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConcurrentQueryFixture, InterruptCheck_Timeout) {

    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("conn_query.ecdb"));
    auto config = ConcurrentQueryMgr::Config::Get();
    config.SetQuota(QueryQuota(std::chrono::seconds(1), 1024));
    config.SetIgnoreDelay(false);
    ConcurrentQueryMgr::Config::Reset(config);

    const auto delay = std::chrono::milliseconds(2000);
    ConcurrentQueryMgr::WithInstance(m_ecdb, [&](auto& mgr) {
        auto req = ECSqlRequest::MakeRequest("with cnt(x) as (values(0) union select x+1 from cnt where x < ? ) select x from cnt", ECSqlParams().BindInt(1, 1));
        req->SetDelay(delay);
        auto r = mgr.Enqueue(std::move(req)).Get();
        EXPECT_EQ(r->GetStatus(), QueryResponse::Status::Timeout);
    });
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConcurrentQueryFixture, InterruptCheck_MemoryLimitExceeded) {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("conn_query.ecdb"));

    auto config = ConcurrentQueryMgr::Config::Get();
    config.SetQuota(QueryQuota(std::chrono::seconds(10), 1000));
    ConcurrentQueryMgr::Config::Reset(config);

    ConcurrentQueryMgr::WithInstance(m_ecdb, [&](auto& mgr) {
        auto req = ECSqlRequest::MakeRequest(
            "with cnt(x) as (values(0) union select x+1 from cnt where x < ? ) select x, CAST(randomblob(1000) AS BINARY) from cnt",
            ECSqlParams().BindInt(1, 3));

        auto r = mgr.Enqueue(std::move(req)).Get();
        EXPECT_EQ(r->GetStatus(), QueryResponse::Status::Partial);
    });

}
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConcurrentQueryFixture, InterruptCheck_TimeLimitExceeded) {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("conn_query.ecdb"));

    auto config = ConcurrentQueryMgr::Config::Get();
    config.SetQuota(QueryQuota(std::chrono::seconds(1), 1000));
    ConcurrentQueryMgr::Config::Reset(config);

    m_ecdb.AddFunction(SleepFunc::Instance());
    ConcurrentQueryMgr::WithInstance(m_ecdb, [&](auto& mgr) {
        auto req = ECSqlRequest::MakeRequest(
            "with cnt(x) as (values(0) union select x+1 from cnt where x < ? ) select x,imodel_sleep(500, x)  from cnt",
            ECSqlParams().BindInt(1, 10));

        auto r = mgr.Enqueue(std::move(req)).Get();
        EXPECT_EQ(r->GetStatus(), QueryResponse::Status::Partial);
    });
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConcurrentQueryFixture, ECSqlParams) {
    int i = 111;
    int64_t i64 = 1111;
    double d = 1111.1111;
    auto p2d = DPoint2d::From(122.332, 344.455);
    auto p3d = DPoint3d::From(432.453, 231.453, 332.334);
    bvector<Byte> bin = {0x22,0xfa, 0x33, 0x1a, 0x33, 0xe2, 0x39, 0xef, 0xcf, 0xd4};
    auto str ="hello, world";
    auto b = true;
    auto id = BeInt64Id(0xeeddff);
    BeIdSet idset;

    idset.insert(BeInt64Id(0xeeddf1));
    idset.insert(BeInt64Id(0xeeddf2));
    idset.insert(BeInt64Id(0xeeddf3));
    idset.insert(BeInt64Id(0xeeddf4));
    idset.insert(BeInt64Id(0xeeddf5));
    idset.insert(BeInt64Id(0xeeddf6));
    idset.insert(BeInt64Id(0xeeddf7));
    idset.insert(BeInt64Id(0xeeddf8));
    idset.insert(BeInt64Id(0xeeddf9));


    std::string expectedStr =  R"({
        "1"  : {
            "type" : 0,
            "value" : true
        },
        "10" : {
             "type" : 7,
            "value" : {
                "x" : 122.33199999999999,
                "y" : 344.45499999999998
            }
        },
        "11" : {
             "type" : 8,
            "value" : {
                "x" : 432.45299999999997,
                "y" : 231.4530,
                "z" : 332.3340
            }
        },
        "12" : {
             "type" : 9,
            "value" : "hello, world"
        },
        "2" : {
             "type" : 10,
            "value" : "IvozGjPiOe/P1A=="
        },
        "3" : {
             "type" : 1,
            "value" : 1111.1111000000001
        },
        "4" : {
             "type" : 2,
            "value" : "0xeeddff"
        },
        "6" : {
             "type" : 3,
            "value" : "+EEDDF1+1*8"
        },
        "7" : {
             "type" : 4,
            "value" : 111
        },
        "8" : {
             "type" : 5,
            "value" : 1111
        },
        "9" : {
             "type" : 6,
            "value" : null
        },
        "b" : {
             "type" : 0,
            "value" : true
        },
        "bin" : {
             "type" : 10,
            "value" : "IvozGjPiOe/P1A=="
        },
        "d" : {
             "type" : 1,
            "value" : 1111.1111000000001
        },
        "i" : {
             "type" : 4,
            "value" : 111
        },
        "i64" : {
             "type" : 5,
            "value" : 1111
        },
        "id" : {
             "type" : 2,
            "value" : "0xeeddff"
        },
        "idset" : {
             "type" : 3,
            "value" : "+EEDDF1+1*8"
        },
        "nul" : {
             "type" : 6,
            "value" : null
        },
        "p2d" : {
             "type" : 7,
            "value" : {
                "x" : 122.33199999999999,
                "y" : 344.45499999999998
            }
        },
        "p3d" : {
            "type" : 8,
            "value" : {
                "x" : 432.45299999999997,
                "y" : 231.4530,
                "z" : 332.3340
            }
        },
        "str" : {
            "type" : 9,
            "value" : "hello, world"
        }
    })";

    auto expectedJson = BeJsDocument(expectedStr);

    ECSqlParams params;
    params.BindBool(1, b);
    params.BindBlob(2, bin);
    params.BindDouble(3, d);
    params.BindId(4, id);
    params.BindIdSet(6, idset);
    params.BindInt(7, i);
    params.BindLong(8, i64);
    params.BindNull(9);
    params.BindPoint2d(10, p2d);
    params.BindPoint3d(11, p3d);
    params.BindString(12, str);

    params.BindBool("b", b);
    params.BindBlob("bin", bin);
    params.BindDouble("d", d);
    params.BindId("id", id);
    params.BindIdSet("idset", idset);
    params.BindInt("i", i);
    params.BindLong("i64", i64);
    params.BindNull("nul");
    params.BindPoint2d("p2d", p2d);
    params.BindPoint3d("p3d", p3d);
    params.BindString("str", str);

    BeJsDocument v;
    params.ToJs(v);
    EXPECT_TRUE(v.isExactEqual(expectedJson)) << "\n  expected: " << v.Stringify() << "\n  actual:   " << expectedJson.Stringify();

    ECSqlParams params1;
    params1.FromJs(v);
    BeJsDocument v2;
    params1.ToJs(v2);
    EXPECT_TRUE(v2.isExactEqual(expectedJson)) << "\n  expected: " << v2.Stringify() << "\n  actual:   " << expectedJson.Stringify();

    ASSERT_EQ( params1.GetParam(1).GetType(), ECSqlParams::ECSqlParam::Type::Boolean);
    ASSERT_EQ( params1.GetParam(2).GetType(), ECSqlParams::ECSqlParam::Type::Blob);
    ASSERT_EQ( params1.GetParam(3).GetType(), ECSqlParams::ECSqlParam::Type::Double);
    ASSERT_EQ( params1.GetParam(4).GetType(), ECSqlParams::ECSqlParam::Type::Id);
    ASSERT_EQ( params1.GetParam(6).GetType(), ECSqlParams::ECSqlParam::Type::IdSet);
    ASSERT_EQ( params1.GetParam(7).GetType(), ECSqlParams::ECSqlParam::Type::Integer);
    ASSERT_EQ( params1.GetParam(8).GetType(), ECSqlParams::ECSqlParam::Type::Long);
    ASSERT_EQ( params1.GetParam(9).GetType(), ECSqlParams::ECSqlParam::Type::Null);
    ASSERT_EQ( params1.GetParam(10).GetType(), ECSqlParams::ECSqlParam::Type::Point2d);
    ASSERT_EQ( params1.GetParam(11).GetType(), ECSqlParams::ECSqlParam::Type::Point3d);
    ASSERT_EQ( params1.GetParam(12).GetType(), ECSqlParams::ECSqlParam::Type::String);

}
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConcurrentQueryFixture, sqlite_only_eval_function_with_no_arg_or_constant_arg_only_once) {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("conn_query.ecdb"));
    m_ecdb.AddFunction(CountFunc::Instance());

    const auto kRowCount = 10;
    if ("function with no arg") {
        CountFunc::Instance().Reset();
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
            "with cnt(x) as (values(1) union select x+1 from cnt where x < ? ) select COUNT_ROW() from cnt"));
        stmt.BindInt(1, kRowCount);
        int idx = 0;
        while(stmt.Step() == BE_SQLITE_ROW) {
            ++idx;
        }
        ASSERT_EQ(idx, kRowCount);
        ASSERT_EQ(CountFunc::Instance()._rowCount, 1); // sql function is only called once
    }
    if ("function with const arg") {
        CountFunc::Instance().Reset();
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
            "with cnt(x) as (values(1) union select x+1 from cnt where x < ? ) select COUNT_ROW(100) from cnt"));
        stmt.BindInt(1, kRowCount);
        int idx = 0;
        while(stmt.Step() == BE_SQLITE_ROW) {
            ++idx;
        }
        ASSERT_EQ(idx, kRowCount);
        ASSERT_EQ(CountFunc::Instance()._rowCount, 1); // sql function is only called once
    }
    if ("function with var arg") {
        CountFunc::Instance().Reset();
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
            "with cnt(x) as (values(1) union select x+1 from cnt where x < ? ) select COUNT_ROW(X) from cnt"));
        stmt.BindInt(1, kRowCount);
        int idx = 0;
        while(stmt.Step() == BE_SQLITE_ROW) {
            ++idx;
        }
        ASSERT_EQ(idx, kRowCount);
        ASSERT_EQ(CountFunc::Instance()._rowCount, kRowCount); // sql function is called for each row
    }
}
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConcurrentQueryFixture, DelayRequest) {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("conn_query.ecdb"));
    ConcurrentQueryMgr::Config conf = ConcurrentQueryMgr::Config::Get();
    conf.SetIgnoreDelay(false);
    ConcurrentQueryMgr::Config::Reset(conf);

    ConcurrentQueryMgr::WithInstance(m_ecdb, [&](auto& mgr) {
        auto req = ECSqlRequest::MakeRequest("with cnt(x) as (values(0) union select x+1 from cnt where x < ? ) select x from cnt", ECSqlParams().BindInt(1, 1));
        const auto delay = std::chrono::milliseconds(250);
        req->SetDelay(delay);
        auto r = mgr.Enqueue(std::move(req)).Get();
        EXPECT_EQ(r->GetStatus(), QueryResponse::Status::Done);
        EXPECT_GT(r->GetStats().TotalTime(), delay);
    });

}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConcurrentQueryFixture, RestartToken) {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("conn_query.ecdb"));
    ConcurrentQueryMgr::Config conf = ConcurrentQueryMgr::Config::Get();
    conf.SetIgnoreDelay(false);
    ConcurrentQueryMgr::Config::Reset(conf);

    ConcurrentQueryMgr::WithInstance(m_ecdb, [&](auto& mgr) {
        const auto sql = "with cnt(x) as (values(0) union select x+1 from cnt where x < ? ) select x from cnt";
        auto req0 = ECSqlRequest::MakeRequest(sql, ECSqlParams().BindInt(1, 5));
        req0->SetRestartToken("test token");
        req0->SetDelay(5000ms);
        auto req1 = ECSqlRequest::MakeRequest(sql, ECSqlParams().BindInt(1, 5));
        req1->SetRestartToken("test token");
        auto r0 = mgr.Enqueue(std::move(req0));
        auto r1 = mgr.Enqueue(std::move(req1));

        auto f0 = r0.Get();
        auto f1 = r1.Get();

        EXPECT_EQ(f0->GetStatus(), QueryResponse::Status::Cancel);
        EXPECT_EQ(f1->GetStatus(), QueryResponse::Status::Done);
    });
}
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConcurrentQueryFixture, FutureAndCallback) {
    auto testSchema = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Foo" >
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    </ECCustomAttributes>
                <ECProperty propertyName="I" typeName="int" />
                <ECProperty propertyName="S" typeName="string" />
            </ECEntityClass>
        </ECSchema>)xml");

    auto populateDb = [&](int rows, int strMaxLength) {
        ECSqlStatement stmt;
        stmt.Prepare(m_ecdb, "INSERT INTO ts.Foo(I,S) VALUES(?,?)");
        for (int i = 0; i < rows;i++) {
            stmt.BindInt(1, rand());
            stmt.BindText(2, Utf8String((rand() % strMaxLength) + 1, 'X').c_str(), IECSqlBinder::MakeCopy::Yes);
            stmt.Step();
            stmt.Reset();
            stmt.ClearBindings();
        };
    };
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("ConcurrentQuery_Simple.ecdb", testSchema));
    const auto rowsInserted = 100;
    populateDb(rowsInserted, 512);

    ReopenECDb(ECDb::OpenParams(Db::OpenMode::Readonly));
    ConcurrentQueryMgr::WithInstance(m_ecdb, [&](auto& mgr){
        auto future = mgr.Enqueue(ECSqlRequest::MakeRequest("select * from ts.foo"));
        std::promise<void> e;
        mgr.Enqueue(ECSqlRequest::MakeRequest("select * from ts.foo"), [&](QueryResponse::Ptr r){
            EXPECT_TRUE(r->IsSuccess());
            e.set_value();
        });
        auto response = future.Get();
        EXPECT_TRUE(response->IsSuccess());
        e.get_future().get();
    });
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConcurrentQueryFixture, ReaderSchema) {
    auto testSchema = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Foo" >
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    </ECCustomAttributes>
                <ECProperty propertyName="I" typeName="int" />
                <ECProperty propertyName="S" typeName="string" />
            </ECEntityClass>
        </ECSchema>)xml");

    auto populateDb = [&](int rows, int strMaxLength) {
        ECSqlStatement stmt;
        stmt.Prepare(m_ecdb, "INSERT INTO ts.Foo(I,S) VALUES(?,?)");
        for (int i = 0; i < rows;i++) {
            stmt.BindInt(1, rand());
            stmt.BindText(2, Utf8String((rand() % strMaxLength) + 1, 'X').c_str(), IECSqlBinder::MakeCopy::Yes);
            stmt.Step();
            stmt.Reset();
            stmt.ClearBindings();
        };
    };
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("ConcurrentQuery_Simple.ecdb", testSchema));
    const auto rowsInserted = 100;
    populateDb(rowsInserted, 512);
    ReopenECDb(ECDb::OpenParams(Db::OpenMode::Readonly));
    ConcurrentQueryMgr::WithInstance(m_ecdb, [&](auto& mgr) {
        ECSqlReader reader(mgr, "select * from ts.foo");
        int rowCount = 0;

        BeJsDocument expectedMeta;
        expectedMeta.Parse(R"([{
            "className": "",
            "accessString": "ECInstanceId",
            "generated": false,
            "index": 0,
            "jsonName": "id",
            "name": "ECInstanceId",
            "extendedType": "Id",
            "typeName": "long"
        }, {
            "className": "",
            "accessString": "ECClassId",
            "generated": false,
            "index": 1,
            "jsonName": "className",
            "name": "ECClassId",
            "extendedType": "ClassId",
            "typeName": "long"
        }, {
            "className": "TestSchema:Foo",
            "accessString": "I",
            "generated": false,
            "index": 2,
            "jsonName": "i",
            "name": "I",
            "extendedType": "",
            "typeName": "int"
        }, {
            "className": "TestSchema:Foo",
            "accessString": "S",
            "generated": false,
            "index": 3,
            "jsonName": "s",
            "name": "S",
            "extendedType": "",
            "typeName": "string"
        }])");
        reader.Next();
        ++rowCount;
        BeJsDocument actualMeta;
        reader.GetColumns().ToJs(actualMeta);
        EXPECT_TRUE(actualMeta.isExactEqual(expectedMeta))
            << "\n  actual:   " << actualMeta.Stringify() << "\n  expected: " << expectedMeta.Stringify();
        while (reader.Next()) {
            ++rowCount;
        }
        EXPECT_EQ(rowCount, rowsInserted);
        ECSqlReader reader1(mgr, "select COUNT(*) from ts.foo");
        reader1.Next();
        BeJsDocument expectedMeta2;
        expectedMeta2.Parse(R"json([{
            "className" : "",
            "accessString": "COUNT(*)",
            "generated" : true,
            "index" : 0,
            "jsonName" : "cOUNT(*)",
            "name" : "COUNT(*)",
            "extendedType" : "",
            "typeName" : "long"
        }])json");
        BeJsDocument actualMeta2;
        reader1.GetColumns().ToJs(actualMeta2);
        EXPECT_TRUE(actualMeta2.isExactEqual(expectedMeta2))
            << "\n  actual:   " << actualMeta2.Stringify() << "\n  expected: " << expectedMeta2.Stringify();
        auto cntJson0 = BeJsDocument(R"j({"cOUNT(*)" : 100.0})j");
        auto cntJson1 = BeJsDocument(R"j({"COUNT(*)" : 100.0})j");
        EXPECT_TRUE(reader1.GetRow().ToJson(ECSqlReader::Row::Format::UseJsonName).isExactEqual(cntJson0)) << "\n  expected: " << reader1.GetRow().ToJson(ECSqlReader::Row::Format::UseJsonName).Stringify() << "\n  actual:   " << cntJson0.Stringify();
        EXPECT_TRUE(reader1.GetRow().ToJson(ECSqlReader::Row::Format::UseName).isExactEqual(cntJson1)) << "\n  expected: " << reader1.GetRow().ToJson(ECSqlReader::Row::Format::UseName).Stringify() << "\n  actual:   " << cntJson1.Stringify();
        EXPECT_EQ(reader1.GetRow()[0].asInt(), 100);
        EXPECT_EQ(reader1.GetRow()["cOUNT(*)"].asInt(), 100);
    });
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConcurrentQueryFixture, ReaderBinding) {
    auto testSchema = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Foo" >
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    </ECCustomAttributes>
                <ECProperty propertyName="I" typeName="int" />
                <ECProperty propertyName="S" typeName="string" />
            </ECEntityClass>
        </ECSchema>)xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("ConcurrentQuery_Simple.ecdb", testSchema));

    ConcurrentQueryMgr::WithInstance(m_ecdb, [&](auto& mgr) {
        ECSqlReader classReader(mgr, "select * from meta.ECClassDef where name=?",
                                ECSqlParams().BindString(1, "Foo"));
        int cols = 0;
        while (classReader.Next()) {
            auto classRow = classReader.GetRow();
            for (int i = 0; i < classReader.GetColumns().size(); ++i, ++cols) {
                auto& col = classReader.GetColumns()[i];

                auto v0 = classRow[i];
                auto v1 = classRow[col];
                auto v2 = classRow[col.GetJsonName()];
                EXPECT_STREQ(v0.Stringify().c_str(), v1.Stringify().c_str());
                EXPECT_STREQ(v1.Stringify().c_str(), v2.Stringify().c_str());
                EXPECT_STREQ(v2.Stringify().c_str(), v0.Stringify().c_str());
            }
        }

        EXPECT_EQ(cols, 11);
        // cte
        ECSqlReader cteReader(mgr, "with cnt(x) as (values(1) union select x+1 from cnt where x < 1000 ) select * from cnt");
        int cntRowCount = 0;
        while (cteReader.Next()) {
            cntRowCount++;
        }
        EXPECT_EQ(cntRowCount, 1000);

        BeIdSet idSet;
        idSet.insert(BeInt64Id(10));
        idSet.insert(BeInt64Id(20));
        idSet.insert(BeInt64Id(30));
        idSet.insert(BeInt64Id(40));
        int vsRowCount = 0;
        ECSqlReader vsReader(mgr, "with cnt(x) as (values(1) union select x+1 from cnt where x < 1000 ) select * from cnt where invirtualset(?, x)",
                             ECSqlParams().BindIdSet(1, idSet));

        while (vsReader.Next()) {
            vsRowCount++;
        }
        EXPECT_EQ(vsRowCount, 4);
    });
}
//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConcurrentQueryFixture, BlobIO) {
    auto testSchema = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Foo" >
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    </ECCustomAttributes>
                <ECProperty propertyName="B" typeName="binary" />
            </ECEntityClass>
        </ECSchema>)xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("ConcurrentQuery_Simple.ecdb", testSchema));

    auto createBuff = [](int size) {
        std::vector<uint8_t> buffer;
        for(auto i =0; i< size; ++i) {
            buffer.push_back((uint8_t)(((float)rand()/RAND_MAX)*94+32));
        }
        buffer.shrink_to_fit();
        return buffer;
    };
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "insert into ts.Foo(ECInstanceId, B) VALUES(?, ?)"));
    std::map<int , std::vector<uint8_t>> buffers;
    const auto kSize = 1024*4;

    for(auto i =0; i< 2; ++i) {
        buffers[i]=createBuff(kSize);
        stmt.ClearBindings();
        stmt.Reset();
        stmt.BindInt(1, i  + 1);
        stmt.BindBlob(2, &buffers[i][0], (int)(buffers[i].size()), IECSqlBinder::MakeCopy::No);
        ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);
    }

    m_ecdb.SaveChanges();
    ConcurrentQueryMgr::WithInstance(m_ecdb, [&](auto& mgr) {
        if ("read a full blob") {
            auto ecId = 1;
            auto freq = mgr.Enqueue(BlobIORequest::MakeRequest("ts.Foo", "B", ecId));
            auto resp = freq.Get();
            EXPECT_TRUE(resp->IsSuccess());
            auto data = resp->template GetAsConst<BlobIOResponse>().GetData();
            auto len = resp->template GetAsConst<BlobIOResponse>().GetLength();
            auto& buff = buffers[ecId - 1];
            EXPECT_EQ(len, buff.size());
            EXPECT_EQ(memcmp(data, &buff[0], len), 0);
        }
        if ("read a partial blob") {
            auto ecId = 1;
            auto freq = mgr.Enqueue(BlobIORequest::MakeRequest("ts.Foo", "B", ecId, QueryLimit(10, 10)));
            auto resp = freq.Get();
            EXPECT_TRUE(resp->IsSuccess());
            auto data = resp->template GetAsConst<BlobIOResponse>().GetData();
            auto len = resp->template GetAsConst<BlobIOResponse>().GetLength();
            auto& buff = buffers[ecId - 1];
            EXPECT_EQ(len, 10);
            EXPECT_EQ(memcmp(data, &buff[0] + 10, len), 0);
        }
        if ("wrong class fail with error") {
            auto freq = mgr.Enqueue(BlobIORequest::MakeRequest("ts.UnknowClass", "B", 1));
            auto resp = freq.Get();
            EXPECT_TRUE(resp->IsError());
            EXPECT_STREQ(resp->GetError().c_str(), "BlobIO: unable to find classname 'ts.UnknowClass'");
        }
        if ("wrong property fail with error") {
            auto freq = mgr.Enqueue(BlobIORequest::MakeRequest("ts.Foo", "UnknownProperty", 1));
            auto resp = freq.Get();
            EXPECT_TRUE(resp->IsError());
            EXPECT_STREQ(resp->GetError().c_str(), "BlobIO: unable to open blob for classname 'ts.Foo' , accessString 'UnknownProperty' for instanceId '0x1'");
        }
        if ("wrong ec instance id fail with error") {
            auto freq = mgr.Enqueue(BlobIORequest::MakeRequest("ts.Foo", "B", 0xffffff));
            auto resp = freq.Get();
            EXPECT_TRUE(resp->IsError());
            printf("%s\n", resp->GetError().c_str());
            EXPECT_STREQ(resp->GetError().c_str(), "BlobIO: unable to open blob for classname 'ts.Foo' , accessString 'B' for instanceId '0xffffff'");
        }

        if ("wrong offset/length fail with error") {
            auto freq = mgr.Enqueue(BlobIORequest::MakeRequest("ts.Foo", "B", 1, QueryLimit(kSize + 1024, 1024)));
            auto resp = freq.Get();
            EXPECT_TRUE(resp->IsError());
            EXPECT_STREQ(resp->GetError().c_str(), "BlobIO: offset + length provided is greater then size of blob");
        }
    });
}
//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConcurrentQueryFixture, CommentAtEndOfECSql) {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("CommentAtEndOfECSql.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECEntityClass typeName="testEntity">
                    <ECProperty propertyName="entity_id" typeName="int" />
                </ECEntityClass>
            </ECSchema>)xml")));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.testEntity(entity_id) VALUES(?)"));
    stmt.BindInt(1, 1);
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);
    m_ecdb.SaveChanges();

    ConcurrentQueryMgr::WithInstance(m_ecdb, [&](auto& mgr) {
        if (true) {
            auto req = ECSqlRequest::MakeRequest("SELECT entity_id FROM ts.testEntity -- This is a comment");
            auto r = mgr.Enqueue(std::move(req)).Get();
            EXPECT_EQ(r->GetStatus(), QueryResponse::Status::Done);

            auto res = ((ECSqlResponse*)r.get());
            BeJsDocument resJson;
            res->ToJs(resJson, true);
            EXPECT_EQ(res->asJsonString(), "[[1]]");
        }

        // Additional test with a WITH clause
        if (true) {
            auto req = ECSqlRequest::MakeRequest(
                "WITH baseQuery AS (SELECT entity_id FROM ts.testEntity) SELECT * FROM baseQuery -- This is a comment");
            auto r = mgr.Enqueue(std::move(req)).Get();
            EXPECT_EQ(r->GetStatus(), QueryResponse::Status::Done);

            auto res = ((ECSqlResponse*)r.get());
            BeJsDocument resJson;
            res->ToJs(resJson, true);
            EXPECT_EQ(res->asJsonString(), "[[1]]");
        }
    });
}
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConcurrentQueryFixture, ReaderBindingForIdSetVirtualTable) {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("ConcurrentQuery_Simple.ecdb"));
    ConcurrentQueryMgr::WithInstance(m_ecdb, [&](auto& mgr) {
        BeIdSet idSet;
        idSet.insert(BeInt64Id(10));
        idSet.insert(BeInt64Id(20));
        idSet.insert(BeInt64Id(30));
        idSet.insert(BeInt64Id(40));
        int vsRowCount = 0;

        ECSqlReader  vsReaderIdSet(mgr, "select id from IdSet(?)",
            ECSqlParams().BindIdSet(1, idSet));

        int i = 1;
        while(vsReaderIdSet.Next()) {
            auto classRow = vsReaderIdSet.GetRow();
            EXPECT_EQ(i*10, BeStringUtilities::ParseHex(classRow[0].asString().c_str()));
            i++;
            vsRowCount++;
        }
        EXPECT_EQ(vsRowCount,4);
    });

    ConcurrentQueryMgr::WithInstance(m_ecdb, [&](auto& mgr) {
        BeIdSet idSet;
        idSet.insert(BeInt64Id(10));
        idSet.insert(BeInt64Id(20));
        idSet.insert(BeInt64Id(30));
        idSet.insert(BeInt64Id(40));
        int vsRowCount = 0;

        ECSqlReader  vsReaderIdSet(mgr, "select ECInstanceId from meta.ECClassDef, IdSet(?) where ECInstanceId = id",
            ECSqlParams().BindIdSet(1, idSet));

        int i = 1;
        while(vsReaderIdSet.Next()) {
            auto classRow = vsReaderIdSet.GetRow();
            EXPECT_EQ(i * 10, BeStringUtilities::ParseHex(classRow[0].asString().c_str()));
            i++;
            vsRowCount++;
        }
        EXPECT_EQ(vsRowCount,4);
    });

    ConcurrentQueryMgr::WithInstance(m_ecdb, [&](auto& mgr) {
        int vsRowCount = 0;
        ECSqlReader  vsReaderIdSet(mgr, "select ECInstanceId from meta.ECClassDef, IdSet(?) where ECInstanceId = id",
            ECSqlParams().BindId(1, BeInt64Id(33)));

        while(vsReaderIdSet.Next())
            {
            vsRowCount++;
        }
        EXPECT_EQ(vsRowCount,0);
    });
}

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConcurrentQueryFixture, ImportSchemaShouldClearQueryCache) {
    // There was a bug that importing a schema did not clean the cached prepared statements for concurrent queries.
    // So running a query that is cached would result in an error because the query needs to be reprepared.
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDbForCurrentTest(SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECEntityClass typeName="testEntity">
                    <ECProperty propertyName="entity_id" typeName="int" />
                </ECEntityClass>
            </ECSchema>)xml")));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.testEntity(entity_id) VALUES(?)"));
    stmt.BindInt(1, 1);
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);
    m_ecdb.SaveChanges();

    ConcurrentQueryMgr::WithInstance(m_ecdb, [&](auto& mgr) {
        auto req = ECSqlRequest::MakeRequest("SELECT entity_id FROM ts.testEntity");
        req->SetUsePrimaryConnection(true);
        auto r = mgr.Enqueue(std::move(req)).Get();
        EXPECT_EQ(r->GetStatus(), QueryResponse::Status::Done);

        auto res = ((ECSqlResponse*) r.get());
        BeJsDocument resJson;
        res->ToJs(resJson, true);
        EXPECT_EQ(res->asJsonString(), "[[1]]");
    });

    // Import a new schema to clear the cache
    SchemaItem updatedSchema(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECEntityClass typeName="testEntity">
            <ECProperty propertyName="entity_id" typeName="int" />
            <ECProperty propertyName="new_prop" typeName="int" />
        </ECEntityClass>
    </ECSchema>)xml");
    ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(updatedSchema));
    // Run an identical query again
    ConcurrentQueryMgr::WithInstance(m_ecdb, [&](auto& mgr) {
        auto req = ECSqlRequest::MakeRequest("SELECT entity_id FROM ts.testEntity");
        req->SetUsePrimaryConnection(true);
        auto r = mgr.Enqueue(std::move(req)).Get();
        EXPECT_EQ(r->GetStatus(), QueryResponse::Status::Done);

        auto res = ((ECSqlResponse*) r.get());
        BeJsDocument resJson;
        res->ToJs(resJson, true);
        EXPECT_EQ(res->asJsonString(), "[[1]]");
    });
}

//---------------------------------------------------------------------------------------
// @bsimethod
// Reproduces the deadlock from https://github.com/iTwin/itwinjs-backlog/issues/2113
// Thread A (main): sqlite3_step holds primary SQLite mutex, ExtractInstFunc fires,
//   calls InstanceReader::GetOrAddClass -> Dispatcher::GetIterable -> needs ECDb mutex.
// Thread B (worker): QueryAdaptorCache::TryGet holds ECDb mutex, ECSqlStatement::Prepare
//   resolves schemas via SchemaPersistenceHelper::GetClassId -> sqlite3_bind_text on
//   primary db -> needs primary SQLite mutex.
// Without the fix, this test deadlocks (times out). With the fix, the worker uses its
// own ECDb for schema resolution, avoiding the AB-BA lock ordering violation.
//
// NOTE: the deadlock window is probabilistic -- it relies on the OS scheduler interleaving
// mainStepThread's sqlite3_step loop with a worker Prepare. On a single-core or lightly loaded
// machine that interleaving may never happen, so a regressed build can pass here without the
// watchdog firing. The test is therefore most reliable on multi-core CI.
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConcurrentQueryFixture, DeadlockReproduction_MainStepVsWorkerPrepare) {
    // Use a schema with a class hierarchy to ensure polymorphic queries trigger extract_inst
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("deadlock_repro.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8" ?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                <ECEntityClass typeName="Base">
                    <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                    </ECCustomAttributes>
                    <ECProperty propertyName="Name" typeName="string" />
                    <ECProperty propertyName="Value" typeName="int" />
                </ECEntityClass>
                <ECEntityClass typeName="DerivedA">
                    <BaseClass>Base</BaseClass>
                    <ECProperty propertyName="ExtraA" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="DerivedB">
                    <BaseClass>Base</BaseClass>
                    <ECProperty propertyName="ExtraB" typeName="double" />
                </ECEntityClass>
            </ECSchema>)xml")));

    // Insert enough rows to keep the main thread stepping for a while
    const int rowCount = 200;
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.DerivedA(Name,[Value],ExtraA) VALUES(?,?,?)"));
        for (int i = 0; i < rowCount; ++i) {
            stmt.Reset();
            stmt.ClearBindings();
            stmt.BindText(1, SqlPrintfString("NameA_%d", i).GetUtf8CP(), IECSqlBinder::MakeCopy::Yes);
            stmt.BindInt(2, i);
            stmt.BindText(3, SqlPrintfString("ExtraA_%d", i).GetUtf8CP(), IECSqlBinder::MakeCopy::Yes);
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
        }
    }
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.DerivedB(Name,[Value],ExtraB) VALUES(?,?,?)"));
        for (int i = 0; i < rowCount; ++i) {
            stmt.Reset();
            stmt.ClearBindings();
            stmt.BindText(1, SqlPrintfString("NameB_%d", i).GetUtf8CP(), IECSqlBinder::MakeCopy::Yes);
            stmt.BindInt(2, i + rowCount);
            stmt.BindDouble(3, i * 1.5);
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
        }
    }
    m_ecdb.SaveChanges();

    // Thread A: main thread steps through a polymorphic query using "$" syntax
    // which triggers extract_inst -> InstanceReader::Seek -> GetOrAddClass -> Dispatcher::GetIterable (needs ECDb mutex)
    // while holding the primary SQLite mutex (via sqlite3_step).
    //
    // Thread B: concurrent query workers prepare ECSQL on the same polymorphic class
    // which, before the fix, would hold the ECDb mutex and then need the primary SQLite
    // mutex for schema resolution.

    std::atomic<bool> mainThreadDone{false};
    std::atomic<int> mainThreadRows{0};
    std::atomic<bool> mainThreadPrepareFailed{false};

    // Start the concurrent query manager
    ConcurrentQueryMgr::WithInstance(m_ecdb, [&](auto& mgr) {
        // Launch a thread that simulates "Thread A" - stepping through extract_inst on main connection
        auto mainStepThread = std::thread([&]() {
            ECSqlStatement stmt;
            // SELECT $ triggers extract_inst for each row (polymorphic - reads full instance)
            auto status = stmt.Prepare(m_ecdb, "SELECT $ FROM ts.Base");
            if (status != ECSqlStatus::Success) {
                mainThreadPrepareFailed = true;
                mainThreadDone = true;
                return;
            }
            while (stmt.Step() == BE_SQLITE_ROW) {
                mainThreadRows++;
                // Small yield to increase interleaving chances
                if (mainThreadRows % 10 == 0)
                    std::this_thread::yield();
            }
            mainThreadDone = true;
        });

        // Meanwhile, enqueue multiple concurrent queries that require fresh prepare
        // (use unique queries to avoid cache hits and force prepare on worker threads)
        const int numConcurrentQueries = 20;
        std::vector<QueryResponse::Future> futures;
        for (int i = 0; i < numConcurrentQueries; ++i) {
            // Each query is slightly different to bypass the statement cache and force a Prepare
            auto ecsql = SqlPrintfString("SELECT Name, [Value] FROM ts.Base WHERE [Value] >= %d", i);
            auto req = ECSqlRequest::MakeRequest(ecsql.GetUtf8CP());
            futures.push_back(mgr.Enqueue(std::move(req)));
        }

        // Also enqueue polymorphic $ queries on the worker to maximize contention
        for (int i = 0; i < numConcurrentQueries; ++i) {
            auto ecsql = SqlPrintfString("SELECT $ FROM ts.Base WHERE [Value] >= %d", i);
            auto req = ECSqlRequest::MakeRequest(ecsql.GetUtf8CP());
            futures.push_back(mgr.Enqueue(std::move(req)));
        }

        // Watchdog: if the deadlock regresses, the worker threads stay stuck holding locks, so
        // mainStepThread.join() (and the result collection below) would block forever. The only
        // signal would then be the whole test run timing out -- slow and easy to blame on the wrong
        // thing. The watchdog instead fails fast with a clear message. abort() is heavy-handed, but
        // stuck threads can't be joined, so it is the only way out. On the success path testCompleted
        // is set at the end of this block, after which the watchdog exits cleanly and is joined.
        std::atomic<bool> testCompleted{false};
        auto watchdog = std::thread([&]() {
            const auto deadline = std::chrono::steady_clock::now() + 60s;
            while (!testCompleted.load()) {
                if (std::chrono::steady_clock::now() > deadline) {
                    fprintf(stderr, "DEADLOCK DETECTED: main step vs worker prepare did not "
                                    "complete within 60s (itwinjs-backlog#2113 regression).\n");
                    fflush(stderr);
                    std::abort();
                }
                std::this_thread::sleep_for(100ms);
            }
        });

        // Wait for the main step thread to finish. If the deadlock regresses, this join blocks
        // until the watchdog above aborts the process with a clear diagnostic.
        mainStepThread.join();

        // Collect all concurrent query results. Future::Get() blocks until each request
        // completes; a regressed deadlock surfaces here (or in the join above) as a hang.
        for (auto& future : futures) {
            auto response = future.Get();
            // All queries should succeed (not deadlock/timeout)
            EXPECT_TRUE(response->GetStatus() == QueryResponse::Status::Done ||
                        response->GetStatus() == QueryResponse::Status::Partial)
                << "Concurrent query failed with status: " << (int)response->GetStatus()
                << " error: " << response->GetError();
        }

        // Verify main thread completed and read all rows
        EXPECT_TRUE(mainThreadDone.load());
        EXPECT_FALSE(mainThreadPrepareFailed);
        EXPECT_EQ(mainThreadRows.load(), rowCount * 2);

        // No deadlock: disarm the watchdog and join it cleanly while testCompleted is still in scope.
        testCompleted = true;
        watchdog.join();
    });
}

//---------------------------------------------------------------------------------------
// @bsimethod
// Worker prepares resolve schemas against a dedicated, shared schema-source connection
// (QueryAdaptorCache::TryGet) rather than each worker's own cold cache. This exercises that path
// under concurrency: many distinct queries (each forcing a fresh prepare across the worker pool)
// must return correct results without deadlocking, and a genuinely invalid ECSQL must still
// surface a prepare error via the worker's-own-connection fallback instead of hanging or being
// silently dropped.
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConcurrentQueryFixture, SharedSchemaSource_ConcurrentPrepareCorrectness) {
    auto testSchema = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="Foo">
                <ECProperty propertyName="I" typeName="int" />
                <ECProperty propertyName="S" typeName="string" />
            </ECEntityClass>
        </ECSchema>)xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("ConcurrentQuery_SharedSchemaSource.ecdb", testSchema));

    const int rowCount = 100;
    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Foo(I,S) VALUES(?,?)"));
        for (int i = 0; i < rowCount; ++i) {
            stmt.Reset();
            stmt.ClearBindings();
            stmt.BindInt(1, i);
            stmt.BindText(2, SqlPrintfString("S_%d", i).GetUtf8CP(), IECSqlBinder::MakeCopy::Yes);
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
        }
    }
    m_ecdb.SaveChanges();

    ConcurrentQueryMgr::WithInstance(m_ecdb, [&](auto& mgr) {
        // Correctness: distinct queries prepared via the shared schema-source connection return the
        // expected rows (WHERE I >= threshold over [0, rowCount) yields rowCount - threshold rows).
        for (int threshold : {0, 25, 50, 99}) {
            ECSqlReader reader(mgr, SqlPrintfString("SELECT I FROM ts.Foo WHERE I >= %d", threshold).GetUtf8CP());
            int count = 0;
            while (reader.Next())
                ++count;
            EXPECT_EQ(count, rowCount - threshold) << "threshold=" << threshold;
        }

        // Concurrency: many distinct queries forcing fresh prepares across the worker pool must all
        // succeed and must not deadlock.
        std::vector<QueryResponse::Future> futures;
        for (int i = 0; i < 50; ++i)
            futures.push_back(mgr.Enqueue(ECSqlRequest::MakeRequest(SqlPrintfString("SELECT S FROM ts.Foo WHERE I = %d", i).GetUtf8CP())));
        for (auto& f : futures) {
            auto response = f.Get();
            EXPECT_TRUE(response->IsSuccess())
                << "status: " << (int)response->GetStatus() << " error: " << response->GetError();
        }

        // Fallback error path: a genuinely invalid ECSQL fails to prepare against the shared
        // schema-source connection and then against the worker's own connection, surfacing a prepare
        // error (rather than hanging or returning success).
        auto badResponse = mgr.Enqueue(ECSqlRequest::MakeRequest("SELECT DoesNotExist FROM ts.Foo")).Get();
        EXPECT_TRUE(badResponse->IsError());
        EXPECT_GE((int)badResponse->GetStatus(), (int)QueryResponse::Status::Error);
    });
}

//---------------------------------------------------------------------------------------
// @bsimethod
// The shared schema-source connection caches schemas across worker prepares. Its cache is never
// cleared mid-flight; instead, any schema change on the primary routes through
// ECDb::ClearECDbCache -> ConcurrentQueryMgr::Shutdown, which tears down (and later rebuilds) the
// whole connection cache including the schema-source connection. This test pins down that invariant:
// after importing a new schema, a worker query against the newly added class must succeed, proving
// the schema-source connection was rebuilt and is not serving a stale cache.
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConcurrentQueryFixture, SharedSchemaSource_SchemaChangeInvalidates) {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("ConcurrentQuery_SchemaChange.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="Foo">
                <ECProperty propertyName="I" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml")));

    // Warm the shared schema-source cache with the original schema, and confirm a class that does not
    // exist yet fails to prepare (against the schema-source connection and the worker fallback).
    ConcurrentQueryMgr::WithInstance(m_ecdb, [&](auto& mgr) {
        EXPECT_TRUE(mgr.Enqueue(ECSqlRequest::MakeRequest("SELECT I FROM ts.Foo")).Get()->IsSuccess());
        EXPECT_TRUE(mgr.Enqueue(ECSqlRequest::MakeRequest("SELECT K FROM ts2.Bar")).Get()->IsError());
    });

    // Import a new schema. This routes through ECDb::ClearECDbCache -> ConcurrentQueryMgr::Shutdown,
    // which tears down the (now stale) shared schema-source connection.
    ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema2" alias="ts2" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="Bar">
                <ECProperty propertyName="K" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml")));
    m_ecdb.SaveChanges();

    // The rebuilt schema-source connection must reflect the new schema: a worker query against the new
    // class now prepares and runs (it would still fail if the shared cache were stale).
    ConcurrentQueryMgr::WithInstance(m_ecdb, [&](auto& mgr) {
        auto response = mgr.Enqueue(ECSqlRequest::MakeRequest("SELECT K FROM ts2.Bar")).Get();
        EXPECT_TRUE(response->IsSuccess())
            << "status: " << (int)response->GetStatus() << " error: " << response->GetError();
    });
}

//---------------------------------------------------------------------------------------
// @bsimethod
// Primary-connection requests must be executed synchronously on the caller thread (see
// RunnableRequestQueue::Enqueue -> ExecuteSynchronously). They must NOT be dispatched to a
// worker thread, because a worker preparing/executing against the primary connection while
// holding the primary ECDb mutex reintroduces the worker/main-thread deadlock guarded against
// in QueryExecutor's worker loop. This test pins down that routing invariant.
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConcurrentQueryFixture, PrimaryConnectionRequestRunsOnCallerThread) {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("primary_conn_thread.ecdb"));
    m_ecdb.AddFunction(ThreadIdFunc::Instance());

    const auto callerThreadId = std::this_thread::get_id();

    ConcurrentQueryMgr::WithInstance(m_ecdb, [&](auto& mgr) {
        // Primary-connection request: runs synchronously on the caller thread, never queued.
        ThreadIdFunc::Instance().Reset();
        {
            auto req = ECSqlRequest::MakeRequest("WITH cnt(x) AS (VALUES(1)) SELECT imodel_thread_id() FROM cnt");
            req->SetUsePrimaryConnection(true);
            auto r = mgr.Enqueue(std::move(req)).Get();
            ASSERT_EQ(r->GetStatus(), QueryResponse::Status::Done);
            ASSERT_TRUE(ThreadIdFunc::Instance().Invoked());
            EXPECT_EQ(ThreadIdFunc::Instance().LastThreadId(), callerThreadId)
                << "primary-connection request must execute on the caller thread, not a worker thread";
        }

        // Non-primary request: dispatched to the worker queue and executed on a worker thread.
        ThreadIdFunc::Instance().Reset();
        {
            auto req = ECSqlRequest::MakeRequest("WITH cnt(x) AS (VALUES(1)) SELECT imodel_thread_id() FROM cnt");
            ASSERT_FALSE(req->UsePrimaryConnection()); // default routes to a worker thread
            auto r = mgr.Enqueue(std::move(req)).Get();
            ASSERT_EQ(r->GetStatus(), QueryResponse::Status::Done);
            ASSERT_TRUE(ThreadIdFunc::Instance().Invoked());
            EXPECT_NE(ThreadIdFunc::Instance().LastThreadId(), callerThreadId)
                << "non-primary request must execute on a worker thread";
        }
    });

    m_ecdb.RemoveFunction(ThreadIdFunc::Instance());
}

//---------------------------------------------------------------------------------------
// @bsimethod
// Regression test: calling Future::Cancel() after the query has already completed must be a safe
// no-op. The RunnableRequest backing the query is destroyed as soon as it completes
// (CachedConnection::ClearRequest), but the Future (and its cancel callback) can outlive it. The
// cancel callback must not dereference the freed request - it should look the request up by id and
// find it already gone. Before the fix the callback captured the request by reference, so this was
// a use-after-free.
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConcurrentQueryFixture, CancelAfterCompletionIsSafe) {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("cancel_after_complete.ecdb"));
    ConcurrentQueryMgr::WithInstance(m_ecdb, [&](auto& mgr) {
        auto future = mgr.Enqueue(ECSqlRequest::MakeRequest("WITH cnt(x) AS (VALUES(1)) SELECT x FROM cnt"));
        auto r = future.Get(); // block until the request completes
        ASSERT_EQ(r->GetStatus(), QueryResponse::Status::Done);
        // Give the worker thread time to run ClearRequest() (which destroys the RunnableRequest)
        // before we cancel, so this actually exercises the post-destruction path. The cancel
        // callback captures the request id and queue pointer by value -- never the request -- so
        // CancelRequest(id) stays a safe no-op once the request has left the queue.
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        future.Cancel();
    });
}

END_ECDBUNITTESTS_NAMESPACE
