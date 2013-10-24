/*--------------------------------------------------------------------------------------+
|
|     $Source: test/TestFixture/TestFixture.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
| Based on http://cplus.about.com/od/howtodothingsi2/a/timing.htm
|
+--------------------------------------------------------------------------------------*/

#include "../ECObjectsTestPCH.h"
#include <Bentley/BeTimeUtilities.h>

USING_NAMESPACE_BENTLEY_LOGGING

BEGIN_BENTLEY_ECOBJECT_NAMESPACE
  
bool ECTestFixture::s_isLoggerInitialized = false;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECTestFixture::ECTestFixture()
    {
    //LoggingConfig::ActivateProvider(CONSOLE_LOGGING_PROVIDER);

    #if defined (WIP_ECOBJECTS_TEST)
    if (!s_isLoggerInitialized)
        {
        LoggingConfig::ActivateProvider(LOG4CXX_LOGGING_PROVIDER);
        LoggingConfig::SetOption(CONFIG_OPTION_CONFIG_FILE, GetLogConfigurationFilename().c_str());

        LoggingConfig::SetSeverity(L"ECObjectsNative", LOG_WARNING);
        s_isLoggerInitialized = true;
        }
    #endif

    BeFileName assetsDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory (assetsDir);
    ECN::ECSchemaReadContext::Initialize (assetsDir);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Josh.Schifter   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECTestFixture::SetUp ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Josh.Schifter   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECTestFixture::TearDown ()
    {
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
WString ECTestFixture::GetTestResultsFilePath (WCharCP fileName)
    {
    BeFileName filePath;
    BeTest::GetHost().GetOutputRoot (filePath);

    WCharCP processorArchitecture = (8 == sizeof(void*)) ? L"Winx64" : L"Winx86";
    filePath.AppendToPath(processorArchitecture);
    filePath.AppendToPath(L"TestResults");
    BeFileName::CreateNewDirectory (filePath.c_str());
    
    filePath.AppendToPath(fileName);
    
    return filePath;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Casey.Mullen                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
WString ECTestFixture::GetLogConfigurationFilename()
    {
    /* ***WIP_ECOBJECTS_TEST
    wchar_t filepath[MAX_PATH];

    if ((0 != GetEnvironmentVariableW(L"BENTLEY_LOGGING_CONFIG", filepath, MAX_PATH)) && (0 ==_waccess(filepath, 0)))
        {
        wprintf (L"ECObjects.dll configuring logging with %s (Set by BENTLEY_LOGGING_CONFIG environment variable.)\n", filepath);
        return filepath;
        }
    else if (SUCCESS == CheckProcessDirectory(filepath, sizeof(filepath)))
        {
        wprintf (L"ECObjects.dll configuring logging using %s. Override by setting BENTLEY_LOGGING_CONFIG in environment.\n", filepath);
        return filepath;
        }
    else if (0 != GetEnvironmentVariableW(L"OutRoot", filepath, MAX_PATH))
        {
        WCharP processorArchitecture = (8 == sizeof(void*)) ? L"Winx64" : L"Winx86";
        wcscat (filepath, processorArchitecture);
        wcscat (filepath, L"\\Product\\ECObjectsTests\\logging.config.xml");
        
        if (0 ==_waccess(filepath, 0))
            {
            wprintf (L"ECObjects.dll configuring logging with %s. Override by setting BENTLEY_LOGGING_CONFIG in environment.\n", filepath);
            return filepath;
            }
        }
    */

    return L"";
    }       

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WString  ECTestFixture::GetDateTime ()
    {
    struct tm timeinfo;
    BeTimeUtilities::ConvertUnixMillisToTm (timeinfo, BeTimeUtilities::GetCurrentTimeAsUnixMillis());   // GMT

    char buff[32];
    strftime (buff, sizeof(buff), "%y/%m/%d", &timeinfo);
    Utf8String dateTime (buff);
    dateTime.append (" ");
    strftime(buff, sizeof(buff), "%H:%M:%S", &timeinfo);
    dateTime.append (buff);
    return WString(dateTime.c_str(), true);
    }
    
END_BENTLEY_ECOBJECT_NAMESPACE

