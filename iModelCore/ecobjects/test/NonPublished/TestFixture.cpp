/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublished/TestFixture.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
| Based on http://cplus.about.com/od/howtodothingsi2/a/timing.htm
|
+--------------------------------------------------------------------------------------*/

#include "../ECObjectsTestPCH.h"

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
* @bsimethod                                    Carole.MacDonald                08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
WString ECTestFixture::GetWorkingDirectoryPath(WCharCP testFixture, WCharCP dataFile)
    {
    BeFileName filePath;
    BeTest::GetHost().GetOutputRoot (filePath);

    WCharP processorArchitecture = (8 == sizeof(void*)) ? L"Winx64" : L"Winx86";
    filePath.AppendToPath(processorArchitecture);
    filePath.AppendToPath(L"build");
    filePath.AppendToPath(L"ECObjectsTests");
    filePath.AppendToPath(L"AtpWorkingRoot");
    filePath.AppendToPath(testFixture);

    BeFileName::CreateNewDirectory (filePath.c_str());

    filePath.AppendToPath (dataFile);
    return filePath;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WString ECTestFixture::GetTestResultsFilePath (WCharCP fileName)
    {
#ifdef WIP_ECOBJECTS_TEST_NEEDS_WORK
    WString path = WString(getenv ("TESTRESULTS_DIR"), false);

    // If they don't, we will put them in the tmp dir.
    if (0 == path.size())
        {
        path = WString (getenv ("OutRoot"), false);
        if (path.size() > 0)
            {
            if (*path.rbegin() != '\\')
                path.append (L"\\");

            WString processorArch = WString (getenv ("DEFAULT_TARGET_PROCESSOR_ARCHITECTURE"), false);
            if (0 == processorArch.size())
                path += L"Winx64";
            else
                path = path + L"Win" + processorArch + L"\\TestResults\\";
            }
        }
    else
        {
        if (*path.rbegin() != '\\')
            path.append (L"\\");
        }

    if (0 == path.size())
        {
        path.AssignA (getenv ("tmp"));
        if (path.size() > 0 && *path.rbegin() != '\\')
            path.append (L"\\");

        path.append (L"TestResults\\");
        }

    path = ReplaceSlashes (path);

    if (fileName)
        path.append (fileName);

    return path;
#else
    BeFileName filePath;
    BeTest::GetHost().GetOutputRoot (filePath);

    WCharP processorArchitecture = (8 == sizeof(void*)) ? L"Winx64" : L"Winx86";
    filePath.AppendToPath(processorArchitecture);
    filePath.AppendToPath(L"TestResults");
    BeFileName::CreateNewDirectory (filePath.c_str());
    
    filePath.AppendToPath(fileName);
    
    return filePath;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
BentleyStatus ECTestFixture::CheckProcessDirectory
(
WCharP filepath, 
ULong32 bufferSize
)
    {
    WString dllPath = GetDllPath();
    if (0 == dllPath.length())
        return ERROR;
        
    wchar_t executingDirectory[_MAX_DIR];
    wchar_t executingDrive[_MAX_DRIVE];
    _wsplitpath(dllPath.c_str(), executingDrive, executingDirectory, NULL, NULL);

    // Look for a file called "logging.config.xml" in the executing process's directory
    _wmakepath(filepath, executingDrive, executingDirectory, L"logging.config.xml", L"xml");
    if (0 == _waccess(filepath, 0))
        return SUCCESS;
    return ERROR;
    }
***WIP_ECOBJECTS_TEST
+---------------+---------------+---------------+---------------+---------------+------*/

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
    wchar_t date[9];
    wchar_t time[9];
    
    _wstrdate(date);
    _wstrtime(time);
    WString dateTime (date);
    dateTime.append (L" ");
    dateTime.append (time);
    return dateTime;
    }


END_BENTLEY_ECOBJECT_NAMESPACE

