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

WString ECFileUtilities::s_dllPath;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Carole.MacDonald 02/10
+---------------+---------------+---------------+---------------+---------------+------*/
WString ECFileUtilities::GetDllPath()
    {
// *** WIP_NONPORT -- we need to ask some kind of host for the "home directory"
    if (s_dllPath.empty())
        {
        BeFileName filePath;
        BeGetModuleFileName (filePath, (void*)&GetDllPath);

        BeFileName dd (BeFileName::DevAndDir, filePath);
        s_dllPath.assign (dd);
        }
    return s_dllPath;
    }     

#elif defined (__unix__)
// *** WIP_NONPORT -- we need to ask some kind of host for the "home directory"
WString ECFileUtilities::GetDllPath() {return WString(getenv("BeGTest_HomeDirectory"));}

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


