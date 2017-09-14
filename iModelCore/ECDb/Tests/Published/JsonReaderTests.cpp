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

//---------------------------------------------------------------------------------------
// @bsiclass                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
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