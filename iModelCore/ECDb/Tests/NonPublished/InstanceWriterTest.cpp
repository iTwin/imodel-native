/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <ctime>
#include <random>

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE
#define PRINT_JSON(NAME, JSON) printf(##NAME##": %s\n", JSON.Stringify(StringifyFormat::Indented).c_str());

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
    static DbResult UpdateInstance(ECDbR ecdb, BeJsConst instance, InstanceWriter::UpdateOptions opts) {
        return ecdb.GetInstanceWriter().Update(instance, opts);
    }
};

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceWriterFixture, complex) {
    auto schema = SchemaItem(
        R"xml(<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                   <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                   <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA" />
                    <ECStructClass typeName="ST" >
                        <ECProperty propertyName="S"       typeName="string"  />
                        <ECProperty propertyName="Json"    typeName="string"  extendedTypeName="Json"   />
                        <ECProperty propertyName="G"       typeName="binary"  extendedTypeName="BeGuid" />
                        <ECProperty propertyName="I"       typeName="int"     />
                        <ECProperty propertyName="D"       typeName="double"  />
                        <ECProperty propertyName="P2d"     typeName="point2d" />
                        <ECProperty propertyName="P3d"     typeName="point3d" />
                        <ECProperty propertyName="Bi"      typeName="binary"  />
                        <ECProperty propertyName="L"       typeName="long"    />
                        <ECProperty propertyName="Geom"    typeName="Bentley.Geometry.Common.IGeometry" />
                        <ECProperty propertyName="B"       typeName="boolean" />
                        <ECProperty propertyName="Dt"      typeName="dateTime" />
                        <ECProperty propertyName="DtUtc"   typeName="dateTime">
                            <ECCustomAttributes>
                                <DateTimeInfo xmlns="CoreCustomAttributes.01.00.00">
                                    <DateTimeKind>Utc</DateTimeKind>
                                </DateTimeInfo>
                            </ECCustomAttributes>
                        </ECProperty>
                        <ECArrayProperty propertyName="AS"       typeName="string"  />
                        <ECArrayProperty propertyName="AJson"    typeName="string"  extendedTypeName="Json"   />
                        <ECArrayProperty propertyName="AG"       typeName="binary"  extendedTypeName="BeGuid" />
                        <ECArrayProperty propertyName="AI"       typeName="int"     />
                        <ECArrayProperty propertyName="AD"       typeName="double"  />
                        <ECArrayProperty propertyName="AP2d"     typeName="point2d" />
                        <ECArrayProperty propertyName="AP3d"     typeName="point3d" />
                        <ECArrayProperty propertyName="ABi"      typeName="binary"  />
                        <ECArrayProperty propertyName="AL"       typeName="long"    />
                        <ECArrayProperty propertyName="AGeom"    typeName="Bentley.Geometry.Common.IGeometry" />
                        <ECArrayProperty propertyName="AB"       typeName="boolean" />
                        <ECArrayProperty propertyName="ADt"      typeName="dateTime" />
                        <ECArrayProperty propertyName="ADtUtc"   typeName="dateTime">
                            <ECCustomAttributes>
                                <DateTimeInfo xmlns="CoreCustomAttributes.01.00.00">
                                    <DateTimeKind>Utc</DateTimeKind>
                                </DateTimeInfo>
                            </ECCustomAttributes>
                        </ECArrayProperty>
                    </ECStructClass>
                    <ECEntityClass typeName="P">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.2.0">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ClassHasCurrentTimeStampProperty xmlns="CoreCustomAttributes.1.0">
                                <PropertyName>LastMod</PropertyName>
                            </ClassHasCurrentTimeStampProperty>
                        </ECCustomAttributes>
                        <ECProperty propertyName="S"       typeName="string"  />
                        <ECProperty propertyName="Json"    typeName="string"  extendedTypeName="Json"   />
                        <ECProperty propertyName="G"       typeName="binary"  extendedTypeName="BeGuid" />
                        <ECProperty propertyName="I"       typeName="int"     />
                        <ECProperty propertyName="D"       typeName="double"  />
                        <ECProperty propertyName="P2d"     typeName="point2d" />
                        <ECProperty propertyName="P3d"     typeName="point3d" />
                        <ECProperty propertyName="Bi"      typeName="binary"  />
                        <ECProperty propertyName="L"       typeName="long"    />
                        <ECProperty propertyName="Geom"    typeName="Bentley.Geometry.Common.IGeometry" />
                        <ECProperty propertyName="B"       typeName="boolean" />
                        <ECProperty propertyName="Dt"      typeName="dateTime" />
                        <ECProperty propertyName="DtUtc"   typeName="dateTime">
                            <ECCustomAttributes>
                                <DateTimeInfo xmlns="CoreCustomAttributes.01.00.00">
                                    <DateTimeKind>Utc</DateTimeKind>
                                </DateTimeInfo>
                            </ECCustomAttributes>
                        </ECProperty>
                        <ECArrayProperty propertyName="AS"       typeName="string"  />
                        <ECArrayProperty propertyName="AJson"    typeName="string"  extendedTypeName="Json"   />
                        <ECArrayProperty propertyName="AG"       typeName="binary"  extendedTypeName="BeGuid" />
                        <ECArrayProperty propertyName="AI"       typeName="int"     />
                        <ECArrayProperty propertyName="AD"       typeName="double"  />
                        <ECArrayProperty propertyName="AP2d"     typeName="point2d" />
                        <ECArrayProperty propertyName="AP3d"     typeName="point3d" />
                        <ECArrayProperty propertyName="ABi"      typeName="binary"  />
                        <ECArrayProperty propertyName="AL"       typeName="long"    />
                        <ECArrayProperty propertyName="AGeom"    typeName="Bentley.Geometry.Common.IGeometry" />
                        <ECArrayProperty propertyName="AB"       typeName="boolean" />
                        <ECArrayProperty propertyName="ADt"      typeName="dateTime" />
                        <ECArrayProperty propertyName="ADtUtc"   typeName="dateTime">
                            <ECCustomAttributes>
                                <DateTimeInfo xmlns="CoreCustomAttributes.01.00.00">
                                    <DateTimeKind>Utc</DateTimeKind>
                                </DateTimeInfo>
                            </ECCustomAttributes>
                        </ECArrayProperty>
                        <ECStructProperty propertyName="Struct" typeName="ST" />
                        <ECStructArrayProperty propertyName="AStruct" typeName="ST" />
                        <ECProperty propertyName="LastMod" typeName="dateTime" readOnly="True">
                            <ECCustomAttributes>
                                <DateTimeInfo xmlns="CoreCustomAttributes.1.0">
                                    <DateTimeKind>Utc</DateTimeKind>
                                </DateTimeInfo>
                                <HiddenProperty xmlns="CoreCustomAttributes.1.0"/>
                            </ECCustomAttributes>
                        </ECProperty>
                        <ECNavigationProperty propertyName="Parent" relationshipName="POwnsChildPs" direction="Backward"/>
                    </ECEntityClass>
                    <ECRelationshipClass typeName="POwnsChildPs" strength="embedding" modifier="None">
                        <Source multiplicity="(0..1)" roleLabel="1" polymorphic="true">
                            <Class class="P"/>
                        </Source>
                        <Target multiplicity="(0..*)" roleLabel="2" polymorphic="true">
                            <Class class="P"/>
                        </Target>
                    </ECRelationshipClass>
                    <ECRelationshipClass typeName="PRefersToPs" strength="referencing" modifier="None">
                        <ECCustomAttributes>
                            <LinkTableRelationshipMap xmlns="ECDbMap.2.0">
                                <CreateForeignKeyConstraints>False</CreateForeignKeyConstraints>
                            </LinkTableRelationshipMap>
                            <ClassMap xmlns="ECDbMap.2.0">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <Source multiplicity="(0..*)" roleLabel="1"  polymorphic="true">
                            <Class class="P"/>
                        </Source>
                        <Target multiplicity="(0..*)"  roleLabel="2" polymorphic="true">
                            <Class class="P"/>
                        </Target>
                    </ECRelationshipClass>
               </ECSchema>)xml");

    ASSERT_EQ(SUCCESS, SetupECDb("first.ecdb", schema));
    m_ecdb.SaveChanges();
    ECDb writeDb;
    ASSERT_EQ(BE_SQLITE_OK, CloneECDb(writeDb, "second.ecdb", BeFileName(m_ecdb.GetDbFileName()), ECDb::OpenParams(ECDb::OpenMode::ReadWrite)));

    auto pInsertSql = R"sql(
        INSERT INTO [ts].[P](
            [S], [Json], [G], [I], [D], [P2d], [P3d], [Bi], [L], [Geom], [B], [Dt], [DtUtc],
            [AS], [AJson], [AG], [AI], [AD], [AP2d], [AP3d], [Abi], [AL], [AGeom], [AB], [ADt], [AdtUtc],
            [Struct],
            [AStruct],
            [Parent])
        VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?))sql";

    auto pInsertRel = R"sql(
        INSERT INTO [ts].[PRefersToPs]([SourceECInstanceId], [SourceECClassId], [TargetECInstanceId], [TargetECClassId]) VALUES(?, ?, ?, ?))sql";

    auto insertRel = [&](ECInstanceKeyCR parent, ECInstanceKeyCR child, ECInstanceKeyR out) {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, pInsertRel));
        stmt.BindId(1, parent.GetInstanceId());
        stmt.BindId(2, parent.GetClassId());
        stmt.BindId(3, child.GetInstanceId());
        stmt.BindId(4, child.GetClassId());
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(out));
        m_ecdb.SaveChanges();
    };
    std::mt19937_64 gen(0);
    auto bindJson = [&](IECSqlBinder& binder) {
        BeJsDocument json;
        std::uniform_int_distribution<int> distrib(0, 0xff);
        json.SetEmptyObject();
        auto v = distrib(gen);
        json["a"] = v;
        json["b"] = "Hello, World";
        return binder.BindText(json.Stringify().c_str(), IECSqlBinder::MakeCopy::Yes);
    };
    auto bindBlob = [&](IECSqlBinder& binder) {
        std::uniform_int_distribution<int> distrib(0, 0xff);
        std::vector<uint8_t> data;
        for (int i = 0; i < 10; i++) {
            auto x = (uint8_t)distrib(gen);
            data.push_back(x);
        }
        return binder.BindBlob((const void*)data.data(), (int)data.size(), IECSqlBinder::MakeCopy::Yes);
    };
    auto bindGeom = [&](IECSqlBinder& binder) {
        std::uniform_real_distribution<double> distrib(-100, 100);
        auto s0 = distrib(gen);
        auto s1 = distrib(gen);
        auto s2 = distrib(gen);
        auto e0 = distrib(gen);
        auto e1 = distrib(gen);
        auto e2 = distrib(gen);

        auto geom = IGeometry::Create(
            ICurvePrimitive::CreateLine(
                DSegment3d::From(s0, s1, s2, e0, e1, e2)));
        return binder.BindGeometry(*geom);
    };
    auto bindText = [&](IECSqlBinder& binder) {
        static auto words = std::vector<std::string>{"suitcase", "echo", "confusion", "contrary", "unity", "application", "talk", "frank", "brother", "particle", "encourage", "exact", "celebration", "prosper", "copyright", "publicity", "safety", "platform", "dress", "sugar", "philosophy"};

        std::uniform_int_distribution<int> distrib(0, (int)words.size() - 1);
        auto k = distrib(gen);
        return binder.BindText(words[k].c_str(), IECSqlBinder::MakeCopy::Yes);
    };
    auto bindGuid = [](IECSqlBinder& binder) {
        return binder.BindGuid(BeGuid(true), IECSqlBinder::MakeCopy::Yes);
    };
    auto bindInt = [&](IECSqlBinder& binder) {
        std::uniform_int_distribution<int> distrib(-1000000, 1000000);
        auto k = distrib(gen);
        return binder.BindInt(k);
    };
    auto bindLong = [&](IECSqlBinder& binder) {
        std::uniform_int_distribution<int> distrib(-1000000, 1000000);
        auto k = distrib(gen);
        return binder.BindInt(k);
    };
    auto bindDouble = [&](IECSqlBinder& binder) {
        std::uniform_real_distribution<double> distrib(-100000, 100000);
        auto k = distrib(gen);
        return binder.BindDouble(k);
    };
    auto bindPoint2d = [&](IECSqlBinder& binder) {
        std::uniform_real_distribution<double> distrib(-100, 100);
        auto x = distrib(gen);
        auto y = distrib(gen);
        return binder.BindPoint2d(DPoint2d::From(x, y));
    };
    auto bindPoint3d = [&](IECSqlBinder& binder) {
        std::uniform_real_distribution<double> distrib(-100, 100);
        auto x = distrib(gen);
        auto y = distrib(gen);
        auto z = distrib(gen);
        return binder.BindPoint3d(DPoint3d::From(x, y, z));
    };
    auto bindBoolean = [&](IECSqlBinder& binder) {
        std::uniform_int_distribution<int> distrib(0, 1);
        auto v = distrib(gen);
        return binder.BindBoolean(v == 1);
    };
    auto bindDateTime = [](IECSqlBinder& binder) {
        return binder.BindDateTime(DateTime::FromString("2021-02-27T19:35:15"));
    };
    auto bindDateTimeUtc = [](IECSqlBinder& binder) {
        return binder.BindDateTime(DateTime::FromString("2025-02-27T19:35:15.672Z"));
    };

    auto bindComplexInstance = [&](ECInstanceKeyR out, ECInstanceKeyCP parent) {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, pInsertSql));
        bindText(stmt.GetBinder(1));
        bindJson(stmt.GetBinder(2));
        bindGuid(stmt.GetBinder(3));
        bindInt(stmt.GetBinder(4));
        bindDouble(stmt.GetBinder(5));
        bindPoint2d(stmt.GetBinder(6));
        bindPoint3d(stmt.GetBinder(7));
        bindBlob(stmt.GetBinder(8));
        bindInt(stmt.GetBinder(9));
        bindGeom(stmt.GetBinder(10));
        bindBoolean(stmt.GetBinder(11));
        bindDateTime(stmt.GetBinder(12));
        bindDateTimeUtc(stmt.GetBinder(13));
        for (int i = 0; i < 3; i++) {
            bindText(stmt.GetBinder(14).AddArrayElement());
        }
        for (int i = 0; i < 3; i++) {
            bindJson(stmt.GetBinder(15).AddArrayElement());
        }
        for (int i = 0; i < 3; i++) {
            bindGuid(stmt.GetBinder(16).AddArrayElement());
        }
        for (int i = 0; i < 3; i++) {
            bindInt(stmt.GetBinder(17).AddArrayElement());
        }
        for (int i = 0; i < 3; i++) {
            bindDouble(stmt.GetBinder(18).AddArrayElement());
        }
        for (int i = 0; i < 3; i++) {
            bindPoint2d(stmt.GetBinder(19).AddArrayElement());
        }
        for (int i = 0; i < 3; i++) {
            bindPoint3d(stmt.GetBinder(20).AddArrayElement());
        }
        for (int i = 0; i < 3; i++) {
            bindBlob(stmt.GetBinder(21).AddArrayElement());
        }
        for (int i = 0; i < 3; i++) {
            bindLong(stmt.GetBinder(22).AddArrayElement());
        }
        for (int i = 0; i < 3; i++) {
            bindGeom(stmt.GetBinder(23).AddArrayElement());
        }
        for (int i = 0; i < 3; i++) {
            bindBoolean(stmt.GetBinder(24).AddArrayElement());
        }
        for (int i = 0; i < 3; i++) {
            bindDateTime(stmt.GetBinder(25).AddArrayElement());
        }
        for (int i = 0; i < 3; i++) {
            bindDateTimeUtc(stmt.GetBinder(26).AddArrayElement());
        }
        auto& structBinder = stmt.GetBinder(27);
        bindText(structBinder["S"]);
        bindJson(structBinder["Json"]);
        bindGuid(structBinder["G"]);
        bindInt(structBinder["I"]);
        bindDouble(structBinder["D"]);
        bindPoint2d(structBinder["P2d"]);
        bindPoint3d(structBinder["P3d"]);
        bindBlob(structBinder["Bi"]);
        bindLong(structBinder["L"]);
        bindGeom(structBinder["Geom"]);
        bindBoolean(structBinder["B"]);
        bindDateTime(structBinder["Dt"]);
        bindDateTimeUtc(structBinder["DtUtc"]);
        for (int i = 0; i < 3; i++) {
            bindText(structBinder["AS"].AddArrayElement());
        }
        for (int i = 0; i < 3; i++) {
            bindJson(structBinder["AJson"].AddArrayElement());
        }
        for (int i = 0; i < 3; i++) {
            bindGuid(structBinder["AG"].AddArrayElement());
        }
        for (int i = 0; i < 3; i++) {
            bindInt(structBinder["AI"].AddArrayElement());
        }
        for (int i = 0; i < 3; i++) {
            bindDouble(structBinder["AD"].AddArrayElement());
        }
        for (int i = 0; i < 3; i++) {
            bindPoint2d(structBinder["AP2d"].AddArrayElement());
        }
        for (int i = 0; i < 3; i++) {
            bindPoint3d(structBinder["AP3d"].AddArrayElement());
        }
        for (int i = 0; i < 3; i++) {
            bindBlob(structBinder["ABi"].AddArrayElement());
        }
        for (int i = 0; i < 3; i++) {
            bindLong(structBinder["AL"].AddArrayElement());
        }
        for (int i = 0; i < 3; i++) {
            bindGeom(structBinder["AGeom"].AddArrayElement());
        }
        for (int i = 0; i < 3; i++) {
            bindBoolean(structBinder["AB"].AddArrayElement());
        }
        for (int i = 0; i < 3; i++) {
            bindDateTime(structBinder["ADt"].AddArrayElement());
        }
        for (int i = 0; i < 3; i++) {
            bindDateTimeUtc(structBinder["ADtUtc"].AddArrayElement());
        }

        auto& structBinderArray = stmt.GetBinder(28);
        for (int j = 0; j < 3; j++) {
            auto& st = structBinderArray.AddArrayElement();
            bindText(st["S"]);
            bindJson(st["Json"]);
            bindGuid(st["G"]);
            bindInt(st["I"]);
            bindDouble(st["D"]);
            bindPoint2d(st["P2d"]);
            bindPoint3d(st["P3d"]);
            bindBlob(st["Bi"]);
            bindLong(st["L"]);
            bindGeom(st["Geom"]);
            bindBoolean(st["B"]);
            bindDateTime(st["Dt"]);
            bindDateTimeUtc(st["DtUtc"]);
            for (int i = 0; i < 3; i++) {
                bindText(st["AS"].AddArrayElement());
            }
            for (int i = 0; i < 3; i++) {
                bindJson(st["AJson"].AddArrayElement());
            }
            for (int i = 0; i < 3; i++) {
                bindGuid(st["AG"].AddArrayElement());
            }
            for (int i = 0; i < 3; i++) {
                bindInt(st["AI"].AddArrayElement());
            }
            for (int i = 0; i < 3; i++) {
                bindDouble(st["AD"].AddArrayElement());
            }
            for (int i = 0; i < 3; i++) {
                bindPoint2d(st["AP2d"].AddArrayElement());
            }
            for (int i = 0; i < 3; i++) {
                bindPoint3d(st["AP3d"].AddArrayElement());
            }
            for (int i = 0; i < 3; i++) {
                bindBlob(st["ABi"].AddArrayElement());
            }
            for (int i = 0; i < 3; i++) {
                bindLong(st["AL"].AddArrayElement());
            }
            for (int i = 0; i < 3; i++) {
                bindGeom(st["AGeom"].AddArrayElement());
            }
            for (int i = 0; i < 3; i++) {
                bindBoolean(st["AB"].AddArrayElement());
            }
            for (int i = 0; i < 3; i++) {
                bindDateTime(st["ADt"].AddArrayElement());
            }
            for (int i = 0; i < 3; i++) {
                bindDateTimeUtc(st["ADtUtc"].AddArrayElement());
            }
        }
        if (parent) {
            stmt.GetBinder(29).BindNavigation(parent->GetInstanceId(), parent->GetClassId());
        }
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(out));
        m_ecdb.SaveChanges();
    };

    ECInstanceKey i1;
    bindComplexInstance(i1, nullptr);
    ECInstanceKey i2;
    bindComplexInstance(i2, &i1);
    ECInstanceKey i3;
    insertRel(i1, i2, i3);

    auto i1Json = ReadInstance(m_ecdb, i1, false);
    ASSERT_TRUE(i1Json.has_value());
    BeJsDocument i1JsVal;
    i1JsVal.Parse(i1Json.value());

    auto i2Json = ReadInstance(m_ecdb, i2, false);
    ASSERT_TRUE(i2Json.has_value());
    BeJsDocument i2JsVal;
    i2JsVal.Parse(i2Json.value());

    auto i3Json = ReadInstance(m_ecdb, i3, false);
    ASSERT_TRUE(i3Json.has_value());
    BeJsDocument i3JsVal;
    i3JsVal.Parse(i3Json.value());

    i1JsVal.removeMember("LastMod"); // this is timestamp property updated on trigger

    if ("copy instance to writeDb") {
        ECInstanceKey j1, j2, j3;
        ASSERT_EQ(BE_SQLITE_DONE, InsertInstance(writeDb, i1JsVal, InstanceWriter::InsertOptions().UseInstanceIdFromJs(), j1));
        ASSERT_EQ(BE_SQLITE_DONE, InsertInstance(writeDb, i2JsVal, InstanceWriter::InsertOptions().UseInstanceIdFromJs(), j2));
        ASSERT_EQ(BE_SQLITE_DONE, InsertInstance(writeDb, i3JsVal, InstanceWriter::InsertOptions().UseInstanceIdFromJs(), j3));
        writeDb.SaveChanges();

        auto j1Json = ReadInstance(writeDb, j1, false);
        ASSERT_TRUE(j1Json.has_value());
        BeJsDocument j1JsVal;
        j1JsVal.Parse(j1Json.value());
        j1JsVal.removeMember("LastMod"); // this is timestamp property updated on trigger

        auto j2Json = ReadInstance(m_ecdb, j2, false);
        ASSERT_TRUE(i2Json.has_value());
        BeJsDocument j2JsVal;
        j2JsVal.Parse(j2Json.value());

        auto j3Json = ReadInstance(m_ecdb, j3, false);
        ASSERT_TRUE(i3Json.has_value());
        BeJsDocument j3JsVal;
        j3JsVal.Parse(j3Json.value());


        ASSERT_STREQ(i1JsVal.Stringify(StringifyFormat::Indented).c_str(), j1JsVal.Stringify(StringifyFormat::Indented).c_str());
        ASSERT_STREQ(i2JsVal.Stringify(StringifyFormat::Indented).c_str(), j2JsVal.Stringify(StringifyFormat::Indented).c_str());
        ASSERT_STREQ(i3JsVal.Stringify(StringifyFormat::Indented).c_str(), j3JsVal.Stringify(StringifyFormat::Indented).c_str());
    }

    if ("update instance to writeDb") {

    }

}
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
    ECInstanceKey key;
    if ("copy instance to writeDb") {

        ASSERT_EQ(BE_SQLITE_DONE, InsertInstance(writeDb, expected, InstanceWriter::InsertOptions().UseInstanceIdFromJs(), key));
        writeDb.SaveChanges();
        BeJsDocument actual;
        auto actualJson = ReadInstance(writeDb, key);
        ASSERT_TRUE(actualJson.has_value());
        actual.Parse(actualJson.value());
        ASSERT_EQ(expectedKey, key);
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

        ASSERT_EQ(BE_SQLITE_DONE, InsertInstance(writeDb, testDoc, opt, actualKey));
        writeDb.SaveChanges();
        BeJsDocument actual;
        auto actualJson = ReadInstance(writeDb, actualKey, true);
        ASSERT_TRUE(actualJson.has_value());
        actual.Parse(actualJson.value());
        ASSERT_STREQ(testDoc.Stringify(StringifyFormat::Indented).c_str(), actual.Stringify(StringifyFormat::Indented).c_str());
    }

    if ("update a js instance") {
        // copy instance to another db use ECSql standard format
        Utf8String testInst = R"json({
            "id": "0x1",
            "className": "TestSchema.P",
            "s": "London"
        })json";
        // UPDATE ts.P SET s = IIF(?, ?, s) WHERE ECInstanceId = 0x1
        BeJsDocument testDoc;
        testDoc.Parse(testInst);
        auto opt = InstanceWriter::UpdateOptions();
        opt.UseJsName(true);
        ASSERT_EQ(BE_SQLITE_DONE, UpdateInstance(writeDb, testDoc, opt));
        writeDb.SaveChanges();
        BeJsDocument actual;
        auto actualJson = ReadInstance(writeDb, key, true);
        ASSERT_TRUE(actualJson.has_value());
        actual.Parse(actualJson.value());
        PRINT_JSON("testDoc", actual);
    }
}
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceWriterFixture, basic2) {
}
END_ECDBUNITTESTS_NAMESPACE
