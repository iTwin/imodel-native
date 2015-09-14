/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Performance/PerformanceECXmlTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "PerformanceTestFixture.h"

#include <Bentley/BeTimeUtilities.h>
using namespace BentleyApi::ECN;

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct PerformanceTestsECXml : PerformanceTestFixture
    {

    void TimeSchema (WCharP schemaName, ECSchemaReadContextPtr   schemaContext)
        {
        ECSchemaPtr schema;
        StopWatch deserializationTimer ("Deserialization", false);
        deserializationTimer.Start ();
        //printf ("Attach to profiler for reading schema...\r\n"); getchar ();
        SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath (schemaName).c_str (), *schemaContext);
        //printf ("Detach from profiler...\r\n"); getchar ();

        deserializationTimer.Stop ();
        EXPECT_EQ (SCHEMA_READ_STATUS_Success, status);

        StopWatch serializationTimer ("Serialization", false);
        Utf8String ecSchemaXml;

        serializationTimer.Start ();
        //printf ("Attach to profiler for writing schema...\r\n"); getchar ();
        SchemaWriteStatus status2 = schema->WriteToXmlString (ecSchemaXml);
        //printf ("Detach from profiler...\r\n"); getchar ();
        serializationTimer.Stop ();
        EXPECT_EQ (SCHEMA_WRITE_STATUS_Success, status2);

        size_t stringLength = ecSchemaXml.length ();

        Utf8String dateTime = ECTestFixture::GetDateTime ();
        ECSchemaReferenceList references = schema->GetReferencedSchemas ();

        bmap<Utf8String, double> results;
        Utf8String deserializingString;
        deserializingString.Sprintf ("De-serializing schema: %s (%d references)", schema->GetFullSchemaName ().c_str (), references.size ());
        results[deserializingString] = deserializationTimer.GetElapsedSeconds ();

        Utf8String serializingString;
        serializingString.Sprintf ("Serializing schema: %s (%d bytes)", schema->GetFullSchemaName ().c_str (), stringLength);
        results[serializingString] = serializationTimer.GetElapsedSeconds ();

        PERFORMANCELOG.infov ("%s, De-serializing schema: %s (%d references), %.4f\n", dateTime.c_str (), schema->GetFullSchemaName ().c_str (), references.size (), deserializationTimer.GetElapsedSeconds ());
        PERFORMANCELOG.infov ("%s, Serializing schema: %s (%d bytes), %.4f\n", dateTime.c_str (), schema->GetFullSchemaName ().c_str (), stringLength, serializationTimer.GetElapsedSeconds ());

        LogResultsToFile (results);
        }

    void TimeInstance (WCharP schemaName, WCharP instanceXmlFile, ECSchemaReadContextPtr   schemaContext)
        {
        ECSchemaPtr   schema;
        //    SchemaReadStatus schemaStatus = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath(L"MismatchedSchema.01.00.ecschema.xml").c_str(), *schemaContext);
        //    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, L"c:\\temp\\data\\ECXA\\Dataset for D-84244\\Schemas\\OpenPlant_3D.01.02.ecschema.xml", *schemaContext);
        SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath (schemaName).c_str (), *schemaContext);

        EXPECT_EQ (SCHEMA_READ_STATUS_Success, status);

        ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext (*schema);

        IECInstancePtr  testInstance;
        InstanceReadStatus instanceStatus;

        StopWatch readingTimer ("Reading", false);
        readingTimer.Start ();
        instanceStatus = IECInstance::ReadFromXmlFile (testInstance, ECTestFixture::GetTestDataPath (instanceXmlFile).c_str (), *instanceContext);
        readingTimer.Stop ();
        EXPECT_EQ (INSTANCE_READ_STATUS_Success, instanceStatus);

        StopWatch writingTimer (L"Serialization", false);
        WString ecInstanceXml;
        writingTimer.Start ();
        InstanceWriteStatus status2 = testInstance->WriteToXmlString (ecInstanceXml, true, true);
        writingTimer.Stop ();
        EXPECT_EQ (INSTANCE_WRITE_STATUS_Success, status2);

        Utf8String dateTime = ECTestFixture::GetDateTime ();
        size_t stringLength = ecInstanceXml.length ();

        PERFORMANCELOG.infov ("%s, Reading instance from class: %s:%s, %.4f\n", dateTime.c_str (), schema->GetFullSchemaName ().c_str (), testInstance->GetClass ().GetName ().c_str (), readingTimer.GetElapsedSeconds ());
        PERFORMANCELOG.infov ("%s, Writing instance from class: %s:%s (%d bytes), %.4f\n", dateTime.c_str (), schema->GetFullSchemaName ().c_str (), testInstance->GetClass ().GetName ().c_str (), stringLength, writingTimer.GetElapsedSeconds ());

        bmap<Utf8String, double> results;
        Utf8String readingString;
        readingString.Sprintf ("Reading instance from class: %s:%s", schema->GetFullSchemaName ().c_str (), testInstance->GetClass ().GetName ().c_str ());
        results[readingString] = readingTimer.GetElapsedSeconds ();

        Utf8String writingString;
        writingString.Sprintf ("Writing instance from class: %s:%s (%d bytes)", schema->GetFullSchemaName ().c_str (), testInstance->GetClass ().GetName ().c_str (), stringLength);
        results[writingString] = writingTimer.GetElapsedSeconds ();

        LogResultsToFile (results);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (PerformanceTestsECXml, ReadingAndWritingSchema)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext ();

    TimeSchema (L"OpenPlant.01.02.ecschema.xml", schemaContext);
    TimeSchema (L"OpenPlant_PID.01.02.ecschema.xml", schemaContext);
    TimeSchema (L"OpenPlant_3D.01.02.ecschema.xml", schemaContext);
    TimeSchema (L"Bentley_Plant.06.00.ecschema.xml", schemaContext);
    TimeSchema (L"CustomAttributeTest.01.00.ecschema.xml", schemaContext);
    };

TEST_F (PerformanceTestsECXml, ReadingAndWritingInstance)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext ();

    TimeInstance (L"ECRules.01.00.ecschema.xml", L"RuleSet.xml", schemaContext);
    TimeInstance (L"OpenPlant_3D.01.02.ecschema.xml", L"OpenPlant_3D_Instance.xml", schemaContext);
    };
END_BENTLEY_ECN_TEST_NAMESPACE
