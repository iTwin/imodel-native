/*--------------------------------------------------------------------------------------+
|
|     $Source: LoggingSDK/src/native/interface/consoleprovider.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#if defined (ANDROID)
    #define USE_ANDROID_LOG
#elif defined (__APPLE__) || defined (__linux__)
    #define USE_SYSLOG
#elif defined (BENTLEY_WIN32)
    // use printf + OutputDebugStringW
    #include <windows.h>
#elif defined (BENTLEY_WINRT)
    // use OutputDebugStringW
    #include <windows.h>
#else
    #error unknown platform
#endif

#if defined (ANDROID)
    #include <android/log.h>
#elif defined (USE_SYSLOG)
    #include <syslog.h>

    #undef LOG_EMERG   
    #undef LOG_ALERT   
    #undef LOG_CRIT    
    #undef LOG_ERR     
    #undef LOG_WARNING 
    #undef LOG_NOTICE  
    #undef LOG_INFO    
    #undef LOG_DEBUG   

    enum SyslogLevels {
    SYSLOG_EMERG     = 0,       /* system is unusable */
    SYSLOG_ALERT     = 1,       /* action must be taken immediately */
    SYSLOG_CRIT      = 2,       /* critical conditions */
    SYSLOG_ERR       = 3,       /* error conditions */
    SYSLOG_WARNING   = 4,       /* warning conditions */
    SYSLOG_NOTICE    = 5,       /* normal but significant condition */
    SYSLOG_INFO      = 6,       /* informational */
    SYSLOG_DEBUG     = 7        /* debug-level messages */
    };

    static int s_syslog_ref_count;

#endif // defined (USE_SYSLOG)

