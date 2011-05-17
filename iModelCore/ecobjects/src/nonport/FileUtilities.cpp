/*-------------------------------------------------------------------------------------+
|
|     $Source: src/nonport/FileUtilities.cpp $
|
|  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <windows.h>
#include "ECObjectsPch.h"

BEGIN_BENTLEY_EC_NAMESPACE

struct ECFileNameIterator
    {
    WIN32_FIND_DATAW m_findData;
    HANDLE           m_findHandle;
    bool             m_valid;
    wchar_t          m_deviceName[_MAX_DRIVE];
    wchar_t          m_dirName[_MAX_DIR];

public:
    ECFileNameIterator (WCharCP path);
    ~ECFileNameIterator ();
    BentleyStatus GetNextFileName (WCharP name);
     };

ECFileNameIterator::ECFileNameIterator (WCharCP path)
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
    
BentleyStatus ECFileNameIterator::GetNextFileName (WCharP name)
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
 
WString ECFileUtilities::s_dllPath = L"";
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Carole.MacDonald 02/10
+---------------+---------------+---------------+---------------+---------------+------*/
WString ECFileUtilities::GetDllPath()
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
    wchar_t filepath[MAX_PATH];
    _wmakepath(filepath, executingDrive, executingDirectory, NULL, NULL);
    s_dllPath = filepath;
    return filepath;
    
    }     

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus GetSchemaFileName (WString& fullFileName, WCharCP schemaPath, bool useLatestCompatibleMatch)
    {
    ECFileNameIterator fileList (schemaPath);
    wchar_t filePath[MAX_PATH];
    UInt32 minorVersion=0;
    UInt32 currentMinorVersion=0;

    while (true)
        {
        if (SUCCESS != fileList.GetNextFileName(filePath))
            break;

        if (!useLatestCompatibleMatch)
            {
            fullFileName = filePath;
            return ECOBJECTS_STATUS_Success;
            }

        if (fullFileName.empty())
            {
            fullFileName = filePath;
            GetMinorVersionFromSchemaFileName (minorVersion, filePath);
            continue;
            }

        if (ECOBJECTS_STATUS_Success != GetMinorVersionFromSchemaFileName (currentMinorVersion, filePath))
            continue;

        if (currentMinorVersion > minorVersion)
            {
            minorVersion = currentMinorVersion;
            fullFileName = filePath;
            }
        }

    if (fullFileName.empty())
        return ECOBJECTS_STATUS_Error;

    return ECOBJECTS_STATUS_Success;
    }

END_BENTLEY_EC_NAMESPACE