/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/JsonReaderTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

struct JsonECSqlSelectAdapterTests : public ECDbTestFixture {};

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                09/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonECSqlSelectAdapterTests, JsonMemberNames)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("JsonMemberNamesInJsonECSqlSelectAdapter.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    ECClassId relClassId = m_ecdb.Schemas().GetClassId("ECSqlTest", "PSAHasP_N1");
    ASSERT_TRUE(relClassId.IsValid());

    ECInstanceKey pKey, psaKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(pKey, "INSERT INTO ecsql.P(ECInstanceId) VALUES (NULL)"));
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.PSA(I,L,PStructProp.i,P) VALUES (10,10,10,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(1, pKey.GetInstanceId(), relClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(psaKey));
    }

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId, ECInstanceId, ECClassId, ECClassId, I, I, P, P.Id, P.RelECClassId FROM ecsql.PSA LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    Utf8String expectedIdStr = psaKey.GetInstanceId().ToHexStr();
    Utf8String expectedNavIdHexStr = pKey.GetInstanceId().ToHexStr();
    Utf8String expectedNavIdStr = pKey.GetInstanceId().ToString();

    JsonECSqlSelectAdapter defaultAdapter(stmt);
    Json::Value defaultJson;
    ASSERT_EQ(SUCCESS, defaultAdapter.GetRow(defaultJson));

    JsonECSqlSelectAdapter javaScriptAdapter(stmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::LowerFirstChar, ECJsonInt64Format::AsDecimalString));
    Json::Value javaScriptJson;
    ASSERT_EQ(SUCCESS, javaScriptAdapter.GetRow(javaScriptJson));

    {
    ASSERT_TRUE(defaultJson.isMember(ECJsonUtilities::json_id())) << defaultJson.ToString().c_str();
    EXPECT_STRCASEEQ(expectedIdStr.c_str(), defaultJson[ECJsonUtilities::json_id()].asCString()) << defaultJson.ToString().c_str();

    ASSERT_TRUE(javaScriptJson.isMember(ECJsonUtilities::json_id())) << javaScriptJson.ToString().c_str();
    EXPECT_STRCASEEQ(expectedIdStr.c_str(), javaScriptJson[ECJsonUtilities::json_id()].asCString()) << javaScriptJson.ToString().c_str();
    }

    {
    Utf8String id1Member(ECJsonUtilities::json_id());
    id1Member.append("_1");
    ASSERT_TRUE(defaultJson.isMember(id1Member.c_str())) << defaultJson.ToString().c_str();
    EXPECT_STRCASEEQ(expectedIdStr.c_str(), defaultJson[id1Member.c_str()].asCString()) << defaultJson.ToString().c_str();

    ASSERT_TRUE(javaScriptJson.isMember(id1Member.c_str())) << javaScriptJson.ToString().c_str();
    EXPECT_STRCASEEQ(expectedIdStr.c_str(), javaScriptJson[id1Member.c_str()].asCString()) << javaScriptJson.ToString().c_str();
    }

    {
    ASSERT_TRUE(defaultJson.isMember(ECJsonUtilities::json_className())) << defaultJson.ToString().c_str();
    EXPECT_STRCASEEQ("ECSqlTest.PSA", defaultJson[ECJsonUtilities::json_className()].asCString()) << defaultJson.ToString().c_str();

    ASSERT_TRUE(javaScriptJson.isMember(ECJsonUtilities::json_className())) << javaScriptJson.ToString().c_str();
    EXPECT_STRCASEEQ("ECSqlTest.PSA", javaScriptJson[ECJsonUtilities::json_className()].asCString()) << javaScriptJson.ToString().c_str();
    }

    {
    Utf8String className1Member(ECJsonUtilities::json_className());
    className1Member.append("_1");

    ASSERT_TRUE(defaultJson.isMember(className1Member.c_str())) << defaultJson.ToString().c_str();
    EXPECT_STRCASEEQ("ECSqlTest.PSA", defaultJson[className1Member.c_str()].asCString()) << defaultJson.ToString().c_str();

    ASSERT_TRUE(javaScriptJson.isMember(className1Member.c_str())) << javaScriptJson.ToString().c_str();
    EXPECT_STRCASEEQ("ECSqlTest.PSA", javaScriptJson[className1Member.c_str()].asCString()) << javaScriptJson.ToString().c_str();
    }

    {
    ASSERT_TRUE(defaultJson.isMember("I")) << defaultJson.ToString().c_str();
    EXPECT_EQ(10, defaultJson["I"].asInt()) << defaultJson.ToString().c_str();

    ASSERT_TRUE(javaScriptJson.isMember("i")) << javaScriptJson.ToString().c_str();
    EXPECT_EQ(10, javaScriptJson["i"].asInt()) << javaScriptJson.ToString().c_str();
    }

    {
    ASSERT_TRUE(defaultJson.isMember("I_1")) << defaultJson.ToString().c_str();
    EXPECT_EQ(10, defaultJson["I_1"].asInt()) << defaultJson.ToString().c_str();

    ASSERT_TRUE(javaScriptJson.isMember("i_1")) << javaScriptJson.ToString().c_str();
    EXPECT_EQ(10, javaScriptJson["i_1"].asInt()) << javaScriptJson.ToString().c_str();
    }

    {
    ASSERT_TRUE(defaultJson.isMember("P") && defaultJson["P"].isObject()) << defaultJson.ToString().c_str();
    EXPECT_STRCASEEQ(expectedNavIdHexStr.c_str(), defaultJson["P"][ECJsonUtilities::json_navId()].asCString()) << defaultJson.ToString().c_str();
    EXPECT_STRCASEEQ("ECSqlTest.PSAHasP_N1", defaultJson["P"][ECJsonUtilities::json_navRelClassName()].asCString()) << defaultJson.ToString().c_str();

    ASSERT_TRUE(javaScriptJson.isMember("p") && javaScriptJson["p"].isObject()) << javaScriptJson.ToString().c_str();
    EXPECT_STRCASEEQ(expectedNavIdHexStr.c_str(), javaScriptJson["p"][ECJsonUtilities::json_navId()].asCString()) << javaScriptJson.ToString().c_str();
    EXPECT_STRCASEEQ("ECSqlTest.PSAHasP_N1", javaScriptJson["p"][ECJsonUtilities::json_navRelClassName()].asCString()) << javaScriptJson.ToString().c_str();

    }

    {
    ASSERT_TRUE(defaultJson.isMember("P.Id")) << defaultJson.ToString().c_str();
    EXPECT_STRCASEEQ(expectedNavIdStr.c_str(), defaultJson["P.Id"].asCString()) << defaultJson.ToString().c_str();

    ASSERT_TRUE(javaScriptJson.isMember("p.Id")) << javaScriptJson.ToString().c_str();
    EXPECT_STRCASEEQ(expectedNavIdStr.c_str(), javaScriptJson["p.Id"].asCString()) << javaScriptJson.ToString().c_str();
    }

    {
    //RelECClassId is not converted to relClassName if explicitly specified in select clause.
    ASSERT_TRUE(defaultJson.isMember("P.RelECClassId")) << defaultJson.ToString().c_str();
    EXPECT_STRCASEEQ(relClassId.ToString().c_str(), defaultJson["P.RelECClassId"].asCString()) << defaultJson.ToString().c_str();

    ASSERT_TRUE(javaScriptJson.isMember("p.RelECClassId")) << javaScriptJson.ToString().c_str();
    EXPECT_STRCASEEQ(relClassId.ToString().c_str(), javaScriptJson["p.RelECClassId"].asCString()) << javaScriptJson.ToString().c_str();
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                09/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonECSqlSelectAdapterTests, SpecialSelectClauseItems)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("SpecialSelectClauseItemsInJsonECSqlSelectAdapter.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                                                        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                                                            <ECStructClass typeName="MyStruct" >
                                                                                <ECProperty propertyName="SomeNumber" typeName="int" />
                                                                            </ECStructClass>
                                                                            <ECEntityClass typeName="Foo" >
                                                                                <ECProperty propertyName="IntProp" typeName="int" />
                                                                                <ECProperty propertyName="PtProp" typeName="Point3d" />
                                                                                <ECStructProperty propertyName="StructProp" typeName="MyStruct" />
                                                                            </ECEntityClass>
                                                                        </ECSchema>)xml")));

    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(key, "INSERT INTO ts.Foo(IntProp,PtProp.X,PtProp.Y,PtProp.Z,StructProp.SomeNumber) VALUES (1000,1000,1000,1000,1000)"));
    
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT NULL, NULL As MyNull FROM ts.Foo"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    JsonECSqlSelectAdapter defaultAdapter(stmt);
    JsonECSqlSelectAdapter javaScriptAdapter(stmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::LowerFirstChar, ECJsonInt64Format::AsDecimalString));
    Json::Value defaultJson, javaScriptJson;

    ASSERT_EQ(SUCCESS, defaultAdapter.GetRow(defaultJson));
    ASSERT_EQ(SUCCESS, javaScriptAdapter.GetRow(javaScriptJson));

    {
    ASSERT_TRUE(defaultJson.isObject()) << stmt.GetECSql() << " - " << defaultJson.ToString().c_str();
    ASSERT_EQ(0, defaultJson.size()) << stmt.GetECSql() << " - " << defaultJson.ToString().c_str();
    ASSERT_TRUE(javaScriptJson.isObject()) << stmt.GetECSql() << " - " << javaScriptJson.ToString().c_str();
    ASSERT_EQ(0, javaScriptJson.size()) << stmt.GetECSql() << " - " << javaScriptJson.ToString().c_str();
    }

    {
    ASSERT_FALSE(defaultJson.isMember("NULL") && defaultJson.isMember("nULL")) << "NULL literal results in null which is ignored by adapter. " << defaultJson.ToString().c_str();
    ASSERT_FALSE(defaultJson.isMember("MyNull")) << "NULL literal even if it has an alias results in null which is ignored by adapter. " << defaultJson.ToString().c_str();

    ASSERT_FALSE(javaScriptJson.isMember("nULL") && javaScriptJson.isMember("NULL")) << "NULL literal results in null which is ignored by adapter. " << javaScriptJson.ToString().c_str();
    ASSERT_FALSE(javaScriptJson.isMember("myNull")) << "NULL literal even if it has an alias results in null which is ignored by adapter. " << javaScriptJson.ToString().c_str();
    stmt.Finalize();
    }

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT 123, 123, 123 AS MyNumber, 123 * 3, 123 * 3 AS MyProduct FROM ts.Foo"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    ASSERT_EQ(SUCCESS, defaultAdapter.GetRow(defaultJson));
    ASSERT_EQ(SUCCESS, javaScriptAdapter.GetRow(javaScriptJson));

    {
    ASSERT_TRUE(defaultJson.isObject()) << defaultJson.ToString().c_str();
    ASSERT_EQ(stmt.GetColumnCount(), (int) defaultJson.size()) << stmt.GetECSql() << " - " << defaultJson.ToString().c_str();

    ASSERT_TRUE(javaScriptJson.isObject()) << javaScriptJson.ToString().c_str();
    ASSERT_EQ(stmt.GetColumnCount(), (int) javaScriptJson.size()) << stmt.GetECSql() << " - " << javaScriptJson.ToString().c_str();
    }

    {
    ASSERT_TRUE(defaultJson.isMember("123")) << stmt.GetECSql() << " - " << defaultJson.ToString().c_str();
    ASSERT_EQ(123, defaultJson["123"].asInt()) << stmt.GetECSql() << " - " << defaultJson.ToString().c_str();

    ASSERT_TRUE(javaScriptJson.isMember("123")) << stmt.GetECSql() << " - " << javaScriptJson.ToString().c_str();
    ASSERT_EQ(123, javaScriptJson["123"].asInt()) << stmt.GetECSql() << " - " << javaScriptJson.ToString().c_str();
    }

    {
    ASSERT_TRUE(defaultJson.isMember("123_1")) << stmt.GetECSql() << " - " << defaultJson.ToString().c_str();
    ASSERT_EQ(123, defaultJson["123_1"].asInt()) << stmt.GetECSql() << " - " << defaultJson.ToString().c_str();

    ASSERT_TRUE(javaScriptJson.isMember("123_1")) << stmt.GetECSql() << " - " << javaScriptJson.ToString().c_str();
    ASSERT_EQ(123, javaScriptJson["123_1"].asInt()) << stmt.GetECSql() << " - " << javaScriptJson.ToString().c_str();
    }

    {
    ASSERT_TRUE(defaultJson.isMember("MyNumber")) << stmt.GetECSql() << " - " << defaultJson.ToString().c_str();
    ASSERT_EQ(123, defaultJson["MyNumber"].asInt()) << stmt.GetECSql() << " - " << defaultJson.ToString().c_str();

    ASSERT_TRUE(javaScriptJson.isMember("myNumber")) << stmt.GetECSql() << " - " << javaScriptJson.ToString().c_str();
    ASSERT_EQ(123, javaScriptJson["myNumber"].asInt()) << stmt.GetECSql() << " - " << javaScriptJson.ToString().c_str();
    }

    {
    ASSERT_TRUE(defaultJson.isMember("123 * 3")) << stmt.GetECSql() << " - " << defaultJson.ToString().c_str();
    ASSERT_EQ(369, defaultJson["123 * 3"].asInt()) << stmt.GetECSql() << " - " << defaultJson.ToString().c_str();

    ASSERT_TRUE(javaScriptJson.isMember("123 * 3")) << stmt.GetECSql() << " - " << javaScriptJson.ToString().c_str();
    ASSERT_EQ(369, javaScriptJson["123 * 3"].asInt()) << stmt.GetECSql() << " - " << javaScriptJson.ToString().c_str();
    }

    {
    ASSERT_TRUE(defaultJson.isMember("MyProduct")) << stmt.GetECSql() << " - " << defaultJson.ToString().c_str();
    ASSERT_EQ(369, defaultJson["MyProduct"].asInt()) << stmt.GetECSql() << " - " << defaultJson.ToString().c_str();

    ASSERT_TRUE(javaScriptJson.isMember("myProduct")) << stmt.GetECSql() << " - " << javaScriptJson.ToString().c_str();
    ASSERT_EQ(369, javaScriptJson["myProduct"].asInt()) << stmt.GetECSql() << " - " << javaScriptJson.ToString().c_str();
    stmt.Finalize();
    }

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT IntProp, IntProp As DupIntProp, IntProp, IntProp, IntProp + 10, IntProp + 10 IntPlus10, IntProp + IntProp, IntProp + IntProp [IntProp plus IntProp] FROM ts.Foo"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    ASSERT_EQ(SUCCESS, defaultAdapter.GetRow(defaultJson));
    ASSERT_EQ(SUCCESS, javaScriptAdapter.GetRow(javaScriptJson));

    {
    ASSERT_TRUE(defaultJson.isObject()) << defaultJson.ToString().c_str();
    ASSERT_EQ(stmt.GetColumnCount(), (int) defaultJson.size()) << stmt.GetECSql() << " - " << defaultJson.ToString().c_str();

    ASSERT_TRUE(javaScriptJson.isObject()) << javaScriptJson.ToString().c_str();
    ASSERT_EQ(stmt.GetColumnCount(), (int) javaScriptJson.size()) << stmt.GetECSql() << " - " << javaScriptJson.ToString().c_str();
    }

    {
    ASSERT_TRUE(defaultJson.isMember("IntProp")) << stmt.GetECSql() << " - " << defaultJson.ToString().c_str();
    ASSERT_EQ(1000, defaultJson["IntProp"].asInt()) << stmt.GetECSql() << " - " << defaultJson.ToString().c_str();

    ASSERT_TRUE(javaScriptJson.isMember("intProp")) << stmt.GetECSql() << " - " << javaScriptJson.ToString().c_str();
    ASSERT_EQ(1000, javaScriptJson["intProp"].asInt()) << stmt.GetECSql() << " - " << javaScriptJson.ToString().c_str();
    }

    {
    ASSERT_TRUE(defaultJson.isMember("DupIntProp")) << stmt.GetECSql() << " - " << defaultJson.ToString().c_str();
    ASSERT_EQ(1000, defaultJson["DupIntProp"].asInt()) << stmt.GetECSql() << " - " << defaultJson.ToString().c_str();

    ASSERT_TRUE(javaScriptJson.isMember("dupIntProp")) << stmt.GetECSql() << " - " << javaScriptJson.ToString().c_str();
    ASSERT_EQ(1000, javaScriptJson["dupIntProp"].asInt()) << stmt.GetECSql() << " - " << javaScriptJson.ToString().c_str();
    }

    {
    ASSERT_TRUE(defaultJson.isMember("IntProp_1")) << stmt.GetECSql() << " - " << defaultJson.ToString().c_str();
    ASSERT_EQ(1000, defaultJson["IntProp_1"].asInt()) << stmt.GetECSql() << " - " << defaultJson.ToString().c_str();

    ASSERT_TRUE(javaScriptJson.isMember("intProp_1")) << stmt.GetECSql() << " - " << javaScriptJson.ToString().c_str();
    ASSERT_EQ(1000, javaScriptJson["intProp_1"].asInt()) << stmt.GetECSql() << " - " << javaScriptJson.ToString().c_str();
    }

    {
    ASSERT_TRUE(defaultJson.isMember("IntProp_2")) << stmt.GetECSql() << " - " << defaultJson.ToString().c_str();
    ASSERT_EQ(1000, defaultJson["IntProp_2"].asInt()) << stmt.GetECSql() << " - " << defaultJson.ToString().c_str();

    ASSERT_TRUE(javaScriptJson.isMember("intProp_2")) << stmt.GetECSql() << " - " << javaScriptJson.ToString().c_str();
    ASSERT_EQ(1000, javaScriptJson["intProp_2"].asInt()) << stmt.GetECSql() << " - " << javaScriptJson.ToString().c_str();
    }

    {
    ASSERT_TRUE(defaultJson.isMember("[IntProp] + 10")) << stmt.GetECSql() << " - " << defaultJson.ToString().c_str();
    ASSERT_EQ(1010, defaultJson["[IntProp] + 10"].asInt()) << stmt.GetECSql() << " - " << defaultJson.ToString().c_str();

    ASSERT_TRUE(javaScriptJson.isMember("[IntProp] + 10")) << stmt.GetECSql() << " - " << javaScriptJson.ToString().c_str();
    ASSERT_EQ(1010, javaScriptJson["[IntProp] + 10"].asInt()) << stmt.GetECSql() << " - " << javaScriptJson.ToString().c_str();
    }

    {
    ASSERT_TRUE(defaultJson.isMember("IntPlus10")) << stmt.GetECSql() << " - " << defaultJson.ToString().c_str();
    ASSERT_EQ(1010, defaultJson["IntPlus10"].asInt()) << stmt.GetECSql() << " - " << defaultJson.ToString().c_str();

    ASSERT_TRUE(javaScriptJson.isMember("intPlus10")) << stmt.GetECSql() << " - " << javaScriptJson.ToString().c_str();
    ASSERT_EQ(1010, javaScriptJson["intPlus10"].asInt()) << stmt.GetECSql() << " - " << javaScriptJson.ToString().c_str();
    }

    {
    ASSERT_TRUE(defaultJson.isMember("[IntProp] + [IntProp]")) << stmt.GetECSql() << " - " << defaultJson.ToString().c_str();
    ASSERT_EQ(2000, defaultJson["[IntProp] + [IntProp]"].asInt()) << stmt.GetECSql() << " - " << defaultJson.ToString().c_str();

    ASSERT_TRUE(javaScriptJson.isMember("[IntProp] + [IntProp]")) << stmt.GetECSql() << " - " << javaScriptJson.ToString().c_str();
    ASSERT_EQ(2000, javaScriptJson["[IntProp] + [IntProp]"].asInt()) << stmt.GetECSql() << " - " << javaScriptJson.ToString().c_str();
    }

    {
    ASSERT_TRUE(defaultJson.isMember("IntProp plus IntProp")) << stmt.GetECSql() << " - " << defaultJson.ToString().c_str();
    ASSERT_EQ(2000, defaultJson["IntProp plus IntProp"].asInt()) << stmt.GetECSql() << " - " << defaultJson.ToString().c_str();

    ASSERT_TRUE(javaScriptJson.isMember("intProp plus IntProp")) << stmt.GetECSql() << " - " << javaScriptJson.ToString().c_str();
    ASSERT_EQ(2000, javaScriptJson["intProp plus IntProp"].asInt()) << stmt.GetECSql() << " - " << javaScriptJson.ToString().c_str();
    stmt.Finalize();
    }

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT PtProp.X, PtProp.Y, PtProp.Z FROM ts.Foo"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(SUCCESS, defaultAdapter.GetRow(defaultJson));
    ASSERT_EQ(SUCCESS, javaScriptAdapter.GetRow(javaScriptJson));

    {
    ASSERT_TRUE(defaultJson.isObject()) << defaultJson.ToString().c_str();
    ASSERT_EQ(stmt.GetColumnCount(), (int) defaultJson.size()) << stmt.GetECSql() << " - " << defaultJson.ToString().c_str();

    ASSERT_TRUE(javaScriptJson.isObject()) << javaScriptJson.ToString().c_str();
    ASSERT_EQ(stmt.GetColumnCount(), (int) javaScriptJson.size()) << stmt.GetECSql() << " - " << javaScriptJson.ToString().c_str();
    }

    {
    ASSERT_TRUE(defaultJson.isMember("PtProp.X")) << stmt.GetECSql() << " - " << defaultJson.ToString().c_str();
    ASSERT_DOUBLE_EQ(1000.0, defaultJson["PtProp.X"].asDouble()) << stmt.GetECSql() << " - " << defaultJson.ToString().c_str();
    ASSERT_TRUE(defaultJson.isMember("PtProp.Y")) << stmt.GetECSql() << " - " << defaultJson.ToString().c_str();
    ASSERT_DOUBLE_EQ(1000.0, defaultJson["PtProp.Y"].asDouble()) << stmt.GetECSql() << " - " << defaultJson.ToString().c_str();
    ASSERT_TRUE(defaultJson.isMember("PtProp.Z")) << stmt.GetECSql() << " - " << defaultJson.ToString().c_str();
    ASSERT_DOUBLE_EQ(1000.0, defaultJson["PtProp.Z"].asDouble()) << stmt.GetECSql() << " - " << defaultJson.ToString().c_str();

    ASSERT_TRUE(javaScriptJson.isMember("ptProp.X")) << stmt.GetECSql() << " - " << javaScriptJson.ToString().c_str();
    ASSERT_DOUBLE_EQ(1000.0, javaScriptJson["ptProp.X"].asDouble()) << stmt.GetECSql() << " - " << javaScriptJson.ToString().c_str();
    ASSERT_TRUE(javaScriptJson.isMember("ptProp.Y")) << stmt.GetECSql() << " - " << javaScriptJson.ToString().c_str();
    ASSERT_DOUBLE_EQ(1000.0, javaScriptJson["ptProp.Y"].asDouble()) << stmt.GetECSql() << " - " << javaScriptJson.ToString().c_str();
    ASSERT_TRUE(javaScriptJson.isMember("ptProp.Z")) << stmt.GetECSql() << " - " << javaScriptJson.ToString().c_str();
    ASSERT_DOUBLE_EQ(1000.0, javaScriptJson["ptProp.Z"].asDouble()) << stmt.GetECSql() << " - " << javaScriptJson.ToString().c_str();
    stmt.Finalize();
    }

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT StructProp.SomeNumber FROM ts.Foo"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(SUCCESS, defaultAdapter.GetRow(defaultJson));
    ASSERT_EQ(SUCCESS, javaScriptAdapter.GetRow(javaScriptJson));

    {
    ASSERT_TRUE(defaultJson.isObject()) << defaultJson.ToString().c_str();
    ASSERT_EQ(stmt.GetColumnCount(), (int) defaultJson.size()) << stmt.GetECSql() << " - " << defaultJson.ToString().c_str();

    ASSERT_TRUE(javaScriptJson.isObject()) << javaScriptJson.ToString().c_str();
    ASSERT_EQ(stmt.GetColumnCount(), (int) javaScriptJson.size()) << stmt.GetECSql() << " - " << javaScriptJson.ToString().c_str();
    }

    {
    ASSERT_TRUE(defaultJson.isMember("StructProp.SomeNumber")) << stmt.GetECSql() << " - " << defaultJson.ToString().c_str();
    ASSERT_EQ(1000, defaultJson["StructProp.SomeNumber"].asDouble()) << stmt.GetECSql() << " - " << defaultJson.ToString().c_str();

    ASSERT_TRUE(javaScriptJson.isMember("structProp.SomeNumber")) << stmt.GetECSql() << " - " << javaScriptJson.ToString().c_str();
    ASSERT_EQ(1000, javaScriptJson["structProp.SomeNumber"].asDouble()) << stmt.GetECSql() << " - " << javaScriptJson.ToString().c_str();
    stmt.Finalize();
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                09/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonECSqlSelectAdapterTests, ReservedWordsCollisions)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("jsonecsqlselectadapter_ReservedWordsCollisions.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                                                        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                                                            <ECEntityClass typeName="Foo" >
                                                                                <ECProperty propertyName="className" typeName="string" />
                                                                            </ECEntityClass>
                                                                            <ECRelationshipClass typeName="Rel" strength="referencing" modifier="None">
                                                                                <ECProperty propertyName="sourceClassName" typeName="string" />
                                                                                <ECProperty propertyName="targetClassName" typeName="string" />
                                                                                <Source multiplicity="(0..*)" roleLabel="has" polymorphic="False"><Class class="Foo"/></Source>
                                                                                <Target multiplicity="(0..*)" roleLabel="referenced by" polymorphic="False"><Class class="Foo"/></Target>
                                                                            </ECRelationshipClass>
                                                                        </ECSchema>)xml")));
    ECInstanceKey fooKey, relKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(fooKey, "INSERT INTO ts.Foo(className) VALUES('Foo class')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(relKey, Utf8PrintfString("INSERT INTO ts.Rel(SourceECInstanceId, TargetECInstanceId, sourceClassName, targetClassName) VALUES(%s,%s,'source','target')",
                                                                                      fooKey.GetInstanceId().ToString().c_str(), fooKey.GetInstanceId().ToString().c_str()).c_str()));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECClassId, className FROM ts.Foo"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    JsonECSqlSelectAdapter defaultAdapter(stmt);
    JsonECSqlSelectAdapter javaScriptAdapter(stmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::LowerFirstChar, ECJsonInt64Format::AsDecimalString));
    Json::Value defaultJson, javaScriptJson;

    ASSERT_EQ(SUCCESS, defaultAdapter.GetRow(defaultJson));
    ASSERT_EQ(SUCCESS, javaScriptAdapter.GetRow(javaScriptJson));

    {
    ASSERT_TRUE(defaultJson.isObject()) << defaultJson.ToString().c_str();
    ASSERT_EQ(2, defaultJson.size()) << defaultJson.ToString().c_str();
    ASSERT_TRUE(javaScriptJson.isObject()) << javaScriptJson.ToString().c_str();
    ASSERT_EQ(2, javaScriptJson.size()) << javaScriptJson.ToString().c_str();
    }

    {
    ASSERT_TRUE(defaultJson.isMember(ECJsonUtilities::json_className())) << defaultJson.ToString().c_str();
    ASSERT_STREQ("TestSchema.Foo", defaultJson[ECJsonUtilities::json_className()].asCString()) << defaultJson.ToString().c_str();

    ASSERT_TRUE(javaScriptJson.isMember(ECJsonUtilities::json_className())) << javaScriptJson.ToString().c_str();
    ASSERT_STREQ("TestSchema.Foo", javaScriptJson[ECJsonUtilities::json_className()].asCString()) << javaScriptJson.ToString().c_str();
    }

    {
    Utf8String className1(ECJsonUtilities::json_className());
    className1.append("_1");
    ASSERT_TRUE(defaultJson.isMember(className1)) << defaultJson.ToString().c_str();
    ASSERT_STREQ("Foo class", defaultJson[className1].asCString()) << defaultJson.ToString().c_str();

    ASSERT_TRUE(javaScriptJson.isMember(className1)) << javaScriptJson.ToString().c_str();
    ASSERT_STREQ("Foo class", javaScriptJson[className1].asCString()) << javaScriptJson.ToString().c_str();
    stmt.Finalize();
    }

    //now using aliases to avoid collision
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECClassId AS SystemClassName, className FROM ts.Foo"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(SUCCESS, defaultAdapter.GetRow(defaultJson));
    ASSERT_EQ(SUCCESS, javaScriptAdapter.GetRow(javaScriptJson));

    {
    ASSERT_TRUE(defaultJson.isObject()) << defaultJson.ToString().c_str();
    ASSERT_EQ(2, defaultJson.size()) << defaultJson.ToString().c_str();
    ASSERT_TRUE(javaScriptJson.isObject()) << javaScriptJson.ToString().c_str();
    ASSERT_EQ(2, javaScriptJson.size()) << javaScriptJson.ToString().c_str();
    }

    {
    ASSERT_TRUE(defaultJson.isMember("SystemClassName")) << defaultJson.ToString().c_str();
    ASSERT_STREQ(fooKey.GetClassId().ToString().c_str(), defaultJson["SystemClassName"].asCString()) << defaultJson.ToString().c_str();
    ASSERT_TRUE(javaScriptJson.isMember("systemClassName")) << javaScriptJson.ToString().c_str();
    ASSERT_STREQ(fooKey.GetClassId().ToString().c_str(), javaScriptJson["systemClassName"].asCString()) << javaScriptJson.ToString().c_str();
    }

    {
    ASSERT_TRUE(defaultJson.isMember("className")) << defaultJson.ToString().c_str();
    ASSERT_STREQ("Foo class", defaultJson["className"].asCString()) << defaultJson.ToString().c_str();
    ASSERT_TRUE(javaScriptJson.isMember("className")) << javaScriptJson.ToString().c_str();
    ASSERT_STREQ("Foo class", javaScriptJson["className"].asCString()) << javaScriptJson.ToString().c_str();
    stmt.Finalize();
    }

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECClassId, className AS UserClassName FROM ts.Foo"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    ASSERT_EQ(SUCCESS, defaultAdapter.GetRow(defaultJson));
    ASSERT_EQ(SUCCESS, javaScriptAdapter.GetRow(javaScriptJson));

    {
    ASSERT_TRUE(defaultJson.isObject()) << defaultJson.ToString().c_str();
    ASSERT_EQ(2, defaultJson.size()) << defaultJson.ToString().c_str();
    ASSERT_TRUE(javaScriptJson.isObject()) << javaScriptJson.ToString().c_str();
    ASSERT_EQ(2, javaScriptJson.size()) << javaScriptJson.ToString().c_str();
    }

    {
    ASSERT_TRUE(defaultJson.isMember("className")) << defaultJson.ToString().c_str();
    ASSERT_STREQ("TestSchema.Foo", defaultJson["className"].asCString()) << defaultJson.ToString().c_str();
    ASSERT_TRUE(javaScriptJson.isMember("className")) << javaScriptJson.ToString().c_str();
    ASSERT_STREQ("TestSchema.Foo", javaScriptJson["className"].asCString()) << javaScriptJson.ToString().c_str();
    }

    {
    ASSERT_TRUE(defaultJson.isMember("UserClassName")) << defaultJson.ToString().c_str();
    ASSERT_STREQ("Foo class", defaultJson["UserClassName"].asCString()) << defaultJson.ToString().c_str();
    ASSERT_TRUE(javaScriptJson.isMember("userClassName")) << javaScriptJson.ToString().c_str();
    ASSERT_STREQ("Foo class", javaScriptJson["userClassName"].asCString()) << javaScriptJson.ToString().c_str();
    stmt.Finalize();
    }

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT SourceECClassId, sourceClassName, TargetECClassId, targetClassName FROM ts.Rel"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(SUCCESS, defaultAdapter.GetRow(defaultJson));
    ASSERT_EQ(SUCCESS, javaScriptAdapter.GetRow(javaScriptJson));

    {
    ASSERT_TRUE(defaultJson.isObject()) << defaultJson.ToString().c_str();
    ASSERT_EQ(4, defaultJson.size()) << defaultJson.ToString().c_str();
    ASSERT_TRUE(javaScriptJson.isObject()) << javaScriptJson.ToString().c_str();
    ASSERT_EQ(4, javaScriptJson.size()) << javaScriptJson.ToString().c_str();
    }

    {
    ASSERT_TRUE(defaultJson.isMember(ECJsonUtilities::json_sourceClassName())) << defaultJson.ToString().c_str();
    ASSERT_STREQ("TestSchema.Foo", defaultJson[ECJsonUtilities::json_sourceClassName()].asCString()) << defaultJson.ToString().c_str();
    ASSERT_TRUE(javaScriptJson.isMember(ECJsonUtilities::json_sourceClassName())) << javaScriptJson.ToString().c_str();
    ASSERT_STREQ("TestSchema.Foo", javaScriptJson[ECJsonUtilities::json_sourceClassName()].asCString()) << javaScriptJson.ToString().c_str();
    }

    {
    Utf8String sourceClassName1(ECJsonUtilities::json_sourceClassName());
    sourceClassName1.append("_1");
    ASSERT_TRUE(defaultJson.isMember(sourceClassName1)) << defaultJson.ToString().c_str();
    ASSERT_STREQ("source", defaultJson[sourceClassName1].asCString()) << defaultJson.ToString().c_str();
    ASSERT_TRUE(javaScriptJson.isMember(sourceClassName1)) << javaScriptJson.ToString().c_str();
    ASSERT_STREQ("source", javaScriptJson[sourceClassName1].asCString()) << javaScriptJson.ToString().c_str();
    }

    {
    ASSERT_TRUE(defaultJson.isMember(ECJsonUtilities::json_targetClassName())) << defaultJson.ToString().c_str();
    ASSERT_STREQ("TestSchema.Foo", defaultJson[ECJsonUtilities::json_targetClassName()].asCString()) << defaultJson.ToString().c_str();
    ASSERT_TRUE(javaScriptJson.isMember(ECJsonUtilities::json_targetClassName())) << javaScriptJson.ToString().c_str();
    ASSERT_STREQ("TestSchema.Foo", javaScriptJson[ECJsonUtilities::json_targetClassName()].asCString()) << javaScriptJson.ToString().c_str();
    }

    {
    Utf8String targetClassName1(ECJsonUtilities::json_targetClassName());
    targetClassName1.append("_1");
    ASSERT_TRUE(defaultJson.isMember(targetClassName1)) << defaultJson.ToString().c_str();
    ASSERT_STREQ("target", defaultJson[targetClassName1].asCString()) << defaultJson.ToString().c_str();
    ASSERT_TRUE(javaScriptJson.isMember(targetClassName1)) << javaScriptJson.ToString().c_str();
    ASSERT_STREQ("target", javaScriptJson[targetClassName1].asCString()) << javaScriptJson.ToString().c_str();
    }
    stmt.Finalize();

    //now using aliases to avoid collision
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT SourceECClassId SystemSourceClassName, sourceClassName, TargetECClassId SystemTargetClassName, targetClassName FROM ts.Rel"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(SUCCESS, defaultAdapter.GetRow(defaultJson));
    ASSERT_EQ(SUCCESS, javaScriptAdapter.GetRow(javaScriptJson));

    {
    ASSERT_TRUE(defaultJson.isObject()) << defaultJson.ToString().c_str();
    ASSERT_EQ(4, defaultJson.size()) << defaultJson.ToString().c_str();
    ASSERT_TRUE(javaScriptJson.isObject()) << javaScriptJson.ToString().c_str();
    ASSERT_EQ(4, javaScriptJson.size()) << javaScriptJson.ToString().c_str();
    }

    {
    ASSERT_TRUE(defaultJson.isMember("SystemSourceClassName")) << defaultJson.ToString().c_str();
    ASSERT_STREQ(fooKey.GetClassId().ToString().c_str(), defaultJson["SystemSourceClassName"].asCString()) << defaultJson.ToString().c_str();
    ASSERT_TRUE(javaScriptJson.isMember("systemSourceClassName")) << javaScriptJson.ToString().c_str();
    ASSERT_STREQ(fooKey.GetClassId().ToString().c_str(), javaScriptJson["systemSourceClassName"].asCString()) << javaScriptJson.ToString().c_str();
    }

    {
    ASSERT_TRUE(defaultJson.isMember("sourceClassName")) << defaultJson.ToString().c_str();
    ASSERT_STREQ("source", defaultJson["sourceClassName"].asCString()) << defaultJson.ToString().c_str();
    ASSERT_TRUE(javaScriptJson.isMember("sourceClassName")) << javaScriptJson.ToString().c_str();
    ASSERT_STREQ("source", javaScriptJson["sourceClassName"].asCString()) << javaScriptJson.ToString().c_str();
    }

    {
    ASSERT_TRUE(defaultJson.isMember("SystemTargetClassName")) << defaultJson.ToString().c_str();
    ASSERT_STREQ(fooKey.GetClassId().ToString().c_str(), defaultJson["SystemTargetClassName"].asCString()) << defaultJson.ToString().c_str();
    ASSERT_TRUE(javaScriptJson.isMember("systemTargetClassName")) << javaScriptJson.ToString().c_str();
    ASSERT_STREQ(fooKey.GetClassId().ToString().c_str(), javaScriptJson["systemTargetClassName"].asCString()) << javaScriptJson.ToString().c_str();
    }

    {
    ASSERT_TRUE(defaultJson.isMember("targetClassName")) << defaultJson.ToString().c_str();
    ASSERT_STREQ("target", defaultJson["targetClassName"].asCString()) << defaultJson.ToString().c_str();
    ASSERT_TRUE(javaScriptJson.isMember("targetClassName")) << javaScriptJson.ToString().c_str();
    ASSERT_STREQ("target", javaScriptJson["targetClassName"].asCString()) << javaScriptJson.ToString().c_str();
    }

    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT SourceECClassId, sourceClassName UserSourceClass, TargetECClassId, targetClassName UserTargetClass FROM ts.Rel"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(SUCCESS, defaultAdapter.GetRow(defaultJson));
    ASSERT_EQ(SUCCESS, javaScriptAdapter.GetRow(javaScriptJson));

    {
    ASSERT_TRUE(defaultJson.isObject()) << defaultJson.ToString().c_str();
    ASSERT_EQ(4, defaultJson.size()) << defaultJson.ToString().c_str();
    ASSERT_TRUE(javaScriptJson.isObject()) << javaScriptJson.ToString().c_str();
    ASSERT_EQ(4, javaScriptJson.size()) << javaScriptJson.ToString().c_str();
    }

    {
    ASSERT_TRUE(defaultJson.isMember(ECJsonUtilities::json_sourceClassName())) << defaultJson.ToString().c_str();
    ASSERT_STREQ("TestSchema.Foo", defaultJson[ECJsonUtilities::json_sourceClassName()].asCString()) << defaultJson.ToString().c_str();
    ASSERT_TRUE(javaScriptJson.isMember(ECJsonUtilities::json_sourceClassName())) << javaScriptJson.ToString().c_str();
    ASSERT_STREQ("TestSchema.Foo", javaScriptJson[ECJsonUtilities::json_sourceClassName()].asCString()) << javaScriptJson.ToString().c_str();
    }

    {
    ASSERT_TRUE(defaultJson.isMember("UserSourceClass")) << defaultJson.ToString().c_str();
    ASSERT_STREQ("source", defaultJson["UserSourceClass"].asCString()) << defaultJson.ToString().c_str();
    ASSERT_TRUE(javaScriptJson.isMember("userSourceClass")) << javaScriptJson.ToString().c_str();
    ASSERT_STREQ("source", javaScriptJson["userSourceClass"].asCString()) << javaScriptJson.ToString().c_str();
    }

    {
    ASSERT_TRUE(defaultJson.isMember(ECJsonUtilities::json_targetClassName())) << defaultJson.ToString().c_str();
    ASSERT_STREQ("TestSchema.Foo", defaultJson[ECJsonUtilities::json_targetClassName()].asCString()) << defaultJson.ToString().c_str();
    ASSERT_TRUE(javaScriptJson.isMember(ECJsonUtilities::json_targetClassName())) << javaScriptJson.ToString().c_str();
    ASSERT_STREQ("TestSchema.Foo", javaScriptJson[ECJsonUtilities::json_targetClassName()].asCString()) << javaScriptJson.ToString().c_str();
    }

    {
    ASSERT_TRUE(defaultJson.isMember("UserTargetClass")) << defaultJson.ToString().c_str();
    ASSERT_STREQ("target", defaultJson["UserTargetClass"].asCString()) << defaultJson.ToString().c_str();
    ASSERT_TRUE(javaScriptJson.isMember("userTargetClass")) << javaScriptJson.ToString().c_str();
    ASSERT_STREQ("target", javaScriptJson["userTargetClass"].asCString()) << javaScriptJson.ToString().c_str();
    }
    stmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                09/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonECSqlSelectAdapterTests, DataTypes)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("jsonecsqlselectadapter_datatypes.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                                                        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                                                            <ECStructClass typeName="Person" >
                                                                                <ECProperty propertyName="Name" typeName="string" />
                                                                                <ECProperty propertyName="Age" typeName="int" />
                                                                            </ECStructClass>
                                                                            <ECEntityClass typeName="Foo" >
                                                                                <ECProperty propertyName="B" typeName="boolean" />
                                                                                <ECProperty propertyName="Bi" typeName="binary" />
                                                                                <ECProperty propertyName="D" typeName="double" />
                                                                                <ECProperty propertyName="Dt" typeName="dateTime" />
                                                                                <ECProperty propertyName="G" typeName="Bentley.Geometry.Common.IGeometry" />
                                                                                <ECProperty propertyName="I" typeName="integer" />
                                                                                <ECProperty propertyName="L" typeName="long" />
                                                                                <ECProperty propertyName="P2D" typeName="Point2d" />
                                                                                <ECProperty propertyName="P3D" typeName="Point3d" />
                                                                                <ECProperty propertyName="S" typeName="String" />
                                                                                <ECArrayProperty propertyName="D_Array" typeName="double" />
                                                                                <ECStructProperty propertyName="Struct" typeName="Person" />
                                                                                <ECStructArrayProperty propertyName="Struct_Array" typeName="Person" />
                                                                                <ECNavigationProperty propertyName="Parent" relationshipName="ParentHasFoo" direction="Backward" />
                                                                            </ECEntityClass>
                                                                            <ECEntityClass typeName="Parent" />
                                                                            <ECRelationshipClass typeName="ParentHasFoo" strength="referencing" modifier="None">
                                                                                <Source multiplicity="(0..1)" roleLabel="has" polymorphic="False"><Class class="Parent"/></Source>
                                                                                <Target multiplicity="(0..*)" roleLabel="referenced by" polymorphic="False"><Class class="Foo"/></Target>
                                                                            </ECRelationshipClass>
                                                                        </ECSchema>)xml")));

    ECInstanceKey parentKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(parentKey, "INSERT INTO ts.Parent(ECInstanceId) VALUES(NULL)"));

    const bool boolVal = true;
    const double doubleVal = 3.14;
    const DateTime dtVal = DateTime(DateTime::Kind::Unspecified, 2017,9,18,13,40);
    const int intVal = 123;
    const int64_t int64Val = INT64_C(123123123123);
    Utf8String int64Str;
    int64Str.Sprintf("%" PRIi64, int64Val);

    const DPoint2d p2dVal = DPoint2d::From(-1.1, 2.5);
    const DPoint3d p3dVal = DPoint3d::From(1.0, 2.0, -3.0);
    IGeometryPtr geomVal = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, 1.0, 1.0, 1.0)));

    void const* blobVal = &int64Val;
    const size_t blobSize = sizeof(int64Val);
    Utf8String blobValBase64Str;
    Base64Utilities::Encode(blobValBase64Str, (Byte const*) blobVal, blobSize);

    Utf8CP stringVal = "Hello, world";
    Utf8CP personName = "Johnny";
    const int personAge = 14;


    ECInstanceKey key;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Foo(B,Bi,D,Dt,G,I,L,P2D,P3D,S,Struct,D_Array,Struct_Array,Parent) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindBoolean(1, boolVal));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindBlob(2, blobVal, blobSize, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(3, doubleVal));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDateTime(4, dtVal));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindGeometry(5, *geomVal));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(6, intVal));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(7, int64Val));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint2d(8, p2dVal));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint3d(9, p3dVal));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(10, stringVal, IECSqlBinder::MakeCopy::No));

    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(11)["Name"].BindText(personName, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(11)["Age"].BindInt(personAge));

    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(12).AddArrayElement().BindDouble(doubleVal));
    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(12).AddArrayElement().BindDouble(doubleVal));

    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(13).AddArrayElement()["Name"].BindText(personName, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(13).AddArrayElement()["Name"].BindText(personName, IECSqlBinder::MakeCopy::No));

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(14, parentKey.GetInstanceId(), m_ecdb.Schemas().GetClassId("TestSchema", "ParentHasFoo")));

    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    }

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId, ECClassId, B,Bi,D,Dt,G,I,L,P2D,P3D,S,Struct,D_Array,Struct_Array,Parent FROM ts.Foo WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    JsonECSqlSelectAdapter adapter(stmt);
    Json::Value actualJson;
    ASSERT_EQ(SUCCESS, adapter.GetRow(actualJson));
    ASSERT_TRUE(actualJson.isObject()) << actualJson.ToString().c_str();
    ASSERT_EQ(16, actualJson.size()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember(ECJsonUtilities::json_id())) << actualJson.ToString().c_str();
    EXPECT_STREQ(key.GetInstanceId().ToHexStr().c_str(), actualJson[ECJsonUtilities::json_id()].asCString()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember(ECJsonUtilities::json_className())) << actualJson.ToString().c_str();
    EXPECT_STREQ("TestSchema.Foo", actualJson[ECJsonUtilities::json_className()].asCString()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember("B")) << actualJson.ToString().c_str();
    EXPECT_EQ(boolVal, actualJson["B"].asBool()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember("Bi")) << actualJson.ToString().c_str();
    EXPECT_STREQ(blobValBase64Str.c_str(), actualJson["Bi"].asCString()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember("D")) << actualJson.ToString().c_str();
    EXPECT_DOUBLE_EQ(doubleVal, actualJson["D"].asDouble()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember("Dt")) << actualJson.ToString().c_str();
    EXPECT_STREQ(dtVal.ToString().c_str(), actualJson["Dt"].asCString()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember("G")) << actualJson.ToString().c_str();
    ASSERT_TRUE(actualJson["G"].isObject()) << actualJson.ToString().c_str();
    EXPECT_STREQ("{\"LineSegment\":{\"endPoint\":[1.0,1.0,1.0],\"startPoint\":[0.0,0.0,0.0]}}", actualJson["G"].ToString().c_str()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember("I")) << actualJson.ToString().c_str();
    EXPECT_EQ(intVal, actualJson["I"].asInt()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember("L")) << actualJson.ToString().c_str();
    EXPECT_STREQ(int64Str.c_str(), actualJson["L"].asCString()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember("P2D")) << actualJson.ToString().c_str();
    ASSERT_TRUE(actualJson["P2D"].isObject()) << actualJson.ToString().c_str();
    EXPECT_DOUBLE_EQ(p2dVal.x, actualJson["P2D"]["x"].asDouble()) << actualJson.ToString().c_str();
    EXPECT_DOUBLE_EQ(p2dVal.y, actualJson["P2D"]["y"].asDouble()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember("P3D")) << actualJson.ToString().c_str();
    ASSERT_TRUE(actualJson["P3D"].isObject()) << actualJson.ToString().c_str();
    EXPECT_DOUBLE_EQ(p3dVal.x, actualJson["P3D"]["x"].asDouble()) << actualJson.ToString().c_str();
    EXPECT_DOUBLE_EQ(p3dVal.y, actualJson["P3D"]["y"].asDouble()) << actualJson.ToString().c_str();
    EXPECT_DOUBLE_EQ(p3dVal.z, actualJson["P3D"]["z"].asDouble()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember("S")) << actualJson.ToString().c_str();
    EXPECT_STREQ(stringVal, actualJson["S"].asCString()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember("Struct")) << actualJson.ToString().c_str();
    ASSERT_TRUE(actualJson["Struct"].isObject()) << actualJson.ToString().c_str();
    EXPECT_STREQ(personName, actualJson["Struct"]["Name"].asCString()) << actualJson.ToString().c_str();
    EXPECT_EQ(personAge, actualJson["Struct"]["Age"].asInt()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember("D_Array")) << actualJson.ToString().c_str();
    ASSERT_TRUE(actualJson["D_Array"].isArray()) << actualJson.ToString().c_str();
    ASSERT_EQ(2, actualJson["D_Array"].size()) << actualJson.ToString().c_str();
    EXPECT_DOUBLE_EQ(doubleVal, actualJson["D_Array"][0].asDouble()) << actualJson.ToString().c_str();
    EXPECT_DOUBLE_EQ(doubleVal, actualJson["D_Array"][1].asDouble()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember("Struct_Array")) << actualJson.ToString().c_str();
    ASSERT_TRUE(actualJson["Struct_Array"].isArray()) << actualJson.ToString().c_str();
    ASSERT_EQ(2, actualJson["Struct_Array"].size()) << actualJson.ToString().c_str();
    EXPECT_STREQ(personName, actualJson["Struct_Array"][0]["Name"].asCString()) << actualJson.ToString().c_str();
    EXPECT_FALSE(actualJson["Struct_Array"][0]["Age"]) << actualJson.ToString().c_str();
    EXPECT_STREQ(personName, actualJson["Struct_Array"][1]["Name"].asCString()) << actualJson.ToString().c_str();
    EXPECT_FALSE(actualJson["Struct_Array"][1]["Age"]) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember("Parent")) << actualJson.ToString().c_str();
    ASSERT_TRUE(actualJson["Parent"].isObject()) << actualJson.ToString().c_str();
    EXPECT_STREQ(parentKey.GetInstanceId().ToHexStr().c_str(), actualJson["Parent"][ECJsonUtilities::json_navId()].asCString()) << actualJson.ToString().c_str();
    ASSERT_TRUE(actualJson["Parent"].isMember(ECJsonUtilities::json_navRelClassName())) << actualJson.ToString().c_str();
    EXPECT_STREQ("TestSchema.ParentHasFoo", actualJson["Parent"][ECJsonUtilities::json_navRelClassName()].asCString()) << actualJson.ToString().c_str();

    //SELECT FROM relationship
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId, ECClassId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM ts.ParentHasFoo WHERE TargetECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    JsonECSqlSelectAdapter relAdapter(stmt);

    actualJson;
    ASSERT_EQ(SUCCESS, relAdapter.GetRow(actualJson));
    ASSERT_TRUE(actualJson.isObject()) << actualJson.ToString().c_str();
    ASSERT_EQ(6, actualJson.size()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember(ECJsonUtilities::json_id())) << actualJson.ToString().c_str();
    EXPECT_STREQ(key.GetInstanceId().ToHexStr().c_str(), actualJson[ECJsonUtilities::json_id()].asCString()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember(ECJsonUtilities::json_className())) << actualJson.ToString().c_str();
    EXPECT_STREQ("TestSchema.ParentHasFoo", actualJson[ECJsonUtilities::json_className()].asCString()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember(ECJsonUtilities::json_sourceId())) << actualJson.ToString().c_str();
    EXPECT_STREQ(parentKey.GetInstanceId().ToHexStr().c_str(), actualJson[ECJsonUtilities::json_sourceId()].asCString()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember(ECJsonUtilities::json_sourceClassName())) << actualJson.ToString().c_str();
    EXPECT_STREQ("TestSchema.Parent", actualJson[ECJsonUtilities::json_sourceClassName()].asCString()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember(ECJsonUtilities::json_targetId())) << actualJson.ToString().c_str();
    EXPECT_STREQ(key.GetInstanceId().ToHexStr().c_str(), actualJson[ECJsonUtilities::json_targetId()].asCString()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember(ECJsonUtilities::json_targetClassName())) << actualJson.ToString().c_str();
    EXPECT_STREQ("TestSchema.Foo", actualJson[ECJsonUtilities::json_targetClassName()].asCString()) << actualJson.ToString().c_str();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                09/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonECSqlSelectAdapterTests, LongDataType)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("jsonecsqlselectadapter_longtype.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                                    <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                                        <ECEntityClass typeName="Foo" >
                                                            <ECProperty propertyName="L" typeName="long" />
                                                        </ECEntityClass>
                                                    </ECSchema>)xml")));

    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(key, "INSERT INTO ts.Foo(L) VALUES(1234567890)"));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT L FROM ts.Foo WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    JsonECSqlSelectAdapter adapter1(stmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::KeepOriginal, ECJsonInt64Format::AsNumber));
    Json::Value actualJson;
    ASSERT_EQ(SUCCESS, adapter1.GetRow(actualJson));
    ASSERT_TRUE(actualJson.isObject()) << actualJson.ToString().c_str();
    ASSERT_EQ(1, actualJson.size()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember("L")) << actualJson.ToString().c_str();
    ASSERT_TRUE(actualJson["L"].isIntegral()) << actualJson.ToString().c_str();
    ASSERT_EQ(1234567890, actualJson["L"].asInt64()) << "ECJsonInt64Format::AsNumber " << actualJson.ToString().c_str();

    JsonECSqlSelectAdapter adapter2(stmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::KeepOriginal, ECJsonInt64Format::AsDecimalString));
    actualJson.clear();
    ASSERT_EQ(SUCCESS, adapter2.GetRow(actualJson));
    ASSERT_TRUE(actualJson.isObject()) << actualJson.ToString().c_str();
    ASSERT_EQ(1, actualJson.size()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember("L")) << actualJson.ToString().c_str();
    ASSERT_TRUE(actualJson["L"].isString()) << actualJson.ToString().c_str();
    ASSERT_STREQ("1234567890", actualJson["L"].asCString()) << "ECJsonInt64Format::AsDecimalString " << actualJson.ToString().c_str();

    JsonECSqlSelectAdapter adapter3(stmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::KeepOriginal, ECJsonInt64Format::AsHexadecimalString));
    actualJson.clear();
    ASSERT_EQ(SUCCESS, adapter3.GetRow(actualJson));
    ASSERT_TRUE(actualJson.isObject()) << actualJson.ToString().c_str();
    ASSERT_EQ(1, actualJson.size()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember("L")) << actualJson.ToString().c_str();
    ASSERT_TRUE(actualJson["L"].isString()) << actualJson.ToString().c_str();
    ASSERT_STREQ(BeInt64Id(1234567890).ToHexStr().c_str(), actualJson["L"].asCString()) << "ECJsonInt64Format::AsHexadecimalString " << actualJson.ToString().c_str();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonECSqlSelectAdapterTests, JsonStructAndArrays)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("jsonecsqlselectadapter_JsonStruct.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                                    <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                                        <ECStructClass typeName="MyStruct" >
                                                            <ECProperty propertyName="Alpha" typeName="int" />
                                                            <ECProperty propertyName="Beta" typeName="double" />
                                                        </ECStructClass>
                                                        <ECEntityClass typeName="Foo" >
                                                            <ECStructProperty propertyName="StructProp" typeName="MyStruct" />
                                                            <ECArrayProperty propertyName="DoubleArray" typeName="double" />
                                                            <ECStructArrayProperty propertyName="StructArray" typeName="MyStruct" />
                                                        </ECEntityClass>
                                                    </ECSchema>)xml")));

    ECInstanceKey key;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Foo(StructProp,DoubleArray,StructArray) VALUES(?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(1)["Alpha"].BindInt(100));
    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(1)["Beta"].BindDouble(1.5));

    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(2).AddArrayElement().BindDouble(1.5));
    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(2).AddArrayElement().BindDouble(2.5));

    IECSqlBinder& structElement1Binder = stmt.GetBinder(3).AddArrayElement();
    ASSERT_EQ(ECSqlStatus::Success, structElement1Binder["Alpha"].BindInt(100));
    ASSERT_EQ(ECSqlStatus::Success, structElement1Binder["Beta"].BindDouble(1.5));

    IECSqlBinder& structElement2Binder = stmt.GetBinder(3).AddArrayElement();
    ASSERT_EQ(ECSqlStatus::Success, structElement2Binder["Alpha"].BindInt(200));
    ASSERT_EQ(ECSqlStatus::Success, structElement2Binder["Beta"].BindDouble(2.5));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    m_ecdb.SaveChanges();
    }

    /* Retrieve the JSON for the inserted Instance */
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT ECInstanceId,ECClassId,StructProp,DoubleArray,StructArray FROM ONLY ts.Foo LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());

    Json::Value actualDefaultJson, actualJavaScriptJson;

    JsonECSqlSelectAdapter defaultAdapter(statement);
    ASSERT_EQ(SUCCESS, defaultAdapter.GetRow(actualDefaultJson));

    JsonECSqlSelectAdapter jsAdapter(statement, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::LowerFirstChar, ECJsonInt64Format::AsDecimalString));
    ASSERT_EQ(SUCCESS, jsAdapter.GetRow(actualJavaScriptJson));

    statement.Finalize();

    Json::Value expectedDefaultJson, expectedJavaScriptJson;
    ASSERT_TRUE(Json::Reader::Parse(R"json({
      "id" : "0x1",
      "className" : "TestSchema.Foo",
      "StructProp" : { "Alpha" : 100, "Beta" : 1.5 },
      "DoubleArray" : [ 1.5, 2.5],
      "StructArray" : [ {"Alpha" : 100, "Beta" : 1.5}, {"Alpha" : 200, "Beta" : 2.5}]
      })json", expectedDefaultJson));

    ASSERT_TRUE(Json::Reader::Parse(R"json({
      "id" : "0x1",
      "className" : "TestSchema.Foo",
      "structProp" : { "alpha" : 100, "beta" : 1.5 },
      "doubleArray" : [ 1.5, 2.5],
      "structArray" : [ {"alpha" : 100, "beta" : 1.5}, {"alpha" : 200, "beta" : 2.5}]
      })json", expectedJavaScriptJson));

    ASSERT_EQ(0, expectedDefaultJson.compare(actualDefaultJson)) << "Expected: " << expectedDefaultJson.ToString() << " | Actual: " << actualDefaultJson.ToString().c_str();
    ASSERT_EQ(0, expectedJavaScriptJson.compare(actualJavaScriptJson)) << "Expected: " << expectedJavaScriptJson.ToString() << " | Actual: " << actualJavaScriptJson.ToString().c_str();
    }

