/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/PerformanceJsonComparison.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <rapidjson/BeRapidJson.h>
#include "PerformanceTests.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsitest                                    Shaun.Sewall                     12/13
//---------------------------------------------------------------------------------------
TEST(PerformanceJsonComparison, ParseJsonUsingStartupCompany)
    {
    //-----------------------------------------------------------------------------------
    //  Construct a test JSON object
    //-----------------------------------------------------------------------------------
    BeFileName inputFile;
    BeTest::GetHost().GetDocumentsRoot(inputFile);
    inputFile.AppendToPath(L"StartupCompany.json");
    Json::Value seedObj(Json::objectValue);
    ECDbTestUtility::ReadJsonInputFromFile(seedObj, inputFile);

    Json::Value rowObj(Json::objectValue);
    Json::Value rowsArray(Json::arrayValue);
    Json::ArrayIndex numRows = 1000;

    for (Json::ArrayIndex i = 0; i < numRows; i++)
        {
        rowObj["index"] = i;
        rowObj["obj"] = seedObj;
        rowsArray[i] = rowObj;
        }

    Json::Value largeObj(Json::objectValue);
    largeObj["rows"] = rowsArray;

    Utf8String largeString = Json::FastWriter().write(largeObj);

    //-----------------------------------------------------------------------------------
    //  Parse using JsonCpp
    //-----------------------------------------------------------------------------------
    StopWatch timer(true);
    Json::Value testObj(Json::objectValue);

    bool parseSuccessful = Json::Reader().parse(largeString, testObj);
    ASSERT_TRUE(parseSuccessful);

    for (Json::ArrayIndex i = 0; i < numRows; i++)
        {
        int intValue = testObj["rows"][i]["index"].asInt();
        ASSERT_EQ(i, intValue);
        }

    timer.Stop();
    printf("Parsing large JSON object (%d rows) with JsonCpp took %.4f seconds\n", numRows, timer.GetElapsedSeconds());
    Utf8String testDetailsParseCpp = "PerformanceJsonComparison,ParseJsonCppUsingStartupCompany";
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), "Parsing large JSON object  having 1000 rows with JsonCpp  using StartupCompany.json", 1000);
    //-----------------------------------------------------------------------------------
    //  Parse using RapidJson
    //-----------------------------------------------------------------------------------
    timer.Start();
    rapidjson::Document document;
    parseSuccessful = !document.Parse<0>(largeString.c_str()).HasParseError();
    ASSERT_TRUE(parseSuccessful);

    for (rapidjson::SizeType i = 0; i < numRows; i++)
        {
        int intValue = document["rows"][i]["index"].GetInt();
        ASSERT_EQ(i, intValue);
        }

    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), "Parsing large JSON object  having 1000 rows with RapidJson  using StartupCompany.json", 1000);
    }

//---------------------------------------------------------------------------------------
// @bsitest                                    Shaun.Sewall                     01/14
//---------------------------------------------------------------------------------------
TEST(PerformanceJsonComparison, AddJson)
    {
    const Json::ArrayIndex numEntries = 1000000;
    //-----------------------------------------------------------------------------------
    //  Add using JsonCpp
    //-----------------------------------------------------------------------------------
    StopWatch timer(true);

    {
    Json::Value testObj(Json::objectValue);
    Json::Value entries(Json::arrayValue);
    testObj["entries"] = entries;

    for (Json::ArrayIndex i = 0; i < numEntries; i++)
        {
        Json::Value obj(Json::objectValue);
        obj["i1"] = 1;
        obj["i2"] = 2;
        obj["i3"] = 3;
        obj["d1"] = 0.1;
        obj["d2"] = 0.2;
        obj["d3"] = 0.3;
        obj["s1"] = "string1";
        obj["s2"] = "string2";
        obj["s3"] = "string3";

        testObj["entries"][i] = obj;
        }

    for (Json::ArrayIndex i = 0; i < numEntries; i++)
        {
        ASSERT_EQ(1, testObj["entries"][i]["i1"].asInt());
        ASSERT_EQ(2, testObj["entries"][i]["i2"].asInt());
        ASSERT_EQ(3, testObj["entries"][i]["i3"].asInt());

        ASSERT_EQ(0.1, testObj["entries"][i]["d1"].asDouble());
        ASSERT_EQ(0.2, testObj["entries"][i]["d2"].asDouble());
        ASSERT_EQ(0.3, testObj["entries"][i]["d3"].asDouble());

        ASSERT_TRUE(0 == strcmp("string1", testObj["entries"][i]["s1"].asCString()));
        ASSERT_TRUE(0 == strcmp("string2", testObj["entries"][i]["s2"].asCString()));
        ASSERT_TRUE(0 == strcmp("string3", testObj["entries"][i]["s3"].asCString()));
        }
    }

    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), "Adding and Verifying entries with JsonCpp", (int) numEntries);
    //-----------------------------------------------------------------------------------
    //  Add using RapidJson
    //-----------------------------------------------------------------------------------
    timer.Start();
    {
    rapidjson::Document document;
    document.SetObject();

    rapidjson::Value entries(rapidjson::kArrayType);
    document.AddMember("entries", entries, document.GetAllocator());

    for (rapidjson::SizeType i = 0; i < numEntries; i++)
        {
        rapidjson::Value obj(rapidjson::kObjectType);
        rapidjson::Value v;

        v.SetInt(1);
        obj.AddMember("i1", v, document.GetAllocator());

        v.SetInt(2);
        obj.AddMember("i2", v, document.GetAllocator());

        v.SetInt(3);
        obj.AddMember("i3", v, document.GetAllocator());

        v.SetDouble(0.1);
        obj.AddMember("d1", v, document.GetAllocator());

        v.SetDouble(0.2);
        obj.AddMember("d2", v, document.GetAllocator());

        v.SetDouble(0.3);
        obj.AddMember("d3", v, document.GetAllocator());

        v.SetString("string1");
        obj.AddMember("s1", v, document.GetAllocator());

        v.SetString("string2");
        obj.AddMember("s2", v, document.GetAllocator());

        v.SetString("string3");
        obj.AddMember("s3", v, document.GetAllocator());

        document["entries"].PushBack(obj, document.GetAllocator());
        }

    for (rapidjson::SizeType i = 0; i < numEntries; i++)
        {
        ASSERT_EQ(1, document["entries"][i]["i1"].GetInt());
        ASSERT_EQ(2, document["entries"][i]["i2"].GetInt());
        ASSERT_EQ(3, document["entries"][i]["i3"].GetInt());

        ASSERT_EQ(0.1, document["entries"][i]["d1"].GetDouble());
        ASSERT_EQ(0.2, document["entries"][i]["d2"].GetDouble());
        ASSERT_EQ(0.3, document["entries"][i]["d3"].GetDouble());

        ASSERT_TRUE(0 == strcmp("string1", document["entries"][i]["s1"].GetString()));
        ASSERT_TRUE(0 == strcmp("string2", document["entries"][i]["s2"].GetString()));
        ASSERT_TRUE(0 == strcmp("string3", document["entries"][i]["s3"].GetString()));
        }
    }

    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), "Adding and Verifying entries with RapidJson", (int) numEntries);
    }

END_ECDBUNITTESTS_NAMESPACE
