/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/ECDB/Performance/PerformanceJsonInserterTests.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <UnitTests/NonPublished/ECDb/ECDbTestProject.h>
#include"PerformanceTestFixture.h"
using namespace BentleyApi::ECN;

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                  Muhammad.Zaighum                  05/13
//+---------------+---------------+---------------+---------------+---------------+------
void ReadJsonInputFromFile (Json::Value& jsonInput, BeFileName& jsonFilePath)
    {
    const byte utf8BOM[] = {0xef, 0xbb, 0xbf};

    Utf8String fileContent;

    BeFile file;
    auto fileStatus = file.Open (jsonFilePath, BeFileAccess::Read);
    ASSERT_TRUE(BeFileStatus::Success ==fileStatus);

    UInt64 rawSize;
    fileStatus = file.GetSize (rawSize);
    ASSERT_TRUE (BeFileStatus::Success == fileStatus && rawSize <= UINT32_MAX);

    UInt32 sizeToRead = (UInt32) rawSize;

    UInt32 sizeRead;
    ScopedArray<byte> scopedBuffer (sizeToRead);
    byte* buffer = scopedBuffer.GetData();
    fileStatus = file.Read (buffer, &sizeRead, sizeToRead);
    ASSERT_TRUE (BeFileStatus::Success == fileStatus && sizeRead == sizeToRead);
    ASSERT_TRUE (buffer[0] == utf8BOM[0] && buffer[1] == utf8BOM[1] && buffer[2] == utf8BOM[2]) << "Json file is expected to be encoded in UTF-8";

    for (UInt32 ii = 3; ii < sizeRead; ii++)
        {
        if (buffer[ii] == '\n' || buffer[ii] == '\r')
            continue;
        fileContent.append (1, buffer[ii]);
        }

    file.Close();

    ASSERT_TRUE (Json::Reader::Parse(fileContent, jsonInput)) << "Error when parsing the JSON file";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                   Muhammad.Zaighum                  05/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST (PerformanceJsonInserter, Insert)
    {
    ECDbTestProject testProject;
    auto& ecdb = testProject.Create ("performancejsoninserter.ecdb", L"eB_PW_CommonSchema_WSB.01.00.ecschema.xml", false);

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
    JsonInserter inserter (ecdb, *documentClass);

    const int repetitionCount = 1000;

    //printf ("Attach to profiler and press any key to continue...\n"); getchar ();

    //-----------------------------------------------------------------------------------
    // Insert using JsonCpp
    //-----------------------------------------------------------------------------------
    StopWatch timer (true);
    for (int i = 0; i < repetitionCount; i++)
        {
        ECInstanceKey id;
        auto insertStatus = inserter.Insert (id, jsonInput);
        ASSERT_EQ (SUCCESS, insertStatus);
        }
    timer.Stop ();
    ecdb.SaveChanges();
    LOG.infov ("Inserting JsonCpp JSON objects into ECDb %d times took %.4f msecs.", repetitionCount, timer.GetElapsedSeconds () * 1000.0);
    PerformanceTestingFrameWork performanceObj;
    EXPECT_TRUE(performanceObj.writeTodb(L"PerformanceTest.ecdb", timer, "PerformanceJsonInserter,Insert", "Inserting JsonCpp JSON objects into ECDb For repetitionCount: 1000" ));
    //-----------------------------------------------------------------------------------
    // Insert using RapidJson
    //-----------------------------------------------------------------------------------
    StopWatch rapidJasintimer(true);
    for (int i = 0; i < repetitionCount; i++)
        {
        ECInstanceKey ecInstanceKey;
        auto insertStatus = inserter.Insert (ecInstanceKey, rapidJsonInput);
        ASSERT_EQ (SUCCESS, insertStatus);
        ASSERT_TRUE (ecInstanceKey.IsValid ());
        }
    rapidJasintimer.Stop();
    ecdb.SaveChanges();
    LOG.infov("Inserting RapidJson JSON objects into ECDb %d times took %.4f msecs.", repetitionCount, rapidJasintimer.GetElapsedSeconds() * 1000.0);
     
    double timeToCheck = rapidJasintimer.GetElapsedSeconds();

    
    

    if (timeToCheck == 500)
    {

    }

    PerformanceTestingFrameWork performanceObjRapidJason;
    EXPECT_TRUE(performanceObjRapidJason.writeTodb(L"PerformanceTest.ecdb", rapidJasintimer, "PerformanceJsonInserter,Insert", "Inserting RapidJson JSON objects into ECDb For repetitionCount:  1000"));
    }

END_ECDBUNITTESTS_NAMESPACE
