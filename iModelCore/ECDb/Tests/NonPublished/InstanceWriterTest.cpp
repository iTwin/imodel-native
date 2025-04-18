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
        if (!ecdb.GetInstanceReader().Seek(pos, [&](const InstanceReader::IRowContext& row, auto _) {
                doc.From(row.GetJson(JsReadOptions().SetAbbreviateBlobs(false).SetUseJsNames(useJsName)));
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
    static DbResult DeleteInstance(ECDbR ecdb, BeJsConst instance, InstanceWriter::DeleteOptions opts) {
        return ecdb.GetInstanceWriter().Delete(instance, opts);
    }

    // Insert an instance into the ECDb as part of the test case data setup
    DbResult insertInstance(Utf8StringCR json, const bool useJsNames = true) {
        if (!m_ecdb.IsDbOpen())
            return BE_SQLITE_ERROR;

        BeJsDocument testDoc;
        testDoc.Parse(json);
        if (testDoc.hasParseError())
            return BE_SQLITE_ERROR;

        auto opt = InstanceWriter::InsertOptions();
        opt.UseInstanceIdFromJs();
        if (useJsNames)
            opt.UseJsNames(true);

        return InsertInstance(m_ecdb, testDoc, opt);
    }

    void SetupECDbWithInstance(Utf8StringCR schemaXml, Utf8StringCR instanceJson) {
        ASSERT_EQ(SUCCESS, SetupECDb(BeTest::GetNameOfCurrentTest(), SchemaItem(schemaXml)));
        insertInstance(instanceJson);
        m_ecdb.SaveChanges();
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
        opt.UseJsNames(true);

        ASSERT_EQ(BE_SQLITE_DONE, InsertInstance(writeDb, testDoc, opt, actualKey));
        writeDb.SaveChanges();
        BeJsDocument actual;
        auto actualJson = ReadInstance(writeDb, actualKey, true);
        ASSERT_TRUE(actualJson.has_value());
        actual.Parse(actualJson.value());
        ASSERT_STREQ(testDoc.Stringify(StringifyFormat::Indented).c_str(), actual.Stringify(StringifyFormat::Indented).c_str());
    }

    if ("update a js instance with incremental update") {
        // copy instance to another db use ECSql standard format
        Utf8String testInst = R"json({
            "id": "0x1",
            "className": "TestSchema.P",
            "s": "London",
            "i": 101
        })json";

        BeJsDocument testDoc;
        testDoc.Parse(testInst);

        auto opt = InstanceWriter::UpdateOptions();
        opt.UseJsNames(true);
        opt.UseIncrementalUpdate(true);
        ASSERT_EQ(BE_SQLITE_DONE, UpdateInstance(writeDb, testDoc, opt));
        writeDb.SaveChanges();

        BeJsDocument actual;
        writeDb.GetInstanceReader().Reset();

        auto actualJson = ReadInstance(writeDb, key, true);
        ASSERT_TRUE(actualJson.has_value());
        actual.Parse(actualJson.value());
        ASSERT_STREQ(actual["s"].asCString(), "London");
        ASSERT_EQ(actual["i"].asInt(), 101);

        BeJsDocument expected;
        expected.Parse(R"json({
            "id": "0x1",
            "className": "TestSchema.P",
            "s": "London",
            "i": 101,
            "d": 3.141592653589793,
            "p2d": {
                "x": 2341.34,
                "y": -4.3322
            },
            "p3d": {
                "x": 1.2344,
                "y": -5.3322,
                "z": -0.0001
            },
            "bi": "encoding=base64;cd2DfQvyUAEK4Q==",
            "l": -72340172838076670.0,
            "dT": "2025-02-27T19:35:15.672Z",
            "b": true
            }
        )json");

        ASSERT_STREQ(expected.Stringify(StringifyFormat::Indented).c_str(), actual.Stringify(StringifyFormat::Indented).c_str());
    }

    if ("update a js instance with no incremental update") {
        // copy instance to another db use ECSql standard format
        Utf8String testInst = R"json({
            "id": "0x1",
            "className": "TestSchema.P",
            "s": "Kiev",
            "i": 2000
        })json";

        BeJsDocument testDoc;
        testDoc.Parse(testInst);

        auto opt = InstanceWriter::UpdateOptions();
        opt.UseJsNames(true);
        ASSERT_EQ(BE_SQLITE_DONE, UpdateInstance(writeDb, testDoc, opt));
        writeDb.SaveChanges();

        BeJsDocument actual;
        writeDb.GetInstanceReader().Reset();
        auto actualJson = ReadInstance(writeDb, key, true);
        ASSERT_TRUE(actualJson.has_value());
        actual.Parse(actualJson.value());

        ASSERT_STREQ(actual["s"].asCString(), "Kiev");
        ASSERT_EQ(actual["i"].asInt(), 2000);

        BeJsDocument expected;
        expected.Parse(R"json({
            "id": "0x1",
            "className": "TestSchema.P",
            "s": "Kiev",
            "i": 2000
            }
        )json");

        ASSERT_STREQ(expected.Stringify(StringifyFormat::Indented).c_str(), actual.Stringify(StringifyFormat::Indented).c_str());
    }

    if ("delete isntance") {
        // copy instance to another db use ECSql standard format
        Utf8String testInst = R"json({
            "id": "0x1",
            "className": "TestSchema.P"
        })json";

        BeJsDocument testDoc;
        testDoc.Parse(testInst);

        auto opt = InstanceWriter::DeleteOptions();
        opt.UseJsNames(true);
        ASSERT_EQ(BE_SQLITE_DONE, DeleteInstance(writeDb, testDoc, opt));
        ASSERT_EQ(1, writeDb.GetModifiedRowCount());
        writeDb.SaveChanges();
        auto actualJson = ReadInstance(writeDb, key, true);
        ASSERT_FALSE(actualJson.has_value());
    }
}

