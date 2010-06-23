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
    testData.append(dataFile);
    return testData;
    
    } 
    
END_BENTLEY_EC_NAMESPACE

