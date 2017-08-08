/*--------------------------------------------------------------------------------------+
|
|     $Source: test/TestFixture/TestFixture.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
| Based on http://cplus.about.com/od/howtodothingsi2/a/timing.htm
|
+--------------------------------------------------------------------------------------*/

#include "../ECObjectsTestPCH.h"
#include <Bentley/BeTimeUtilities.h>
#include "TestFixture.h"

USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE
  
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECTestFixture::ECTestFixture()
    {
    BeFileName assetsDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory (assetsDir);
    ECN::ECSchemaReadContext::Initialize (assetsDir);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Carole.MacDonald 02/10
+---------------+---------------+---------------+---------------+---------------+------*/
WString ECTestFixture::GetTestDataPath(WCharCP dataFile)
    {
    BeFileName testData;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory (testData);
    testData.AppendToPath (L"SeedData");
    testData.AppendToPath (dataFile);
    return testData;
    } 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
WString ECTestFixture::GetTempDataPath(WCharCP dataFile)
    {
    BeFileName testData;
    BeTest::GetHost().GetOutputRoot (testData);
    testData.AppendToPath (dataFile);
    return testData;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ECTestFixture::GetDateTime ()
    {
    struct tm timeinfo;
    BeTimeUtilities::ConvertUnixMillisToTm (timeinfo, BeTimeUtilities::GetCurrentTimeAsUnixMillis());   // GMT

    char buff[32];
    strftime (buff, sizeof(buff), "%y/%m/%d", &timeinfo);
    Utf8String dateTime (buff);
    dateTime.append (" ");
    strftime(buff, sizeof(buff), "%H:%M:%S", &timeinfo);
    dateTime.append (buff);
    return dateTime.c_str();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    08/2017
//---------------+---------------+---------------+---------------+---------------+-------
// static
void ECTestUtility::ExpectSchemaDeserializationFailure(Utf8CP schemaXml, SchemaReadStatus expectedError)
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    EXPECT_EQ(expectedError, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    08/2017
//---------------+---------------+---------------+---------------+---------------+-------
// static
void ECTestUtility::AssertSchemaDeserializationFailure(Utf8CP schemaXml, SchemaReadStatus expectedError)
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(expectedError, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }
    
END_BENTLEY_ECN_TEST_NAMESPACE

