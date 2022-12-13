/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC
#include <ECDb/ConcurrentQueryManager.h>

BEGIN_ECDBUNITTESTS_NAMESPACE

struct InstanceReaderFixture : ECDbTestFixture {};

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceReaderFixture, ecsql_read_instance) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("instanceReader.ecdb"));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, R"sql(
        SELECT $ FROM meta.ECClassDef WHERE Description='Relates the property to its PropertyCategory.'
    )sql"));

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    printf("%s\n\n", stmt.GetNativeSql());
    printf("%s\n\n", stmt.GetValueText(0));

}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceReaderFixture, ecsql_read_property) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("instanceReader.ecdb"));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, R"sql(
        SELECT $ -> name FROM meta.ECClassDef WHERE Description='Relates the property to its PropertyCategory.'
    )sql"));

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    printf("%s\n\n", stmt.GetNativeSql());
    printf("%s\n\n", stmt.GetValueText(0));

}
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceReaderFixture, instance_reader) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("instanceReader.ecdb"));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, R"sql(
        SELECT ECClassId, ECInstanceId, EXTRACT_INST('meta.ecClassDef',ECInstanceId) FROM meta.ECClassDef WHERE Description='Relates the property to its PropertyCategory.'
    )sql"));

    BeJsDocument doc;
    doc.Parse(R"json({
        "ECInstanceId": "0x2e",
        "ECClassId": "ECDbMeta.ECClassDef",
        "Schema": {
            "Id": "0x4",
            "RelECClassId": "ECDbMeta.SchemaOwnsClasses"
        },
        "Name": "PropertyHasCategory",
        "Description": "Relates the property to its PropertyCategory.",
        "Type": 1,
        "Modifier": 2,
        "RelationshipStrength": 0,
        "RelationshipStrengthDirection": 1
    })json");
    auto& reader = m_ecdb.GetInstanceReader();
    if(stmt.Step() == BE_SQLITE_ROW) {
        ECInstanceKey instanceKey (stmt.GetValueId<ECClassId>(0), stmt.GetValueId<ECInstanceId>(1));
        auto pos = InstanceReader::Position(stmt.GetValueId<ECInstanceId>(1), stmt.GetValueId<ECClassId>(0));
        ASSERT_EQ(true, reader.Seek(pos,[&](InstanceReader::IRowContext const& row){
            EXPECT_STRCASEEQ(doc.Stringify(StringifyFormat::Indented).c_str(), row.GetJson().Stringify(StringifyFormat::Indented).c_str());
        }));
       
        
        //printf("%s\n", reader.ToJsonString(InstanceReader::JsonParams().SetIndent(true)).c_str());
    }
}
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceReaderFixture, extract_prop) {
    ASSERT_EQ(SUCCESS, SetupECDb("PROP_EXISTS.ecdb", SchemaItem(
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
               </ECSchema>)xml")));
    m_ecdb.SaveChanges();

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
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    const auto sql =
        "SELECT                                        "
        "   EXTRACT_INST(ecClassId, ecInstanceId),"
        "   EXTRACT_PROP(ecClassId, ecInstanceId, 's'  ),"
        "   EXTRACT_PROP(ecClassId, ecInstanceId, 'i'  ),"
        "   EXTRACT_PROP(ecClassId, ecInstanceId, 'd'  ),"
        "   EXTRACT_PROP(ecClassId, ecInstanceId, 'p2d'),"
        "   EXTRACT_PROP(ecClassId, ecInstanceId, 'p3d'),"
        "   EXTRACT_PROP(ecClassId, ecInstanceId, 'bi' ),"
        "   EXTRACT_PROP(ecClassId, ecInstanceId, 'l'  ),"
        "   EXTRACT_PROP(ecClassId, ecInstanceId, 'dt' ),"
        "   EXTRACT_PROP(ecClassId, ecInstanceId, 'b'  ) "
        "FROM ts.P                                     ";
    
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, sql));
    ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);

    int i = 1;
    ASSERT_STRCASEEQ(stmt.GetValueText(i++), kText);
    ASSERT_EQ(stmt.GetValueInt(i++), kI);
    ASSERT_EQ(stmt.GetValueDouble(i++), kD);
    ASSERT_STRCASEEQ(stmt.GetValueText(i++), "{\"X\":2.0,\"Y\":4.0}"); // return as json
    ASSERT_STRCASEEQ(stmt.GetValueText(i++), "{\"X\":4.0,\"Y\":5.0,\"Z\":6.0}"); // return as json
    ASSERT_EQ(memcmp(stmt.GetValueBlob(i++), &kBi[0], kBiLen), 0);
    ASSERT_EQ(stmt.GetValueInt64(i++), kL);
    ASSERT_TRUE(stmt.GetValueDateTime (i++).Equals(kDt, true));
    ASSERT_EQ(stmt.GetValueBoolean(i++), kB);

}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceReaderFixture, prop_exists) {
    ASSERT_EQ(SUCCESS, SetupECDb("PROP_EXISTS.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                   <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                   <ECEntityClass typeName="Base" modifier="Abstract">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00"/>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Prop1" typeName="string" />
                        <ECProperty propertyName="Prop2" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub">
                      <BaseClass>Base</BaseClass>
                      <ECProperty propertyName="SubProp1" typeName="string" />
                      <ECProperty propertyName="SubProp2" typeName="int" />
                     </ECEntityClass>
               </ECSchema>)xml")));
    m_ecdb.SaveChanges();

    if ("case sensitive check") {
        const auto sql =
            "SELECT "
            "   PROP_EXISTS(ec_classid('ts.Base'), 'ECClassId'   ),"
            "   PROP_EXISTS(ec_classid('ts.Base'), 'ECInstanceId')," 
            "   PROP_EXISTS(ec_classid('ts.Base'), 'Prop1'       ),"
            "   PROP_EXISTS(ec_classId('ts.base'), 'Prop2'       )," 
            "   PROP_EXISTS(ec_classid('ts.Base'), 'SubProp1'    ),"
            "   PROP_EXISTS(ec_classid('ts.Base'), 'SubProp2'    ),"
            "   PROP_EXISTS(ec_classid('ts.Sub'),  'ECClassId'   )," 
            "   PROP_EXISTS(ec_classid('ts.Sub'),  'ECInstanceId'),"
            "   PROP_EXISTS(ec_classid('ts.Sub'),  'Prop1'       ),"
            "   PROP_EXISTS(ec_classId('ts.Sub'),  'Prop2'       ),"
            "   PROP_EXISTS(ec_classid('ts.Sub'),  'SubProp1'    ),"
            "   PROP_EXISTS(ec_classid('ts.Sub'),  'SubProp2'    )";

        ECSqlStatement stmt;   
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, sql));

        ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);

        int i = 0;
        ASSERT_EQ(stmt.GetValueInt(i++), 1);
        ASSERT_EQ(stmt.GetValueInt(i++), 1);
        ASSERT_EQ(stmt.GetValueInt(i++), 1);
        ASSERT_EQ(stmt.GetValueInt(i++), 1);
        ASSERT_EQ(stmt.GetValueInt(i++), 0); // SubProp1 does not exist in base
        ASSERT_EQ(stmt.GetValueInt(i++), 0); // SubProp2 does not exist in base
        ASSERT_EQ(stmt.GetValueInt(i++), 1);
        ASSERT_EQ(stmt.GetValueInt(i++), 1);
        ASSERT_EQ(stmt.GetValueInt(i++), 1);
        ASSERT_EQ(stmt.GetValueInt(i++), 1);
        ASSERT_EQ(stmt.GetValueInt(i++), 1);
        ASSERT_EQ(stmt.GetValueInt(i++), 1);
    }

    if ("case insensitive check") {
        const auto sql =
            "SELECT "
            "   PROP_EXISTS(ec_classid('ts.Base'), 'ECCLASSID'   ),"
            "   PROP_EXISTS(ec_classid('ts.Base'), 'ECINSTANCEID')," 
            "   PROP_EXISTS(ec_classid('ts.Base'), 'PROP1'       ),"
            "   PROP_EXISTS(ec_classId('ts.base'), 'PROP2'       )," 
            "   PROP_EXISTS(ec_classid('ts.Base'), 'SUBPROP1'    ),"
            "   PROP_EXISTS(ec_classid('ts.Base'), 'SUBPROP2'    ),"
            "   PROP_EXISTS(ec_classid('ts.Sub'),  'ECCLASSID'   )," 
            "   PROP_EXISTS(ec_classid('ts.Sub'),  'ECINSTANCEID'),"
            "   PROP_EXISTS(ec_classid('ts.Sub'),  'PROP1'       ),"
            "   PROP_EXISTS(ec_classId('ts.Sub'),  'PROP2'       ),"
            "   PROP_EXISTS(ec_classid('ts.Sub'),  'SUBPROP1'    ),"
            "   PROP_EXISTS(ec_classid('ts.Sub'),  'SUBPROP2'    )";
            
        ECSqlStatement stmt;   
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, sql));
        ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);

        int i = 0;
        ASSERT_EQ(stmt.GetValueInt(i++), 1);
        ASSERT_EQ(stmt.GetValueInt(i++), 1);
        ASSERT_EQ(stmt.GetValueInt(i++), 1);
        ASSERT_EQ(stmt.GetValueInt(i++), 1);
        ASSERT_EQ(stmt.GetValueInt(i++), 0); // SubProp1 does not exist in base
        ASSERT_EQ(stmt.GetValueInt(i++), 0); // SubProp2 does not exist in base
        ASSERT_EQ(stmt.GetValueInt(i++), 1);
        ASSERT_EQ(stmt.GetValueInt(i++), 1);
        ASSERT_EQ(stmt.GetValueInt(i++), 1);
        ASSERT_EQ(stmt.GetValueInt(i++), 1);
        ASSERT_EQ(stmt.GetValueInt(i++), 1);
        ASSERT_EQ(stmt.GetValueInt(i++), 1);
    }
}
END_ECDBUNITTESTS_NAMESPACE
