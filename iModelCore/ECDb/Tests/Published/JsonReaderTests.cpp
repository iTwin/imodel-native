/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/JsonReaderTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
    ASSERT_EQ(SUCCESS, SetupECDb("JsonMemberNamesInJsonECSqlSelectAdapter.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.ecschema.xml")));

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
    JsonECSqlSelectAdapter adapter(stmt);
    Json::Value json;
    ASSERT_EQ(SUCCESS, adapter.GetRow(json));

    ASSERT_TRUE(json.isMember(ECJsonUtilities::json_id())) << json.ToString().c_str();
    EXPECT_STRCASEEQ(psaKey.GetInstanceId().ToHexStr().c_str(), json[ECJsonUtilities::json_id()].asCString()) << json.ToString().c_str();

    ASSERT_TRUE(json.isMember((Utf8String(ECJsonUtilities::json_id()) + "_1").c_str())) << json.ToString().c_str();
    EXPECT_STRCASEEQ(psaKey.GetInstanceId().ToHexStr().c_str(), json[(Utf8String(ECJsonUtilities::json_id()) + "_1").c_str()].asCString()) << json.ToString().c_str();

    ASSERT_TRUE(json.isMember(ECJsonUtilities::json_className())) << json.ToString().c_str();
    EXPECT_STRCASEEQ("ECSqlTest.PSA", json[ECJsonUtilities::json_className()].asCString()) << json.ToString().c_str();

    ASSERT_TRUE(json.isMember((Utf8String(ECJsonUtilities::json_className()) + "_1").c_str())) << json.ToString().c_str();
    EXPECT_STRCASEEQ("ECSqlTest.PSA", json[(Utf8String(ECJsonUtilities::json_className()) + "_1").c_str()].asCString()) << json.ToString().c_str();

    ASSERT_TRUE(json.isMember((Utf8String(ECJsonUtilities::json_id()) + "_1").c_str())) << json.ToString().c_str();
    EXPECT_STRCASEEQ(psaKey.GetInstanceId().ToHexStr().c_str(), json[(Utf8String(ECJsonUtilities::json_id()) + "_1").c_str()].asCString()) << json.ToString().c_str();

    ASSERT_TRUE(json.isMember("i")) << json.ToString().c_str();
    EXPECT_EQ(10, json["i"].asInt()) << json.ToString().c_str();

    ASSERT_TRUE(json.isMember("i_1")) << json.ToString().c_str();
    EXPECT_EQ(10, json["i_1"].asInt()) << json.ToString().c_str();

    ASSERT_TRUE(json.isMember("p") && json["p"].isObject()) << json.ToString().c_str();
    EXPECT_STRCASEEQ(pKey.GetInstanceId().ToHexStr().c_str(), json["p"][ECJsonUtilities::json_navId()].asCString()) << json.ToString().c_str();
    EXPECT_STRCASEEQ("ECSqlTest.PSAHasP_N1", json["p"][ECJsonUtilities::json_navRelClassName()].asCString()) << json.ToString().c_str();

    ASSERT_TRUE(json.isMember("p.id")) << json.ToString().c_str();
    EXPECT_STRCASEEQ(pKey.GetInstanceId().ToString().c_str(), json["p.id"].asCString()) << json.ToString().c_str();

    //RelECClassId is not converted to relClassName if explicitly specified in select clause.
    ASSERT_TRUE(json.isMember("p.relECClassId")) << json.ToString().c_str();
    EXPECT_STRCASEEQ(relClassId.ToString().c_str(), json["p.relECClassId"].asCString()) << json.ToString().c_str();
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
    JsonECSqlSelectAdapter adapter(stmt);
    Json::Value json;

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT NULL, NULL As MyNull FROM ts.Foo"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(SUCCESS, adapter.GetRow(json));

    ASSERT_TRUE(json.isObject()) << stmt.GetECSql() << " - " << json.ToString().c_str();
    ASSERT_EQ(0, json.size()) << stmt.GetECSql() << " - " << json.ToString().c_str();

    ASSERT_FALSE(json.isMember("nULL") && json.isMember("NULL")) << "NULL literal results in null which is ignored by adapter. " << json.ToString().c_str();
    ASSERT_FALSE(json.isMember("myNull")) << "NULL literal even if it has an alias results in null which is ignored by adapter. " << json.ToString().c_str();
    stmt.Finalize();

    json.clear();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT 123, 123, 123 AS MyNumber, 123 * 3, 123 * 3 AS MyProduct FROM ts.Foo"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(SUCCESS, adapter.GetRow(json));

    ASSERT_TRUE(json.isObject()) << json.ToString().c_str();
    ASSERT_EQ(stmt.GetColumnCount(), (int) json.size()) << stmt.GetECSql() << " - " << json.ToString().c_str();

    ASSERT_TRUE(json.isMember("123")) << stmt.GetECSql() << " - " << json.ToString().c_str();
    ASSERT_EQ(123, json["123"].asInt()) << stmt.GetECSql() << " - " << json.ToString().c_str();

    ASSERT_TRUE(json.isMember("123_1")) << stmt.GetECSql() << " - " << json.ToString().c_str();
    ASSERT_EQ(123, json["123_1"].asInt()) << stmt.GetECSql() << " - " << json.ToString().c_str();

    ASSERT_TRUE(json.isMember("myNumber")) << stmt.GetECSql() << " - " << json.ToString().c_str();
    ASSERT_EQ(123, json["myNumber"].asInt()) << stmt.GetECSql() << " - " << json.ToString().c_str();

    ASSERT_TRUE(json.isMember("123 * 3")) << stmt.GetECSql() << " - " << json.ToString().c_str();
    ASSERT_EQ(369, json["123 * 3"].asInt()) << stmt.GetECSql() << " - " << json.ToString().c_str();

    ASSERT_TRUE(json.isMember("myProduct")) << stmt.GetECSql() << " - " << json.ToString().c_str();
    ASSERT_EQ(369, json["myProduct"].asInt()) << stmt.GetECSql() << " - " << json.ToString().c_str();
    stmt.Finalize();


    json.clear();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT IntProp, IntProp As DupIntProp, IntProp, IntProp, IntProp + 10, IntProp + 10 IntPlus10, IntProp + IntProp, IntProp + IntProp [IntProp plus IntProp] FROM ts.Foo"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(SUCCESS, adapter.GetRow(json));

    ASSERT_TRUE(json.isObject()) << json.ToString().c_str();
    ASSERT_EQ(stmt.GetColumnCount(), (int) json.size()) << stmt.GetECSql() << " - " << json.ToString().c_str();

    ASSERT_TRUE(json.isMember("intProp")) << stmt.GetECSql() << " - " << json.ToString().c_str();
    ASSERT_EQ(1000, json["intProp"].asInt()) << stmt.GetECSql() << " - " << json.ToString().c_str();

    ASSERT_TRUE(json.isMember("dupIntProp")) << stmt.GetECSql() << " - " << json.ToString().c_str();
    ASSERT_EQ(1000, json["dupIntProp"].asInt()) << stmt.GetECSql() << " - " << json.ToString().c_str();

    ASSERT_TRUE(json.isMember("intProp_1")) << stmt.GetECSql() << " - " << json.ToString().c_str();
    ASSERT_EQ(1000, json["intProp_1"].asInt()) << stmt.GetECSql() << " - " << json.ToString().c_str();

    ASSERT_TRUE(json.isMember("intProp_2")) << stmt.GetECSql() << " - " << json.ToString().c_str();
    ASSERT_EQ(1000, json["intProp_2"].asInt()) << stmt.GetECSql() << " - " << json.ToString().c_str();

    ASSERT_TRUE(json.isMember("[IntProp] + 10")) << stmt.GetECSql() << " - " << json.ToString().c_str();
    ASSERT_EQ(1010, json["[IntProp] + 10"].asInt()) << stmt.GetECSql() << " - " << json.ToString().c_str();

    ASSERT_TRUE(json.isMember("intPlus10")) << stmt.GetECSql() << " - " << json.ToString().c_str();
    ASSERT_EQ(1010, json["intPlus10"].asInt()) << stmt.GetECSql() << " - " << json.ToString().c_str();

    ASSERT_TRUE(json.isMember("[IntProp] + [IntProp]")) << stmt.GetECSql() << " - " << json.ToString().c_str();
    ASSERT_EQ(2000, json["[IntProp] + [IntProp]"].asInt()) << stmt.GetECSql() << " - " << json.ToString().c_str();

    ASSERT_TRUE(json.isMember("intProp plus IntProp")) << stmt.GetECSql() << " - " << json.ToString().c_str();
    ASSERT_EQ(2000, json["intProp plus IntProp"].asInt()) << stmt.GetECSql() << " - " << json.ToString().c_str();
    stmt.Finalize();

    json.clear();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT PtProp.X, PtProp.Y, PtProp.Z FROM ts.Foo"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(SUCCESS, adapter.GetRow(json));

    ASSERT_TRUE(json.isObject()) << json.ToString().c_str();
    ASSERT_EQ(stmt.GetColumnCount(), (int) json.size()) << stmt.GetECSql() << " - " << json.ToString().c_str();

    ASSERT_TRUE(json.isMember("ptProp.x")) << stmt.GetECSql() << " - " << json.ToString().c_str();
    ASSERT_DOUBLE_EQ(1000.0, json["ptProp.x"].asDouble()) << stmt.GetECSql() << " - " << json.ToString().c_str();

    ASSERT_TRUE(json.isMember("ptProp.y")) << stmt.GetECSql() << " - " << json.ToString().c_str();
    ASSERT_DOUBLE_EQ(1000.0, json["ptProp.y"].asDouble()) << stmt.GetECSql() << " - " << json.ToString().c_str();
    ASSERT_TRUE(json.isMember("ptProp.z")) << stmt.GetECSql() << " - " << json.ToString().c_str();
    ASSERT_DOUBLE_EQ(1000.0, json["ptProp.z"].asDouble()) << stmt.GetECSql() << " - " << json.ToString().c_str();
    stmt.Finalize();

    json.clear();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT StructProp.SomeNumber FROM ts.Foo"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(SUCCESS, adapter.GetRow(json));

    ASSERT_TRUE(json.isObject()) << json.ToString().c_str();
    ASSERT_EQ(stmt.GetColumnCount(), (int) json.size()) << stmt.GetECSql() << " - " << json.ToString().c_str();

    ASSERT_TRUE(json.isMember("structProp.someNumber")) << stmt.GetECSql() << " - " << json.ToString().c_str();
    ASSERT_EQ(1000, json["structProp.someNumber"].asDouble()) << stmt.GetECSql() << " - " << json.ToString().c_str();
    stmt.Finalize();
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
    JsonECSqlSelectAdapter adapter(stmt);
    Json::Value actualJson;

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECClassId, className FROM ts.Foo"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    ASSERT_EQ(SUCCESS, adapter.GetRow(actualJson));
    ASSERT_TRUE(actualJson.isObject()) << actualJson.ToString().c_str();
    ASSERT_EQ(2, actualJson.size()) << actualJson.ToString().c_str();
    ASSERT_TRUE(actualJson.isMember(ECJsonUtilities::json_className())) << actualJson.ToString().c_str();
    ASSERT_STREQ("TestSchema.Foo", actualJson[ECJsonUtilities::json_className()].asCString()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember((Utf8String(ECJsonUtilities::json_className()) + "_1").c_str())) << actualJson.ToString().c_str();
    ASSERT_STREQ("Foo class", actualJson[(Utf8String(ECJsonUtilities::json_className()) + "_1")].asCString()) << actualJson.ToString().c_str();
    stmt.Finalize();

    //now using aliases to avoid collision
    actualJson.clear();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECClassId AS SystemClassName, className FROM ts.Foo"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(SUCCESS, adapter.GetRow(actualJson));
    ASSERT_TRUE(actualJson.isObject()) << actualJson.ToString().c_str();
    ASSERT_EQ(2, actualJson.size()) << actualJson.ToString().c_str();
    ASSERT_TRUE(actualJson.isMember("systemClassName")) << actualJson.ToString().c_str();
    ASSERT_STREQ(fooKey.GetClassId().ToString().c_str(), actualJson["systemClassName"].asCString()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember("className")) << actualJson.ToString().c_str();
    ASSERT_STREQ("Foo class", actualJson["className"].asCString()) << actualJson.ToString().c_str();
    stmt.Finalize();

    actualJson.clear();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECClassId, className AS UserClassName FROM ts.Foo"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(SUCCESS, adapter.GetRow(actualJson));
    ASSERT_TRUE(actualJson.isObject()) << actualJson.ToString().c_str();
    ASSERT_EQ(2, actualJson.size()) << actualJson.ToString().c_str();
    ASSERT_TRUE(actualJson.isMember(ECJsonUtilities::json_className())) << actualJson.ToString().c_str();
    ASSERT_STREQ("TestSchema.Foo", actualJson[ECJsonUtilities::json_className()].asCString()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember("userClassName")) << actualJson.ToString().c_str();
    ASSERT_STREQ("Foo class", actualJson["userClassName"].asCString()) << actualJson.ToString().c_str();
    stmt.Finalize();

    actualJson.clear();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT SourceECClassId, sourceClassName, TargetECClassId, targetClassName FROM ts.Rel"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(SUCCESS, adapter.GetRow(actualJson));
    ASSERT_TRUE(actualJson.isObject()) << actualJson.ToString().c_str();
    ASSERT_EQ(4, actualJson.size()) << actualJson.ToString().c_str();
    ASSERT_TRUE(actualJson.isMember(ECJsonUtilities::json_sourceClassName())) << actualJson.ToString().c_str();
    ASSERT_STREQ("TestSchema.Foo", actualJson[ECJsonUtilities::json_sourceClassName()].asCString()) << actualJson.ToString().c_str();
    ASSERT_TRUE(actualJson.isMember((Utf8String(ECJsonUtilities::json_sourceClassName()) + "_1").c_str())) << actualJson.ToString().c_str();
    ASSERT_STREQ("source", actualJson[(Utf8String(ECJsonUtilities::json_sourceClassName()) + "_1").c_str()].asCString()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember(ECJsonUtilities::json_targetClassName())) << actualJson.ToString().c_str();
    ASSERT_STREQ("TestSchema.Foo", actualJson[ECJsonUtilities::json_targetClassName()].asCString()) << actualJson.ToString().c_str();
    ASSERT_TRUE(actualJson.isMember((Utf8String(ECJsonUtilities::json_targetClassName()) + "_1").c_str())) << actualJson.ToString().c_str();
    ASSERT_STREQ("target", actualJson[(Utf8String(ECJsonUtilities::json_targetClassName()) + "_1").c_str()].asCString()) << actualJson.ToString().c_str();
    stmt.Finalize();

    //now using aliases to avoid collision
    actualJson.clear();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT SourceECClassId SystemSourceClassName, sourceClassName, TargetECClassId SystemTargetClassName, targetClassName FROM ts.Rel"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(SUCCESS, adapter.GetRow(actualJson));
    ASSERT_TRUE(actualJson.isObject()) << actualJson.ToString().c_str();
    ASSERT_EQ(4, actualJson.size()) << actualJson.ToString().c_str();
    ASSERT_TRUE(actualJson.isMember("systemSourceClassName")) << actualJson.ToString().c_str();
    ASSERT_STREQ(fooKey.GetClassId().ToString().c_str(), actualJson["systemSourceClassName"].asCString()) << actualJson.ToString().c_str();
    ASSERT_TRUE(actualJson.isMember("sourceClassName")) << actualJson.ToString().c_str();
    ASSERT_STREQ("source", actualJson["sourceClassName"].asCString()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember("systemTargetClassName")) << actualJson.ToString().c_str();
    ASSERT_STREQ(fooKey.GetClassId().ToString().c_str(), actualJson["systemTargetClassName"].asCString()) << actualJson.ToString().c_str();
    ASSERT_TRUE(actualJson.isMember("targetClassName")) << actualJson.ToString().c_str();
    ASSERT_STREQ("target", actualJson["targetClassName"].asCString()) << actualJson.ToString().c_str();
    stmt.Finalize();

    actualJson.clear();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT SourceECClassId, sourceClassName UserSourceClass, TargetECClassId, targetClassName UserTargetClass FROM ts.Rel"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(SUCCESS, adapter.GetRow(actualJson));
    ASSERT_TRUE(actualJson.isObject()) << actualJson.ToString().c_str();
    ASSERT_EQ(4, actualJson.size()) << actualJson.ToString().c_str();
    ASSERT_TRUE(actualJson.isMember(ECJsonUtilities::json_sourceClassName())) << actualJson.ToString().c_str();
    ASSERT_STREQ("TestSchema.Foo", actualJson[ECJsonUtilities::json_sourceClassName()].asCString()) << actualJson.ToString().c_str();
    ASSERT_TRUE(actualJson.isMember("userSourceClass")) << actualJson.ToString().c_str();
    ASSERT_STREQ("source", actualJson["userSourceClass"].asCString()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember(ECJsonUtilities::json_targetClassName())) << actualJson.ToString().c_str();
    ASSERT_STREQ("TestSchema.Foo", actualJson[ECJsonUtilities::json_targetClassName()].asCString()) << actualJson.ToString().c_str();
    ASSERT_TRUE(actualJson.isMember("userTargetClass")) << actualJson.ToString().c_str();
    ASSERT_STREQ("target", actualJson["userTargetClass"].asCString()) << actualJson.ToString().c_str();
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

    ASSERT_TRUE(actualJson.isMember("b")) << actualJson.ToString().c_str();
    EXPECT_EQ(boolVal, actualJson["b"].asBool()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember("bi")) << actualJson.ToString().c_str();
    EXPECT_STREQ(blobValBase64Str.c_str(), actualJson["bi"].asCString()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember("d")) << actualJson.ToString().c_str();
    EXPECT_DOUBLE_EQ(doubleVal, actualJson["d"].asDouble()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember("dt")) << actualJson.ToString().c_str();
    EXPECT_STREQ(dtVal.ToString().c_str(), actualJson["dt"].asCString()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember("g")) << actualJson.ToString().c_str();
    ASSERT_TRUE(actualJson["g"].isObject()) << actualJson.ToString().c_str();
    EXPECT_STREQ("{\"LineSegment\":{\"endPoint\":[1.0,1.0,1.0],\"startPoint\":[0.0,0.0,0.0]}}", actualJson["g"].ToString().c_str()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember("i")) << actualJson.ToString().c_str();
    EXPECT_EQ(intVal, actualJson["i"].asInt()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember("l")) << actualJson.ToString().c_str();
    EXPECT_STREQ(int64Str.c_str(), actualJson["l"].asCString()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember("p2D")) << actualJson.ToString().c_str();
    ASSERT_TRUE(actualJson["p2D"].isObject()) << actualJson.ToString().c_str();
    EXPECT_DOUBLE_EQ(p2dVal.x, actualJson["p2D"]["x"].asDouble()) << actualJson.ToString().c_str();
    EXPECT_DOUBLE_EQ(p2dVal.y, actualJson["p2D"]["y"].asDouble()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember("p3D")) << actualJson.ToString().c_str();
    ASSERT_TRUE(actualJson["p3D"].isObject()) << actualJson.ToString().c_str();
    EXPECT_DOUBLE_EQ(p3dVal.x, actualJson["p3D"]["x"].asDouble()) << actualJson.ToString().c_str();
    EXPECT_DOUBLE_EQ(p3dVal.y, actualJson["p3D"]["y"].asDouble()) << actualJson.ToString().c_str();
    EXPECT_DOUBLE_EQ(p3dVal.z, actualJson["p3D"]["z"].asDouble()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember("s")) << actualJson.ToString().c_str();
    EXPECT_STREQ(stringVal, actualJson["s"].asCString()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember("struct")) << actualJson.ToString().c_str();
    ASSERT_TRUE(actualJson["struct"].isObject()) << actualJson.ToString().c_str();
    EXPECT_STREQ(personName, actualJson["struct"]["name"].asCString()) << actualJson.ToString().c_str();
    EXPECT_EQ(personAge, actualJson["struct"]["age"].asInt()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember("d_Array")) << actualJson.ToString().c_str();
    ASSERT_TRUE(actualJson["d_Array"].isArray()) << actualJson.ToString().c_str();
    ASSERT_EQ(2, actualJson["d_Array"].size()) << actualJson.ToString().c_str();
    EXPECT_DOUBLE_EQ(doubleVal, actualJson["d_Array"][0].asDouble()) << actualJson.ToString().c_str();
    EXPECT_DOUBLE_EQ(doubleVal, actualJson["d_Array"][1].asDouble()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember("struct_Array")) << actualJson.ToString().c_str();
    ASSERT_TRUE(actualJson["struct_Array"].isArray()) << actualJson.ToString().c_str();
    ASSERT_EQ(2, actualJson["struct_Array"].size()) << actualJson.ToString().c_str();
    EXPECT_STREQ(personName, actualJson["struct_Array"][0]["name"].asCString()) << actualJson.ToString().c_str();
    EXPECT_FALSE(actualJson["struct_Array"][0]["age"]) << actualJson.ToString().c_str();
    EXPECT_STREQ(personName, actualJson["struct_Array"][1]["name"].asCString()) << actualJson.ToString().c_str();
    EXPECT_FALSE(actualJson["struct_Array"][1]["age"]) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember("parent")) << actualJson.ToString().c_str();
    ASSERT_TRUE(actualJson["parent"].isObject()) << actualJson.ToString().c_str();
    EXPECT_STREQ(parentKey.GetInstanceId().ToHexStr().c_str(), actualJson["parent"][ECJsonUtilities::json_navId()].asCString()) << actualJson.ToString().c_str();
    ASSERT_TRUE(actualJson["parent"].isMember(ECJsonUtilities::json_navRelClassName())) << actualJson.ToString().c_str();
    EXPECT_STREQ("TestSchema.ParentHasFoo", actualJson["parent"][ECJsonUtilities::json_navRelClassName()].asCString()) << actualJson.ToString().c_str();

    //SELECT FROM relationship
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId, ECClassId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM ts.ParentHasFoo WHERE TargetECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    JsonECSqlSelectAdapter relAdapter(stmt);
    actualJson;
    actualJson.clear();
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

    JsonECSqlSelectAdapter adapter1(stmt, JsonECSqlSelectAdapter::FormatOptions(ECJsonInt64Format::AsNumber));
    Json::Value actualJson;
    ASSERT_EQ(SUCCESS, adapter1.GetRow(actualJson));
    ASSERT_TRUE(actualJson.isObject()) << actualJson.ToString().c_str();
    ASSERT_EQ(1, actualJson.size()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember("l")) << actualJson.ToString().c_str();
    ASSERT_TRUE(actualJson["l"].isIntegral()) << actualJson.ToString().c_str();
    ASSERT_EQ(1234567890, actualJson["l"].asInt64()) << "ECJsonInt64Format::AsNumber " << actualJson.ToString().c_str();

    JsonECSqlSelectAdapter adapter2(stmt, JsonECSqlSelectAdapter::FormatOptions(ECJsonInt64Format::AsDecimalString));
    actualJson.clear();
    ASSERT_EQ(SUCCESS, adapter2.GetRow(actualJson));
    ASSERT_TRUE(actualJson.isObject()) << actualJson.ToString().c_str();
    ASSERT_EQ(1, actualJson.size()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember("l")) << actualJson.ToString().c_str();
    ASSERT_TRUE(actualJson["l"].isString()) << actualJson.ToString().c_str();
    ASSERT_STREQ("1234567890", actualJson["l"].asCString()) << "ECJsonInt64Format::AsDecimalString " << actualJson.ToString().c_str();

    JsonECSqlSelectAdapter adapter3(stmt, JsonECSqlSelectAdapter::FormatOptions(ECJsonInt64Format::AsHexadecimalString));
    actualJson.clear();
    ASSERT_EQ(SUCCESS, adapter3.GetRow(actualJson));
    ASSERT_TRUE(actualJson.isObject()) << actualJson.ToString().c_str();
    ASSERT_EQ(1, actualJson.size()) << actualJson.ToString().c_str();

    ASSERT_TRUE(actualJson.isMember("l")) << actualJson.ToString().c_str();
    ASSERT_TRUE(actualJson["l"].isString()) << actualJson.ToString().c_str();
    ASSERT_STREQ(BeInt64Id(1234567890).ToHexStr().c_str(), actualJson["l"].asCString()) << "ECJsonInt64Format::AsHexadecimalString " << actualJson.ToString().c_str();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonECSqlSelectAdapterTests, JsonValueStruct)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("insertUsingJsonAPI.ecdb", SchemaItem::CreateForFile("StartupCompany.02.00.ecschema.xml")));

    ECSchemaCP startupSchema = m_ecdb.Schemas().GetSchema("StartupCompany");
    ECClassCP anglesStructClass = startupSchema->GetClassCP("AnglesStruct");
    ASSERT_TRUE(anglesStructClass != nullptr);
    ECClassCP fooClass = startupSchema->GetClassCP("Foo");
    ASSERT_TRUE(fooClass != nullptr);

    ECValue doubleValue;
    doubleValue.SetDouble(12.345);
    ECValue intValue;
    intValue.SetInteger(67);
    ECValue anglesStructValue;
    IECInstancePtr anglesStruct = anglesStructClass->GetDefaultStandaloneEnabler()->CreateInstance(0);
    anglesStruct->SetValue("Alpha", doubleValue);
    anglesStruct->SetValue("Beta", doubleValue);
    anglesStructValue.SetStruct(anglesStruct.get());

    IECInstancePtr foo = fooClass->GetDefaultStandaloneEnabler()->CreateInstance(0);
    ECObjectsStatus status;
    status = foo->SetValue("intFoo", intValue);
    status = foo->SetValue("doubleFoo", doubleValue);
    status = foo->SetValue("anglesFoo.Alpha", doubleValue);
    status = foo->SetValue("anglesFoo.Beta", doubleValue);
    foo->AddArrayElements("arrayOfIntsFoo", 3);
    foo->AddArrayElements("arrayOfAnglesStructsFoo", 3);
    for (int ii = 0; ii < 3; ii++)
        {
        status = foo->SetValue("arrayOfIntsFoo", intValue, ii);
        status = foo->SetValue("arrayOfAnglesStructsFoo", anglesStructValue, ii);
        }

    ECInstanceInserter fooInserter(m_ecdb, *fooClass, nullptr);
    fooInserter.Insert(*foo);
    m_ecdb.SaveChanges();

    /* Retrieve the JSON for the inserted Instance */
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT * FROM ONLY stco.Foo"));
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());

    JsonECSqlSelectAdapter jsonAdapter(statement);
    Json::Value actualJson;
    ASSERT_EQ(SUCCESS, jsonAdapter.GetRow(actualJson));
    statement.Finalize();

    Json::Value expectedJson;
    ASSERT_TRUE(Json::Reader::Parse(R"json({
      "id" : "0X1",
      "className" : "StartupCompany.Foo",
      "anglesFoo" : 
            {
            "alpha" : 12.345000000000001,
            "beta" : 12.345000000000001
            },
      "arrayOfAnglesStructsFoo" : [
         {
            "alpha" : 12.345000000000001,
            "beta" : 12.345000000000001
         },
         {
            "alpha" : 12.345000000000001,
            "beta" : 12.345000000000001
         },
         {
            "alpha" : 12.345000000000001,
            "beta" : 12.345000000000001
         }
        ],
      "arrayOfIntsFoo" : [ 67, 67, 67 ],
      "doubleFoo" : 12.345000000000001,
      "intFoo" : 67})json", expectedJson));

    ASSERT_EQ(0, expectedJson.compare(actualJson)) << actualJson.ToString().c_str();
    }

