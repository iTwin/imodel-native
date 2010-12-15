/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/nativeatp/Published/TestFixture.cpp $
|
|  $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
| Based on http://cplus.about.com/od/howtodothingsi2/a/timing.htm
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsTestPCH.h"
#include "TestFixture.h"

#if defined (COMPILING_PUBLISHED_TESTS) || defined (COMPILING_SCENARIO_TESTS)
   // Need to reach in and grab this header since it won't be part of the published API yet we still
   // need to utilize it in the published API tests
   #include <..\PublicApi\Logging\bentleylogging.h>
#endif

USING_NAMESPACE_BENTLEY_LOGGING

BEGIN_BENTLEY_EC_NAMESPACE
  
#define MAX_INTERNAL_INSTANCES  0
#define MAX_INTERNAL_SCHEMAS    0

#define DEBUG_ECSCHEMA_LEAKS
#define DEBUG_IECINSTANCE_LEAKS

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void*    getDLLInstance ()
    {
    MEMORY_BASIC_INFORMATION    mbi;
    if (VirtualQuery ((void*)&getDLLInstance, &mbi, sizeof mbi))
        return mbi.AllocationBase;

    return 0;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      03/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool DirExists (wchar_t const* dir)
    {
    DWORD attributes = GetFileAttributesW(dir);
    return attributes != INVALID_FILE_ATTRIBUTES;
    }
 
 /*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      07/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool CreateDirectoryRecursive (wchar_t const * path, bool failIfExists)
    {
    if (DirExists (path))
        return true;
    std::wstring tempPath = path;
    std::wstring incPath = L"";
    size_t i, sz = tempPath.size();
    for (i = 0; i < sz; ++i)
        {
        if (tempPath[i] == L'/')
            tempPath[i] = L'\\';
        }
    // Some shells like TCC won't allow you to mkdir with a full path so do it piecewise.
    while (tempPath.size() > 0)
        {
        std::size_t pos = tempPath.find (L"\\");
        std::wstring part = tempPath.substr (0, pos);
        tempPath = tempPath.substr (pos+1, tempPath.size());
        incPath += part;
        incPath += L"\\";
        if (!DirExists (incPath.c_str()))
            {
            if (!CreateDirectoryW (incPath.c_str(), NULL))
                return false;
            }
        else
            if (failIfExists)
                return false;
        }
    return (DirExists (path));
    }

 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECTestFixture::ECTestFixture()
    {
    LoggingConfig::ActivateProvider(CONSOLE_LOGGING_PROVIDER);

    // Eventually this will switch to the Log4cxx Provider
    // LoggingConfig::ActivateProvider(LOG4CXX_LOGGING_PROVIDER);
     //LoggingConfig::SetOption(CONFIG_OPTION_CONFIG_FILE, GetLogConfigurationFilename().c_str());

    //LoggingConfig::SetSeverity(L"ECObjectsNative", LOG_TRACE);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Josh.Schifter   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECTestFixture::SetUp ()
    {
    IECInstance::Debug_ResetAllocationStats();
    ECSchema::Debug_ResetAllocationStats();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Josh.Schifter   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECTestFixture::TearDown ()
    {
#if defined (DEBUG_ECSCHEMA_LEAKS)
    ECSchema::Debug_DumpAllocationStats(L"PostTest");
#endif

#if defined (DEBUG_IECINSTANCE_LEAKS)
    IECInstance::Debug_DumpAllocationStats(L"PostTest");
#endif

    if (_WantSchemaLeakDetection())
        TestForECSchemaLeaks(); 

    if (_WantInstanceLeakDetection())
        TestForIECInstanceLeaks(); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/10
+---------------+---------------+---------------+---------------+---------------+------*/
void    ECTestFixture::TestForECSchemaLeaks ()
    {
    int numLiveSchemas = 0;

    ECSchema::Debug_GetAllocationStats (&numLiveSchemas, NULL, NULL);
    
    if (numLiveSchemas > MAX_INTERNAL_SCHEMAS)
        {
        char message[1024];
        sprintf (message, "TestForECSchemaLeaks found that there are %d Schemas still alive. Anything more than %d is flagged as an error!\n", 
            numLiveSchemas, MAX_INTERNAL_SCHEMAS);

        ECSchema::Debug_ReportLeaks();

        EXPECT_TRUE (numLiveSchemas <= MAX_INTERNAL_SCHEMAS) << message;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
void    ECTestFixture::TestForIECInstanceLeaks ()
    {
    int numLiveInstances = 0;

    IECInstance::Debug_GetAllocationStats (&numLiveInstances, NULL, NULL);
    
    if (numLiveInstances > MAX_INTERNAL_INSTANCES)
        {
        char message[1024];
        sprintf (message, "TestForIECInstanceLeaks found that there are %d IECInstances still alive. Anything more than %d is flagged as an error!\n", 
            numLiveInstances, MAX_INTERNAL_INSTANCES);

        bvector<bwstring> classNamesToExclude;
        IECInstance::Debug_ReportLeaks (classNamesToExclude);

        EXPECT_TRUE (numLiveInstances <= MAX_INTERNAL_INSTANCES) << message;
        }
    }

bwstring ECTestFixture::s_dllPath = L"";
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bwstring ECTestFixture::GetDllPath()
    {
    if (s_dllPath.empty())
        {
        HINSTANCE ecobjectsHInstance = (HINSTANCE) getDLLInstance();
        wchar_t strExePath [MAX_PATH];
        if (0 == (GetModuleFileNameW (ecobjectsHInstance, strExePath, MAX_PATH)))
            return L"";
            
        wchar_t executingDirectory[_MAX_DIR];
        wchar_t executingDrive[_MAX_DRIVE];
        _wsplitpath(strExePath, executingDrive, executingDirectory, NULL, NULL);
        wchar_t filepath[_MAX_PATH];
        _wmakepath(filepath, executingDrive, executingDirectory, NULL, NULL);
        s_dllPath = filepath;
        }
    return s_dllPath;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Carole.MacDonald 02/10
+---------------+---------------+---------------+---------------+---------------+------*/
bwstring ECTestFixture::GetTestDataPath(const wchar_t *dataFile)
    {
    bwstring testData(GetDllPath());
    testData.append(L"SeedData\\");
    testData.append(dataFile);
    return testData;
    } 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bwstring ECTestFixture::GetWorkingDirectoryPath(const wchar_t *testFixture, const wchar_t *dataFile)
    {
    wchar_t path[_MAX_PATH];

    if (0 == GetEnvironmentVariableW(L"OutRoot", path, _MAX_PATH))
        {
        GetEnvironmentVariableW(L"tmp", path, _MAX_PATH);
        }

    bwstring filePath(path);
    if (filePath.size() == 0)
        return filePath;

    if (*filePath.rbegin() != '\\')
        filePath.append (L"\\");

    wchar_t * processorArchitecture = (8 == sizeof(void*)) ? L"Winx64" : L"Winx86";
    filePath.append(processorArchitecture);
    filePath.append(L"\\");
    filePath.append(L"ECFramework\\build\\AtpWorkingRoot\\");
    filePath.append(testFixture);
    filePath.append(L"\\");

    CreateDirectoryRecursive(filePath.c_str(), false);

    filePath.append(dataFile);
    return filePath;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECTestFixture::CheckProcessDirectory
(
wchar_t *filepath, 
DWORD bufferSize
)
    {
    bwstring dllPath = GetDllPath();
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Casey.Mullen                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bwstring ECTestFixture::GetLogConfigurationFilename()
    {
    wchar_t filepath[_MAX_PATH];

    if ((0 != GetEnvironmentVariableW(L"BENTLEY_LOGGING_CONFIG", filepath, _MAX_PATH)) && (0 ==_waccess(filepath, 0)))
        {
        wprintf (L"ECObjects.dll configuring logging with %s (Set by BENTLEY_LOGGING_CONFIG environment variable.)\n", filepath);
        return filepath;
        }
    else if (SUCCESS == CheckProcessDirectory(filepath, sizeof(filepath)))
        {
        wprintf (L"ECObjects.dll configuring logging using %s. Override by setting BENTLEY_LOGGING_CONFIG in environment.\n", filepath);
        return filepath;
        }
    else if (0 != GetEnvironmentVariableW(L"OutRoot", filepath, _MAX_PATH))
        {
        wchar_t * processorArchitecture = (8 == sizeof(void*)) ? L"Winx64" : L"Winx86";
        wcscat (filepath, processorArchitecture);
        wcscat (filepath, L"\\Product\\ECFrameworkNativeTest\\Tests\\logging.config.xml");
        
        if (0 ==_waccess(filepath, 0))
            {
            wprintf (L"ECObjects.dll configuring logging with %s. Override by setting BENTLEY_LOGGING_CONFIG in environment.\n", filepath);
            return filepath;
            }
        }

    return L"";
    }       

END_BENTLEY_EC_NAMESPACE