struct JsonReaderTests : public ECDbTestFixture {};


//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonReaderTests, PartialPoints)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("jsonreaderpartialpoints.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.PSA(P2D.X,P3D.Y,PStructProp.p2d.y,PStructProp.p3d.z) VALUES(1.0, 2.0, 3.0, 4.0)"));
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << stmt.GetECSql();
    stmt.Finalize();

    ECSqlStatement selStmt;
    ASSERT_EQ(ECSqlStatus::Success, selStmt.Prepare(m_ecdb, "SELECT P2D,P3D,PStructProp.p2d,PStructProp.p3d FROM ecsql.PSA WHERE ECInstanceId=?"));

    ASSERT_EQ(ECSqlStatus::Success, selStmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, selStmt.Step());

    Json::Value defaultJson, javaScriptJson;
    JsonECSqlSelectAdapter defaultAdapter(selStmt);
    JsonECSqlSelectAdapter javaScriptAdapter(selStmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::LowerFirstChar, ECJsonInt64Format::AsDecimalString));

    ASSERT_EQ(SUCCESS, defaultAdapter.GetRow(defaultJson));
    ASSERT_EQ(SUCCESS, javaScriptAdapter.GetRow(javaScriptJson));
    selStmt.Finalize();

    ASSERT_TRUE(defaultJson.isObject()) << defaultJson.ToString().c_str();
    ASSERT_TRUE(javaScriptJson.isObject()) << javaScriptJson.ToString().c_str();

    //ECSqlStatement fills the NULL coordinates with the SQLite defaults for NULL which is 0
    {
    ASSERT_TRUE(defaultJson.isMember("P2D"));
    ASSERT_TRUE(defaultJson["P2D"].isObject());
    ASSERT_DOUBLE_EQ(1, defaultJson["P2D"][ECJsonUtilities::json_x()].asDouble());
    ASSERT_DOUBLE_EQ(0, defaultJson["P2D"][ECJsonUtilities::json_y()].asDouble());

    ASSERT_TRUE(javaScriptJson.isMember("p2D"));
    ASSERT_TRUE(javaScriptJson["p2D"].isObject());
    ASSERT_DOUBLE_EQ(1, javaScriptJson["p2D"][ECJsonUtilities::json_x()].asDouble());
    ASSERT_DOUBLE_EQ(0, javaScriptJson["p2D"][ECJsonUtilities::json_y()].asDouble());
    }

    {
    ASSERT_TRUE(defaultJson.isMember("P3D"));
    ASSERT_TRUE(defaultJson["P3D"].isObject());
    ASSERT_DOUBLE_EQ(0, defaultJson["P3D"][ECJsonUtilities::json_x()].asDouble());
    ASSERT_DOUBLE_EQ(2, defaultJson["P3D"][ECJsonUtilities::json_y()].asDouble());
    ASSERT_DOUBLE_EQ(0, defaultJson["P3D"][ECJsonUtilities::json_z()].asDouble());

    ASSERT_TRUE(javaScriptJson.isMember("p3D"));
    ASSERT_TRUE(javaScriptJson["p3D"].isObject());
    ASSERT_DOUBLE_EQ(0, javaScriptJson["p3D"][ECJsonUtilities::json_x()].asDouble());
    ASSERT_DOUBLE_EQ(2, javaScriptJson["p3D"][ECJsonUtilities::json_y()].asDouble());
    ASSERT_DOUBLE_EQ(0, javaScriptJson["p3D"][ECJsonUtilities::json_z()].asDouble());
    }

    {
    ASSERT_TRUE(defaultJson.isMember("PStructProp.p2d"));
    ASSERT_TRUE(defaultJson["PStructProp.p2d"].isObject());
    ASSERT_DOUBLE_EQ(0.0, defaultJson["PStructProp.p2d"][ECJsonUtilities::json_x()].asDouble());
    ASSERT_DOUBLE_EQ(3.0, defaultJson["PStructProp.p2d"][ECJsonUtilities::json_y()].asDouble());

    ASSERT_TRUE(javaScriptJson.isMember("pStructProp.p2d"));
    ASSERT_TRUE(javaScriptJson["pStructProp.p2d"].isObject());
    ASSERT_DOUBLE_EQ(0.0, javaScriptJson["pStructProp.p2d"][ECJsonUtilities::json_x()].asDouble());
    ASSERT_DOUBLE_EQ(3.0, javaScriptJson["pStructProp.p2d"][ECJsonUtilities::json_y()].asDouble());
    }

    {
    ASSERT_TRUE(defaultJson["PStructProp.p3d"].isObject());
    ASSERT_DOUBLE_EQ(0.0, defaultJson["PStructProp.p3d"][ECJsonUtilities::json_x()].asDouble());
    ASSERT_DOUBLE_EQ(0.0, defaultJson["PStructProp.p3d"][ECJsonUtilities::json_y()].asDouble());
    ASSERT_DOUBLE_EQ(4.0, defaultJson["PStructProp.p3d"][ECJsonUtilities::json_z()].asDouble());

    ASSERT_TRUE(javaScriptJson["pStructProp.p3d"].isObject());
    ASSERT_DOUBLE_EQ(0.0, javaScriptJson["pStructProp.p3d"][ECJsonUtilities::json_x()].asDouble());
    ASSERT_DOUBLE_EQ(0.0, javaScriptJson["pStructProp.p3d"][ECJsonUtilities::json_y()].asDouble());
    ASSERT_DOUBLE_EQ(4.0, javaScriptJson["pStructProp.p3d"][ECJsonUtilities::json_z()].asDouble());
    }

    ECClassCP testClass = m_ecdb.Schemas().GetClass("ECSqlTest", "PSA");
    ASSERT_TRUE(testClass != nullptr);
    JsonReader defaultReader(m_ecdb, *testClass);
    ASSERT_TRUE(defaultReader.IsValid());
    ASSERT_EQ(SUCCESS, defaultReader.Read(defaultJson, key.GetInstanceId()));

    JsonReader javaScriptReader(m_ecdb, *testClass, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::LowerFirstChar, ECJsonInt64Format::AsDecimalString));
    ASSERT_TRUE(javaScriptReader.IsValid());
    ASSERT_EQ(SUCCESS, javaScriptReader.Read(javaScriptJson, key.GetInstanceId()));

    ASSERT_TRUE(defaultJson.isObject()) << defaultJson.ToString().c_str();
    ASSERT_TRUE(javaScriptJson.isObject()) << javaScriptJson.ToString().c_str();

    {
    ASSERT_TRUE(defaultJson.isMember("P2D"));
    ASSERT_TRUE(defaultJson["P2D"].isObject());
    ASSERT_DOUBLE_EQ(1, defaultJson["P2D"][ECJsonUtilities::json_x()].asDouble());
    ASSERT_DOUBLE_EQ(0, defaultJson["P2D"][ECJsonUtilities::json_y()].asDouble());

    ASSERT_TRUE(javaScriptJson.isMember("p2D"));
    ASSERT_TRUE(javaScriptJson["p2D"].isObject());
    ASSERT_DOUBLE_EQ(1, javaScriptJson["p2D"][ECJsonUtilities::json_x()].asDouble());
    ASSERT_DOUBLE_EQ(0, javaScriptJson["p2D"][ECJsonUtilities::json_y()].asDouble());
    }

    {
    ASSERT_TRUE(defaultJson.isMember("P3D"));
    ASSERT_TRUE(defaultJson["P3D"].isObject());
    ASSERT_DOUBLE_EQ(0, defaultJson["P3D"][ECJsonUtilities::json_x()].asDouble());
    ASSERT_DOUBLE_EQ(2, defaultJson["P3D"][ECJsonUtilities::json_y()].asDouble());
    ASSERT_DOUBLE_EQ(0, defaultJson["P3D"][ECJsonUtilities::json_z()].asDouble());

    ASSERT_TRUE(javaScriptJson.isMember("p3D"));
    ASSERT_TRUE(javaScriptJson["p3D"].isObject());
    ASSERT_DOUBLE_EQ(0, javaScriptJson["p3D"][ECJsonUtilities::json_x()].asDouble());
    ASSERT_DOUBLE_EQ(2, javaScriptJson["p3D"][ECJsonUtilities::json_y()].asDouble());
    ASSERT_DOUBLE_EQ(0, javaScriptJson["p3D"][ECJsonUtilities::json_z()].asDouble());
    }

    {
    ASSERT_DOUBLE_EQ(0.0, defaultJson["PStructProp"]["p2d"][ECJsonUtilities::json_x()].asDouble());
    ASSERT_DOUBLE_EQ(3.0, defaultJson["PStructProp"]["p2d"][ECJsonUtilities::json_y()].asDouble());

    ASSERT_DOUBLE_EQ(0.0, javaScriptJson["pStructProp"]["p2d"][ECJsonUtilities::json_x()].asDouble());
    ASSERT_DOUBLE_EQ(3.0, javaScriptJson["pStructProp"]["p2d"][ECJsonUtilities::json_y()].asDouble());
    }

    {
    ASSERT_DOUBLE_EQ(0.0, defaultJson["PStructProp"]["p3d"][ECJsonUtilities::json_x()].asDouble());
    ASSERT_DOUBLE_EQ(0.0, defaultJson["PStructProp"]["p3d"][ECJsonUtilities::json_y()].asDouble());
    ASSERT_DOUBLE_EQ(4.0, defaultJson["PStructProp"]["p3d"][ECJsonUtilities::json_z()].asDouble());

    ASSERT_DOUBLE_EQ(0.0, javaScriptJson["pStructProp"]["p3d"][ECJsonUtilities::json_x()].asDouble());
    ASSERT_DOUBLE_EQ(0.0, javaScriptJson["pStructProp"]["p3d"][ECJsonUtilities::json_y()].asDouble());
    ASSERT_DOUBLE_EQ(4.0, javaScriptJson["pStructProp"]["p3d"][ECJsonUtilities::json_z()].asDouble());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                09/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonReaderTests, RoundTrip_ReadThenInsert)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("jsonroundtrip_readtheninsert.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    ECInstanceKey pKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(pKey, "INSERT INTO ecsql.P(ECInstanceId) VALUES(NULL)"));

    ECClassCP psaClass = m_ecdb.Schemas().GetClass("ECSqlTest", "PSA");
    ASSERT_TRUE(psaClass != nullptr);

    const ECClassId navRelClassId = m_ecdb.Schemas().GetClassId("ECSqlTest", "PSAHasP_N1");
    ASSERT_TRUE(navRelClassId.IsValid());

    const int numericVal = 123;
    const DPoint2d pt2dVal = DPoint2d::From(1.0, 2.0);
    const DPoint3d pt3dVal = DPoint3d::From(1.0, 2.0, 3.0);
    const DateTime dtVal(DateTime::Kind::Utc, 2017, 9, 21, 10, 0);
    std::vector<Utf8CP> stringArrayVal {"Hello", "world"};

    ECInstanceKey psaKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,"INSERT INTO ecsql.PSA(I,L,P2D,P3D,DtUtc,P,S_Array,PStructProp.i) VALUES(?,?,?,?,?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, numericVal));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(2, (int64_t) numericVal));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint2d(3, pt2dVal));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint3d(4, pt3dVal));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDateTime(5, dtVal));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(6, pKey.GetInstanceId(), navRelClassId));
    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(7).AddArrayElement().BindText(stringArrayVal[0], IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(7).AddArrayElement().BindText(stringArrayVal[1], IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(8, numericVal));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(psaKey));
    }

    auto validate = [&] (ECInstanceId newId)
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * FROM ecsql.PSA WHERE ECInstanceId=?"));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, newId));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        const int colCount = stmt.GetColumnCount();
        for (int i = 0; i < colCount; i++)
            {
            Utf8StringCR propName = stmt.GetColumnInfo(i).GetProperty()->GetName();
            if (propName.EqualsIAscii("I"))
                ASSERT_EQ(numericVal, stmt.GetValueInt(i)) << propName.c_str();
            else if (propName.EqualsIAscii("L"))
                ASSERT_EQ((int64_t) numericVal, stmt.GetValueInt64(i)) << propName.c_str();
            else if (propName.EqualsIAscii("P2D"))
                ASSERT_TRUE(pt2dVal.AlmostEqual(stmt.GetValuePoint2d(i))) << propName.c_str();
            else if (propName.EqualsIAscii("P3D"))
                ASSERT_TRUE(pt3dVal.AlmostEqual(stmt.GetValuePoint3d(i))) << propName.c_str();
            else if (propName.EqualsIAscii("DtUtc"))
                ASSERT_EQ(dtVal, stmt.GetValueDateTime(i)) << propName.c_str();
            else if (propName.EqualsIAscii("P"))
                {
                ECClassId actualRelClassId;
                ASSERT_EQ(pKey.GetInstanceId(), stmt.GetValueNavigation<ECInstanceId>(i, &actualRelClassId)) << propName.c_str();
                ASSERT_EQ(navRelClassId, actualRelClassId) << propName.c_str();
                }
            else if (propName.EqualsIAscii("S_Array"))
                {
                IECSqlValue const& val = stmt.GetValue(i);
                ASSERT_EQ(2, val.GetArrayLength());
                int arrayIx = 0;
                for (IECSqlValue const& arrayEl : stmt.GetValue(i).GetArrayIterable())
                    {
                    ASSERT_STREQ(stringArrayVal[arrayIx], arrayEl.GetText()) << propName.c_str() << "[" << arrayIx << "]";
                    arrayIx++;
                    }
                }
            else if (propName.EqualsIAscii("PStructProp"))
                {
                IECSqlValue const& val = stmt.GetValue(i);
                for (IECSqlValue const& memberVal : val.GetStructIterable())
                    {
                    if (memberVal.GetColumnInfo().GetProperty()->GetName().Equals("i"))
                        ASSERT_EQ(numericVal, memberVal.GetInt()) << propName.c_str() << ".i";
                    else
                        ASSERT_TRUE(memberVal.IsNull()) << propName.c_str() << "." << memberVal.GetColumnInfo().GetProperty()->GetName().c_str();
                    }
                }
            else if (propName.EqualsIAscii("ECInstanceId"))
                ASSERT_EQ(newId, stmt.GetValueId<ECInstanceId>(i)) << "ECInstanceId";
            else if (propName.EqualsIAscii("ECClassId"))
                ASSERT_EQ(psaClass->GetId(), stmt.GetValueId<ECClassId>(i)) << "ECClassId";
            else
                ASSERT_TRUE(stmt.IsValueNull(i)) << propName;
            }
        };

    JsonInserter inserter(m_ecdb, *psaClass, nullptr);
    ECInstanceKey newKey;
    for (JsonECSqlSelectAdapter::FormatOptions const& formatOption : {JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::KeepOriginal, ECJsonInt64Format::AsNumber),
                                                                      JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::LowerFirstChar, ECJsonInt64Format::AsNumber),
                                                                     JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::KeepOriginal, ECJsonInt64Format::AsDecimalString),
                                                                     JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::LowerFirstChar, ECJsonInt64Format::AsDecimalString),
                                                                     JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::KeepOriginal, ECJsonInt64Format::AsHexadecimalString),
                                                                     JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::LowerFirstChar, ECJsonInt64Format::AsHexadecimalString)})
        {
        JsonReader reader(m_ecdb, psaKey.GetClassId(), formatOption);
        ASSERT_TRUE(reader.IsValid());

        Json::Value actualJson;
        ASSERT_EQ(SUCCESS, reader.Read(actualJson, psaKey.GetInstanceId()));

        actualJson.removeMember(ECJsonUtilities::json_id());
        ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(newKey, actualJson)) << "Insert after removing id member from read JSON ";
        validate(newKey.GetInstanceId());

        actualJson.removeMember(ECJsonUtilities::json_className());
        ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(newKey, actualJson)) << "Insert after removing id and className member from read JSON";
        validate(newKey.GetInstanceId());
        }
    }
END_ECDBUNITTESTS_NAMESPACE