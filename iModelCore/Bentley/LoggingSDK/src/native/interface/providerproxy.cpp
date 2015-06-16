/*--------------------------------------------------------------------------------------+
|
|     $Source: LoggingSDK/src/native/interface/providerproxy.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "bsilogprivate.h"

USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_BENTLEY_LOGGING_PROVIDER

LogProviderProxy& BentleyApi::NativeLogging::GetLogProvider ()
    {
    static LogProviderProxy* s_proxy;
    if (NULL == s_proxy)
        s_proxy = new LogProviderProxy;
    return *s_proxy;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
LogProviderProxy::LogProviderProxy ( void ) :
    m_pLogProvider (NULL)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
LogProviderProxy::~LogProviderProxy ( void )
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE LogProviderProxy::Initialize ( void )
    {
    // Make sure we have a log provider registered
    if ( NULL == m_pLogProvider )
        {
        return ERROR;
        }

    for ( OptionMap::iterator it = m_optionCache.begin() ; it != m_optionCache.end() ; it++ )
        {
        m_pLogProvider->SetOption ( (*it).first.c_str(), (*it).second.c_str() );
        }

    return m_pLogProvider->Initialize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE LogProviderProxy::Uninitialize ( void )
    {
    // Make sure we have a log provider registered
    if ( NULL == m_pLogProvider )
        {
        return ERROR;
        }

    int status = m_pLogProvider->Uninitialize();

    m_emergencyProvider.Uninitialize();

    unregisterProvider();

    m_optionCache.clear();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE LogProviderProxy::CreateLogger
(
WCharCP nameSpace,
Provider::ILogProviderContext** ppContext
)
    {
    BeAssert ( NULL != ppContext );

    // Make sure we have a log provider registered
    if ( NULL == m_pLogProvider )
        {
        if ( m_emergencyProvider.IsActive() )
            {
            return m_emergencyProvider.CreateLogger ( nameSpace, ppContext );
            }

        return ERROR;
        }

    return m_pLogProvider->CreateLogger ( nameSpace, ppContext );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE LogProviderProxy::DestroyLogger
(
Provider::ILogProviderContext * pContext
)
    {
    // Make sure we have a log provider registered
    if ( NULL == m_pLogProvider )
        {
        if ( m_emergencyProvider.IsActive() )
            {
            return m_emergencyProvider.DestroyLogger ( pContext );
            }

        return ERROR;
        }

    return m_pLogProvider->DestroyLogger ( pContext );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
void STDCALL_ATTRIBUTE LogProviderProxy::LogMessage
(
Provider::ILogProviderContext*  pContext,
SEVERITY                        sev,
WCharCP                         msg
)
    {
    BeAssert ( NULL != msg );

    // Make sure we have a log provider registered
    if ( NULL == m_pLogProvider )
        {
        if ( m_emergencyProvider.IsActive() )
            {
            m_emergencyProvider.LogMessage ( pContext, sev, msg );
            }

        return;
        }

    m_pLogProvider->LogMessage ( pContext, sev, msg );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
void STDCALL_ATTRIBUTE LogProviderProxy::LogMessage
(
Provider::ILogProviderContext*  pContext,
SEVERITY                        sev,
Utf8CP                          msg
)
    {
    BeAssert ( NULL != msg );

    // Make sure we have a log provider registered
    if ( NULL == m_pLogProvider )
        {
        if ( m_emergencyProvider.IsActive() )
            {
            m_emergencyProvider.LogMessage ( pContext, sev, msg );
            }

        return;
        }

    m_pLogProvider->LogMessage ( pContext, sev, msg );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
bool STDCALL_ATTRIBUTE LogProviderProxy::IsSeverityEnabled
(
Provider::ILogProviderContext*    pContext,
SEVERITY                          sev
)
    {
    if (  NULL == pContext )
        {
        return false;
        }

    // Make sure we have a log provider registered
    if ( NULL == m_pLogProvider )
        {
        if ( m_emergencyProvider.IsActive() )
            {
            return m_emergencyProvider.IsSeverityEnabled ( pContext, sev );
            }

        return false;
        }

    return m_pLogProvider->IsSeverityEnabled ( pContext, sev );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE LogProviderProxy::SetOption
(
WCharCP attribName,
WCharCP attribValue
)
    {
    BeAssert ( NULL != attribName );
    BeAssert ( NULL != attribValue );

    OptionMap::iterator it = m_optionCache.find ( attribName );

    if ( it == m_optionCache.end() )
        {
        m_optionCache.insert ( OptionMap::value_type(attribName, attribValue) );
        }
    else
        {
        (*it).second = attribValue;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE LogProviderProxy::GetOption
(
WCharCP attribName,
WCharP  attribValue,
uint32_t valueSize
)
    {
    BeAssert ( NULL != attribName );
    BeAssert ( NULL != attribValue );

    // Make sure we have a log provider registered
    if ( NULL == m_pLogProvider )
        {
        return ERROR;
        }

    return m_pLogProvider->GetOption ( attribName, attribValue, valueSize );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE LogProviderProxy::SetSeverity
(
WCharCP nameSpace,
SEVERITY severity
)
    {
    BeAssert ( NULL != nameSpace );

    // Make sure we have a log provider registered
    if ( NULL == m_pLogProvider )
        {
        return ERROR;
        }

    return m_pLogProvider->SetSeverity ( nameSpace, severity );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
bool LogProviderProxy::isProviderRegistered ()
    {
    if ( NULL == m_pLogProvider )
        {
        return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
bool LogProviderProxy::isEmergencyProvider ()
    {
    return m_emergencyProvider.IsActive();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
Provider::ILogProvider* LogProviderProxy::registerProvider
(
Provider::ILogProvider* pProvider
)
    {
    BeSystemMutexHolder beCS;

    if (m_pLogProvider != nullptr)
        return m_pLogProvider;

    m_pLogProvider = pProvider;
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
Provider::ILogProvider* LogProviderProxy::unregisterProvider ()
    {
    BeSystemMutexHolder beCS;
    
    Provider::ILogProvider* was = m_pLogProvider;
    m_pLogProvider = nullptr;
    return was;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
Provider::ILogProvider* LogProviderProxy::GetProvider ()
    {
    return m_pLogProvider;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
EmergencyLogProvider::EmergencyLogProvider ( void ) :
    m_emergencyProviderType(UNKNOWN_LOGGING_PROVIDER),
    m_emergencyProvider(NULL)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
EmergencyLogProvider::~EmergencyLogProvider ( void )
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE EmergencyLogProvider::Uninitialize ( void )
    {
    m_emergencyProviderType= UNKNOWN_LOGGING_PROVIDER;
    m_emergencyProviderOutputPath.clear();
    if ( NULL !=  m_emergencyProvider )
        {
        m_emergencyProvider->Uninitialize();
        m_emergencyProvider = NULL;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool EmergencyLogProvider::IsActive ( void )
    {
#if defined (BENTLEY_WINRT) // *** WIP_WINRT_GETENV
    return false;
#else

    if ( UNKNOWN_LOGGING_PROVIDER == m_emergencyProviderType )
        {   
        char const* env = getenv ( "EMERGENCY_LOGGING_SEVERITY" );
        WString sevStr (env? env: "", false);

        SEVERITY sev = GetSeverityFromText ( sevStr.c_str() );
        
        env = getenv ( "EMERGENCY_LOGGING" );
        WString val (env? env: "", false);

        if ( val.EqualsI ( L"CONSOLE" ))
            {
            m_emergencyProvider = new ConsoleProvider();

            m_emergencyProvider->SetOption ( CONFIG_OPTION_DEFAULT_SEVERITY, GetSeverityText(sev) );

            m_emergencyProvider->Initialize();

            m_emergencyProviderType = CONSOLE_LOGGING_PROVIDER;
            }
        else if ( val.substr(0,4).EqualsI (L"FILE") )
            {
            m_emergencyProvider = new SimpleFileProvider();

            m_emergencyProvider->SetOption( CONFIG_OPTION_OUTPUT_FILE, val.c_str()+5 );

            m_emergencyProvider->SetOption ( CONFIG_OPTION_DEFAULT_SEVERITY, GetSeverityText(sev) );

            m_emergencyProvider->Initialize();


            m_emergencyProviderType = SIMPLEFILE_LOGGING_PROVIDER;
            }
        else
            {
            m_emergencyProviderType = NULL_LOGGING_PROVIDER;
            }
        }

    if ( NULL_LOGGING_PROVIDER == m_emergencyProviderType )
        {
        return false;
        }

    return true;

#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE EmergencyLogProvider::CreateLogger
(
WCharCP nameSpace,
Provider::ILogProviderContext** ppContext
)
    {
    BeAssert ( NULL != ppContext );

    // Make sure we have a log provider registered
    if ( NULL == m_emergencyProvider )
        {
        return ERROR;
        }

    return m_emergencyProvider->CreateLogger ( nameSpace, ppContext );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE EmergencyLogProvider::DestroyLogger
(
Provider::ILogProviderContext * pContext
)
    {
    BeAssert ( NULL != pContext );

    // Make sure we have a log provider registered
    if ( NULL == m_emergencyProvider )
        {
        return ERROR;
        }

    return m_emergencyProvider->DestroyLogger ( pContext );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
void STDCALL_ATTRIBUTE EmergencyLogProvider::LogMessage
(
Provider::ILogProviderContext*  pContext,
SEVERITY                        sev,
WCharCP                         msg
)
    {
    BeAssert ( NULL != pContext );
    BeAssert ( NULL != msg );

    // Make sure we have a log provider registered
    if ( NULL == m_emergencyProvider )
        {
        return;
        }

    m_emergencyProvider->LogMessage ( pContext, sev, msg );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
void STDCALL_ATTRIBUTE EmergencyLogProvider::LogMessage
(
Provider::ILogProviderContext*  pContext,
SEVERITY                        sev,
Utf8CP                          msg
)
    {
    BeAssert ( NULL != pContext );
    BeAssert ( NULL != msg );

    // Make sure we have a log provider registered
    if ( NULL == m_emergencyProvider )
        {
        return;
        }

    m_emergencyProvider->LogMessage ( pContext, sev, msg );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
bool STDCALL_ATTRIBUTE EmergencyLogProvider::IsSeverityEnabled
(
Provider::ILogProviderContext*    pContext,
SEVERITY                          sev
)
    {
    if (  NULL == pContext )
        {
        return false;
        }

    // Make sure we have a log provider registered
    if ( NULL == m_emergencyProvider )
        {
        return false;
        }

    return m_emergencyProvider->IsSeverityEnabled ( pContext, sev );
    }
