/*--------------------------------------------------------------------------------------+
|
|     $Source: LoggingSDK/src/native/interface/loggingconfig.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "bsilogprivate.h"
#include <Bentley/BeFileName.h>

USING_NAMESPACE_BENTLEY_LOGGING

MaxMessageSize BentleyApi::NativeLogging::g_maxMessageSize;

typedef Provider::ILogProvider* (*pfn_InstanceLoggingProvider) ( void );

#if defined (BENTLEY_WIN32)
#include <windows.h>
static void dummy() {;}
/*---------------------------------------------------------------------------------**//**
* Copied from the deprecated BeGetModuleFileName.
* @bsimethod                                    sam.wilson                      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus getModuleFileName (BeFileName& moduleName, void* addr)
    {
    MEMORY_BASIC_INFORMATION mbi;
    HINSTANCE h = VirtualQuery (addr, &mbi, sizeof mbi)? (HINSTANCE)mbi.AllocationBase: (HINSTANCE)addr;

    WChar tModuleName[MAX_PATH];
    if (0 == ::GetModuleFileNameW (h, tModuleName, MAX_PATH)) 
        return ERROR;

    moduleName.SetName(tModuleName);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   04/10
* @bsimethod                                                    sam.wilson      04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void* getProcAddress (WCharCP moduleName, CharCP functionName)
    {
    // Just try to load the library
    HMODULE hLib = LoadLibraryW ( moduleName );

    if ( NULL == hLib )
        {
        // That did not work so look for next to this dll
        BeFileName winPath;
        getModuleFileName (winPath, &dummy);

        winPath.PopDir ();
        winPath.AppendToPath (moduleName);

        hLib = LoadLibraryW ( winPath.GetName() );

        if ( NULL == hLib )
            {
            // That did not work so look for next to the exe
            getModuleFileName (winPath, NULL);

            winPath.PopDir ();
            winPath.AppendToPath (moduleName);

            hLib = LoadLibraryW ( winPath );
            }
        }

    if ( NULL == hLib )
        {
        // If we could not load it after all that, time to give up
        return NULL;
        }

    // Get the function pointers
    return GetProcAddress( hLib, functionName );
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t MaxMessageSize::Set ( uint32_t size )
    {
    return m_maxMessageSize = size;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t MaxMessageSize::Get ()
    {
    return m_maxMessageSize.load();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE LoggingConfig::ActivateProvider
(
LoggingProviderType type
)
    {
    Provider::ILogProvider* pProvider = NULL;

    switch ( type )
        {
#if defined (BENTLEY_WIN32)
        case ( LOG4CXX_LOGGING_PROVIDER ):
            {
            pProvider = LoggingConfig::LoadProvider ( L"bentleylog4cxx3.dll" );
            if (NULL==pProvider)
                return ActivateProvider (CONSOLE_LOGGING_PROVIDER);
            break;
            }
        case ( MANAGED_LOGGING_PROVIDER ):
            {
            pProvider = LoggingConfig::LoadProvider ( L"Bentley.logging.interop-2.0.dll" );
            break;
            }
#else
        // use console logging if a Windows-specific provider is requested on a non-Windows platform
        case ( LOG4CXX_LOGGING_PROVIDER ):
        case ( MANAGED_LOGGING_PROVIDER ):
#endif
        case ( CONSOLE_LOGGING_PROVIDER ):
            {
            pProvider = new Provider::ConsoleProvider();
            break;
            }
        case ( SIMPLEFILE_LOGGING_PROVIDER ):
            {
            pProvider = new Provider::SimpleFileProvider();
            break;
            }
        default:
            {
            return ERROR;
            }
        }

    if (NULL == pProvider)
        return ERROR;

    return LoggingConfig::ActivateProvider ( pProvider );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE LoggingConfig::ActivateProvider
(
Provider::ILogProvider* pProvider
)
    {
    if ( NULL == pProvider )
        {
        throw std::invalid_argument ( "pProvider" );
        }

    // If we already registered one, error out
    if ( NULL != GetLogProvider().registerProvider ( pProvider ) )
        {
        return ERROR;
        }

    if ( SUCCESS != GetLogProvider().Initialize() )
        {
        return ERROR;
        }

    GetLoggerCache().AddProvider ( pProvider );

    return SUCCESS;
    }

#if defined (BENTLEY_WIN32)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
Provider::ILogProvider* STDCALL_ATTRIBUTE LoggingConfig::LoadProvider
(
WCharCP moduleName
)
    {
    if ( NULL == moduleName )
        {
        throw std::invalid_argument ( "moduleName" );
        }

    pfn_InstanceLoggingProvider pfn_Ilp = (pfn_InstanceLoggingProvider) getProcAddress (moduleName, "InstanceLoggingProvider");

    if ( NULL == pfn_Ilp )
        {
        return NULL;
        }

    Provider::ILogProvider* pProvider = pfn_Ilp();

    return pProvider;
    }
#endif

#if defined (BENTLEY_WIN32)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE LoggingConfig::LoadAndActivateProvider
(
WCharCP moduleName
)
    {
    Provider::ILogProvider* pProvider = LoggingConfig::LoadProvider ( moduleName );

    if ( NULL == pProvider )
        {
        return ERROR;
        }

    return LoggingConfig::ActivateProvider ( pProvider );
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE LoggingConfig::DeactivateProvider ( void )
    {
    Provider::ILogProvider* pProvider = GetLogProvider().GetProvider();

    // If we already registered one, error out
    if ( NULL == pProvider )
        {
        return ERROR;
        }

    GetLoggerCache().RemoveProvider ( pProvider );

    return GetLogProvider().Uninitialize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool STDCALL_ATTRIBUTE LoggingConfig::IsProviderActive ( void )
    {
    return GetLogProvider().isProviderRegistered();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE LoggingConfig::GetOption
(
WCharCP attribName,
WCharP  attribValue,
uint32_t valueSize
)
    {
    if ( NULL == attribName || NULL == attribValue )
        {
        return ERROR;
        }

    return GetLogProvider().GetOption(attribName,attribValue,valueSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE LoggingConfig::SetOption
(
WCharCP attribName,
WCharCP attribValue
)
    {
    if ( NULL == attribName || NULL == attribValue )
        {
        return ERROR;
        }

    return GetLogProvider().SetOption(attribName,attribValue);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE LoggingConfig::SetSeverity
(
WCharCP     nameSpace,
SEVERITY    severity
)
    {
    if ( NULL == nameSpace || severity > LOG_FATAL || severity < LOG_TRACE )
        {
        return ERROR;
        }

    return GetLogProvider().SetSeverity(nameSpace,severity);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Vincas.Razma    05/15
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE LoggingConfig::SetSeverity
(
Utf8CP      nameSpace,
SEVERITY    severity
)
    {
    return SetSeverity (WString (nameSpace, true).c_str (), severity);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t STDCALL_ATTRIBUTE LoggingConfig::SetMaxMessageSize
(
uint32_t size
)
    {
    return g_maxMessageSize.Set ( size );
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
LoggingConfig::LoggingConfig ( void )
    {
    }
