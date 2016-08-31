/*--------------------------------------------------------------------------------------+
|
|     $Source: src/managed/interop/managedinterop.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#using <mscorlib.dll>
#using <Bentley.loggingG06.dll>

namespace ManagedApi = ::Bentley::Logging;

#include <windows.h>
#include <stdio.h>
#include <mbctype.h>
#include <string.h>
#include <locale.h>
#include <cwchar>
#include <vcclr.h>
#include "exception"
#include "assert.h"
#include "bentleyloginterop.h"

namespace NativeApi = ::BentleyApi::NativeLogging;
namespace NativeProviderApi = ::BentleyApi::NativeLogging::Provider;

BEGIN_BENTLEY_LOGGING_NAMESPACE;

namespace Provider {

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
class ManagedInteropProviderContext : public NativeProviderApi::ILogProviderContext
        {
    public:
        ManagedInteropProviderContext ( ManagedApi::ILogger^ plog );

        ~ManagedInteropProviderContext ( void );

        gcroot<ManagedApi::ILogger^>    pLogger;

        };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
ManagedInteropProviderContext::ManagedInteropProviderContext
(
ManagedApi::ILogger^ plog
) : pLogger ( plog )
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
ManagedInteropProviderContext::~ManagedInteropProviderContext ( void )
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
static ManagedApi::SEVERITY translateSeverity
(
NativeApi::SEVERITY    sev
)
    {
// This is a preprocessor hack because these symbols tend to be used for other purposes
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
        case NativeApi::LOG_FATAL:
            {
            return ManagedApi::SEVERITY::FATAL;
            break;
            }
        case NativeApi::LOG_ERROR:
            {
            return ManagedApi::SEVERITY::ERROR;
            break;
            }
        case NativeApi::LOG_WARNING:
            {
            return ManagedApi::SEVERITY::WARNING;
            break;
            }
        case NativeApi::LOG_INFO:
            {
            return ManagedApi::SEVERITY::INFO;
            break;
            }
        case NativeApi::LOG_DEBUG:
            {
            return ManagedApi::SEVERITY::DEBUG;
            break;
            }
        case NativeApi::LOG_TRACE:
            {
            return ManagedApi::SEVERITY::TRACE;
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

    return ManagedApi::SEVERITY::FATAL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
ManagedInteropProvider::ManagedInteropProvider ( void )
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
ManagedInteropProvider::~ManagedInteropProvider ( void )
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
int __stdcall ManagedInteropProvider::Initialize ()
    {
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
int __stdcall ManagedInteropProvider::Uninitialize ()
    {
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
int __stdcall ManagedInteropProvider::SetOption
(
wchar_t const* attribName,
wchar_t const* attribValue
)
    {
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
int __stdcall ManagedInteropProvider::GetOption
(
wchar_t const* attribName,
wchar_t *  attribValue,
unsigned int  valueSize
)
    {
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
int  __stdcall ManagedInteropProvider::SetSeverity
(
wchar_t const* _namespace,
SEVERITY severity
)
    {
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
int __stdcall ManagedInteropProvider::CreateLogger
(
wchar_t const*                                     nameSpace,
NativeProviderApi::ILogProviderContext **   ppContext
)
    {
    ManagedApi::ILogger^ pLogger = ManagedApi::LoggerFactory::createLogger ( gcnew ::System::String(nameSpace) );

    assert ( pLogger != nullptr );

    *ppContext = new ManagedInteropProviderContext ( pLogger );

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
int __stdcall ManagedInteropProvider::DestroyLogger
(
NativeProviderApi::ILogProviderContext *    pContext
)
    {

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
bool __stdcall ManagedInteropProvider::IsSeverityEnabled
(
NativeProviderApi::ILogProviderContext *    context,
NativeApi::SEVERITY                         sev
)
    {
    ManagedInteropProviderContext* pContext = reinterpret_cast<ManagedInteropProviderContext*>(context);

    assert ( NULL != pContext );

    return pContext->pLogger->isSeverityEnabled ( translateSeverity(sev) );;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
void __stdcall ManagedInteropProvider::LogMessage
(
NativeProviderApi::ILogProviderContext *    context,
NativeApi::SEVERITY                         sev,
wchar_t const*                                   msg
)
    {
    ManagedInteropProviderContext* pContext = reinterpret_cast<ManagedInteropProviderContext*>(context);

    assert ( NULL != pContext );

    pContext->pLogger->message ( translateSeverity(sev), gcnew ::System::String(msg) );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
void __stdcall ManagedInteropProvider::LogMessage
(
NativeProviderApi::ILogProviderContext *    context,
NativeApi::SEVERITY                         sev,
char const*                                 msg
)
    {
    ManagedInteropProviderContext* pContext = reinterpret_cast<ManagedInteropProviderContext*>(context);

    assert ( NULL != pContext );

    pContext->pLogger->message ( translateSeverity(sev), gcnew ::System::String(msg) );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
ILogProvider* InstanceLoggingProvider( void )
    {
    return new ManagedInteropProvider();
    }

} // End the Provider namespace



END_BENTLEY_LOGGING_NAMESPACE;

