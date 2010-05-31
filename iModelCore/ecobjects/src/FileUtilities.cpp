/*-------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/native/FileUtilities.cpp $
|
|  $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <windows.h>
#include "ECObjectsPch.h"

BEGIN_BENTLEY_EC_NAMESPACE

ECFileNameIterator::ECFileNameIterator (const wchar_t * path)
    {
    m_valid = true;
    m_findHandle = ::FindFirstFileW (path, &m_findData);
    _wsplitpath(path, m_deviceName, m_dirName, NULL, NULL);
    }
    
ECFileNameIterator::~ECFileNameIterator ()
    {
    if (INVALID_HANDLE_VALUE != m_findHandle)
        ::FindClose (m_findHandle);
    }
    
BentleyStatus ECFileNameIterator::GetNextFileName (wchar_t * name)
    {
    if (INVALID_HANDLE_VALUE == m_findHandle || !m_valid)
        return  ERROR;

    _wmakepath (name, m_deviceName, m_dirName, m_findData.cFileName, NULL);
    m_valid = 0 != ::FindNextFileW (m_findHandle, &m_findData);
    return  SUCCESS;
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
 
std::wstring ECFileUtilities::s_dllPath = L"";
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Carole.MacDonald 02/10
+---------------+---------------+---------------+---------------+---------------+------*/
std::wstring ECFileUtilities::GetDllPath()
    {
    if (!s_dllPath.empty())
        return s_dllPath;

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
    return filepath;
    
    }     
END_BENTLEY_EC_NAMESPACE