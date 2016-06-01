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
    {
    //---------------------------------------------------------------------------------------
    // @bsimethod                                      Muhammad Hassan                  05/16
    //+---------------+---------------+---------------+---------------+---------------+------
    bool WriteJsonToFile(WCharCP path, const Json::Value& jsonValue)
        {
        Utf8String strValue = Json::StyledWriter().write(jsonValue);
        FILE* file = fopen(Utf8String(path).c_str(), "w");

        if (file == NULL)
            {
            BeAssert(false);
            return false;
            }
        fwprintf(file, L"%ls", WString(strValue.c_str(), BentleyCharEncoding::Utf8).c_str());
        fclose(file);
        return true;
        }
    };

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
                        "    <ECRelationshipClass typeName='AHasA' strength='referencing' modifier='Sealed'>"
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

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonReaderTests, JsonValueStruct)
    {
    ECDbR ecdb = SetupECDb("insertUsingJsonAPI.ecdb", BeFileName(L"StartupCompany.02.00.ecschema.xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    ECSchemaCP startupSchema = ecdb.Schemas().GetECSchema("StartupCompany");
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

    ECInstanceInserter fooInserter(GetECDb(), *fooClass);
    fooInserter.Insert(*foo);
    ecdb.SaveChanges();

    /* Retrieve the JSON for the inserted Instance */
    ECSqlStatement statement;
    ECSqlStatus prepareStatus = statement.Prepare(ecdb, "SELECT * FROM ONLY stco.Foo");
    ASSERT_TRUE(ECSqlStatus::Success == prepareStatus);
    DbResult stepStatus = statement.Step();
    ASSERT_EQ(BE_SQLITE_ROW, stepStatus);

    Json::Value jsonValue;
    JsonECSqlSelectAdapter jsonAdapter(statement);
    jsonAdapter.GetRowDisplayInfo(jsonValue);
    bool stat = jsonAdapter.GetRow(jsonValue["Row"]);
    ASSERT_TRUE(stat);
    statement.Finalize();

    // Read benchmark JSON
    BeFileName jsonInputFile;
    BeTest::GetHost().GetDocumentsRoot(jsonInputFile);
    jsonInputFile.AppendToPath(L"ECDb");
    jsonInputFile.AppendToPath(L"StartupCompany.json");

    Json::Value jsonInput;
    ECDbTestUtility::ReadJsonInputFromFile(jsonInput, jsonInputFile);

    /* Validate */
    int compare = jsonInput.compare(jsonValue);
    if (0 != compare)
        {
        BeFileName afterImportFile;
        BeTest::GetHost().GetOutputRoot(afterImportFile);
        afterImportFile.AppendToPath(L"StartupCompany-AfterImport.json");
        WriteJsonToFile(afterImportFile.GetName(), jsonValue);
        ASSERT_TRUE(false) << "Inserted and Retrieved Json doesn't match";
        }
    }

END_ECDBUNITTESTS_NAMESPACE