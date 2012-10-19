/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Performance/TestFixture.cpp $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
| Based on http://cplus.about.com/od/howtodothingsi2/a/timing.htm
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsTestPCH.h"
#include "TestFixture.h"

#if defined (COMPILING_PUBLISHED_TESTS)
   // Need to reach in and grab this header since it won't be part of the published API yet we still
   // need to utilize it in the published API tests
   #include <..\PublicApi\Logging\bentleylogging.h>
#endif

USING_NAMESPACE_BENTLEY_LOGGING

BEGIN_BENTLEY_EC_NAMESPACE
  
#define MAX_INTERNAL_INSTANCES  0
#define MAX_INTERNAL_SCHEMAS    0

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
bool DirExists (WCharCP dir)
    {
    DWORD attributes = GetFileAttributesW(dir);
    return attributes != INVALID_FILE_ATTRIBUTES;
    }
 
 /*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      07/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool CreateDirectoryRecursive (WCharCP path, bool failIfExists)
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

bool ECTestFixture::s_isLoggerInitialized = false;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECTestFixture::ECTestFixture()
    {
    //LoggingConfig::ActivateProvider(CONSOLE_LOGGING_PROVIDER);

    if (!s_isLoggerInitialized)
        {
        LoggingConfig::ActivateProvider(LOG4CXX_LOGGING_PROVIDER);
        LoggingConfig::SetOption(CONFIG_OPTION_CONFIG_FILE, GetLogConfigurationFilename().c_str());

        LoggingConfig::SetSeverity(L"ECObjectsNative", LOG_WARNING);
        s_isLoggerInitialized = true;
        }
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

WString ECTestFixture::s_dllPath = L"";
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
WString ECTestFixture::GetDllPath()
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
        wchar_t filepath[MAX_PATH];
        _wmakepath(filepath, executingDrive, executingDirectory, NULL, NULL);
        s_dllPath = filepath;
        }
    return s_dllPath;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Carole.MacDonald 02/10
+---------------+---------------+---------------+---------------+---------------+------*/
WString ECTestFixture::GetTestDataPath(WCharCP dataFile)
    {
    WString testData(GetDllPath());
    testData.append(L"SeedData\\");
    testData.append(dataFile);
    return testData;
    } 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
WString ECTestFixture::GetWorkingDirectoryPath(WCharCP testFixture, WCharCP dataFile)
    {
    wchar_t path[MAX_PATH];

    if (0 == GetEnvironmentVariableW(L"OutRoot", path, MAX_PATH))
        {
        GetEnvironmentVariableW(L"tmp", path, MAX_PATH);
        }

    WString filePath(path);
    if (filePath.size() == 0)
        return filePath;

    if (*filePath.rbegin() != '\\')
        filePath.append (L"\\");

    WCharP processorArchitecture = (8 == sizeof(void*)) ? L"Winx64" : L"Winx86";
    filePath.append(processorArchitecture);
    filePath.append(L"\\");
    filePath.append(L"build\\ECObjectsTests\\AtpWorkingRoot\\");
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
WCharP filepath, 
DWORD bufferSize
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Casey.Mullen                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
WString ECTestFixture::GetLogConfigurationFilename()
    {
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
WString ReplaceDelimiters (WStringCR in, WStringCR oldDelimiter, WStringCR newDelimiter)
    {
    WString out = in;
    size_t index = out.find (oldDelimiter, 0);
    while (std::string::npos != index)
        {
        out = out.replace (index, 1, newDelimiter);
        index = out.find (oldDelimiter, 0);
        }
    return out;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
WString ReplaceSlashes (WStringCR in)
    {
    WString slash (L"/");
    WString backSlash (L"\\");
    return ReplaceDelimiters (in, slash, backSlash);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WString ECTestFixture::GetTestResultsFilePath (WCharCP fileName)
    {
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
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ECTestFixture::MakeDirContainingFile (WCharCP filePath)
    {
    WChar dir[_MAX_DIR];
    WChar dir2[_MAX_DIR];
    _wsplitpath (filePath, dir, dir2, NULL, NULL);
    _wmakepath (dir, dir, dir2, NULL, NULL);

    WCharP endLastPath = dir;
    while (true)
        {
        WCharP slash = wcschr (endLastPath, '\\');
        if (NULL != slash)
            *slash = 0;
        if (GetFileAttributesW (dir)==0xFFFFFFFF) 
            {
            if (false == CreateDirectoryW (dir,0))
                return ERROR;
            }
        if (NULL == slash)
            return SUCCESS;

        *slash = '\\';
        endLastPath = slash + 1;
        }
    return ERROR;
    }

END_BENTLEY_EC_NAMESPACE

