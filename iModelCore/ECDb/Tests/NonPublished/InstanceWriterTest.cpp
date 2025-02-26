/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"



USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

struct InstanceWriterFixture : ECDbTestFixture {};
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceWriterFixture, basic) {
    auto schema = SchemaItem(
        R"xml(<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                   <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                   <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA" />
                    <ECEntityClass typeName="P">
                        <ECProperty propertyName="s"    typeName="string"  />
                        <ECProperty propertyName="i"    typeName="int"     />
                        <ECProperty propertyName="d"    typeName="double"  />
                        <ECProperty propertyName="p2d"  typeName="point2d" />
                        <ECProperty propertyName="p3d"  typeName="point3d" />
                        <ECProperty propertyName="bi"   typeName="binary"  />
                        <ECProperty propertyName="l"    typeName="long"    />
                        <ECProperty propertyName="dt"   typeName="dateTime">
                            <ECCustomAttributes>
                                <DateTimeInfo xmlns="CoreCustomAttributes.01.00.00">
                                    <DateTimeKind>Utc</DateTimeKind>
                                </DateTimeInfo>
                            </ECCustomAttributes>
                        </ECProperty>
                        <ECProperty propertyName="b"    typeName="boolean" />
                    </ECEntityClass>
               </ECSchema>)xml");

    ASSERT_EQ(SUCCESS, SetupECDb("first.ecdb", schema));
    m_ecdb.SaveChanges();

    ECDb secondDb;
    ASSERT_EQ(BE_SQLITE_OK, CloneECDb(secondDb, "second.ecdb", BeFileName(m_ecdb.GetDbFileName()), ECDb::OpenParams(ECDb::OpenMode::ReadWrite)));

     // sample primitive type
     const bool kB = true;
     const DateTime kDt = DateTime::GetCurrentTimeUtc();
     const double kD = 3.13;
     const DPoint2d kP2d = DPoint2d::From(2, 4);
     const DPoint3d kP3d = DPoint3d::From(4, 5, 6);
     const int kBiLen = 10;
     const int kI = 0x3432;
     const int64_t kL = 0xfefefefefefefefe;
     const uint8_t kBi[kBiLen] = {0x71, 0xdd, 0x83, 0x7d, 0x0b, 0xf2, 0x50, 0x01, 0x0a, 0xe1};
     const char* kText = "Hello, World";

     ECInstanceKey key;
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
         ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
         m_ecdb.SaveChanges();
    }

    // copy instance to another db use ECSql standard format
    InstanceWriter writer(secondDb);
    if ("copy a row from first to second") {
        BeJsDocument val;
        InstanceReader::Position pos(key.GetInstanceId(), key.GetClassId());
        m_ecdb.GetInstanceReader().Seek(pos, [&](const InstanceReader::IRowContext& row) {
            val.From(row.GetJson());
        });

        ASSERT_EQ(BE_SQLITE_DONE, writer.Insert(val, InstanceWriter::InsertOptions()));
        secondDb.SaveChanges();
    }
}

END_ECDBUNITTESTS_NAMESPACE
