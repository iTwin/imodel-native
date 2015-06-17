/*--------------------------------------------------------------------------------------+
|
|     $Source: LoggingSDK/src/native/interface/loggingmanager.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "bsilogprivate.h"

USING_NAMESPACE_BENTLEY_LOGGING

LoggerCache& BentleyApi::NativeLogging::GetLoggerCache()
    {
    static LoggerCache* s_cache;
    if (NULL == s_cache)
        s_cache = new LoggerCache;
    return *s_cache;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
ILogger* STDCALL_ATTRIBUTE LoggingManager::GetLogger
(
WCharCP nameSpace
)
    {
    return GetLoggerCache().GetLogger ( nameSpace );
    }

ILogger* LoggingManager::GetLogger (Utf8CP ns) {return GetLogger(WString(ns, true).c_str());}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
ILogger* STDCALL_ATTRIBUTE LoggingManager::CreateUncachedLogger
(
WCharCP nameSpace
)
    {
    return GetLoggerCache().CreateLogger ( nameSpace );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE LoggingManager::DestroyUncachedLogger
(
ILogger* pLogger
)
    {
    return GetLoggerCache().DestroyLogger ( pLogger );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE LoggingManager::ReleaseLogger
(
ILogger* pLogger
)
    {
    if ( NULL == pLogger )
        {
        throw std::invalid_argument ( "pLogger" );
        }

    // NEEDSWORK.
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
LoggingManager::LoggingManager ( void )
    {
    }
