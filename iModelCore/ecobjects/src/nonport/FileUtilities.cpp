/*-------------------------------------------------------------------------------------+
|
|     $Source: src/nonport/FileUtilities.cpp $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include <Bentley/BeFileListIterator.h>

BEGIN_BENTLEY_EC_NAMESPACE

#if defined (_WIN32) // WIP_NONPORT *** rewrite this entire file in terms of BeFileName, BeFileListIterator, etc.
#include <windows.h>

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

#elif defined (__unix__)
WString ECFileUtilities::GetDllPath(){return L"";}

#endif    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus GetSchemaFileName (WString& fullFileName, UInt32& foundMinorVersion, WCharCP schemaPath, bool useLatestCompatibleMatch)
    {
    WString     schemaPathWithWildcard = schemaPath;
    schemaPathWithWildcard += L"*";

    BeFileListIterator  fileList (schemaPathWithWildcard.c_str(), false);
    BeFileName          filePath;
    UInt32 currentMinorVersion=0;

    while (SUCCESS == fileList.GetNextFileName (filePath))
        {
        WCharCP     fileName = filePath.GetName();

        if (!useLatestCompatibleMatch)
            {
            fullFileName = fileName;
            return ECOBJECTS_STATUS_Success;
            }

        if (fullFileName.empty())
            {
            fullFileName = fileName;
            GetMinorVersionFromSchemaFileName (foundMinorVersion, fileName);
            continue;
            }

        if (ECOBJECTS_STATUS_Success != GetMinorVersionFromSchemaFileName (currentMinorVersion, fileName))
            continue;

        if (currentMinorVersion > foundMinorVersion)
            {
            foundMinorVersion = currentMinorVersion;
            fullFileName = fileName;
            }
        }

    if (fullFileName.empty())
        return ECOBJECTS_STATUS_Error;

    return ECOBJECTS_STATUS_Success;
    }

END_BENTLEY_EC_NAMESPACE


