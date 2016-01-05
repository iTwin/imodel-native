/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/PerformanceJsonInserterTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceTests.h"
#include "../BackDoor/PublicAPI/BackDoor/ECDb/ECDbTestProject.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass                                   Muhammad.Zaighum                  05/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST(PerformanceJsonInserter, InsertJsonCppUsingPresistanceAPI)
{
    ECDbTestProject testProject;
    ECDbR ecdb = testProject.Create("performancejsoninserter.ecdb", L"eB_PW_CommonSchema_WSB.01.00.ecschema.xml", false);

    // Read JSON input from file
    BeFileName jsonInputFile;
    BeTest::GetHost().GetDocumentsRoot(jsonInputFile);
    jsonInputFile.AppendToPath (L"ECDb");
    jsonInputFile.AppendToPath(L"FieldEngineerStructArray.json");

    // Parse JSON value using JsonCpp
    Json::Value jsonInput;
    ECDbTestUtility::ReadJsonInputFromFile(jsonInput, jsonInputFile);
    ECClassCP documentClass = ecdb.Schemas().GetECClass("eB_PW_CommonSchema_WSB", "Document");
    ASSERT_TRUE(documentClass != nullptr);
    JsonInserter inserter(ecdb, *documentClass);
    const int repetitionCount = 10000;

    //----------------------------------------------------------------------------------- 
    // Insert using JsonCpp
    //-----------------------------------------------------------------------------------
    StopWatch timer(true);
    for (int i = 0; i < repetitionCount; i++)
    {
        ECInstanceKey key;
        ASSERT_EQ(SUCCESS, inserter.Insert (key, jsonInput));
        ASSERT_TRUE (key.IsValid ());
    }
    timer.Stop();
    ecdb.SaveChanges();

    ECSqlStatement statement;
    statement.Prepare (ecdb, "SELECT COUNT(*) FROM eBPWC.Document");
    DbResult::BE_SQLITE_ROW, statement.Step ();
    ASSERT_EQ (repetitionCount, statement.GetValueInt (0)) << "Expected Number of Instances not inserted in Db";

    LOG.infov("Inserting JsonCpp JSON objects into ECDb %d times took %.4f msecs.", repetitionCount, timer.GetElapsedSeconds() * 1000.0);
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), "Inserting JsonCpp JSON objects into ECDb For repetitionCount", repetitionCount);
}


//---------------------------------------------------------------------------------------
// @bsiclass                                   Muhammad.Zaighum                  05/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST(PerformanceJsonInserter, InsertRapidJsonUsingPresistanceAPI)
{
    ECDbTestProject testProject;
    auto& ecdb = testProject.Create("performancejsoninserter.ecdb", L"eB_PW_CommonSchema_WSB.01.00.ecschema.xml", false);

    // Read JSON input from file
    BeFileName jsonInputFile;
    BeTest::GetHost().GetDocumentsRoot(jsonInputFile);
    jsonInputFile.AppendToPath (L"ECDb");
    jsonInputFile.AppendToPath(L"FieldEngineerStructArray.json");

    // Parse JSON value using JsonCpp
    Json::Value jsonInput;
    ECDbTestUtility::ReadJsonInputFromFile(jsonInput, jsonInputFile);

    // Parse JSON value using RapidJson
    rapidjson::Document rapidJsonInput;
    bool parseSuccessful = !rapidJsonInput.Parse<0>(Json::FastWriter().write(jsonInput).c_str()).HasParseError();
    ASSERT_TRUE(parseSuccessful);

    ECClassCP documentClass = ecdb.Schemas().GetECClass("eB_PW_CommonSchema_WSB", "Document");
    ASSERT_TRUE(documentClass != nullptr);
    JsonInserter inserter(ecdb, *documentClass);

    const int repetitionCount = 10000;
    //-----------------------------------------------------------------------------------
    // Insert using RapidJson
    //-----------------------------------------------------------------------------------
    StopWatch timer(true);
    for (int i = 0; i < repetitionCount; i++)
    {
        ECInstanceKey ecInstanceKey;
        ASSERT_EQ(SUCCESS, inserter.Insert (ecInstanceKey, rapidJsonInput));
        ASSERT_TRUE(ecInstanceKey.IsValid());
    }
    timer.Stop();
    ecdb.SaveChanges();

    ECSqlStatement statement;
    statement.Prepare (ecdb, "SELECT COUNT(*) FROM eBPWC.Document");
    DbResult::BE_SQLITE_ROW, statement.Step ();
    ASSERT_EQ (repetitionCount, statement.GetValueInt (0)) << "Expected Number of Instances not inserted in Db";

    LOG.infov ("Inserting RapidJson JSON objects into ECDb %d times took %.4f msecs.", repetitionCount, timer.GetElapsedSeconds () * 1000.0);
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), "Inserting RapidJson JSON objects into ECDb For repetitionCount", repetitionCount);
}

END_ECDBUNITTESTS_NAMESPACE
