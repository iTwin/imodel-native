/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Performance/ECXmlPerformanceTests.cpp $
|
|  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"
#include <objbase.h>
#include <comdef.h>
#include "TestFixture.h"
#include "StopWatch.h"

BEGIN_BENTLEY_EC_NAMESPACE

struct ECXmlPerformanceTest   : ECTestFixture {};

void TimeSchema
(
WCharP schemaName,
ECSchemaReadContextPtr   schemaContext,
FILE* logFile
)
    {
    ECSchemaP schema;
    StopWatch deserializationTimer(L"Deserialization", false);
    deserializationTimer.Start();
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath( schemaName).c_str(), *schemaContext);

    deserializationTimer.Stop();
    EXPECT_EQ (SCHEMA_READ_STATUS_Success, status);  
    LPSTREAM stream = NULL;
    //HRESULT res = ::CreateStreamOnHGlobal(NULL,TRUE,&stream);
    ::CreateStreamOnHGlobal(NULL,TRUE,&stream);

    StopWatch serializationTimer(L"Serialization", false);
    serializationTimer.Start();
    SchemaWriteStatus status2 = schema->WriteToXmlStream(stream);
    serializationTimer.Stop();
    EXPECT_EQ (SCHEMA_READ_STATUS_Success, status2);  

    STATSTG statInfo;
    stream->Stat(&statInfo, STATFLAG_NONAME);
    unsigned long filelength = statInfo.cbSize.LowPart;

    WString dateTime = ECTestFixture::GetDateTime ();
    ECSchemaReferenceList references = schema->GetReferencedSchemas();
    fwprintf (logFile, L"%s, De-serializing schema: %s (%d references), %.4f\n", dateTime.c_str(), schema->GetFullSchemaName(), references.size(), deserializationTimer.GetElapsedSeconds());
    fwprintf (logFile, L"%s, Serializing schema: %s (%d bytes), %.4f\n",dateTime.c_str(), schema->GetFullSchemaName(), filelength, serializationTimer.GetElapsedSeconds());

    }

void TimeInstance
(
WCharP schemaName,
WCharP instanceXmlFile,
ECSchemaReadContextPtr   schemaContext,
FILE* logFile
)
    {
    ECSchemaP   schema;
    //    SchemaReadStatus schemaStatus = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath(L"MismatchedSchema.01.00.ecschema.xml").c_str(), *schemaContext);
    //    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, L"c:\\temp\\data\\ECXA\\Dataset for D-84244\\Schemas\\OpenPlant_3D.01.02.ecschema.xml", *schemaContext);
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath(schemaName).c_str(), *schemaContext);

    EXPECT_EQ (SCHEMA_READ_STATUS_Success, status);

    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext (*schema);

    IECInstancePtr  testInstance;
    InstanceReadStatus instanceStatus;

    StopWatch readingTimer(L"Reading", false);
    readingTimer.Start();
    instanceStatus = IECInstance::ReadFromXmlFile (testInstance, ECTestFixture::GetTestDataPath(instanceXmlFile).c_str(), *instanceContext);
    readingTimer.Stop();
    EXPECT_EQ (INSTANCE_READ_STATUS_Success, instanceStatus);

    LPSTREAM stream = NULL;
    //HRESULT res = ::CreateStreamOnHGlobal(NULL,TRUE,&stream);
    ::CreateStreamOnHGlobal(NULL,TRUE,&stream);

    StopWatch writingTimer(L"Serialization", false);
    writingTimer.Start();
    InstanceWriteStatus status2 = testInstance->WriteToXmlStream(stream, true, true, false);
    writingTimer.Stop();
    EXPECT_EQ (INSTANCE_WRITE_STATUS_Success, status2);  

    WString dateTime = ECTestFixture::GetDateTime ();
    STATSTG statInfo;
    stream->Stat(&statInfo, STATFLAG_NONAME);
    unsigned long filelength = statInfo.cbSize.LowPart;

    fwprintf (logFile, L"%s, Reading instance from class: %s:%s, %.4f\n", dateTime.c_str(), schema->GetFullSchemaName(), testInstance->GetClass().GetName(), readingTimer.GetElapsedSeconds());
    fwprintf (logFile, L"%s, Writing instance from class: %s:%s (%d bytes), %.4f\n",dateTime.c_str(), schema->GetFullSchemaName(), testInstance->GetClass().GetName(), filelength, writingTimer.GetElapsedSeconds());

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECXmlPerformanceTest, ReadingAndWritingSchema)
    {
    EXPECT_EQ (S_OK, CoInitialize(NULL)); 

    ECSchemaCachePtr                    schemaOwner = ECSchemaCache::Create();
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext(*schemaOwner);

    FILE* logFile=NULL;

    WString logfilePath = GetTestResultsFilePath (L"ECObjectsPerformanceResults.csv");
    MakeDirContainingFile (logfilePath.c_str());

    bool existingFile = (0 == _waccess_s(logfilePath.c_str(), 0 ));

    _wfopen_s(&logFile, logfilePath.c_str(), L"a+"); 
    wprintf (L"CSV Results filename: %s\n", logfilePath.c_str());

    if (!existingFile)
        fwprintf (logFile, L"Date, Test Description, Time (secs)\n");
    TimeSchema(L"OpenPlant.01.02.ecschema.xml", schemaContext, logFile);
    TimeSchema(L"OpenPlant_PID.01.02.ecschema.xml", schemaContext, logFile);
    TimeSchema(L"OpenPlant_3D.01.02.ecschema.xml", schemaContext, logFile);
    TimeSchema(L"Bentley_Plant.06.00.ecschema.xml", schemaContext, logFile);

    fclose(logFile);
    CoUninitialize();
    };

TEST_F(ECXmlPerformanceTest, ReadingAndWritingInstance)
    {
    //This test will verify that the xml deserializer will not fail on mismatched or malformed properties.
    // must call CoInitialize - schema deserialization requires it.
    EXPECT_EQ (S_OK, CoInitialize(NULL)); 

    ECSchemaCachePtr                    schemaOwner = ECSchemaCache::Create();
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext(*schemaOwner);

    FILE* logFile=NULL;

    WString logfilePath = GetTestResultsFilePath (L"ECObjectsPerformanceResults.csv");
    MakeDirContainingFile (logfilePath.c_str());

    bool existingFile = (0 == _waccess_s(logfilePath.c_str(), 0 ));

    _wfopen_s(&logFile, logfilePath.c_str(), L"a+"); 
    wprintf (L"CSV Results filename: %s\n", logfilePath.c_str());

    if (!existingFile)
        fwprintf (logFile, L"Date, Test Description, Time (secs)\n");
    TimeInstance(L"ECRules.01.00.ecschema.xml", L"RuleSet.xml", schemaContext, logFile);
    TimeInstance(L"OpenPlant_3D.01.02.ecschema.xml", L"OpenPlant_3D_Instance.xml", schemaContext, logFile);
    fclose(logFile);

    CoUninitialize();
    };
END_BENTLEY_EC_NAMESPACE
