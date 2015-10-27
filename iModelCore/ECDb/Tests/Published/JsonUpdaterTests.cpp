/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/JsonUpdaterTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "SchemaImportTestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE
struct JsonUpdaterTests : SchemaImportTestFixture
    {};

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonUpdaterTests, UpdateRelationshipProperty)
    {
    ECDb db;
    ECInstanceId a1;
    ECInstanceId a2;
    ECInstanceId rel;
    {
    TestItem testItem("<?xml version='1.0' encoding='utf-8'?>"
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
    AssertSchemaImport(db, asserted, testItem, "updaterelationshipprop.ecdb");
    ASSERT_FALSE(asserted);

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "INSERT INTO ts.A (P1) VALUES(?)"));


    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, 111));
    ECInstanceKey newKey;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey));
    a1 = newKey.GetECInstanceId();

    stmt.Reset();
    stmt.ClearBindings();   

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, 222));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey));
    a2 = newKey.GetECInstanceId();

    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "INSERT INTO ts.AHasA (SourceECInstanceId, TargetECInstanceId, Name) VALUES(?,?,'good morning')"));

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, a1));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, a2));

    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey));
    rel = newKey.GetECInstanceId();
    }

    ECClassCP relClass = db.Schemas().GetECClass("test", "AHasA");
    ASSERT_TRUE(relClass != nullptr);
    JsonReader reader(db, relClass->GetId());
    Json::Value relationshipJson;
    ASSERT_EQ(SUCCESS, reader.ReadInstance(relationshipJson, rel, JsonECSqlSelectAdapter::FormatOptions (ECValueFormat::RawNativeValues)));
    ASSERT_STREQ("good morning", relationshipJson["Name"].asCString());
    printf("%s\r\n", relationshipJson.toStyledString().c_str());
    
    // Update relationship properties
    JsonUpdater updater(db, *relClass);

    //via Json::Value API
    Utf8CP expectedVal = "good afternoon";
    relationshipJson["Name"] = expectedVal;
    printf("%s\r\n", relationshipJson.toStyledString().c_str());
    ASSERT_EQ(SUCCESS, updater.Update(relationshipJson));

    ECSqlStatement checkStmt;
    ASSERT_EQ(ECSqlStatus::Success, checkStmt.Prepare(db, "SELECT NULL FROM ts.AHasA WHERE ECInstanceId=? AND Name=?"));

    ASSERT_EQ(ECSqlStatus::Success, checkStmt.BindId(1, rel));
    ASSERT_EQ(ECSqlStatus::Success, checkStmt.BindText(2, expectedVal, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_ROW, checkStmt.Step());
    checkStmt.Reset();
    checkStmt.ClearBindings();

    //via rapidjson
    expectedVal = "good evening";
    rapidjson::Document relationshipRapidJson;
    relationshipRapidJson.SetObject();
    relationshipRapidJson.AddMember("Name", expectedVal, relationshipRapidJson.GetAllocator());

    ASSERT_EQ(SUCCESS, updater.Update(rel, relationshipRapidJson));

    ASSERT_EQ(ECSqlStatus::Success, checkStmt.BindId(1, rel));
    ASSERT_EQ(ECSqlStatus::Success, checkStmt.BindText(2, expectedVal, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_ROW, checkStmt.Step());
    checkStmt.Reset();
    checkStmt.ClearBindings();
    }

END_ECDBUNITTESTS_NAMESPACE
