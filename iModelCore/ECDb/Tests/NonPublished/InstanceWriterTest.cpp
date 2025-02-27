/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE
#define  PRINT_JSON(NAME, JSON)     printf(## NAME ## ": %s\n", JSON.Stringify(StringifyFormat::Indented).c_str());

struct InstanceWriterFixture : ECDbTestFixture {
    static std::optional<Utf8String> ReadInstance(ECDbCR ecdb, const ECInstanceKey& key, bool useJsName = false) {
        BeJsDocument doc;
        InstanceReader::Position pos(key.GetInstanceId(), key.GetClassId());
        if (!ecdb.GetInstanceReader().Seek(pos, [&](const InstanceReader::IRowContext& row) {
                doc.From(row.GetJson(InstanceReader::JsonParams().SetAbbreviateBlobs(false).SetUseJsName(useJsName)));
            })) {
            return std::nullopt;
        }
        return doc.Stringify(StringifyFormat::Indented);
    };
    static DbResult InsertInstance(ECDbR ecdb, BeJsConst instance, InstanceWriter::InsertOptions opts, ECInstanceKey& key) {
        return ecdb.GetInstanceWriter().Insert(instance, opts, key);
    }
    static DbResult InsertInstance(ECDbR ecdb, BeJsConst instance, InstanceWriter::InsertOptions opts) {
        return ecdb.GetInstanceWriter().Insert(instance, opts);
    }
};
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceWriterFixture, basic) {
    auto schema = SchemaItem(
        R"xml(<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                   <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                   <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA" />
                    <ECEntityClass typeName="P">
                        <ECProperty propertyName="S"    typeName="string"  />
                        <ECProperty propertyName="I"    typeName="int"     />
                        <ECProperty propertyName="D"    typeName="double"  />
                        <ECProperty propertyName="P2d"  typeName="point2d" />
                        <ECProperty propertyName="P3d"  typeName="point3d" />
                        <ECProperty propertyName="Bi"   typeName="binary"  />
                        <ECProperty propertyName="L"    typeName="long"    />
                        <ECProperty propertyName="DT"   typeName="dateTime">
                            <ECCustomAttributes>
                                <DateTimeInfo xmlns="CoreCustomAttributes.01.00.00">
                                    <DateTimeKind>Utc</DateTimeKind>
                                </DateTimeInfo>
                            </ECCustomAttributes>
                        </ECProperty>
                        <ECProperty propertyName="B"    typeName="boolean" />
                    </ECEntityClass>
               </ECSchema>)xml");

    ASSERT_EQ(SUCCESS, SetupECDb("first.ecdb", schema));
    m_ecdb.SaveChanges();

    ECDb writeDb;
    ASSERT_EQ(BE_SQLITE_OK, CloneECDb(writeDb, "second.ecdb", BeFileName(m_ecdb.GetDbFileName()), ECDb::OpenParams(ECDb::OpenMode::ReadWrite)));

    // sample primitive type
    const bool kB = true;
    const DateTime kDt = DateTime::FromString("2025-02-27T19:35:15.672Z");
    const double kD = PI;
    const DPoint2d kP2d = DPoint2d::From(2341.34, -4.3322);
    const DPoint3d kP3d = DPoint3d::From(1.2344, -5.3322, -0.0001);
    const int kBiLen = 10;
    const int kI = 0x3432;
    const int64_t kL = 0xfefefefefefefefe;
    const uint8_t kBi[kBiLen] = {0x71, 0xdd, 0x83, 0x7d, 0x0b, 0xf2, 0x50, 0x01, 0x0a, 0xe1};
    const char* kText = "Hello, World";

    ECInstanceKey expectedKey;
    if ("insert a row with primitive value") {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success,
                  stmt.Prepare(m_ecdb, "INSERT INTO ts.P(s,i,d,p2d,p3d,bi,l,dt,b) VALUES(?,?,?,?,?,?,?,?,?)"));

        // bind primitive types
        stmt.BindText(1, kText, IECSqlBinder::MakeCopy::No);
        stmt.BindInt(2, kI);
        stmt.BindDouble(3, kD);
        stmt.BindPoint2d(4, kP2d);
        stmt.BindPoint3d(5, kP3d);
        stmt.BindBlob(6, &kBi[0], kBiLen, IECSqlBinder::MakeCopy::No);
        stmt.BindInt64(7, kL);
        stmt.BindDateTime(8, kDt);
        stmt.BindBoolean(9, kB);
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(expectedKey));
        m_ecdb.SaveChanges();
    }

    BeJsDocument expected;

    auto expectedJson = ReadInstance(m_ecdb, expectedKey);
    ASSERT_TRUE(expectedJson.has_value());
    expected.Parse(expectedJson.value());

    if ("copy instance to writeDb") {
        ECInstanceKey actualKey;
        ASSERT_EQ( BE_SQLITE_DONE, InsertInstance(writeDb, expected, InstanceWriter::InsertOptions().UseInstanceIdFromJs(), actualKey));
        writeDb.SaveChanges();
        BeJsDocument actual;
        auto actualJson = ReadInstance(writeDb, actualKey);
        ASSERT_TRUE(actualJson.has_value());
        actual.Parse(actualJson.value());
        ASSERT_EQ(expectedKey, actualKey);
        ASSERT_STREQ(expectedJson.value().c_str(), actualJson.value().c_str());
    }

    if ("insert a js instance") {
        // copy instance to another db use ECSql standard format
        Utf8String testInst = R"json({
            "id": "0x1234",
            "className": "TestSchema.P",
            "s": "What is this?",
            "i": -223,
            "d": -3.141592653589793,
            "p2d": {
                "x": 341.34,
                "y": -4.322
            },
            "p3d": {
                "x": 11.2344,
                "y": -15.3322,
                "z": -20.0001
            },
            "bi": "encoding=base64;cd2DfQvyUAEK4Q==",
            "l": 100000,
            "dT": "2027-02-27T19:35:15.672Z",
            "b": false
        })json";

        BeJsDocument testDoc;
        ECInstanceKey actualKey;
        testDoc.Parse(testInst);
        auto opt = InstanceWriter::InsertOptions();
        opt.UseInstanceIdFromJs();
        opt.UseJsName(true);

        ASSERT_EQ( BE_SQLITE_DONE, InsertInstance(writeDb, testDoc, opt, actualKey));
        writeDb.SaveChanges();
        BeJsDocument actual;
        auto actualJson = ReadInstance(writeDb, actualKey, true);
        ASSERT_TRUE(actualJson.has_value());
        actual.Parse(actualJson.value());
        ASSERT_STREQ(testDoc.Stringify(StringifyFormat::Indented).c_str(), actual.Stringify(StringifyFormat::Indented).c_str());
    }

}

END_ECDBUNITTESTS_NAMESPACE
