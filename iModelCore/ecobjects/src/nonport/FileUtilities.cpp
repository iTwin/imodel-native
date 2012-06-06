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
#include <ECObjects/ECValue.h>
#include <Bentley/BeFileName.h>
#include <Bentley/BeAssert.h>

USING_NAMESPACE_EC

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


