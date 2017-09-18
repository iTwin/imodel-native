/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/JsonUpdaterTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

struct JsonUpdaterTests : ECDbTestFixture {};

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonUpdaterTests, InvalidInput)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("jsonupdatertests.ecdb"));

    ECClassCP testClass = m_ecdb.Schemas().GetClass("ECDbFileInfo", "ExternalFileInfo");
    ASSERT_TRUE(testClass != nullptr);

    ECInstanceKey key;
    {
    JsonInserter inserter(m_ecdb, *testClass, nullptr);
    ASSERT_TRUE(inserter.IsValid());

    Json::Value val;
    val["Name"] = "test.txt";

    ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(key, val));
    }

    JsonUpdater updater(m_ecdb, *testClass, nullptr);
    ASSERT_TRUE(updater.IsValid());

    Json::Value val;
    ASSERT_EQ(BE_SQLITE_ERROR, updater.Update(ECInstanceId(), val)) << "Empty JSON value";

    val["Name"] = "test2.txt";
    ASSERT_EQ(BE_SQLITE_OK, updater.Update(key.GetInstanceId(), val)) << "JSON value without id member";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                  10/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonUpdaterTests, UpdateRelationshipProperty)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("updaterelationshipprop.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                                            "<ECSchema schemaName='test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                                            "    <ECEntityClass typeName='A' >"
                                                            "        <ECProperty propertyName='P1' typeName='int' />"
                                                            "    </ECEntityClass>"
                                                            "    <ECRelationshipClass typeName='AHasA' strength='referencing' modifier='Sealed'>"
                                                            "        <ECProperty propertyName='Name' typeName='string' />"
                                                            "        <Source cardinality='(0,N)' polymorphic='False'><Class class='A'/></Source>"
                                                            "        <Target cardinality='(0,N)' polymorphic='False'><Class class='A'/></Target>"
                                                            "    </ECRelationshipClass>"
                                                            "</ECSchema>")));


    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.A (P1) VALUES(?)"));


    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, 111));
    ECInstanceKey sourceKey;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(sourceKey));
    ECInstanceId sourceInstanceId = sourceKey.GetInstanceId();

    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, 222));
    ECInstanceKey targetKey;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(targetKey));
    ECInstanceId targetInstanceId = targetKey.GetInstanceId();

    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.AHasA (SourceECInstanceId, TargetECInstanceId, Name) VALUES(?,?,'good morning')"));

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, sourceInstanceId));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, targetInstanceId));

    ECInstanceKey relKey;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(relKey));
    ECInstanceId relInstanceId = relKey.GetInstanceId();

    ECClassCP relClass = m_ecdb.Schemas().GetClass("test", "AHasA");
    ASSERT_TRUE(relClass != nullptr);
    JsonReader reader(m_ecdb, *relClass);
    Json::Value relationshipJson;
    ASSERT_EQ(SUCCESS, reader.Read(relationshipJson, relInstanceId));
    ASSERT_STREQ("good morning", relationshipJson["name"].asCString()) << relationshipJson.ToString().c_str();

    // Update relationship properties
    JsonUpdater updater(m_ecdb, *relClass, nullptr);
    ASSERT_TRUE(updater.IsValid());

    /*
    * Update relationship properties via Json::Value API
    */
    Utf8CP expectedVal = "good afternoon";
    relationshipJson["name"] = expectedVal;
    //printf ("%s\r\n", relationshipJson.toStyledString ().c_str ());
    ASSERT_EQ(BE_SQLITE_OK, updater.Update(relInstanceId, relationshipJson, sourceKey, targetKey));

    ECSqlStatement checkStmt;
    ASSERT_EQ(ECSqlStatus::Success, checkStmt.Prepare(m_ecdb, "SELECT NULL FROM ts.AHasA WHERE ECInstanceId=? AND Name=?"));

    ASSERT_EQ(ECSqlStatus::Success, checkStmt.BindId(1, relInstanceId));
    ASSERT_EQ(ECSqlStatus::Success, checkStmt.BindText(2, expectedVal, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_ROW, checkStmt.Step());
    checkStmt.Reset();
    checkStmt.ClearBindings();

    /*
    * Update relationship properties via rapidjson
    */
    expectedVal = "good evening";
    rapidjson::Document relationshipRapidJson;
    relationshipRapidJson.SetObject();
    relationshipRapidJson.AddMember("name", rapidjson::StringRef(expectedVal), relationshipRapidJson.GetAllocator());

    ASSERT_EQ(BE_SQLITE_OK, updater.Update(relInstanceId, relationshipRapidJson, sourceKey, targetKey));

    ASSERT_EQ(ECSqlStatus::Success, checkStmt.BindId(1, relInstanceId));
    ASSERT_EQ(ECSqlStatus::Success, checkStmt.BindText(2, expectedVal, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_ROW, checkStmt.Step());
    checkStmt.Reset();
    checkStmt.ClearBindings();
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonUpdaterTests, UpdateProperties)
    {
    ECInstanceKey key;
    {
    ASSERT_EQ(SUCCESS, SetupECDb("updateClassProperties.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                        "<ECSchema schemaName='testSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                        "    <ECEntityClass typeName='A' >"
                        "        <ECProperty propertyName='P1' typeName='int' />"
                        "        <ECProperty propertyName='P2' typeName='string' />"
                        "        <ECProperty propertyName='P3' typeName='double' readOnly='True'/>"
                        "    </ECEntityClass>"
                        "</ECSchema>")));


    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.A (P1, P2, P3) VALUES(100, 'JsonTest', 1000.10)"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, stmt.Step(key));
    }

    ECClassCP ecClass = m_ecdb.Schemas().GetClass("testSchema", "A");
    ASSERT_TRUE(ecClass != nullptr);
    JsonReader reader(m_ecdb,*ecClass);
    Json::Value ecClassJson;
    ASSERT_EQ(SUCCESS, reader.Read(ecClassJson, key.GetInstanceId()));
    ASSERT_EQ(100, ecClassJson["p1"].asInt());
    ASSERT_STREQ("JsonTest", ecClassJson["p2"].asCString());
    ASSERT_DOUBLE_EQ(1000.10, ecClassJson["p3"].asDouble());
    //printf ("%s\r\n", ecClassJson.toStyledString ().c_str ());

    // Update ecClass properties
    JsonUpdater updater(m_ecdb, *ecClass, nullptr);
    ASSERT_TRUE(updater.IsValid());

    /*
    * Update class properties via Json::Value API
    */
    Utf8CP expectedVal = "Json API";
    ecClassJson["p1"] = 200;
    ecClassJson["p2"] = expectedVal;
    ecClassJson["p3"] = 2000.20;
    ASSERT_EQ(BE_SQLITE_OK, updater.Update(key.GetInstanceId(), ecClassJson));

    ECSqlStatement checkStmt;
    ASSERT_EQ(ECSqlStatus::Success, checkStmt.Prepare(m_ecdb, "SELECT NULL FROM ts.A WHERE ECInstanceId=? AND P1=? AND P2=? AND P3=?"));

    ASSERT_EQ(ECSqlStatus::Success, checkStmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, checkStmt.BindInt(2, 200));
    ASSERT_EQ(ECSqlStatus::Success, checkStmt.BindText(3, expectedVal, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, checkStmt.BindDouble(4, 1000.10)); //Readonly values will not be updated

    ASSERT_EQ(BE_SQLITE_ROW, checkStmt.Step());
    checkStmt.Reset();
    checkStmt.ClearBindings();

    /*
    * Update class properties via rapidjson
    */
    expectedVal = "RapidJson";
    rapidjson::Document ecClassRapidJson;
    ecClassRapidJson.SetObject();
    ecClassRapidJson.AddMember("p1", 300, ecClassRapidJson.GetAllocator());
    ecClassRapidJson.AddMember("p2", rapidjson::StringRef(expectedVal), ecClassRapidJson.GetAllocator());
    ecClassRapidJson.AddMember("p3", 3000.30, ecClassRapidJson.GetAllocator());

    ASSERT_EQ(BE_SQLITE_OK, updater.Update(key.GetInstanceId(), ecClassRapidJson));

    ASSERT_EQ(ECSqlStatus::Success, checkStmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, checkStmt.BindInt(2, 300));
    ASSERT_EQ(ECSqlStatus::Success, checkStmt.BindText(3, expectedVal, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, checkStmt.BindDouble(4, 1000.10));//Readonly values will not be updated
    ASSERT_EQ(BE_SQLITE_ROW, checkStmt.Step());
    checkStmt.Reset();
    checkStmt.ClearBindings();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                              Ramanujam.Raman                   10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonUpdaterTests, CommonGeometryJsonSerialization)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("cgjsonserialization.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8' ?>"
                                                                  "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                                                  "   <ECEntityClass typeName='SpatialLocation' >"
                                                                  "       <ECProperty propertyName='Center' typeName='Bentley.Geometry.Common.IGeometry' />"
                                                                  "       <ECProperty propertyName='URP' typeName='Bentley.Geometry.Common.IGeometry' />"
                                                                  "       <ECProperty propertyName='LLP' typeName='Bentley.Geometry.Common.IGeometry' />"
                                                                  "   <ECProperty propertyName='Location' typeName='Bentley.Geometry.Common.IGeometry'/>"
                                                                  "   </ECEntityClass>"
                                                                  "</ECSchema>")));
    ASSERT_EQ(SUCCESS, PopulateECDb(3));

    ECClassCP spatialClass = m_ecdb.Schemas().GetClass("Test", "SpatialLocation");
    ASSERT_TRUE(nullptr != spatialClass);

    Utf8String expectedJson(R"json({ "location" : { "Polygon" : { "Point" : [[17.672993680591887, 48.591463341759322, 0.0], 
                                                                    [26.072599028627593, 48.591463341759322, 0.0],
                                                                    [26.072599028627593, 54.891247623320339, 0.0],
                                                                    [17.672993680591887, 54.891247623320339, 0.0],
                                                                    [17.672993680591887, 48.591463341759322, 0.0]] }},
                    "lLP" : {
                        "Coordinate" : {
                            "xyz" : [17.672993680591887, 48.591463341759322, 0.0]
                        }
                    },
                    "uRP" : {
                        "Coordinate" : {
                            "xyz" : [26.072599028627593, 54.891247623320339, 0.0]
                        }
                    },
                    "center" : {
                        "Coordinate" : {
                            "xyz" : [21.87279635460974, 51.741355482539831, 0.0]
                        }
                    }
                        }
                       )json");

    Json::Value expectedJsonCppValue;
    ASSERT_TRUE(Json::Reader::Parse(expectedJson, expectedJsonCppValue));

    rapidjson::Document expectedRapidJsonValue;
    ASSERT_FALSE(expectedRapidJsonValue.Parse<0>(expectedJson.c_str()).HasParseError());

    // Insert using RapidJson API
    JsonInserter inserter(m_ecdb, *spatialClass, nullptr);
    ASSERT_TRUE(inserter.IsValid());
    ECInstanceKey rapidJsonInstanceKey;
    ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(rapidJsonInstanceKey, expectedRapidJsonValue));

    // Insert using JsonCpp API
    ECInstanceKey jsonCppInstanceKey;
    ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(jsonCppInstanceKey, expectedJsonCppValue));

    m_ecdb.SaveChanges();

    // Validate
    JsonReader reader(m_ecdb, *spatialClass);
    ASSERT_TRUE(reader.IsValid());

    for (ECInstanceId id : {rapidJsonInstanceKey.GetInstanceId(), jsonCppInstanceKey.GetInstanceId()})
        {
        Json::Value actualJson;
        ASSERT_EQ(SUCCESS, reader.Read(actualJson, id));
        ASSERT_TRUE(actualJson.isObject()) << id.ToString().c_str();
        for (int coordIx = 0; coordIx < 3; coordIx++)
            {
            ASSERT_DOUBLE_EQ(expectedJsonCppValue["Center"]["Coordinate"]["xyz"][coordIx].asDouble(), actualJson["Center"]["Coordinate"]["xyz"][coordIx].asDouble()) << id.ToString().c_str() << " Actual Json: " << actualJson.ToString().c_str();
            ASSERT_DOUBLE_EQ(expectedJsonCppValue["LLP"]["Coordinate"]["xyz"][coordIx].asDouble(), actualJson["LLP"]["Coordinate"]["xyz"][coordIx].asDouble()) << id.ToString().c_str() << " Actual Json: " << actualJson["Center"]["Coordinate"]["xyz"].ToString().c_str();
            ASSERT_DOUBLE_EQ(expectedJsonCppValue["URP"]["Coordinate"]["xyz"][coordIx].asDouble(), actualJson["URP"]["Coordinate"]["xyz"][coordIx].asDouble()) << id.ToString().c_str() << " Actual Json: " << actualJson["Center"]["Coordinate"]["xyz"].ToString().c_str();
            }

        for (int i = 0; i < (int) expectedJsonCppValue["Location"]["Polygon"]["Point"].size(); i++)
            {
            for (int coordIx = 0; coordIx < 3; coordIx++)
                {
                ASSERT_DOUBLE_EQ(expectedJsonCppValue["Location"]["Polygon"]["Point"][i][coordIx].asDouble(), actualJson["Location"]["Polygon"]["Point"][i][coordIx].asDouble()) << id.ToString().c_str();
                }
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonUpdaterTests, ReadonlyAndCalculatedProperties)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("updateClassProperties.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='testSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.00' prefix='bsca' />"
        "     <ECEntityClass typeName='Foo' >"
        "       <ECProperty propertyName='Num' typeName='int' readOnly ='True' />"
        "       <ECProperty propertyName='Square' typeName='string' >"
        "           <ECCustomAttributes>"
        "               <CalculatedECPropertySpecification xmlns = 'Bentley_Standard_CustomAttributes.01.00'>"
        "                   <ECExpression>this.Num * this.Num</ECExpression>"
        "               </CalculatedECPropertySpecification>"
        "           </ECCustomAttributes>"
        "       </ECProperty>"
        "     </ECEntityClass>"
        "</ECSchema>")));

    ECClassCP ecClass = m_ecdb.Schemas().GetClass("testSchema", "Foo");

    const int oldNum = 2;
    Utf8CP oldSquare = "4";
    const int newNum = 3;
    Utf8CP newSquare = "9";

    //Insert test instance
    ECInstanceKey key;
    Json::Value properties;
    properties["Num"] = oldNum;
    properties["Square"] = oldSquare;
    JsonInserter inserter(m_ecdb, *ecClass, nullptr);
    ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(key, properties));

    //Update test instance
    properties["Num"] = newNum;

    ECSqlStatement validateStmt;
    ASSERT_EQ(ECSqlStatus::Success, validateStmt.Prepare(m_ecdb, "SELECT Num, Square FROM ts.Foo WHERE ECInstanceId=?"));

    //default updater
    {
    Savepoint sp(m_ecdb, "default updater");
    JsonUpdater updater(m_ecdb, *ecClass, nullptr);
    ASSERT_TRUE(updater.IsValid());
    ASSERT_EQ(BE_SQLITE_OK, updater.Update(key.GetInstanceId(), properties));

    ASSERT_EQ(ECSqlStatus::Success, validateStmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, validateStmt.Step());

    ASSERT_EQ(oldNum, validateStmt.GetValueInt(0)) << "Readonly property Num is expected to not be modified without ECSQLOPTION ReadonlyPropertiesAreUpdatable";
    ASSERT_STREQ(newSquare, validateStmt.GetValueText(1)) << "Calculated property Square is expected to be modified without ECSQLOPTION ReadonlyPropertiesAreUpdatable (calc properties are always updated)";
    validateStmt.Reset();
    validateStmt.ClearBindings();

    sp.Cancel();
    }

    //updater with readonly prop options
    {
    JsonUpdater updater(m_ecdb, *ecClass, nullptr, "ReadonlyPropertiesAreUpdatable");
    ASSERT_TRUE(updater.IsValid());
    ASSERT_EQ(BE_SQLITE_OK, updater.Update(key.GetInstanceId(), properties));

    ASSERT_EQ(ECSqlStatus::Success, validateStmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, validateStmt.Step());

    ASSERT_EQ(newNum, validateStmt.GetValueInt(0)) << "Readonly property Num is expected to be modified with ECSQLOPTION ReadonlyPropertiesAreUpdatable";
    ASSERT_STREQ(newSquare, validateStmt.GetValueText(1)) << "Calculated property Square is expected to be modified with ECSQLOPTION ReadonlyPropertiesAreUpdatable";
    validateStmt.Reset();
    validateStmt.ClearBindings();
    }

    }

END_ECDBUNITTESTS_NAMESPACE
