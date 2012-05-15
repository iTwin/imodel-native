/*-------------------------------------------------------------------------------------+
|
|     $Source: src/nonport/FileUtilities.cpp $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#if defined (_WIN32) // WIP_NONPORT
#include <Windows.h>
#endif
#include <ECObjects/ECObjectsAPI.h>
#include "../FileUtilities.h"
#include <ECObjects/ECValue.h>
#include <Bentley/BeFileName.h>
#include <Bentley/BeAssert.h>

USING_NAMESPACE_EC

#if defined (_WIN32) // WIP_NONPORT

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
* Time in local time zone
* @bsimethod                                    Bill.Steinbock                  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SystemTime SystemTime::GetLocalTime()
    {
#if defined (_WIN32) // WIP_NONPORT
    SYSTEMTIME wtime;
    ::GetLocalTime(&wtime);
    SystemTime time;
    memcpy (&time, &wtime, sizeof(time));
    return time;
#elif defined (__unix__)
    BeAssert (false && "*** TBD - convert UTC to local time");
    return GetSystemTime();
#endif
    }


