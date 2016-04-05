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

//---------------------------------------------------------------------------------------
// @bsiclass                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
struct JsonReaderTests : public SchemaImportTestFixture
    {};

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
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
                        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                        "<ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.13' prefix='bsca' />"
                        "        <ECCustomAttributes>"
                        "            <RelatedItemsDisplaySpecifications xmlns='Bentley_Standard_CustomAttributes.01.13'>"
                        "                <Specifications>"
                        "                   <RelatedItemsDisplaySpecification>"
                        "                   <ParentClass>ts:A1</ParentClass>"
                        "                   <RelationshipPath>ts:AHasA</RelationshipPath>"
                        "                   <DerivedClasses>"
                        "                   <string>ts:A2</string>"
                        "                   </DerivedClasses>"
                        "                   </RelatedItemsDisplaySpecification>"
                        "                </Specifications>"
                        "            </RelatedItemsDisplaySpecifications>"
                        "        </ECCustomAttributes>"
                        "    <ECEntityClass typeName='A' >"
                        "        <ECProperty propertyName='Aprop' typeName='int' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='A1' >"
                        "        <BaseClass>A</BaseClass>"
                        "        <ECProperty propertyName='A1prop' typeName='int' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='A2' >"
                        "        <BaseClass>A</BaseClass>"
                        "        <ECProperty propertyName='A2prop' typeName='int' />"
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "INSERT INTO ts.A (Aprop) VALUES(?)"));

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

    ECClassCP entityClass = db.Schemas().GetECClass("TestSchema", "A");
    ASSERT_TRUE(entityClass != nullptr);
    JsonReader reader(db, entityClass->GetId());
    Json::Value classJsonWithRelatedInstances;
    Json::Value jsonDisplayInfo;
    ASSERT_EQ(SUCCESS, reader.Read(classJsonWithRelatedInstances, jsonDisplayInfo, sourceInstanceId, JsonECSqlSelectAdapter::FormatOptions(ECValueFormat::RawNativeValues)));
    //test needs to be enhanced once fixed Defect 383266
    }

END_ECDBUNITTESTS_NAMESPACE