namespace InstanceWriterErrorHandlingTests {
    // Static data for tests
    const auto schemaXml = R"xml(
        <ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA" />

            <ECStructClass typeName="StructClass" >
                <ECProperty propertyName="StructClassProp" typeName="string"  />
            </ECStructClass>

            <ECRelationshipClass typeName="POwnsChildPs" strength="embedding" modifier="None">
                <Source multiplicity="(0..1)" roleLabel="1" polymorphic="true">
                    <Class class="P"/>
                </Source>
                <Target multiplicity="(0..*)" roleLabel="2" polymorphic="true">
                    <Class class="P"/>
                </Target>
            </ECRelationshipClass>

            <ECEntityClass typeName="P">
                <ECProperty propertyName="S"    typeName="string"  />
                <ECProperty propertyName="I"    typeName="int"     />
                <ECProperty propertyName="D"    typeName="double"  />
                <ECProperty propertyName="P2d"  typeName="point2d" />
                <ECProperty propertyName="P3d"  typeName="point3d" />
                <ECProperty propertyName="Bi"   typeName="binary" extendedTypeName="BeGuid" />
                <ECProperty propertyName="L"    typeName="long"    />
                <ECProperty propertyName="DT"   typeName="dateTime">
                    <ECCustomAttributes>
                        <DateTimeInfo xmlns="CoreCustomAttributes.01.00.00">
                            <DateTimeKind>Utc</DateTimeKind>
                        </DateTimeInfo>
                    </ECCustomAttributes>
                </ECProperty>
                <ECProperty propertyName="B"    typeName="boolean" />
                <ECProperty propertyName="Geom" typeName="Bentley.Geometry.Common.IGeometry" />
                <ECArrayProperty propertyName="Array_Str"       typeName="string"  />
                <ECStructProperty propertyName="Struct" typeName="StructClass" />
                <ECStructArrayProperty propertyName="Struct_Array" typeName="StructClass" />
                <ECNavigationProperty propertyName="Parent" relationshipName="POwnsChildPs" direction="Backward"/>
            </ECEntityClass>
        </ECSchema>)xml";

    const auto baseJson = R"json({
        "id": "0x1235",
        "className": "TestSchema.P",
        "s": "What is this?",
        "i": -223,
        "d": -3.141592653589793,
        "p2d": {"x": 341.34, "y": -4.322},
        "p3d": {"x": 11.2344, "y": -15.3322, "z": -20.0001},
        "bi": "encoding=base64;cd2DfQvyUAEK4Q==",
        "l": 100000,
        "dT": "2027-02-27T19:35:15.672Z",
        "b": false,
        "geom": {"lineString": [[1,1,1], [0,0,0], [3,3,3], [2,2,2]]},
        "array_Str": ["a", "b", "c"],
        "struct": {"StructClassProp": "struct property"},
        "struct_Array": [{"StructClassProp": "struct property 1"}, {"StructClassProp": "struct property 2"}],
        "parent": { "id": "0x1230", "relClassName": "TestSchema.POwnsChildPs" }
    })json";

    const auto baseJsonWithoutJsNames = R"json({
        "ECInstanceId": "0x1235",
        "ECClassId": "0x58",
        "s": "What is this?",
        "i": -223,
        "d": -3.141592653589793,
        "p2d": {"X": 341.34, "Y": -4.322},
        "p3d": {"X": 11.2344, "Y": -15.3322, "Z": -20.0001},
        "bi": "encoding=base64;cd2DfQvyUAEK4Q==",
        "l": 100000,
        "dT": "2027-02-27T19:35:15.672Z",
        "b": false,
        "geom": {"lineString": [[1,1,1], [0,0,0], [3,3,3], [2,2,2]]},
        "array_Str": ["a", "b", "c"],
        "struct": {"StructClassProp": "struct property"},
        "struct_Array": [{"StructClassProp": "struct property 1"}, {"StructClassProp": "struct property 2"}],
        "parent": { "Id": "0x1230", "RelECClassId": "0x5A" }
    })json";

    const auto commonTestCases = std::vector<std::tuple<unsigned int, Utf8String, Utf8String, Utf8String>>{
        // Positive Test
        { 1, "", "", "" },

        // Test data properties
        // Test string property "s"
        { 2, "\"s\": \"What is this?\"", "\"s\": 123", "Failed to bind property. Expected string for property S, got 123" },
        
        // Test int property "i"
        { 3, "\"i\": -223", "\"i\": \"not an int\"", "Failed to bind property. Expected integer for property I, got \"not an int\"" },
        
        // Test double property "d"
        { 4, "\"d\": -3.141592653589793", "\"d\": \"not a double\"", "Failed to bind property. Expected integer for property D, got \"not a double\"" },
        
        // Test binary property "bi"
        { 5, "\"bi\": \"encoding=base64;cd2DfQvyUAEK4Q==\"", "\"bi\": 12345", "Failed to bind property. Expected binary/base64 for property Bi, got 12345" },
        { 6, "\"bi\": \"encoding=base64;cd2DfQvyUAEK4Q==\"", "\"bi\": \"not a binary\"", "Failed to bind property. Failed to parse guid from string: not a binary" },
        
        // Test long property "l"
        { 7, "\"l\": 100000", "\"l\": \"not a long\"", "Failed to bind property. Expected long for property L, got \"not a long\"" },
        
        // Test dateTime property "dT"
        { 8, "\"dT\": \"2027-02-27T19:35:15.672Z\"", "\"dT\": 20270227", "Failed to bind property. Expected string/datetime for property DT, got 20270227" },
        { 9, "\"dT\": \"2027-02-27T19:35:15.672Z\"", "\"dT\": \"27/02/2027\"", "Failed to bind property. Failed to parse datetime from string: 27/02/2027" },
        
        // Test boolean property "b"
        { 10, "\"b\": false", "\"b\": 123", "Failed to bind property. Expected boolean for property B, got 123" },
        
        // Test geometry property "geom"
        { 11, "\"geom\": {\"lineString\": [[1,1,1], [0,0,0], [3,3,3], [2,2,2]]}", "\"geom\": 123", "Failed to bind property. Expected string/json for property Geom, got 123" },
        { 12, "\"geom\": {\"lineString\": [[1,1,1], [0,0,0], [3,3,3], [2,2,2]]}", "\"geom\": \"not a geometry\"", "Failed to bind property. Failed to parse geometry from json string: \"not a geometry\"" },
        { 13, "\"geom\": {\"lineString\": [[1,1,1], [0,0,0], [3,3,3], [2,2,2]]}", "\"geom\": {\"lineString\": [[\"1\",\"1\",\"1\"], [0,0,0], [3,3,3]]}", "Failed to bind property. Failed to parse geometry from json object: {\"lineString\":[[\"1\",\"1\",\"1\"],[0,0,0],[3,3,3]]}" },
        
        // Test array property "array_Str"
        { 14, "\"array_Str\": [\"a\", \"b\", \"c\"]", "\"array_Str\": 123", "Failed to bind property. Expected array for primitive array property, got 123" },
        { 15, "\"array_Str\": [\"a\", \"b\", \"c\"]", "\"array_Str\": \"not an array\"", "Failed to bind property. Expected array for primitive array property, got \"not an array\"" },
        { 16, "\"array_Str\": [\"a\", \"b\", \"c\"]", "\"array_Str\": [\"a\", \"b\", 123]", "Failed to bind property. Expected string for property Array_Str, got 123" },
        
        // Test struct property "struct"
        { 17, "\"struct\": {\"StructClassProp\": \"struct property\"}", "\"struct\": 123", "Failed to bind property. Expected instance to be of type object for struct property, got 123" },
        { 18, "\"struct\": {\"StructClassProp\": \"struct property\"}", "\"struct\": \"not a struct\"", "Failed to bind property. Expected instance to be of type object for struct property, got \"not a struct\"" },
        { 19, "\"struct\": {\"StructClassProp\": \"struct property\"}", "\"struct\": {\"StructClassProp\": 123}", "Failed to bind property. Expected string for property StructClassProp, got 123" },
        { 20, "\"struct\": {\"StructClassProp\": \"struct property\"}", "\"struct\": {\"StructClassProp\": \"struct property\", \"extra\": \"extra property\"}", "" },
        
        // Test struct array property "struct_Array"
        { 21, "\"struct_Array\": [{\"StructClassProp\": \"struct property 1\"}, {\"StructClassProp\": \"struct property 2\"}]", "\"struct_Array\": 123", "Failed to bind property. Expected array for struct array property, got 123" },
        { 22, "\"struct_Array\": [{\"StructClassProp\": \"struct property 1\"}, {\"StructClassProp\": \"struct property 2\"}]", "\"struct_Array\": \"not an array\"", "Failed to bind property. Expected array for struct array property, got \"not an array\"" },
        { 23, "\"struct_Array\": [{\"StructClassProp\": \"struct property 1\"}, {\"StructClassProp\": \"struct property 2\"}]", "\"struct_Array\": [{\"StructClassProp\": 123}]", "Failed to bind property. Expected string for property StructClassProp, got 123" },
        { 24, "\"struct_Array\": [{\"StructClassProp\": \"struct property 1\"}, {\"StructClassProp\": \"struct property 2\"}]", "\"struct_Array\": [{\"StructClassProp\": \"struct property 1\"}, {\"StructClassProp\": \"struct property 2\", \"extra\": \"extra property\"}]", "" },
    };

    const auto jsNamesTestCases = std::vector<std::tuple<unsigned int, Utf8String, Utf8String, Utf8String>>{
        // Test point2d property "p2d"
        { 25, "\"p2d\": {\"x\": 341.34, \"y\": -4.322}", "\"p2d\": \"not a point2d\"", "Failed to bind property. Expected instance to be of type object for Point2d property, got \"not a point2d\"" },
        { 26, "\"p2d\": {\"x\": 341.34, \"y\": -4.322}", "\"p2d\": {\"x\": 341.34, \"y\": \"-4.322\"}", "Failed to bind property. Expected instance to be of type object with x and y numeric members for Point2d property, got {\"x\":341.34,\"y\":\"-4.322\"}" },
        { 27, "\"p2d\": {\"x\": 341.34, \"y\": -4.322}", "\"p2d\": {\"x\": \"341.34\", \"y\": \"-4.322\"}", "Failed to bind property. Expected instance to be of type object with x and y numeric members for Point2d property, got {\"x\":\"341.34\",\"y\":\"-4.322\"}" },
        
        // Test point3d property "p3d"
        { 28, "\"p3d\": {\"x\": 11.2344, \"y\": -15.3322, \"z\": -20.0001}", "\"p3d\": \"not a point3d\"", "Failed to bind property. Expected instance to be of type object for Point3d property, got \"not a point3d\"" },
        { 29, "\"p3d\": {\"x\": 11.2344, \"y\": -15.3322, \"z\": -20.0001}", "\"p3d\": {\"x\": 11.2344, \"y\": -15.3322, \"z\": \"-20.0001\"}", "Failed to bind property. Expected instance to be of type object with x, y and z numeric members for Point3d property, got {\"x\":11.2344,\"y\":-15.3322,\"z\":\"-20.0001\"}" },
        { 30, "\"p3d\": {\"x\": 11.2344, \"y\": -15.3322, \"z\": -20.0001}", "\"p3d\": {\"x\": \"11.2344\", \"y\": \"-15.3322\", \"z\": \"-20.0001\"}", "Failed to bind property. Expected instance to be of type object with x, y and z numeric members for Point3d property, got {\"x\":\"11.2344\",\"y\":\"-15.3322\",\"z\":\"-20.0001\"}" },

        // Test navigation property "parent"
        { 31, "\"parent\": { \"id\": \"0x1230\", \"relClassName\": \"TestSchema.POwnsChildPs\" }", "\"parent\": 123", "" },
        { 32, "\"parent\": { \"id\": \"0x1230\", \"relClassName\": \"TestSchema.POwnsChildPs\" }", "\"parent\": \"not a navigation\"", "Failed to bind property. Value supplied is not a valid decimal or hexadecimal value for the ECInstanceId/id field \"not a navigation\"" },
        { 33, "\"parent\": { \"id\": \"0x1230\", \"relClassName\": \"TestSchema.POwnsChildPs\" }", "\"parent\": { \"id\": [\"0x1250\"], \"relClassName\": \"TestSchema.POwnsChildPs\" }", "Failed to bind property. Expected id for navigation property, got [\"0x1250\"]" },
        { 34, "\"parent\": { \"id\": \"0x1230\", \"relClassName\": \"TestSchema.POwnsChildPs\" }", "\"parent\": { \"id\": \"0x1230\", \"relClassName\": \"90\" }", "Failed to bind property. Failed to find class with name: 90" },
        { 35, "\"parent\": { \"id\": \"0x1230\", \"relClassName\": \"TestSchema.POwnsChildPs\" }", "\"parent\": { \"id\": \"0x1230\", \"relClassName\": [\"TestSchema.POwnsChildPs\"] }", "Failed to bind property. Expected relClassId/relClassName for navigation property, got [\"TestSchema.POwnsChildPs\"]" },
        { 36, "\"parent\": { \"id\": \"0x1230\", \"relClassName\": \"TestSchema.POwnsChildPs\" }", "\"parent\": { \"id\": \"0x1230\", \"relClassName\": \"NonExistentClass\" }", "Failed to bind property. Failed to find class with name: NonExistentClass" },

        // Test system properties
        { 37, "\"className\": \"TestSchema.P\"", "\"className\": \"123\"", "Failed to get ECClassId/className/classFullName. Failed to find class with name: 123" },
        { 38, "\"className\": \"TestSchema.P\"", "\"className\": 123", "Failed to get ECClassId/className/classFullName. Expected string for class name, got 123" },
        { 39, "\"className\": \"TestSchema.P\"", "\"className\": \"not a class\"", "Failed to get ECClassId/className/classFullName. Failed to find class with name: not a class" },
        { 40, "\"id\": \"0x1235\"", "\"id\": 123", "Failed to get ECInstanceId/id. Expected string for id, got 123" },
        { 41, "\"id\": \"0x1235\"", "\"id\": \"not an id\"", "Failed to get ECInstanceId/id. Value supplied is not a valid decimal value \"not an id\"" },
    };

    const auto withoutJsNamesTestCases = std::vector<std::tuple<unsigned int, Utf8String, Utf8String, Utf8String>>{
        // Test point2d property "p2d"
        { 25, "\"p2d\": {\"X\": 341.34, \"Y\": -4.322}", "\"p2d\": \"not a point2d\"", "Failed to bind property. Expected instance to be of type object for Point2d property, got \"not a point2d\"" },
        { 26, "\"p2d\": {\"X\": 341.34, \"Y\": -4.322}", "\"p2d\": {\"X\": 341.34, \"Y\": \"-4.322\"}", "Failed to bind property. Expected instance to be of type object with x and y numeric members for Point2d property, got {\"X\":341.34,\"Y\":\"-4.322\"}" },
        { 27, "\"p2d\": {\"X\": 341.34, \"Y\": -4.322}", "\"p2d\": {\"X\": \"341.34\", \"Y\": \"-4.322\"}", "Failed to bind property. Expected instance to be of type object with x and y numeric members for Point2d property, got {\"X\":\"341.34\",\"Y\":\"-4.322\"}" },
        
        // Test point3d property "p3d"
        { 28, "\"p3d\": {\"X\": 11.2344, \"Y\": -15.3322, \"Z\": -20.0001}", "\"p3d\": \"not a point3d\"", "Failed to bind property. Expected instance to be of type object for Point3d property, got \"not a point3d\"" },
        { 29, "\"p3d\": {\"X\": 11.2344, \"Y\": -15.3322, \"Z\": -20.0001}", "\"p3d\": {\"X\": 11.2344, \"Y\": -15.3322, \"Z\": \"-20.0001\"}", "Failed to bind property. Expected instance to be of type object with x, y and z numeric members for Point3d property, got {\"X\":11.2344,\"Y\":-15.3322,\"Z\":\"-20.0001\"}" },
        { 30, "\"p3d\": {\"X\": 11.2344, \"Y\": -15.3322, \"Z\": -20.0001}", "\"p3d\": {\"X\": \"11.2344\", \"Y\": \"-15.3322\", \"Z\": \"-20.0001\"}", "Failed to bind property. Expected instance to be of type object with x, y and z numeric members for Point3d property, got {\"X\":\"11.2344\",\"Y\":\"-15.3322\",\"Z\":\"-20.0001\"}" },

        // Test navigation property "parent"
        { 31, "\"parent\": { \"Id\": \"0x1230\", \"RelECClassId\": \"0x5A\" }", "\"parent\": 123", "" },
        { 32, "\"parent\": { \"Id\": \"0x1230\", \"RelECClassId\": \"0x5A\" }", "\"parent\": \"not a navigation\"", "Failed to bind property. Value supplied is not a valid decimal or hexadecimal value for the ECInstanceId/id field \"not a navigation\"" },
        { 33, "\"parent\": { \"Id\": \"0x1230\", \"RelECClassId\": \"0x5A\" }", "\"parent\": { \"Id\": [\"0x1250\"], \"RelECClassId\": \"0x5A\" }", "Failed to bind property. Expected id for navigation property, got [\"0x1250\"]" },
        { 34, "\"parent\": { \"Id\": \"0x1230\", \"RelECClassId\": \"0x5A\" }", "\"parent\": { \"Id\": \"0x1230\", \"RelECClassId\": \"99999\" }", "" },
        { 35, "\"parent\": { \"Id\": \"0x1230\", \"RelECClassId\": \"0x5A\" }", "\"parent\": { \"Id\": \"0x1230\", \"RelECClassId\": [\"0x5A\"] }", "Failed to bind property. Expected relClassId/relClassName for navigation property, got [\"0x5A\"]" },
        { 36, "\"parent\": { \"Id\": \"0x1230\", \"RelECClassId\": \"0x5A\" }", "\"parent\": { \"Id\": \"0x1230\", \"RelECClassId\": \"NonExistentClass\" }", "Failed to bind property. Value supplied is not a valid decimal or hexadecimal value for the RelECClassId field \"NonExistentClass\"" },

        // Test system properties
        { 37, "\"ECClassId\": \"0x58\"", "\"ECClassId\": \"not a class\"", "Failed to get ECClassId/className/classFullName. Value supplied is not a valid decimal or hexadecimal value for the ECClassId field \"not a class\"" },
        { 38, "\"ECInstanceId\": \"0x1235\"", "\"ECInstanceId\": 9999", "" },
        { 39, "\"ECInstanceId\": 1235", "\"ECInstanceId\": 9999", "" },
        { 40, "\"ECInstanceId\": \"0x1235\"", "\"ECInstanceId\": \"not an id\"", "Failed to get ECInstanceId/id. Value supplied is not a valid decimal or hexadecimal value \"not an ECInstanceId\"" },
    };
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceWriterFixture, InsertInstanceErrorHandling) {
    // Setup the ECDb with the schema
    SetupECDbWithInstance(InstanceWriterErrorHandlingTests::schemaXml, R"json({ "id": "0x1230", "className": "TestSchema.P"})json");
    ASSERT_TRUE(m_ecdb.IsDbOpen());

    const auto schema = m_ecdb.Schemas().GetSchema("TestSchema");
    ASSERT_TRUE(schema);
    const auto classP = schema->GetClassCP("P");
    ASSERT_TRUE(classP);

    auto combinedTestCases = InstanceWriterErrorHandlingTests::commonTestCases;
    combinedTestCases.insert(combinedTestCases.end(), InstanceWriterErrorHandlingTests::withoutJsNamesTestCases.begin(), InstanceWriterErrorHandlingTests::withoutJsNamesTestCases.end());

    // Run the test case suite for inserting instances
    for (const auto& [testCaseNumber, propertyToReplace, propertyToReplaceWith, expectedErrorMessage] : combinedTestCases) {
        Utf8String testJson = InstanceWriterErrorHandlingTests::baseJsonWithoutJsNames;
        testJson.ReplaceAll(propertyToReplace.c_str(), propertyToReplaceWith.c_str());

        testJson.ReplaceAll("{ \"id\"", "{ \"Id\"");
        testJson.ReplaceAll("relClassName", "RelECClassId");
        testJson.ReplaceAll("id", "ECInstanceId");
        testJson.ReplaceAll("\"className\": \"TestSchema.P\"", Utf8PrintfString("\"ECClassId\": \"%s\"", classP->GetId().ToHexStr().c_str()).c_str());

        m_ecdb.GetInstanceWriter().Reset(); // Clear the last error message

        ASSERT_EQ(Utf8String::IsNullOrEmpty(expectedErrorMessage.c_str()) ? BE_SQLITE_DONE : BE_SQLITE_ERROR, insertInstance(testJson, false)) << "Test case " << testCaseNumber << " failed.";
        EXPECT_STREQ(expectedErrorMessage.c_str(), m_ecdb.GetInstanceWriter().GetLastError().c_str()) << "Test case " << testCaseNumber << " did not report the expected error.";

        m_ecdb.AbandonChanges();    // Make sure successful inserts don't affect the next test
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceWriterFixture, InsertInstanceErrorHandlingUseJsNames) {
    // Setup the ECDb with the schema
    SetupECDbWithInstance(InstanceWriterErrorHandlingTests::schemaXml, R"json({ "id": "0x1230", "className": "TestSchema.P"})json");
    ASSERT_TRUE(m_ecdb.IsDbOpen());

    auto combinedTestCases = InstanceWriterErrorHandlingTests::commonTestCases;
    combinedTestCases.insert(combinedTestCases.end(), InstanceWriterErrorHandlingTests::jsNamesTestCases.begin(), InstanceWriterErrorHandlingTests::jsNamesTestCases.end());

    // Run the test case suite for inserting instances
    for (const auto& [testCaseNumber, propertyToReplace, propertyToReplaceWith, expectedErrorMessage] : combinedTestCases) {
        Utf8String testJson = InstanceWriterErrorHandlingTests::baseJson;
        testJson.ReplaceAll(propertyToReplace.c_str(), propertyToReplaceWith.c_str());

        m_ecdb.GetInstanceWriter().Reset(); // Clear the last error message

        ASSERT_EQ(Utf8String::IsNullOrEmpty(expectedErrorMessage.c_str()) ? BE_SQLITE_DONE : BE_SQLITE_ERROR, insertInstance(testJson)) << "Test case " << testCaseNumber << " failed.";
        EXPECT_STREQ(expectedErrorMessage.c_str(), m_ecdb.GetInstanceWriter().GetLastError().c_str()) << "Test case " << testCaseNumber << " did not report the expected error.";

        m_ecdb.AbandonChanges();    // Make sure successful inserts don't affect the next test
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceWriterFixture, UpdateInstanceErrorHandling) {
    // Setup the ECDb with the schema
    SetupECDbWithInstance(InstanceWriterErrorHandlingTests::schemaXml, InstanceWriterErrorHandlingTests::baseJsonWithoutJsNames);
    ASSERT_TRUE(m_ecdb.IsDbOpen());

    insertInstance(R"json({ "id": "0x1230", "className": "TestSchema.P"})json");

    const auto schema = m_ecdb.Schemas().GetSchema("TestSchema");
    ASSERT_TRUE(schema);
    const auto classP = schema->GetClassCP("P");
    ASSERT_TRUE(classP);

    auto combinedTestCases = InstanceWriterErrorHandlingTests::commonTestCases;
    combinedTestCases.insert(combinedTestCases.end(), InstanceWriterErrorHandlingTests::withoutJsNamesTestCases.begin(), InstanceWriterErrorHandlingTests::withoutJsNamesTestCases.end());

    // Run the test case suite for updating instances
    for (const auto& [testCaseNumber, propertyToReplace, propertyToReplaceWith, expectedErrorMessage] : combinedTestCases) {
        Utf8String testJson = InstanceWriterErrorHandlingTests::baseJsonWithoutJsNames;
        testJson.ReplaceAll(propertyToReplace.c_str(), propertyToReplaceWith.c_str());

        testJson.ReplaceAll("{ \"id\"", "{ \"Id\"");
        testJson.ReplaceAll("relClassName", "RelECClassId");
        testJson.ReplaceAll("id", "ECInstanceId");
        testJson.ReplaceAll("\"className\": \"TestSchema.P\"", Utf8PrintfString("\"ECClassId\": \"%s\"", classP->GetId().ToHexStr().c_str()).c_str());

        BeJsDocument testDoc;
        testDoc.Parse(testJson);
        ASSERT_FALSE(testDoc.hasParseError()) << "Test case " << testCaseNumber << " failed to parse json.";

        m_ecdb.GetInstanceWriter().Reset(); // Clear the last error message

        ASSERT_EQ(Utf8String::IsNullOrEmpty(expectedErrorMessage.c_str()) ? BE_SQLITE_DONE : BE_SQLITE_ERROR, UpdateInstance(m_ecdb, testDoc, InstanceWriter::UpdateOptions())) << "Test case " << testCaseNumber << " failed.";
        EXPECT_STREQ(expectedErrorMessage.c_str(), m_ecdb.GetInstanceWriter().GetLastError().c_str()) << "Test case " << testCaseNumber << " did not report the expected error.";

        m_ecdb.AbandonChanges();    // Make sure successful updates don't affect the next test
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceWriterFixture, UpdateInstanceErrorHandlingUseJsNames) {
    // Setup the ECDb with the schema
    SetupECDbWithInstance(InstanceWriterErrorHandlingTests::schemaXml, InstanceWriterErrorHandlingTests::baseJson);
    ASSERT_TRUE(m_ecdb.IsDbOpen());

    insertInstance(R"json({ "id": "0x1230", "className": "TestSchema.P"})json");

    auto combinedTestCases = InstanceWriterErrorHandlingTests::commonTestCases;
    combinedTestCases.insert(combinedTestCases.end(), InstanceWriterErrorHandlingTests::jsNamesTestCases.begin(), InstanceWriterErrorHandlingTests::jsNamesTestCases.end());

    // Run the test case suite for updating instances
    for (const auto& [testCaseNumber, propertyToReplace, propertyToReplaceWith, expectedErrorMessage] : combinedTestCases) {
        Utf8String testJson = InstanceWriterErrorHandlingTests::baseJson;
        testJson.ReplaceAll(propertyToReplace.c_str(), propertyToReplaceWith.c_str());

        BeJsDocument testDoc;
        testDoc.Parse(testJson);
        ASSERT_FALSE(testDoc.hasParseError()) << "Test case " << testCaseNumber << " failed to parse json.";
        auto opt = InstanceWriter::UpdateOptions();
        opt.UseJsNames(true);

        m_ecdb.GetInstanceWriter().Reset(); // Clear the last error message

        ASSERT_EQ(Utf8String::IsNullOrEmpty(expectedErrorMessage.c_str()) ? BE_SQLITE_DONE : BE_SQLITE_ERROR, UpdateInstance(m_ecdb, testDoc, opt)) << "Test case " << testCaseNumber << " failed.";
        EXPECT_STREQ(expectedErrorMessage.c_str(), m_ecdb.GetInstanceWriter().GetLastError().c_str()) << "Test case " << testCaseNumber << " did not report the expected error.";

        m_ecdb.AbandonChanges();    // Make sure successful updates don't affect the next test
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceWriterFixture, DeleteInstanceErrorHandling) {
    // Setup the ECDb with the schema
    SetupECDbWithInstance(InstanceWriterErrorHandlingTests::schemaXml, InstanceWriterErrorHandlingTests::baseJson);
    ASSERT_TRUE(m_ecdb.IsDbOpen());

    // Run the test case suite for updating instances
    for (const auto& [testCaseNumber, jsonInstance, useJsNames, shouldSucceed] : std::vector<std::tuple<unsigned int, Utf8String, bool, bool>>{
        { 1, R"json({ "id": "0x1235", "className": "TestSchema.P"})json", true, true },
        { 2, R"json({ "id": 1235, "className": "TestSchema.P"})json", true, false },
        { 3, R"json({ "id": "not a string", "className": "TestSchema.P"})json", true, false },
        { 4, R"json({ "id": ["0x1235"], "className": "TestSchema.P"})json", true, false },

        { 5, R"json({ "id": "0x1235", "className": "NonExistentClass"})json", true, false },
        { 6, R"json({ "id": "0x1235", "className": "0x58"})json", true, false },
        { 7, R"json({ "id": "0x1235", "className": ["NonExistentClass"]})json", true, false },

        { 8, R"json({ "ECInstanceId": "0x1235", "ECClassId": "0x58"})json", false, true },
        { 9, R"json({ "ECInstanceId": 1235, "ECClassId": "0x58"})json", false, true },
        { 10, R"json({ "ECInstanceId": "not a string", "ECClassId": "0x58"})json", false, false },
        { 11, R"json({ "ECInstanceId": ["0x1235"], "ECClassId": "0x58"})json", false, false },

        { 12, R"json({ "ECInstanceId": "0x1235", "ECClassId": "NonExistentClass"})json", false, false },
        { 13, R"json({ "ECInstanceId": "0x1235", "ECClassId": ["NonExistentClass"]})json", false, false },
    }) {
        BeJsDocument testDoc;
        testDoc.Parse(jsonInstance);
        ASSERT_FALSE(testDoc.hasParseError()) << "Test case " << testCaseNumber << " failed to parse json.";
        auto opt = InstanceWriter::DeleteOptions();
        opt.UseJsNames(useJsNames);

        m_ecdb.GetInstanceWriter().Reset(); // Clear the last error message

        ASSERT_EQ(shouldSucceed ? BE_SQLITE_DONE : BE_SQLITE_ERROR, DeleteInstance(m_ecdb, testDoc, opt)) << "Test case " << testCaseNumber << " failed.";
        if (!shouldSucceed)
            EXPECT_STREQ("Failed to get ECInstanceId/id and ECClassId/className/classFullName.", m_ecdb.GetInstanceWriter().GetLastError().c_str()) << "Test case " << testCaseNumber << " did not report the expected error.";

        m_ecdb.AbandonChanges();    // Make sure successful updates don't affect the next test
    }
}
END_ECDBUNITTESTS_NAMESPACE
