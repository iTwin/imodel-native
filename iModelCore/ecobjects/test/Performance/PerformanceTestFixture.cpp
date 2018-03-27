/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Performance/PerformanceTestFixture.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "PerformanceTestFixture.h"

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
// static
void PerformanceTestFixture::LogResultsToFile(bmap<Utf8String, double> results)
    {
    FILE* logFile= nullptr;

    BeFileName dir;
    BeTest::GetHost().GetOutputRoot(dir);
    WCharCP processorArchitecture = (8 == sizeof(void*)) ? L"Winx64" : L"Winx86";
    dir.AppendToPath(processorArchitecture);
    dir.AppendToPath(L"TestResults");
    if (!dir.DoesPathExist())
        BeFileName::CreateNewDirectory (dir.c_str());

    dir.AppendToPath (L"ECObjectsPerformanceResults.csv");

    bool existingFile = dir.DoesPathExist();

    logFile = fopen(dir.GetNameUtf8().c_str(), "a+"); 
    PERFORMANCELOG.infov(L"CSV Results filename: %ls\n", dir.GetName());

    if (!existingFile)
        fprintf(logFile, "Date, Test Description, Baseline, Time (secs)\n");

    Utf8String dateTime = DateTime::GetCurrentTime().ToString();

    for (auto const& pair : results)
        fprintf(logFile, "%s, %s,, %.4f\n", dateTime.c_str(), pair.first.c_str(), pair.second);

    fclose(logFile);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
// static
void PerformanceTestFixture::TimeSchema(WCharCP schemaName, ECSchemaReadContextPtr schemaContext, Utf8String testcaseName, Utf8String testName)
    {
    ECSchemaPtr schema;
    StopWatch deserializationTimer("Deserialization", false);
    deserializationTimer.Start();
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(schemaName).c_str(), *schemaContext);

    deserializationTimer.Stop();
    EXPECT_EQ(SchemaReadStatus::Success, status);

    StopWatch serializationTimer("Serialization", false);
    Utf8String ecSchemaXml;

    serializationTimer.Start();
    SchemaWriteStatus status2 = schema->WriteToXmlString(ecSchemaXml, ECVersion::V3_0);
    serializationTimer.Stop();

    EXPECT_EQ(SchemaWriteStatus::Success, status2);

    size_t stringLength = ecSchemaXml.length();

    Utf8String dateTime = ECTestFixture::GetDateTime();
    ECSchemaReferenceList references = schema->GetReferencedSchemas();

    bmap<Utf8String, double> results;
    Utf8String deserializingString;
    deserializingString.Sprintf("De-serializing schema: %s (%" PRIx64 " references)", schema->GetFullSchemaName().c_str(), (uint64_t)references.size());
    results[deserializingString] = deserializationTimer.GetElapsedSeconds();

    Utf8String serializingString;
    serializingString.Sprintf("Serializing schema: %s (%" PRIx64 " bytes)", schema->GetFullSchemaName().c_str(), (uint64_t)stringLength);
    results[serializingString] = serializationTimer.GetElapsedSeconds();

    PERFORMANCELOG.infov("%s, De-serializing schema: %s (%" PRIx64 " references), %.4f\n", dateTime.c_str(), schema->GetFullSchemaName().c_str(), (uint64_t)references.size(), deserializationTimer.GetElapsedSeconds());
    PERFORMANCELOG.infov("%s, Serializing schema: %s (%" PRIx64 " bytes), %.4f\n", dateTime.c_str(), schema->GetFullSchemaName().c_str(), (uint64_t)stringLength, serializationTimer.GetElapsedSeconds());

    LOGTODB(testcaseName.c_str(), testName.c_str(), deserializationTimer.GetElapsedSeconds(), -1, Utf8PrintfString("De-serializing sheme: %s", schema->GetFullSchemaName().c_str()).c_str());
    LOGTODB(testcaseName.c_str(), testName.c_str(), serializationTimer.GetElapsedSeconds(), -1, Utf8PrintfString("Serializing sheme: %s", schema->GetFullSchemaName().c_str()).c_str());

    LogResultsToFile(results);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
// static
void PerformanceTestFixture::TimeInstance(WCharCP schemaName, WCharCP instanceXmlFile, ECSchemaReadContextPtr schemaContext, Utf8String testcaseName, Utf8String testName)
    {
    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(schemaName).c_str(), *schemaContext);

    EXPECT_EQ(SchemaReadStatus::Success, status);

    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext(*schema);

    IECInstancePtr  testInstance;
    InstanceReadStatus instanceStatus;

    StopWatch readingTimer("Reading", false);
    readingTimer.Start();
    instanceStatus = IECInstance::ReadFromXmlFile(testInstance, ECTestFixture::GetTestDataPath(instanceXmlFile).c_str(), *instanceContext);
    readingTimer.Stop();
    EXPECT_EQ(InstanceReadStatus::Success, instanceStatus);

    StopWatch writingTimer("Serialization", false);
    WString ecInstanceXml;
    writingTimer.Start();
    InstanceWriteStatus status2 = testInstance->WriteToXmlString(ecInstanceXml, true, true);
    writingTimer.Stop();
    EXPECT_EQ(InstanceWriteStatus::Success, status2);

    Utf8String dateTime = ECTestFixture::GetDateTime();
    size_t stringLength = ecInstanceXml.length();

    PERFORMANCELOG.infov("%s, Reading instance from class: %s:%s, %.4f\n", dateTime.c_str(), schema->GetFullSchemaName().c_str(), testInstance->GetClass().GetName().c_str(), readingTimer.GetElapsedSeconds());
    PERFORMANCELOG.infov("%s, Writing instance from class: %s:%s (%" PRIx64 " bytes), %.4f\n", dateTime.c_str(), schema->GetFullSchemaName().c_str(), testInstance->GetClass().GetName().c_str(), (uint64_t)stringLength, writingTimer.GetElapsedSeconds());

    LOGTODB(testcaseName.c_str(), testName.c_str(), readingTimer.GetElapsedSeconds(), -1, Utf8PrintfString("Reading instance from class: %s:%s", schema->GetFullSchemaName().c_str(), testInstance->GetClass().GetName().c_str()).c_str());
    LOGTODB(testcaseName.c_str(), testName.c_str(), writingTimer.GetElapsedSeconds(), -1, Utf8PrintfString("Writing instance from class: %s:%s", schema->GetFullSchemaName().c_str(), testInstance->GetClass().GetName().c_str()).c_str());

    bmap<Utf8String, double> results;
    Utf8String readingString;
    readingString.Sprintf("Reading instance from class: %s:%s", schema->GetFullSchemaName().c_str(), testInstance->GetClass().GetName().c_str());
    results[readingString] = readingTimer.GetElapsedSeconds();

    Utf8String writingString;
    writingString.Sprintf("Writing instance from class: %s:%s (%" PRIx64 " bytes)", schema->GetFullSchemaName().c_str(), testInstance->GetClass().GetName().c_str(), (uint64_t)stringLength);
    results[writingString] = writingTimer.GetElapsedSeconds();

    LogResultsToFile(results);
    }

END_BENTLEY_ECN_TEST_NAMESPACE
