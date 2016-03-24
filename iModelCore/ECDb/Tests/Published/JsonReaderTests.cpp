/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/JsonReaderTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "SchemaImportTestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

struct JsonReaderTests : public SchemaImportTestFixture
    {};

TEST_F(JsonReaderTests, ReadInstanceAlongWithRelatedInstances)
    {
    ECDb db;

    ECInstanceId sourceInstanceId;
    ECInstanceId targetInstanceId;
    ECInstanceId relInstanceId;

    ECInstanceKey sourceKey;
    ECInstanceKey targetKey;

    {
    SchemaItem testItem("<?xml version='1.0' encoding='utf-8'?>"
                        "<ECSchema schemaName='test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                        "    <ECEntityClass typeName='A' >"
                        "        <ECProperty propertyName='P1' typeName='int' />"
                        "    </ECEntityClass>"
                        "    <ECRelationshipClass typeName='AHasA' strength='referencing'>"
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
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(sourceKey));
    sourceInstanceId = sourceKey.GetECInstanceId();

    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, 222));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(targetKey));
    targetInstanceId = targetKey.GetECInstanceId();

    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "INSERT INTO ts.AHasA (SourceECInstanceId, TargetECInstanceId, Name) VALUES(?,?,'good morning')"));

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, sourceInstanceId));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, targetInstanceId));

    ECInstanceKey relKey;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(relKey));
    relInstanceId = relKey.GetECInstanceId();
    }

    ECClassCP entityClass = db.Schemas().GetECClass("test", "A");
    ASSERT_TRUE(entityClass != nullptr);
    JsonReader reader(db, entityClass->GetId());
    Json::Value classJsonWithRelatedInstances;
    Json::Value jsonDisplayInfo;
    ASSERT_EQ(SUCCESS, reader.Read(classJsonWithRelatedInstances, jsonDisplayInfo, sourceInstanceId, JsonECSqlSelectAdapter::FormatOptions(ECValueFormat::RawNativeValues)));
    //test needs to be enhanced once fixed Defect 383266
    }

END_ECDBUNITTESTS_NAMESPACE