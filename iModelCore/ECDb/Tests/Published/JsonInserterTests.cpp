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
    };

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (JsonInserterTests, InsertJsonCppJSON)
    {
    ECDbR ecdb = SetupECDb("insertUsingJsonAPI.ecdb", BeFileName(L"JsonTests.01.00.ecschema.xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    // Read JSON input from file
    BeFileName jsonInputFile;
    BeTest::GetHost ().GetDocumentsRoot (jsonInputFile);
    jsonInputFile.AppendToPath (L"ECDb");
    jsonInputFile.AppendToPath (L"JsonTestClass.json");

    Json::Value jsonInput;
    ECDbTestUtility::ReadJsonInputFromFile (jsonInput, jsonInputFile);

    ECClassCP documentClass = ecdb.Schemas ().GetECClass ("JsonTests", "Document");
    ASSERT_TRUE (documentClass != nullptr);
    JsonInserter inserter (ecdb, *documentClass);

    //----------------------------------------------------------------------------------- 
    // Insert using JsonCpp
    //-----------------------------------------------------------------------------------
    ECInstanceKey id;
    ASSERT_EQ (SUCCESS, inserter.Insert (id, jsonInput));

    ECSqlStatement statement;
    ASSERT_EQ (ECSqlStatus::Success, statement.Prepare (ecdb, "SELECT NULL FROM jt.Document WHERE ECInstanceId=? AND Name=?"));
    ASSERT_EQ (ECSqlStatus::Success, statement.BindId (1, id.GetECInstanceId ()));
    ASSERT_EQ (ECSqlStatus::Success, statement.BindText (2, "A-Model.pdf", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ (DbResult::BE_SQLITE_ROW, statement.Step ());
    statement.Finalize ();

    //verify other Overload
    ASSERT_EQ (SUCCESS, inserter.Insert (jsonInput));
    ASSERT_EQ (ECSqlStatus::Success, statement.Prepare (ecdb, "SELECT COUNT(*) FROM jt.Document"));
    ASSERT_EQ (DbResult::BE_SQLITE_ROW, statement.Step ());
    ASSERT_EQ (2, statement.GetValueInt (0));
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (JsonInserterTests, InsertRapidJson)
    {
    ECDbR ecdb = SetupECDb("InsertUsingRapidJson.ecdb", BeFileName(L"JsonTests.01.00.ecschema.xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    // Read JSON input from file
    BeFileName jsonInputFile;
    BeTest::GetHost ().GetDocumentsRoot (jsonInputFile);
    jsonInputFile.AppendToPath (L"ECDb");
    jsonInputFile.AppendToPath (L"JsonTestClass.json");

    Json::Value jsonInput;
    ECDbTestUtility::ReadJsonInputFromFile (jsonInput, jsonInputFile);

    //to rapidjson
    rapidjson::Document rapidJsonInput;
    bool parseSuccessful = !rapidJsonInput.Parse<0> (Json::FastWriter ().write (jsonInput).c_str ()).HasParseError ();
    ASSERT_TRUE (parseSuccessful);

    ECClassCP documentClass = ecdb.Schemas ().GetECClass ("JsonTests", "Document");
    ASSERT_TRUE (documentClass != nullptr);
    JsonInserter inserter (ecdb, *documentClass);

    //-----------------------------------------------------------------------------------
    // Insert using rapidjson
    //-----------------------------------------------------------------------------------
    ECInstanceKey id;
    ASSERT_EQ (SUCCESS, inserter.Insert (id, rapidJsonInput));

    ECSqlStatement statement;
    ASSERT_EQ (ECSqlStatus::Success ,statement.Prepare (ecdb, "SELECT NULL FROM jt.Document WHERE ECInstanceId=? AND Name=?"));
    ASSERT_EQ (ECSqlStatus::Success, statement.BindId (1, id.GetECInstanceId ()));
    ASSERT_EQ (ECSqlStatus::Success, statement.BindText (2, "A-Model.pdf", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ (DbResult::BE_SQLITE_ROW, statement.Step ());
    }

END_ECDBUNITTESTS_NAMESPACE