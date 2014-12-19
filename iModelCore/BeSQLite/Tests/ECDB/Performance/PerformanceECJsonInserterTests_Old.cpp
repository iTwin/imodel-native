/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/ECDB/Performance/PerformanceECJsonInserterTests_Old.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <UnitTests/NonPublished/ECDb/ECDbTestProject.h>
#include"PerformanceTestFixture.h"
using namespace BentleyApi::ECN;

BEGIN_ECDBUNITTESTS_NAMESPACE

extern void ReadJsonInputFromFile (Json::Value& jsonInput, BeFileName& jsonFilePath);

//---------------------------------------------------------------------------------------
// @bsiclass                                   Krischan.Eberle                  05/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST (PerformanceECJsonInserter_Old, Insert)
    {
    ECDbTestProject testProject;
    auto& ecdb = testProject.Create ("performanceecjsoninserter.ecdb", L"eB_PW_CommonSchema_WSB.01.00.ecschema.xml", false);

    // Read JSON input from file
    BeFileName jsonInputFile;
    BeTest::GetHost().GetDocumentsRoot (jsonInputFile);
    jsonInputFile.AppendToPath (L"DgnDb");
    jsonInputFile.AppendToPath (L"FieldEngineerStructArray.json");

    // Parse JSON value using JsonCpp
    Json::Value jsonInput;
    ReadJsonInputFromFile (jsonInput, jsonInputFile);

    // Parse JSON value using RapidJson
    rapidjson::Document rapidJsonInput;
    bool parseSuccessful = !rapidJsonInput.Parse<0>(Json::FastWriter().write(jsonInput).c_str()).HasParseError();
    ASSERT_TRUE (parseSuccessful);

    ECClassP documentClass = nullptr;
    ecdb.GetEC().GetSchemaManager().GetECClass (documentClass, "eB_PW_CommonSchema_WSB", "Document");
    ASSERT_TRUE (documentClass != nullptr);
    ECJsonInserter inserter (ecdb, documentClass->GetId());

    const int repetitionCount = 1000;

    //printf ("Attach to profiler and press any key to continue...\n"); getchar ();

    //-----------------------------------------------------------------------------------
    // Insert using JsonCpp
    //-----------------------------------------------------------------------------------
    StopWatch timer (true);
    for (int i = 0; i < repetitionCount; i++)
        {
        auto insertStatus = inserter.Insert (jsonInput);
        ASSERT_EQ (SUCCESS, insertStatus);
        }
    timer.Stop ();
    ecdb.SaveChanges();
    LOG.infov ("Inserting JsonCpp JSON objects into ECDb %d times took %.4f msecs.", repetitionCount, timer.GetElapsedSeconds () * 1000.0);
    PerformanceTestingFrameWork performanceObj;
    EXPECT_TRUE(performanceObj.writeTodb(L"PerformanceTest.ecdb", timer, "PerformanceECJsonInserter,Insert", "Inserting JsonCpp JSON objects into ECDb For repetitionCount 1000  "));
    //-----------------------------------------------------------------------------------
    // Insert using RapidJson
    //-----------------------------------------------------------------------------------
    StopWatch timerRapidJason(true);
    for (int i = 0; i < repetitionCount; i++)
        {
        ECInstanceId ecInstanceId;
        auto insertStatus = inserter.Insert (&ecInstanceId, rapidJsonInput);
        ASSERT_EQ (SUCCESS, insertStatus);
        ASSERT_TRUE (ecInstanceId.IsValid ());
        }
    timerRapidJason.Stop();
    ecdb.SaveChanges();
    LOG.infov ("Inserting RapidJson JSON objects into ECDb %d times took %.4f msecs.", repetitionCount, timer.GetElapsedSeconds() * 1000.0);
    PerformanceTestingFrameWork performanceObjRapidJason;
    EXPECT_TRUE(performanceObjRapidJason.writeTodb(L"PerformanceTest.ecdb", timerRapidJason, "PerformanceECJsonInserter,Insert", "Inserting RapidJson JSON objects into ECDb For repetitionCount 1000 "));
    }

END_ECDBUNITTESTS_NAMESPACE