struct JsonReaderTests : public ECDbTestFixture {};


//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonReaderTests, PartialPoints)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("jsonreaderpartialpoints.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.ecschema.xml")));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.PSA(P2D.X,P3D.Y,PStructProp.p2d.y,PStructProp.p3d.z) VALUES(1.0, 2.0, 3.0, 4.0)"));
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << stmt.GetECSql();
    stmt.Finalize();

    ECSqlStatement selStmt;
    ASSERT_EQ(ECSqlStatus::Success, selStmt.Prepare(m_ecdb, "SELECT P2D,P3D,PStructProp.p2d,PStructProp.p3d FROM ecsql.PSA WHERE ECInstanceId=?"));
    JsonECSqlSelectAdapter adapter(selStmt);

    ASSERT_EQ(ECSqlStatus::Success, selStmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, selStmt.Step());
    Json::Value actualJson;
    ASSERT_EQ(SUCCESS, adapter.GetRow(actualJson));
    selStmt.Finalize();
    ASSERT_TRUE(actualJson.isObject()) << actualJson.ToString().c_str();
    //ECSqlStatement fills the NULL coordinates with the SQLite defaults for NULL which is 0
    ASSERT_TRUE(actualJson.isMember("p2D"));
    ASSERT_TRUE(actualJson["p2D"].isObject());
    ASSERT_DOUBLE_EQ(1, actualJson["p2D"]["x"].asDouble());
    ASSERT_DOUBLE_EQ(0, actualJson["p2D"]["y"].asDouble());

    ASSERT_TRUE(actualJson.isMember("p3D"));
    ASSERT_TRUE(actualJson["p3D"].isObject());
    ASSERT_DOUBLE_EQ(0, actualJson["p3D"]["x"].asDouble());
    ASSERT_DOUBLE_EQ(2, actualJson["p3D"]["y"].asDouble());
    ASSERT_DOUBLE_EQ(0, actualJson["p3D"]["z"].asDouble());

    ASSERT_TRUE(actualJson.isMember("pStructProp.p2d"));
    ASSERT_TRUE(actualJson["pStructProp.p2d"].isObject());
    ASSERT_DOUBLE_EQ(0.0, actualJson["pStructProp.p2d"]["x"].asDouble());
    ASSERT_DOUBLE_EQ(3.0, actualJson["pStructProp.p2d"]["y"].asDouble());

    ASSERT_TRUE(actualJson["pStructProp.p3d"].isObject());
    ASSERT_DOUBLE_EQ(0.0, actualJson["pStructProp.p3d"]["x"].asDouble());
    ASSERT_DOUBLE_EQ(0.0, actualJson["pStructProp.p3d"]["y"].asDouble());
    ASSERT_DOUBLE_EQ(4.0, actualJson["pStructProp.p3d"]["z"].asDouble());

    actualJson = Json::Value();
    ECClassCP testClass = m_ecdb.Schemas().GetClass("ECSqlTest", "PSA");
    ASSERT_TRUE(testClass != nullptr);
    JsonReader reader(m_ecdb, *testClass);
    ASSERT_TRUE(reader.IsValid());
    ASSERT_EQ(SUCCESS, reader.Read(actualJson, key.GetInstanceId()));

    ASSERT_TRUE(actualJson.isObject()) << actualJson.ToString().c_str();

    ASSERT_DOUBLE_EQ(1, actualJson["p2D"]["x"].asDouble());
    ASSERT_DOUBLE_EQ(0, actualJson["p2D"]["y"].asDouble());

    ASSERT_DOUBLE_EQ(0, actualJson["p3D"]["x"].asDouble());
    ASSERT_DOUBLE_EQ(2, actualJson["p3D"]["y"].asDouble());
    ASSERT_DOUBLE_EQ(0, actualJson["p3D"]["z"].asDouble());

    ASSERT_DOUBLE_EQ(0.0, actualJson["pStructProp"]["p2d"]["x"].asDouble());
    ASSERT_DOUBLE_EQ(3.0, actualJson["pStructProp"]["p2d"]["y"].asDouble());

    ASSERT_DOUBLE_EQ(0.0, actualJson["pStructProp"]["p3d"]["x"].asDouble());
    ASSERT_DOUBLE_EQ(0.0, actualJson["pStructProp"]["p3d"]["y"].asDouble());
    ASSERT_DOUBLE_EQ(4.0, actualJson["pStructProp"]["p3d"]["z"].asDouble());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                09/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonReaderTests, RoundTrip_ReadThenInsert)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("jsonroundtrip_readtheninsert.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.ecschema.xml")));

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
    for (ECJsonInt64Format formatOption : {ECJsonInt64Format::AsNumber, ECJsonInt64Format::AsDecimalString, ECJsonInt64Format::AsHexadecimalString})
        {
        JsonReader reader(m_ecdb, psaKey.GetClassId(), JsonECSqlSelectAdapter::FormatOptions(formatOption));
        ASSERT_TRUE(reader.IsValid()) << "ECJsonInt64Format::" << (int) formatOption;

        Json::Value actualJson;
        ASSERT_EQ(SUCCESS, reader.Read(actualJson, psaKey.GetInstanceId())) << "ECJsonInt64Format::" << (int) formatOption;

        actualJson.removeMember(ECJsonUtilities::json_id());
        ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(newKey, actualJson)) << "Insert after removing id member from read JSON | ECJsonInt64Format::" << (int) formatOption;
        validate(newKey.GetInstanceId());

        actualJson.removeMember(ECJsonUtilities::json_className());
        ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(newKey, actualJson)) << "Insert after removing id and className member from read JSON | ECJsonInt64Format::" << (int) formatOption;
        validate(newKey.GetInstanceId());
        }
    }
END_ECDBUNITTESTS_NAMESPACE