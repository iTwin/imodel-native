/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/JsonUpdaterTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "ECDbTestProject.h"
#include "SchemaImportTestFixture.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

struct JsonUpdaterTests : SchemaImportTestFixture
    {
    //---------------------------------------------------------------------------------------
    // @bsimethod                              Ramanujam.Raman                   10/15
    //+---------------+---------------+---------------+---------------+---------------+------
    void ValidateCGPoint (JsonValueCR expectedValue, JsonValueCR actualValue)
        {
        ASSERT_TRUE (!expectedValue.isNull ());

        for (int ii = 0; ii < 3; ii++)
            {
            double expectedCoord = expectedValue[ii].asDouble ();
            double actualCoord = actualValue[ii].asDouble ();
            ASSERT_EQ (expectedCoord, actualCoord);
            }
        }
    //
    //---------------------------------------------------------------------------------------
    // @bsimethod                              Ramanujam.Raman                   10/15
    //+---------------+---------------+---------------+---------------+---------------+------
    void ValidateCGPointArray (JsonValueCR expectedValue, JsonValueCR actualValue)
        {
        ASSERT_TRUE (!expectedValue.isNull ());
        ASSERT_TRUE (expectedValue.isArray ());
        ASSERT_TRUE (expectedValue.size () == actualValue.size ());

        for (int ii = 0; ii < (int)expectedValue.size (); ii++)
            ValidateCGPoint (expectedValue[ii], actualValue[ii]);
        }
    //
    //---------------------------------------------------------------------------------------
    // @bsimethod                              Ramanujam.Raman                   10/15
    //+---------------+---------------+---------------+---------------+---------------+------
    void ValidateSpatialInstance (ECDbR db, ECInstanceKey spatialInstanceKey, JsonValueCR expectedJsonValue)
        {
        JsonReader reader (db, spatialInstanceKey.GetECClassId ());
        Json::Value actualJsonValue;
        BentleyStatus status = reader.ReadInstance (actualJsonValue, spatialInstanceKey.GetECInstanceId (), JsonECSqlSelectAdapter::FormatOptions (ECValueFormat::RawNativeValues));
        ASSERT_EQ (SUCCESS, status);

        printf ("%s\r\n", actualJsonValue.toStyledString ().c_str ());
        /*Utf8String expectedStrValue = Json::StyledWriter().write(expectedJsonValue);
        Utf8String actualStrValue = Json::StyledWriter().write(actualJsonValue);
        ASSERT_STREQ(expectedStrValue.c_str(), actualStrValue.c_str());*/

        ValidateCGPoint (expectedJsonValue["Center"]["Coordinate"]["xyz"], actualJsonValue["Center"]["Coordinate"]["xyz"]);
        ValidateCGPoint (expectedJsonValue["LLP"]["Coordinate"]["xyz"], actualJsonValue["LLP"]["Coordinate"]["xyz"]);
        ValidateCGPoint (expectedJsonValue["URP"]["Coordinate"]["xyz"], actualJsonValue["URP"]["Coordinate"]["xyz"]);

        ValidateCGPointArray (expectedJsonValue["Location"]["Polygon"]["Point"], actualJsonValue["Location"]["Polygon"]["Point"]);
        }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (JsonUpdaterTests, UpdateRelationshipProperty)
    {
    ECDb db;

    ECInstanceId sourceInstanceId;
    ECInstanceId targetInstanceId;
    ECInstanceId relInstanceId;

    ECInstanceKey sourceKey;
    ECInstanceKey targetKey;

    {
    SchemaItem testItem ("<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECClass typeName='A' isDomainClass='True'>"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "    </ECClass>"
        "    <ECRelationshipClass typeName='AHasA' isDomainClass='True'>"
        "        <ECProperty propertyName='Name' typeName='string' />"
        "        <Source cardinality='(0,N)' polymorphic='False'><Class class='A'/></Source>"
        "        <Target cardinality='(0,N)' polymorphic='False'><Class class='A'/></Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>",
        true, "");

    bool asserted = false;
    AssertSchemaImport (db, asserted, testItem, "updaterelationshipprop.ecdb");
    ASSERT_FALSE (asserted);

    ECSqlStatement stmt;
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (db, "INSERT INTO ts.A (P1) VALUES(?)"));


    ASSERT_EQ (ECSqlStatus::Success, stmt.BindInt (1, 111));
    ASSERT_EQ (BE_SQLITE_DONE, stmt.Step (sourceKey));
    sourceInstanceId = sourceKey.GetECInstanceId ();

    stmt.Reset ();
    stmt.ClearBindings ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.BindInt (1, 222));
    ASSERT_EQ (BE_SQLITE_DONE, stmt.Step (targetKey));
    targetInstanceId = targetKey.GetECInstanceId ();

    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (db, "INSERT INTO ts.AHasA (SourceECInstanceId, TargetECInstanceId, Name) VALUES(?,?,'good morning')"));

    ASSERT_EQ (ECSqlStatus::Success, stmt.BindId (1, sourceInstanceId));
    ASSERT_EQ (ECSqlStatus::Success, stmt.BindId (2, targetInstanceId));

    ECInstanceKey relKey;
    ASSERT_EQ (BE_SQLITE_DONE, stmt.Step (relKey));
    relInstanceId = relKey.GetECInstanceId ();
    }

    ECClassCP relClass = db.Schemas ().GetECClass ("test", "AHasA");
    ASSERT_TRUE (relClass != nullptr);
    JsonReader reader (db, relClass->GetId ());
    Json::Value relationshipJson;
    ASSERT_EQ (SUCCESS, reader.ReadInstance (relationshipJson, relInstanceId, JsonECSqlSelectAdapter::FormatOptions (ECValueFormat::RawNativeValues)));
    ASSERT_STREQ ("good morning", relationshipJson["Name"].asCString ());
    printf ("%s\r\n", relationshipJson.toStyledString ().c_str ());

    // Update relationship properties
    JsonUpdater updater (db, *relClass);

    /*
    * Update relationship properties via Json::Value API
    */
    Utf8CP expectedVal = "good afternoon";
    relationshipJson["Name"] = expectedVal;
    printf ("%s\r\n", relationshipJson.toStyledString ().c_str ());
    ASSERT_EQ (SUCCESS, updater.Update (relInstanceId, relationshipJson, sourceKey, targetKey));

    ECSqlStatement checkStmt;
    ASSERT_EQ (ECSqlStatus::Success, checkStmt.Prepare (db, "SELECT NULL FROM ts.AHasA WHERE ECInstanceId=? AND Name=?"));

    ASSERT_EQ (ECSqlStatus::Success, checkStmt.BindId (1, relInstanceId));
    ASSERT_EQ (ECSqlStatus::Success, checkStmt.BindText (2, expectedVal, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ (BE_SQLITE_ROW, checkStmt.Step ());
    checkStmt.Reset ();
    checkStmt.ClearBindings ();

    /*
    * Update relationship properties via rapidjson
    */
    expectedVal = "good evening";
    rapidjson::Document relationshipRapidJson;
    relationshipRapidJson.SetObject ();
    relationshipRapidJson.AddMember ("Name", expectedVal, relationshipRapidJson.GetAllocator ());

    ASSERT_EQ (SUCCESS, updater.Update (relInstanceId, relationshipRapidJson, sourceKey, targetKey));

    ASSERT_EQ (ECSqlStatus::Success, checkStmt.BindId (1, relInstanceId));
    ASSERT_EQ (ECSqlStatus::Success, checkStmt.BindText (2, expectedVal, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ (BE_SQLITE_ROW, checkStmt.Step ());
    checkStmt.Reset ();
    checkStmt.ClearBindings ();
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (JsonUpdaterTests, UpdateProperties)
    {
    ECDb ecdb;
    ECInstanceKey key;
    {
    SchemaItem testItem ("<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='testSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECClass typeName='A' isDomainClass='True'>"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "        <ECProperty propertyName='P2' typeName='string' />"
        "        <ECProperty propertyName='P3' typeName='double' />"
        "    </ECClass>"
        "</ECSchema>",
        true, "");

    bool asserted = false;
    AssertSchemaImport (ecdb, asserted, testItem, "updateClassProperties.ecdb");
    ASSERT_FALSE (asserted);

    ECSqlStatement stmt;
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "INSERT INTO ts.A (P1, P2, P3) VALUES(100, 'JsonTest', 1000.10)"));
    ASSERT_EQ (DbResult::BE_SQLITE_DONE, stmt.Step (key));
    }

    ECClassCP ecClass = ecdb.Schemas ().GetECClass ("testSchema", "A");
    ASSERT_TRUE (ecClass != nullptr);
    JsonReader reader (ecdb, ecClass->GetId ());
    Json::Value ecClassJson;
    ASSERT_EQ (SUCCESS, reader.ReadInstance (ecClassJson, key.GetECInstanceId(), JsonECSqlSelectAdapter::FormatOptions (ECValueFormat::RawNativeValues)));
    ASSERT_EQ (100, ecClassJson["P1"].asInt());
    ASSERT_STREQ ("JsonTest", ecClassJson["P2"].asCString ());
    ASSERT_EQ (1000.10, ecClassJson["P3"].asDouble ());
    printf ("%s\r\n", ecClassJson.toStyledString ().c_str ());

    // Update ecClass properties
    JsonUpdater updater (ecdb, *ecClass);

    /*
    * Update class properties via Json::Value API
    */
    Utf8CP expectedVal = "Json API";
    ecClassJson["P1"] = 200;
    ecClassJson["P2"] = expectedVal;
    ecClassJson["P3"] = 2000.20;
    printf ("%s\r\n", ecClassJson.toStyledString ().c_str ());
    ASSERT_EQ (SUCCESS, updater.Update (key.GetECInstanceId (), ecClassJson));

    ECSqlStatement checkStmt;
    ASSERT_EQ (ECSqlStatus::Success, checkStmt.Prepare (ecdb, "SELECT NULL FROM ts.A WHERE ECInstanceId=? AND P1=? AND P2=? AND P3=?"));

    ASSERT_EQ (ECSqlStatus::Success, checkStmt.BindId (1, key.GetECInstanceId()));
    ASSERT_EQ (ECSqlStatus::Success, checkStmt.BindInt (2, 200));
    ASSERT_EQ (ECSqlStatus::Success, checkStmt.BindText (3, expectedVal, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ (ECSqlStatus::Success, checkStmt.BindDouble (4, 2000.20));

    ASSERT_EQ (BE_SQLITE_ROW, checkStmt.Step ());
    checkStmt.Reset ();
    checkStmt.ClearBindings ();

    /*
    * Update class properties via rapidjson
    */
    expectedVal = "RapidJson";
    rapidjson::Document ecClassRapidJson;
    ecClassRapidJson.SetObject ();
    ecClassRapidJson.AddMember ("P1", 300, ecClassRapidJson.GetAllocator ());
    ecClassRapidJson.AddMember ("P2", expectedVal, ecClassRapidJson.GetAllocator ());
    ecClassRapidJson.AddMember ("P3", 3000.30, ecClassRapidJson.GetAllocator ());

    ASSERT_EQ (SUCCESS, updater.Update (key.GetECInstanceId(), ecClassRapidJson));

    ASSERT_EQ (ECSqlStatus::Success, checkStmt.BindId (1, key.GetECInstanceId()));
    ASSERT_EQ (ECSqlStatus::Success, checkStmt.BindInt (2, 300));
    ASSERT_EQ (ECSqlStatus::Success, checkStmt.BindText (3, expectedVal, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ (ECSqlStatus::Success, checkStmt.BindDouble (4, 3000.30));
    ASSERT_EQ (BE_SQLITE_ROW, checkStmt.Step ());
    checkStmt.Reset ();
    checkStmt.ClearBindings ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                              Ramanujam.Raman                   10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (JsonUpdaterTests, CommonGeometryJsonSerialization)
    {
    ECDbR ecdb = SetupECDb ("cgjsonserialization.ecdb", SchemaItem ("<?xml version='1.0' encoding='utf-8' ?>"
        "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "<ECClass typeName='SpatialLocation' isDomainClass='True'>"
        "  <ECProperty propertyName='Center' typeName='Bentley.Geometry.Common.IGeometry' />"
        "  <ECProperty propertyName='URP' typeName='Bentley.Geometry.Common.IGeometry' />"
        "  <ECProperty propertyName='LLP' typeName='Bentley.Geometry.Common.IGeometry' />"
        "  <ECProperty propertyName='Location' typeName='Bentley.Geometry.Common.IGeometry'/>"
        "</ECClass>"
        "</ECSchema>"), 3);

    ASSERT_TRUE (ecdb.IsDbOpen ());
    ECClassCP spatialClass = ecdb.Schemas ().GetECClass ("Test", "SpatialLocation");
    ASSERT_TRUE (nullptr != spatialClass);

    BeFileName pathname;
    BeTest::GetHost ().GetDocumentsRoot (pathname);
    pathname.AppendToPath (L"ECDb");
    pathname.AppendToPath (L"CommonGeometry.json");

    Json::Value expectedJsonCppValue;
    ECDbTestUtility::ReadJsonInputFromFile (expectedJsonCppValue, pathname);
    printf ("%s\r\n", expectedJsonCppValue.toStyledString ().c_str ());

    rapidjson::Document expectedRapidJsonValue;
    bool parseSuccessful = !expectedRapidJsonValue.Parse<0> (Json::FastWriter ().write (expectedJsonCppValue).c_str ()).HasParseError ();
    ASSERT_TRUE (parseSuccessful);

    // Insert using RapidJson API
    JsonInserter inserter (ecdb, *spatialClass);
    ECInstanceKey rapidJsonInstanceKey;
    ASSERT_EQ (SUCCESS, inserter.Insert (rapidJsonInstanceKey, expectedRapidJsonValue));

    // Insert using JsonCpp API
    ECInstanceKey jsonCppInstanceKey;
    ASSERT_EQ (SUCCESS, inserter.Insert (jsonCppInstanceKey, expectedJsonCppValue));

    ecdb.SaveChanges ();

    // Validate
    ValidateSpatialInstance (ecdb, rapidJsonInstanceKey, expectedJsonCppValue);
    ValidateSpatialInstance (ecdb, jsonCppInstanceKey, expectedJsonCppValue);
    }

END_ECDBUNITTESTS_NAMESPACE