#include "bsilogprivate.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_BENTLEY_LOGGING_PROVIDER

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP BentleyApi::NativeLogging::Provider::GetSeverityText (SEVERITY sev)
    {
    switch ( sev )
        {
        case LOG_FATAL:     return LOG_TEXT_FATAL;
        case LOG_ERROR:     return LOG_TEXT_ERROR;
        case LOG_WARNING:   return LOG_TEXT_WARNING;
        case LOG_INFO:      return LOG_TEXT_INFO;
        case LOG_DEBUG:     return LOG_TEXT_DEBUG;
        case LOG_TRACE:     return LOG_TEXT_TRACE;
        }
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP BentleyApi::NativeLogging::Provider::GetSeverityUtf8Text (SEVERITY sev)
    {
    switch ( sev )
        {
        case LOG_FATAL:     return LOG_UTF8TEXT_FATAL;
        case LOG_ERROR:     return LOG_UTF8TEXT_ERROR;
        case LOG_WARNING:   return LOG_UTF8TEXT_WARNING;
        case LOG_INFO:      return LOG_UTF8TEXT_INFO;
        case LOG_DEBUG:     return LOG_UTF8TEXT_DEBUG;
        case LOG_TRACE:     return LOG_UTF8TEXT_TRACE;
        }
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
SEVERITY BentleyApi::NativeLogging::Provider::GetSeverityFromText
(
WCharCP sev
)
    {
    SEVERITY  severity = LOG_WARNING;

    if ( NULL == sev )
        {
        return severity;
        }

    if ( 0 == BeStringUtilities::Wcsicmp ( sev, LOG_TEXT_FATAL ) )
        {
        severity = LOG_FATAL;
        }
    else if ( 0 == BeStringUtilities::Wcsicmp ( sev, LOG_TEXT_ERROR ) )
        {
        severity = LOG_ERROR;
        }
    else if ( 0 == BeStringUtilities::Wcsicmp ( sev, LOG_TEXT_WARNING ) )
        {
        severity = LOG_WARNING;
        }
    else if ( 0 == BeStringUtilities::Wcsicmp ( sev, LOG_TEXT_INFO ) )
        {
        severity = LOG_INFO;
        }
    else if ( 0 == BeStringUtilities::Wcsicmp ( sev, LOG_TEXT_DEBUG ) )
        {
        severity = LOG_DEBUG;
        }
    else if ( 0 == BeStringUtilities::Wcsicmp ( sev, LOG_TEXT_TRACE ) )
        {
        severity = LOG_TRACE;
        }

    return severity;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
SeverityMap::SeverityMap ( void ) : m_defaultSeverity ( LOG_ERROR )
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
SeverityMap::SeverityMap ( SEVERITY defaultSeverity ) : m_defaultSeverity ( defaultSeverity )
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
SeverityMap::~SeverityMap ( void )
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
SEVERITY SeverityMap::GetDefaultSeverity ( void )
    {
    return m_defaultSeverity;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
void SeverityMap::SetDefaultSeverity
(
SEVERITY    severity
)
    {
    m_defaultSeverity = severity;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
int SeverityMap::SetSeverity
(
WCharCP     nameSpace,
SEVERITY    severity
)
    {
    BeAssert ( NULL != nameSpace );

    BeMutexHolder lock(m_lock);

    bmap<WString,SEVERITY>::iterator it = m_severity.find ( nameSpace );

    if ( it != m_severity.end() )
        {
        (*it).second = severity;
        }
    else
        {
        m_severity.insert ( bmap<WString,SEVERITY>::value_type(nameSpace,severity) );
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool SeverityMap::IsSeverityEnabled
(
WCharCP     nameSpace,
SEVERITY    sev
)
    {
    BeAssert ( NULL != nameSpace );

    BeMutexHolder lock(m_lock);

    SEVERITY    severity = m_defaultSeverity;

    bmap<WString,SEVERITY>::iterator it = m_severity.find ( nameSpace );

    if ( it != m_severity.end() )
        {
        severity = (*it).second;
        }

    if ( sev < severity )
        {
        return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
ConsoleProvider::ConsoleProvider ( void ) : m_severity ( LOG_ERROR )
    {
#if defined (USE_SYSLOG)
    if (0 == s_syslog_ref_count++)
        openlog ("Be", (LOG_CONS|LOG_PERROR), LOG_USER);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
ConsoleProvider::~ConsoleProvider ( void )
    {
#if defined (USE_SYSLOG)
    if (0 == --s_syslog_ref_count)
        closelog ();
#endif
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE ConsoleProvider::Initialize ( void )
    {
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE ConsoleProvider::Uninitialize ( void )
    {
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE ConsoleProvider::CreateLogger
(
WCharCP                 nameSpace,
ILogProviderContext**   ppContext
)
    {
    if ( NULL == ppContext || NULL == nameSpace )
        {
        return ERROR;
        }

    *ppContext = reinterpret_cast<ILogProviderContext*>(new WString ( nameSpace ));

    if ( NULL == *ppContext )
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE ConsoleProvider::DestroyLogger
(
ILogProviderContext*    pContext
)
    {
    if ( NULL == pContext )
        {
        return ERROR;
        }

    WString* pNs = reinterpret_cast<WString*>(pContext);

    if ( NULL == pNs )
        {
        return ERROR;
        }

    delete pNs;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
void STDCALL_ATTRIBUTE ConsoleProvider::LogMessage
(
ILogProviderContext*    pContext,
SEVERITY                sev,
WCharCP                 msg
)
    {
    if ( NULL == pContext || NULL == msg )
        {
        return;
        }

    WString* pNs = reinterpret_cast<WString*>(pContext);

    BeAssert ( NULL != pNs );

    if ( !m_severity.IsSeverityEnabled (pNs->c_str(), sev) )
        {
        return;
        }

#if defined (BENTLEY_WIN32)

    // Because it is so easy for developers to screw up printf format strings, we're
    // going to protect ourselves from an exception and log a message saying that
    // our developers are stupid and to please contact support
    BSILOG_TRY
        {
        WPrintfString formattedMessage ( L"%-8ls %-20ls %ls\n", GetSeverityText(sev), pNs->c_str(), msg );
        OutputDebugStringW (formattedMessage.c_str());
        fwprintf ( stdout, L"%ls", formattedMessage.c_str() );
        }
    BSILOG_CATCH
        {
        fwprintf ( stdout, L"Formating output string caused an exception!!!" );
        }

#elif defined (BENTLEY_WINRT)

    WPrintfString formattedMessage ( L"%-8ls %-20ls %ls\n", GetSeverityText(sev), pNs->c_str(), msg );
    OutputDebugStringW (formattedMessage.c_str());

#elif defined (__unix__)

    LogMessage (pContext, sev, Utf8String(msg).c_str());

#else

    #error unknown platform

#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
void STDCALL_ATTRIBUTE ConsoleProvider::LogMessage
(
ILogProviderContext*    pContext,
SEVERITY                sev,
Utf8CP                  msg
)
    {
    if ( NULL == pContext || NULL == msg )
        {
        return;
        }

    WString* pNs = reinterpret_cast<WString*>(pContext);

    BeAssert ( NULL != pNs );

    if ( !m_severity.IsSeverityEnabled (pNs->c_str(), sev) )
        {
        return;
        }

    Utf8String nsUtf8 (pNs->c_str());

#if defined (USE_ANDROID_LOG)

    int asev;
    switch (sev)
        {
        case LOG_FATAL:   asev = ANDROID_LOG_FATAL;   break;
        case LOG_ERROR:   asev = ANDROID_LOG_ERROR;   break;
        case LOG_WARNING: asev = ANDROID_LOG_WARN;    break;
        case LOG_INFO:    asev = ANDROID_LOG_INFO;    break;
        case LOG_DEBUG:   asev = ANDROID_LOG_DEBUG;   break;
        case LOG_TRACE:   asev = ANDROID_LOG_VERBOSE; break;
        default:          asev = ANDROID_LOG_INFO;    break;
        }

    __android_log_print (asev, nsUtf8.c_str (), msg);

#elif defined (USE_SYSLOG)

    int asev;
    switch (sev)
        {
        case LOG_FATAL:   asev = SYSLOG_EMERG;   break;
        case LOG_ERROR:   asev = SYSLOG_ERR;     break;
        case LOG_WARNING: asev = SYSLOG_WARNING; break;
        case LOG_INFO:    asev = SYSLOG_INFO;    break;
        case LOG_DEBUG:   asev = SYSLOG_DEBUG;   break;
        case LOG_TRACE:   asev = SYSLOG_DEBUG;   break;
        default:          asev = SYSLOG_INFO;    break;
        }

    syslog (asev, "%s: %s", nsUtf8.c_str(), msg);

#else

    LogMessage (pContext, sev, WString(msg,true).c_str());

#endif
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE ConsoleProvider::SetOption
(
WCharCP attribName,
WCharCP attribValue
)
    {
    BeAssert ( NULL != attribName );
    BeAssert ( NULL != attribValue );

    if ( 0 == wcscmp ( attribName, CONFIG_OPTION_DEFAULT_SEVERITY ) )
        {
        m_severity.SetDefaultSeverity ( GetSeverityFromText(attribValue) );
        }
    else
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE ConsoleProvider::GetOption
(
WCharCP attribName,
WCharP  attribValue,
uint32_t valueSize
)
    {
    BeAssert ( NULL != attribName );
    BeAssert ( NULL != attribValue );

    if ( 0 == wcscmp ( attribName, CONFIG_OPTION_DEFAULT_SEVERITY ) )
        {
        BeStringUtilities::Wcsncpy ( attribValue, valueSize, GetSeverityText(m_severity.GetDefaultSeverity()) );
        }
    else
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE ConsoleProvider::SetSeverity
(
WCharCP     nameSpace,
SEVERITY    severity
)
    {
    BeAssert ( NULL != nameSpace );

    return m_severity.SetSeverity (nameSpace,severity);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool STDCALL_ATTRIBUTE ConsoleProvider::IsSeverityEnabled
(
ILogProviderContext*    pContext,
SEVERITY                severity
)
    {
    BeAssert ( NULL != pContext );

    WString* pNs = reinterpret_cast<WString*>(pContext);

    BeAssert ( NULL != pNs );

    return m_severity.IsSeverityEnabled (pNs->c_str(),severity);
    }


