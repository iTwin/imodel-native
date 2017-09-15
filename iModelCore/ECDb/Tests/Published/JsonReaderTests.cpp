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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId, ECInstanceId, I, I, P, P.Id, P.RelECClassId FROM ecsql.PSA LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    JsonECSqlSelectAdapter adapter(stmt);
    Json::Value json;
    ASSERT_EQ(SUCCESS, adapter.GetRow(json));

    ASSERT_TRUE(json.isMember(ECJsonUtilities::json_id())) << json.ToString().c_str();
    EXPECT_STRCASEEQ(psaKey.GetInstanceId().ToHexStr().c_str(), json[ECJsonUtilities::json_id()].asCString()) << json.ToString().c_str();

    ASSERT_TRUE(json.isMember((Utf8String(ECJsonUtilities::json_id()) + "_1").c_str())) << json.ToString().c_str();
    EXPECT_STRCASEEQ(psaKey.GetInstanceId().ToHexStr().c_str(), json[(Utf8String(ECJsonUtilities::json_id()) + "_1").c_str()].asCString()) << json.ToString().c_str();

    ASSERT_TRUE(json.isMember("I")) << json.ToString().c_str();
    EXPECT_EQ(10, json["I"].asInt()) << json.ToString().c_str();

    ASSERT_TRUE(json.isMember("I_1")) << json.ToString().c_str();
    EXPECT_EQ(10, json["I_1"].asInt()) << json.ToString().c_str();

    ASSERT_TRUE(json.isMember("P") && json["P"].isObject()) << json.ToString().c_str();
    EXPECT_STRCASEEQ(pKey.GetInstanceId().ToHexStr().c_str(), json["P"][ECJsonUtilities::json_navId()].asCString()) << json.ToString().c_str();
    EXPECT_STRCASEEQ("ECSqlTest.PSAHasP_N1", json["P"][ECJsonUtilities::json_navRelClassName()].asCString()) << json.ToString().c_str();

    ASSERT_TRUE(json.isMember("P.Id")) << json.ToString().c_str();
    EXPECT_STRCASEEQ(pKey.GetInstanceId().ToString().c_str(), json["P.Id"].asCString()) << json.ToString().c_str();

    ASSERT_TRUE(json.isMember("P.RelECClassId")) << json.ToString().c_str();
    EXPECT_STRCASEEQ(relClassId.ToString().c_str(), json["P.RelECClassId"].asCString()) << json.ToString().c_str();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                09/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonECSqlSelectAdapterTests, SpecialSelectClauseItems)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("SpecialSelectClauseItemsInJsonECSqlSelectAdapter.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.ecschema.xml")));

    ECInstanceKey pKey, psaKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(pKey, "INSERT INTO ecsql.P(ECInstanceId) VALUES (1000)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(psaKey, "INSERT INTO ecsql.PSA(I,L,P2D.X,P2D.Y,PStructProp.i,PStructProp.l,P.Id) VALUES (10,10,10.0,10.0,10,10,1000)"));
    
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId, I + 10, (I + 10) Alias1, I + L, (I + L) Alias2, P2D.X, PStructProp.i, P.Id FROM ecsql.PSA LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    JsonECSqlSelectAdapter adapter(stmt);
    Json::Value json;
    ASSERT_EQ(SUCCESS, adapter.GetRow(json));
    
    ASSERT_TRUE(json.isMember(ECJsonUtilities::json_id())) << json.ToString().c_str();
    ASSERT_STRCASEEQ(psaKey.GetInstanceId().ToHexStr().c_str(), json[ECJsonUtilities::json_id()].asCString()) << json.ToString().c_str();
    
    ASSERT_TRUE(json.isMember("[I] + 10")) << json.ToString().c_str();
    EXPECT_EQ(20, json["[I] + 10"].asInt()) << json.ToString().c_str();
    
    ASSERT_TRUE(json.isMember("Alias1")) << json.ToString().c_str();
    EXPECT_EQ(20, json["Alias1"].asInt()) << json.ToString().c_str();
    
    ASSERT_TRUE(json.isMember("[I] + [L]")) << json.ToString().c_str();
    EXPECT_EQ(20, json["[I] + [L]"].asInt()) << json.ToString().c_str();
    
    ASSERT_TRUE(json.isMember("Alias2")) << json.ToString().c_str();
    EXPECT_EQ(20, json["Alias2"].asInt()) << json.ToString().c_str();
    
    ASSERT_TRUE(json.isMember("P2D.X")) << json.ToString().c_str();
    EXPECT_DOUBLE_EQ(10.0, json["P2D.X"].asDouble()) << json.ToString().c_str();
    
    ASSERT_TRUE(json.isMember("PStructProp.i")) << json.ToString().c_str();
    EXPECT_EQ(10, json["PStructProp.i"].asInt()) << json.ToString().c_str();

    ASSERT_TRUE(json.isMember("P.Id")) << json.ToString().c_str();
    EXPECT_STRCASEEQ("1000", json["P.Id"].asCString()) << json.ToString().c_str();
    }


