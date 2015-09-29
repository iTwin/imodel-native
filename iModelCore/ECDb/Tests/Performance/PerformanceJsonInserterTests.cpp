/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/PerformanceJsonInserterTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceTests.h"

using namespace BentleyApi::ECN;

BEGIN_ECDBUNITTESTS_NAMESPACE
extern void ReadJsonInputFromFile(Json::Value& jsonInput, BeFileName& jsonFilePath);

//---------------------------------------------------------------------------------------
// @bsiclass                                   Muhammad.Zaighum                  05/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST(PerformanceJsonInserter, InsertJsonCppUsingPresistanceAPI)
{
    ECDbTestProject testProject;
    auto& ecdb = testProject.Create("performancejsoninserter.ecdb", L"eB_PW_CommonSchema_WSB.01.00.ecschema.xml", false);

    // Read JSON input from file
    BeFileName jsonInputFile;
    BeTest::GetHost().GetDocumentsRoot(jsonInputFile);
    jsonInputFile.AppendToPath(L"DgnDb");
    jsonInputFile.AppendToPath(L"FieldEngineerStructArray.json");

    // Parse JSON value using JsonCpp
    Json::Value jsonInput;
    ReadJsonInputFromFile(jsonInput, jsonInputFile);
    ECClassCP documentClass = ecdb.Schemas().GetECClass("eB_PW_CommonSchema_WSB", "Document");
    ASSERT_TRUE(documentClass != nullptr);
    ASSERT_TRUE(documentClass != nullptr);
    JsonInserter inserter(ecdb, *documentClass);
    const int repetitionCount = 1000;
    //----------------------------------------------------------------------------------- 
    // Insert using JsonCpp
    //-----------------------------------------------------------------------------------
    StopWatch timer(true);
    for (int i = 0; i < repetitionCount; i++)
    {
        ECInstanceKey id;
        auto insertStatus = inserter.Insert(id, jsonInput);
        ASSERT_EQ(SUCCESS, insertStatus);
    }
    timer.Stop();
    ecdb.SaveChanges();
    LOG.infov("Inserting JsonCpp JSON objects into ECDb %d times took %.4f msecs.", repetitionCount, timer.GetElapsedSeconds() * 1000.0);
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), "Inserting JsonCpp JSON objects into ECDb For repetitionCount", 1000);
}


//---------------------------------------------------------------------------------------
// @bsiclass                                   Muhammad.Zaighum                  05/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST(PerformanceJsonInserter, InsertRapidJsonInsertJasonCppUsingPresistanceAPI)
{
    ECDbTestProject testProject;
    auto& ecdb = testProject.Create("performancejsoninserter.ecdb", L"eB_PW_CommonSchema_WSB.01.00.ecschema.xml", false);

    // Read JSON input from file
    BeFileName jsonInputFile;
    BeTest::GetHost().GetDocumentsRoot(jsonInputFile);
    jsonInputFile.AppendToPath(L"DgnDb");
    jsonInputFile.AppendToPath(L"FieldEngineerStructArray.json");

    // Parse JSON value using JsonCpp
    Json::Value jsonInput;
    ReadJsonInputFromFile(jsonInput, jsonInputFile);

    // Parse JSON value using RapidJson
    rapidjson::Document rapidJsonInput;
    bool parseSuccessful = !rapidJsonInput.Parse<0>(Json::FastWriter().write(jsonInput).c_str()).HasParseError();
    ASSERT_TRUE(parseSuccessful);

    ECClassCP documentClass = ecdb.Schemas().GetECClass("eB_PW_CommonSchema_WSB", "Document");
    ASSERT_TRUE(documentClass != nullptr);
    JsonInserter inserter(ecdb, *documentClass);

    const int repetitionCount = 1000;
    //-----------------------------------------------------------------------------------
    // Insert using RapidJson
    //-----------------------------------------------------------------------------------
    StopWatch rapidJasintimer(true);
    for (int i = 0; i < repetitionCount; i++)
    {
        ECInstanceKey ecInstanceKey;
        auto insertStatus = inserter.Insert(ecInstanceKey, rapidJsonInput);
        ASSERT_EQ(SUCCESS, insertStatus);
        ASSERT_TRUE(ecInstanceKey.IsValid());
    }
    rapidJasintimer.Stop();
    ecdb.SaveChanges();
    LOG.infov("Inserting RapidJson JSON objects into ECDb %d times took %.4f msecs.", repetitionCount, rapidJasintimer.GetElapsedSeconds() * 1000.0);

    LOGTODB(TEST_DETAILS, rapidJasintimer.GetElapsedSeconds(), "Inserting RapidJson JSON objects into ECDb For repetitionCount", 1000);
}

END_ECDBUNITTESTS_NAMESPACE
