/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceTests.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

struct PerformanceJsonInserter : ECDbTestFixture
    {};

//---------------------------------------------------------------------------------------
// @bsiclass                                   Muhammad.Zaighum                  05/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceJsonInserter, InsertJsonCppUsingPresistanceAPI)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("performancejsoninserter.ecdb", SchemaItem::CreateForFile("JsonTests.01.00.00.ecschema.xml")));

    // Read JSON input from file
    BeFileName jsonInputFile;
    BeTest::GetHost().GetDocumentsRoot(jsonInputFile);
    jsonInputFile.AppendToPath(L"ECDb");
    jsonInputFile.AppendToPath(L"JsonTestClass.json");

    // Parse JSON value using JsonCpp
    Json::Value jsonInput;
    TestUtilities::ReadFile(jsonInput, jsonInputFile);
    ECClassCP documentClass = m_ecdb.Schemas().GetClass("JsonTests", "Document");
    ASSERT_TRUE(documentClass != nullptr);
    JsonInserter inserter(m_ecdb, *documentClass, nullptr);
    const int repetitionCount = 1000;

    //----------------------------------------------------------------------------------- 
    // Insert using JsonCpp
    //-----------------------------------------------------------------------------------
    StopWatch timer(true);
    for (int i = 0; i < repetitionCount; i++)
        {
        ECInstanceKey key;
        ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(key, jsonInput));
        ASSERT_TRUE(key.IsValid());
        }
    timer.Stop();
    m_ecdb.SaveChanges();

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT COUNT(*) FROM jt.Document"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(repetitionCount, statement.GetValueInt(0)) << "Expected Number of Instances not inserted in Db";

    LOG.infov("Inserting JsonCpp JSON objects into ECDb %d times took %.4f msecs.", repetitionCount, timer.GetElapsedSeconds() * 1000.0);
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), repetitionCount, "Inserting JsonCpp JSON objects into ECDb");
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                   Muhammad.Zaighum                  05/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceJsonInserter, InsertRapidJsonUsingPresistanceAPI)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("performancejsoninserter.ecdb", SchemaItem::CreateForFile("JsonTests.01.00.00.ecschema.xml")));

    // Read JSON input from file
    BeFileName jsonInputFile;
    BeTest::GetHost().GetDocumentsRoot(jsonInputFile);
    jsonInputFile.AppendToPath(L"ECDb");
    jsonInputFile.AppendToPath(L"JsonTestClass.json");

    // Parse JSON value using JsonCpp
    Json::Value jsonInput;
    TestUtilities::ReadFile(jsonInput, jsonInputFile);

    // Parse JSON value using RapidJson
    rapidjson::Document rapidJsonInput;
    ASSERT_EQ(SUCCESS, TestUtilities::ParseJson(rapidJsonInput, Json::FastWriter().write(jsonInput)));

    ECClassCP documentClass = m_ecdb.Schemas().GetClass("JsonTests", "Document");
    ASSERT_TRUE(documentClass != nullptr);
    JsonInserter inserter(m_ecdb, *documentClass, nullptr);

    const int repetitionCount = 1000;
    //-----------------------------------------------------------------------------------
    // Insert using RapidJson
    //-----------------------------------------------------------------------------------
    StopWatch timer(true);
    for (int i = 0; i < repetitionCount; i++)
        {
        ECInstanceKey ecInstanceKey;
        ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(ecInstanceKey, rapidJsonInput));
        ASSERT_TRUE(ecInstanceKey.IsValid());
        }
    timer.Stop();
    m_ecdb.SaveChanges();

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT COUNT(*) FROM jt.Document"));
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(repetitionCount, statement.GetValueInt(0)) << "Expected Number of Instances not inserted in Db";

    LOG.infov("Inserting RapidJson JSON objects into ECDb %d times took %.4f msecs.", repetitionCount, timer.GetElapsedSeconds() * 1000.0);
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), repetitionCount, "Inserting RapidJson JSON objects into ECDb");
    }

END_ECDBUNITTESTS_NAMESPACE