struct JsonReaderTests : public ECDbTestFixture {};

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonReaderTests, JsonValueStruct)
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
      "$ECInstanceId" : "0X1",
      "$ECClassKey" : "StartupCompany.Foo",
      "anglesFoo" : 
            {
            "Alpha" : 12.345000000000001,
            "Beta" : 12.345000000000001
            },
      "arrayOfAnglesStructsFoo" : [
         {
            "Alpha" : 12.345000000000001,
            "Beta" : 12.345000000000001
         },
         {
            "Alpha" : 12.345000000000001,
            "Beta" : 12.345000000000001
         },
         {
            "Alpha" : 12.345000000000001,
            "Beta" : 12.345000000000001
         }
        ],
      "arrayOfIntsFoo" : [ 67, 67, 67 ],
      "doubleFoo" : 12.345000000000001,
      "intFoo" : 67})json", expectedJson));

    ASSERT_EQ(0, expectedJson.compare(actualJson)) << actualJson.ToString().c_str();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonReaderTests, PartialPoints)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("jsonreaderpartialpoints.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.ecschema.xml")));

    ECClassCP testClass = m_ecdb.Schemas().GetClass("ECSqlTest", "PSA");
    ASSERT_TRUE(testClass != nullptr);

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
    ASSERT_TRUE(actualJson.isMember("P2D"));
    ASSERT_TRUE(actualJson["P2D"].isObject());
    ASSERT_DOUBLE_EQ(1, actualJson["P2D"]["x"].asDouble());
    ASSERT_DOUBLE_EQ(0, actualJson["P2D"]["y"].asDouble());

    ASSERT_TRUE(actualJson.isMember("P3D"));
    ASSERT_TRUE(actualJson["P3D"].isObject());
    ASSERT_DOUBLE_EQ(0, actualJson["P3D"]["x"].asDouble());
    ASSERT_DOUBLE_EQ(2, actualJson["P3D"]["y"].asDouble());
    ASSERT_DOUBLE_EQ(0, actualJson["P3D"]["z"].asDouble());

    ASSERT_TRUE(actualJson.isMember("PStructProp.p2d"));
    ASSERT_TRUE(actualJson["PStructProp.p2d"].isObject());
    ASSERT_DOUBLE_EQ(0.0, actualJson["PStructProp.p2d"]["x"].asDouble());
    ASSERT_DOUBLE_EQ(3.0, actualJson["PStructProp.p2d"]["y"].asDouble());

    ASSERT_TRUE(actualJson["PStructProp.p3d"].isObject());
    ASSERT_DOUBLE_EQ(0.0, actualJson["PStructProp.p3d"]["x"].asDouble());
    ASSERT_DOUBLE_EQ(0.0, actualJson["PStructProp.p3d"]["y"].asDouble());
    ASSERT_DOUBLE_EQ(4.0, actualJson["PStructProp.p3d"]["z"].asDouble());

    actualJson = Json::Value();
    JsonReader reader(m_ecdb, *testClass);
    ASSERT_TRUE(reader.IsValid());
    ASSERT_EQ(SUCCESS, reader.Read(actualJson, key.GetInstanceId()));

    ASSERT_TRUE(actualJson.isObject()) << actualJson.ToString().c_str();

    ASSERT_DOUBLE_EQ(1, actualJson["P2D"]["x"].asDouble());
    ASSERT_DOUBLE_EQ(0, actualJson["P2D"]["y"].asDouble());

    ASSERT_DOUBLE_EQ(0, actualJson["P3D"]["x"].asDouble());
    ASSERT_DOUBLE_EQ(2, actualJson["P3D"]["y"].asDouble());
    ASSERT_DOUBLE_EQ(0, actualJson["P3D"]["z"].asDouble());

    ASSERT_DOUBLE_EQ(0.0, actualJson["PStructProp"]["p2d"]["x"].asDouble());
    ASSERT_DOUBLE_EQ(3.0, actualJson["PStructProp"]["p2d"]["y"].asDouble());

    ASSERT_DOUBLE_EQ(0.0, actualJson["PStructProp"]["p3d"]["x"].asDouble());
    ASSERT_DOUBLE_EQ(0.0, actualJson["PStructProp"]["p3d"]["y"].asDouble());
    ASSERT_DOUBLE_EQ(4.0, actualJson["PStructProp"]["p3d"]["z"].asDouble());
    }

END_ECDBUNITTESTS_NAMESPACE