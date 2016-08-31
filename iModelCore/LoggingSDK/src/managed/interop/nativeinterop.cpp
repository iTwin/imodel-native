/*--------------------------------------------------------------------------------------+
|
|     $Source: src/managed/interop/nativeinterop.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#using <mscorlib.dll>
#using <Bentley.loggingG06.dll>

namespace ManagedApi = ::Bentley::Logging;
namespace ManagedProviderApi = ::Bentley::Logging::Provider;

#include <windows.h>
#include <stdio.h>
#include <mbctype.h>
#include <string.h>
#include <locale.h>
#include <cwchar>
#include <vcclr.h>
#include <exception>
#include <assert.h>
#include <Logging/bentleylogging.h>

namespace NativeApi = ::BentleyApi::NativeLogging;

BEGIN_BENTLEY_LOGGING_NAMESPACE;
namespace Provider {
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
ref class NativeInteropProviderContext : public ManagedProviderApi::ILogProviderContext
        {
    public:
        NativeInteropProviderContext ( NativeApi::ILogger* plog );

        ~NativeInteropProviderContext ( void );

        NativeApi::ILogger*    pLogger;

        };


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
public ref class NativeInteropProvider : public ManagedProviderApi::ILogProvider
    {
    public:
        NativeInteropProvider ( void );

        ~NativeInteropProvider ( void );

        virtual int createLogger( ::System::String^ nameSpace, ManagedProviderApi::ILogProviderContext^% ppContext );

        virtual int destroyLogger( ManagedProviderApi::ILogProviderContext^ pContext );

        virtual bool isSeverityEnabled( ManagedProviderApi::ILogProviderContext^ context, ManagedApi::SEVERITY sev );

        virtual int logMessage( ManagedProviderApi::ILogProviderContext^ context, ManagedApi::SEVERITY sev, ::System::String^ msg );
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
static NativeApi::SEVERITY translateSeverity
(
ManagedApi::SEVERITY    sev
)
    {
// This is a preprocessor hack because we these symbols tend to be used for other purposes
#ifdef FATAL
#define _SAVE_FATAL FATAL
#undef FATAL
#endif
#ifdef ERROR
#define _SAVE_ERROR ERROR
#undef ERROR
#endif
#ifdef WARNING
#define _SAVE_WARNING WARNING
#undef WARNING
#endif
#ifdef INFO
#define _SAVE_INFO INFO
#undef INFO
#endif
#ifdef DEBUG
#define _SAVE_DEBUG DEBUG
#undef DEBUG
#endif
#ifdef TRACE
#define _SAVE_TRACE TRACE
#undef TRACE
#endif

    switch ( sev )
        {
        case ManagedApi::SEVERITY::FATAL:
            {
            return NativeApi::LOG_FATAL;
            break;
            }
        case ManagedApi::SEVERITY::ERROR:
            {
            return NativeApi::LOG_ERROR;
            break;
            }
        case ManagedApi::SEVERITY::WARNING:
            {
            return NativeApi::LOG_WARNING;
            break;
            }
        case ManagedApi::SEVERITY::INFO:
            {
            return NativeApi::LOG_INFO;
            break;
            }
        case ManagedApi::SEVERITY::DEBUG:
            {
            return NativeApi::LOG_DEBUG;
            break;
            }
        case ManagedApi::SEVERITY::TRACE:
            {
            return NativeApi::LOG_TRACE;
            break;
            }
        }

#ifdef _SAVE_FATAL
#define FATAL _SAVE_FATAL
#undef _SAVE_FATAL
#endif
#ifdef _SAVE_ERROR
#define ERROR _SAVE_ERROR
#undef _SAVE_ERROR
#endif
#ifdef _SAVE_WARNING
#define WARNING _SAVE_WARNING
#undef _SAVE_WARNING
#endif
#ifdef _SAVE_INFO
#define INFO _SAVE_INFO
#undef _SAVE_INFO
#endif
#ifdef _SAVE_DEBUG
#define DEBUG _SAVE_DEBUG
#undef _SAVE_DEBUG
#endif
#ifdef _SAVE_TRACE
#define TRACE _SAVE_TRACE
#undef _SAVE_TRACE
#endif

    return NativeApi::LOG_FATAL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
NativeInteropProviderContext::NativeInteropProviderContext
(
NativeApi::ILogger* plog
) : pLogger ( plog )
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
NativeInteropProviderContext::~NativeInteropProviderContext ( void )
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
NativeInteropProvider::NativeInteropProvider ( void )
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
NativeInteropProvider::~NativeInteropProvider ( void )
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
int NativeInteropProvider::createLogger
(
::System::String^                           nameSpace,
ManagedProviderApi::ILogProviderContext^%   ppContext
)
    {
    cli::pin_ptr<const wchar_t> pNativeNamespace = PtrToStringChars(nameSpace);

    NativeApi::ILogger* pLogger = NativeApi::LoggingManager::GetLogger ( pNativeNamespace );

    assert ( NULL != pLogger );

    ppContext = gcnew NativeInteropProviderContext ( pLogger );

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
int NativeInteropProvider::destroyLogger
(
ManagedProviderApi::ILogProviderContext^    pContext
)
    {

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
bool NativeInteropProvider::isSeverityEnabled
(
ManagedProviderApi::ILogProviderContext^    context,
ManagedApi::SEVERITY                        sev
)
    {
    NativeInteropProviderContext^ pContext = dynamic_cast<NativeInteropProviderContext^>(context);

    assert ( pContext != nullptr );

    assert ( NULL != pContext->pLogger );

    return pContext->pLogger->isSeverityEnabled ( translateSeverity(sev) );;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
int NativeInteropProvider::logMessage
(
ManagedProviderApi::ILogProviderContext^    context,
ManagedApi::SEVERITY                        sev,
::System::String^                           msg
)
    {
    NativeInteropProviderContext^ pContext = dynamic_cast<NativeInteropProviderContext^>(context);

    assert ( pContext != nullptr );

    assert ( NULL != pContext->pLogger );

    cli::pin_ptr<const wchar_t> pNativeMsg = PtrToStringChars(msg);

    assert ( pNativeMsg != nullptr );

    pContext->pLogger->message ( translateSeverity(sev), pNativeMsg );

    return SUCCESS;
    }


} // End of Provider namespace

END_BENTLEY_LOGGING_NAMESPACE;

