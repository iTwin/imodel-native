/*--------------------------------------------------------------------------------------+
|
|     $Source: LoggingSDK/src/native/interface/bsilog.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "bsilogprivate.h"
#include <algorithm>

USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_BENTLEY

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
bool BsiLogger::isSeverityEnabled
(
SEVERITY sev
) const
    {
    return GetLogProvider().IsSeverityEnabled ( m_context, sev );
    }

#if defined (BENTLEY_WIN32)||defined (BENTLEY_WINRT)
/*---------------------------------------------------------------------------------**//**
* The log4cxx-0.9.7 release doesn't output non-latin character sets properly. This
* manifests itself as a "hang" in the log output. Therefore, we sanitize the output
* and convert any non-latin characters to '?'.
* @bsimethod                                                    MichaelMcCarty  12/2006

+---------------+---------------+---------------+---------------+---------------+------*/
static bool isNonLatinCharacter (wchar_t c)
    {
    return !iswascii(c);
    }

/*---------------------------------------------------------------------------------**//**
* The log4cxx-0.9.7 release doesn't output non-latin character sets properly. This
* manifests itself as a "hang" in the log output. Therefore, we sanitize the output
* and convert any non-latin characters to '?'.
* @bsimethod                                                    MichaelMcCarty  12/2006
+---------------+---------------+---------------+---------------+---------------+------*/
static wchar_t replaceNonLatinCharacters (wchar_t c)
    {
    return isNonLatinCharacter(c)? L'?': c;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
void BsiLogger::message
(
SEVERITY        sev,
WCharCP         msg
)
    {
    if (!isSeverityEnabled (sev))
        return;

#if defined (BENTLEY_WIN32)||defined (BENTLEY_WINRT)

    size_t len = wcslen (msg);
    if (len < g_maxMessageSize.Get() && std::find_if (msg,msg+len,isNonLatinCharacter) == msg+len)
        {
        GetLogProvider().LogMessage ( m_context, sev, msg );
        }
    else
        {
        size_t tempStrLen = std::min<size_t> (len, g_maxMessageSize.Get()-1);   // (does not include trailing \0)

        ScopedArray<WChar> tempBuffer (tempStrLen+1);
        WCharP tempStr = tempBuffer.GetData();
        BeStringUtilities::Wcsncpy (tempStr, tempStrLen+1, msg, tempStrLen);
        tempStr[tempStrLen] = L'\0';

        std::transform (tempStr, tempStr+tempStrLen, tempStr, replaceNonLatinCharacters);

        GetLogProvider().LogMessage ( m_context, sev, tempStr );
        }

#elif defined (__unix__)

    GetLogProvider().LogMessage ( m_context, sev, Utf8String(msg).c_str() );

#else
    
    #error unknown platform

#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
void BsiLogger::message
(
SEVERITY        sev,
Utf8CP          msg
)
    {
    if (!isSeverityEnabled (sev))
        return;

    size_t len = strlen (msg);
    if (len < g_maxMessageSize.Get())
        {
        GetLogProvider().LogMessage ( m_context, sev, msg );
        }
    else
        {
        size_t tempStrLen = std::min<size_t> (len, g_maxMessageSize.Get()-1);

        ScopedArray<char> tempBuffer (tempStrLen+1);
        char* tempStr = tempBuffer.GetData();
        strncpy (tempStr, msg, tempStrLen);
        tempStr[tempStrLen] = '\0';

        GetLogProvider().LogMessage ( m_context, sev, tempStr );
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
void BsiLogger::messageva
(
SEVERITY    sev,
WCharCP     msg,
va_list     args
)
    {
    // Optimization - Are we actually going to output something before calling the printfs
    if (!isSeverityEnabled (sev))
        return;

    ScopedArray<WChar> buffer (g_maxMessageSize.Get());
    BeStringUtilities::Vsnwprintf (buffer.GetData(), g_maxMessageSize.Get(), msg, args); 

    message (sev, buffer.GetData());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
void BsiLogger::messageva
(
SEVERITY    sev,
Utf8CP      msg,
va_list     args
)
    {
    // Optimization - Are we actually going to output something before calling the printfs
    if (!isSeverityEnabled (sev))
        return;

    ScopedArray<char> buffer (g_maxMessageSize.Get());
    BeStringUtilities::Vsnprintf (buffer.GetData(), g_maxMessageSize.Get(), msg, args); 

    message (sev, buffer.GetData());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
void BsiLogger::messageva
(
WCharCP     nameSpace,
SEVERITY    sev,
WCharCP     msg,
va_list     args
)
    {
    ILogger* pLogger = LoggingManager::GetLogger ( nameSpace );
    if ( NULL != pLogger )
        pLogger->messageva ( sev, msg, args );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
void BsiLogger::messageva
(
Utf8CP      nameSpace,
SEVERITY    sev,
Utf8CP      msg,
va_list     args
)
    {
    ILogger* pLogger = LoggingManager::GetLogger ( nameSpace );
    if ( NULL != pLogger )
        pLogger->messageva ( sev, msg, args );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
void BsiLogger::message
(
WCharCP     nameSpace,
SEVERITY    sev,
WCharCP     msg
)
    {
    ILogger* pLogger = LoggingManager::GetLogger ( nameSpace );
    if ( NULL != pLogger )
        pLogger->message ( sev, msg );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
void BsiLogger::message
(
Utf8CP      nameSpace,
SEVERITY    sev,
Utf8CP      msg
)
    {
    ILogger* pLogger = LoggingManager::GetLogger ( nameSpace );
    if ( NULL != pLogger )
        pLogger->message ( sev, msg );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
void BsiLogger::messagev
(
SEVERITY sev,
WCharCP msg,
...
)
    {
    va_list args;
    va_start ( args, msg );
    this->messageva ( sev, msg, args );
    va_end ( args );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
void BsiLogger::messagev
(
SEVERITY sev,
Utf8CP   msg,
...
)
    {
    va_list args;
    va_start ( args, msg );
    this->messageva ( sev, msg, args );
    va_end ( args );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
void BsiLogger::messagev
(
WCharCP     nameSpace,
SEVERITY    sev,
WCharCP     msg,
...
)
    {
    va_list args;
    va_start ( args, msg );
    this->messageva ( nameSpace, sev, msg, args );
    va_end ( args );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
void BsiLogger::messagev
(
Utf8CP      nameSpace,
SEVERITY    sev,
Utf8CP      msg,
...
)
    {
    va_list args;
    va_start ( args, msg );
    this->messageva ( nameSpace, sev, msg, args );
    va_end ( args );
    }


///////////////////////////////////////////////////////////////////////////////////////////
/////////////////////// SHORT CUTS ////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

#define LOGGER_SHORT_CUTS(METHOD,SEVERITY,STRTYPE)\
void BsiLogger::METHOD (STRTYPE msg) {this->message ( SEVERITY, msg );}\
void BsiLogger::METHOD##v (STRTYPE msg, ...)\
    {\
    va_list args;\
    va_start ( args, msg );\
    this->messageva ( SEVERITY, msg, args );\
    va_end ( args );\
    }

LOGGER_SHORT_CUTS(fatal,    LOG_FATAL,  WCharCP)
LOGGER_SHORT_CUTS(error,    LOG_ERROR,  WCharCP)
LOGGER_SHORT_CUTS(warning,  LOG_WARNING,WCharCP)
LOGGER_SHORT_CUTS(info,     LOG_INFO,   WCharCP)
LOGGER_SHORT_CUTS(debug,    LOG_DEBUG,  WCharCP)
LOGGER_SHORT_CUTS(trace,    LOG_TRACE,  WCharCP)
LOGGER_SHORT_CUTS(fatal,    LOG_FATAL,  Utf8CP)
LOGGER_SHORT_CUTS(error,    LOG_ERROR,  Utf8CP)
LOGGER_SHORT_CUTS(warning,  LOG_WARNING,Utf8CP)
LOGGER_SHORT_CUTS(info,     LOG_INFO,   Utf8CP)
LOGGER_SHORT_CUTS(debug,    LOG_DEBUG,  Utf8CP)
LOGGER_SHORT_CUTS(trace,    LOG_TRACE,  Utf8CP)
