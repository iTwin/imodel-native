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

BEGIN_BENTLEY_EC_NAMESPACE
  
#define MAX_INTERNAL_INSTANCES  0
#define MAX_INTERNAL_SCHEMAS    0

#define DEBUG_ECSCHEMA_LEAKS
#define DEBUG_IECINSTANCE_LEAKS

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Josh.Schifter   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECTestFixture::SetUp ()
    {
    IECInstance::Debug_ResetAllocationStats();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Josh.Schifter   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECTestFixture::TearDown ()
    {
#if defined (DEBUG_ECSCHEMA_LEAKS)
    BackDoor::ECSchema::Debug_GetLeakDetector().ReportStats(L"PostTest");
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
void    testForLeaks (ILeakDetector& detector, wchar_t const * leakName)
    {
    Int32           numLeaks = detector.CheckForLeaks();

    EXPECT_TRUE (0 == numLeaks)  << "Found " << numLeaks << " leaks of " << leakName;

    if (0 != numLeaks)
        detector.ResetStats();  // So that this leak doesn't make the next test fail.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/10
+---------------+---------------+---------------+---------------+---------------+------*/
void    ECTestFixture::TestForECSchemaLeaks ()
    {
    testForLeaks (BackDoor::ECSchema::Debug_GetLeakDetector(), L"ECSchemaLeakDetector");
    testForLeaks (BackDoor::ECClass::Debug_GetLeakDetector(), L"ECClassLeakDetector");
    testForLeaks (BackDoor::ECProperty::Debug_GetLeakDetector(), L"ECPropertyLeakDetector");
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

        std::vector<bwstring> classNamesToExclude;
        IECInstance::Debug_ReportLeaks (classNamesToExclude);

        EXPECT_TRUE (numLiveInstances <= MAX_INTERNAL_INSTANCES) << message;
        }
    }

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

std::wstring ECTestFixture::s_dllPath = L"";
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Carole.MacDonald 02/10
+---------------+---------------+---------------+---------------+---------------+------*/
std::wstring ECTestFixture::GetTestDataPath(const wchar_t *dataFile)
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
        
    std::wstring testData(s_dllPath);
    testData.append(L"SeedData\\");
    testData.append(dataFile);
    return testData;
    } 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
std::wstring ECTestFixture::GetWorkingDirectoryPath(const wchar_t *testFixture, const wchar_t *dataFile)
    {
    wchar_t path[_MAX_PATH];

    if (0 == GetEnvironmentVariableW(L"OutRoot", path, _MAX_PATH))
        {
        GetEnvironmentVariableW(L"tmp", path, _MAX_PATH);
        }

    std::wstring filePath(path);
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
    
END_BENTLEY_EC_NAMESPACE

