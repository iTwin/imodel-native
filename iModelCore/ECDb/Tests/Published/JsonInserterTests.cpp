/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/JsonInserterTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

struct JsonInserterTests : public ECDbTestFixture
    {

    //---------------------------------------------------------------------------------------
    // @bsimethod                                      Muhammad Hassan                  02/16
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
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonInserterTests, InsertJsonCppJSON)
    {
    ECDbR ecdb = SetupECDb("insertUsingJsonAPI.ecdb", BeFileName(L"JsonTests.01.00.ecschema.xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    // Read JSON input from file
    BeFileName jsonInputFile;
    BeTest::GetHost().GetDocumentsRoot(jsonInputFile);
    jsonInputFile.AppendToPath(L"ECDb");
    jsonInputFile.AppendToPath(L"JsonTestClass.json");

    Json::Value jsonInput;
    ECDbTestUtility::ReadJsonInputFromFile(jsonInput, jsonInputFile);

    ECClassCP documentClass = ecdb.Schemas().GetECClass("JsonTests", "Document");
    ASSERT_TRUE(documentClass != nullptr);
    JsonInserter inserter(ecdb, *documentClass);

    //----------------------------------------------------------------------------------- 
    // Insert using JsonCpp
    //-----------------------------------------------------------------------------------
    ECInstanceKey id;
    ASSERT_EQ(SUCCESS, inserter.Insert(id, jsonInput));
    ecdb.SaveChanges();

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT NULL FROM jt.Document WHERE ECInstanceId=? AND Name=?"));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindId(1, id.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindText(2, "A-Model.pdf", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    statement.Finalize();

    /* Retrieve the previously imported instance as JSON */
    ECSqlStatus prepareStatus = statement.Prepare(ecdb, "SELECT * FROM ONLY jt.Document");
    ASSERT_TRUE(ECSqlStatus::Success == prepareStatus);
    DbResult stepStatus = statement.Step();
    ASSERT_EQ(BE_SQLITE_ROW, stepStatus);

    Json::Value afterImportJson;
    JsonECSqlSelectAdapter jsonAdapter(statement, JsonECSqlSelectAdapter::FormatOptions(ECValueFormat::RawNativeValues));
    bool status = jsonAdapter.GetRowInstance(afterImportJson, documentClass->GetId());
    ASSERT_TRUE(status);
    statement.Finalize();

    /* Validate */
    int compare = jsonInput.compare(afterImportJson);
    if (0 != compare)
        {
        BeFileName afterImportFile;
        BeTest::GetHost().GetOutputRoot(afterImportFile);
        afterImportFile.AppendToPath(L"JsonTestClass-AfterImport.json");
        WriteJsonToFile(afterImportFile.GetName(), afterImportJson);
        ASSERT_TRUE(false) << "Inserted and Retrieved Json doesn't match";
        }

    //verify Json Insertion using the other Overload
    ASSERT_EQ(SUCCESS, inserter.Insert(jsonInput));
    ecdb.SaveChanges();

    //Verify Inserted Instances.
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT COUNT(*) FROM jt.Document"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(2, statement.GetValueInt(0));
    statement.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonInserterTests, InsertRapidJson)
    {
    ECDbR ecdb = SetupECDb("InsertUsingRapidJson.ecdb", BeFileName(L"JsonTests.01.00.ecschema.xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    // Read JSON input from file
    BeFileName jsonInputFile;
    BeTest::GetHost().GetDocumentsRoot(jsonInputFile);
    jsonInputFile.AppendToPath(L"ECDb");
    jsonInputFile.AppendToPath(L"JsonTestClass.json");

    Json::Value jsonInput;
    ECDbTestUtility::ReadJsonInputFromFile(jsonInput, jsonInputFile);

    //to rapidjson
    rapidjson::Document rapidJsonInput;
    bool parseSuccessful = !rapidJsonInput.Parse<0>(Json::FastWriter().write(jsonInput).c_str()).HasParseError();
    ASSERT_TRUE(parseSuccessful);

    ECClassCP documentClass = ecdb.Schemas().GetECClass("JsonTests", "Document");
    ASSERT_TRUE(documentClass != nullptr);
    JsonInserter inserter(ecdb, *documentClass);

    //-----------------------------------------------------------------------------------
    // Insert using rapidjson
    //-----------------------------------------------------------------------------------
    ECInstanceKey id;
    ASSERT_EQ(SUCCESS, inserter.Insert(id, rapidJsonInput));
    ecdb.SaveChanges();

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT NULL FROM jt.Document WHERE ECInstanceId=? AND Name=?"));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindId(1, id.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindText(2, "A-Model.pdf", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    statement.Finalize();

    /* Retrieve the previously imported instance as JSON */
    ECSqlStatus prepareStatus = statement.Prepare(ecdb, "SELECT * FROM ONLY jt.Document");
    ASSERT_TRUE(ECSqlStatus::Success == prepareStatus);
    DbResult stepStatus = statement.Step();
    ASSERT_EQ(BE_SQLITE_ROW, stepStatus);

    Json::Value afterImportJson;
    JsonECSqlSelectAdapter jsonAdapter(statement, JsonECSqlSelectAdapter::FormatOptions(ECValueFormat::RawNativeValues));
    bool status = jsonAdapter.GetRowInstance(afterImportJson, documentClass->GetId());
    ASSERT_TRUE(status);
    statement.Finalize();

    /* Validate */
    int compare = jsonInput.compare(afterImportJson);
    if (0 != compare)
        {
        BeFileName afterImportFile;
        BeTest::GetHost().GetOutputRoot(afterImportFile);
        afterImportFile.AppendToPath(L"JsonTestClass-AfterImport.json");
        WriteJsonToFile(afterImportFile.GetName(), afterImportJson);
        ASSERT_TRUE(false) << "Inserted and Retrieved Json doesn't match";
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonInserterTests, CreateRoot_ExistingRoot_ReturnsSameKey_ECDBTEST)
    {
    ECDbTestFixture::Initialize();
    ECDbR ecdb = SetupECDb("schemaupgradetest.ecdb", BeFileName(L"DSCacheSchema.01.03.ecschema.xml"));
    ecdb.SaveChanges();

    IECClassLocaterR classLocater = ecdb.GetClassLocater();
    ECClassCP rootClass = classLocater.LocateClass("DSCacheSchema", "Root");
    ASSERT_NE(nullptr, rootClass);

    // Names
    Utf8String rootName = "Foo";

    // Test quety for same instance
    Utf8String ecsql = "SELECT ECInstanceId FROM [DSC].[Root] WHERE [Name] = ? LIMIT 1 ";
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, ecsql.c_str()));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindText(1, rootName.c_str(), IECSqlBinder::MakeCopy::No));
    EXPECT_EQ(BE_SQLITE_DONE, statement.Step());

    // Insert one instnace
    Json::Value rootInstance;
    rootInstance["Name"] = rootName;
    rootInstance["Persistance"] = 0;

    JsonInserter inserter(ecdb, *rootClass);
    ASSERT_EQ(SUCCESS, inserter.Insert(rootInstance));

    // Try again
    statement.Reset();
    statement.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, statement.BindText(1, rootName.c_str(), IECSqlBinder::MakeCopy::No));
    EXPECT_EQ(BE_SQLITE_ROW, statement.Step());
    EXPECT_EQ(1, statement.GetValueId <ECInstanceId>(0).GetValue());
    }

END_ECDBUNITTESTS_NAMESPACE