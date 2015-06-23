/*--------------------------------------------------------------------------------------+
|
|     $Source: Bentley/nonport/BeDebugLog.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeDebugLog.h>

#if defined (BENTLEY_WIN32)||defined (BENTLEY_WINRT)
    #include <windows.h>
#elif defined (ANDROID)
    #include <android/log.h>
#else
    #include <stdio.h>
#endif

#include <Logging/bentleylogging.h>

USING_NAMESPACE_BENTLEY

/*---------------------------------------------------------------------------------**//**
* Should not be called directly! Instead use the BeDebugLog macro in Bentley/BeDebugLog.h                                                                                       
*
* @bsimethod                                    Shaun.Sewall                    12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void            BeDebugLogFunctions::PerformBeDebugLog
(
Utf8CP          message,
Utf8CP          fileName,
unsigned        lineNumber
)
    {
#if defined (BENTLEY_WIN32)||defined (BENTLEY_WINRT)
    wchar_t newline = message[strlen(message)-1] != '\n' ? L'\n' : L' ';
    WPrintfString msg(L"BeDebugLog: %ls %d - %ls%lc", WString(fileName, BentleyCharEncoding::Utf8).c_str(), lineNumber, WString(message, BentleyCharEncoding::Utf8).c_str(), newline);
    OutputDebugStringW (msg.c_str());
//    fwprintf (stderr, msg.c_str());
#elif defined (ANDROID)
    __android_log_print (ANDROID_LOG_VERBOSE, "BeDebugLog", "%s [%s:%u]", message, fileName, lineNumber);
#elif defined (__APPLE__)
    CharCP  name = strrchr (fileName, '/');
    name = (NULL == name ? fileName : name+1);
    printf ("BeDebugLog: %s [%s:%u]\n", message, name, lineNumber);
#else
    NativeLogging::LoggingManager::GetLogger("BeDebugLog")->debugv ("%hs [%hs:%u]", message, fileName, lineNumber);
#endif
    }
