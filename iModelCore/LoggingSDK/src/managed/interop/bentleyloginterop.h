/*--------------------------------------------------------------------------------------+
|
|     $Source: src/managed/interop/bentleyloginterop.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*__BENTLEY_INTERNAL_ONLY__*/
#pragma once

#ifndef __cplusplus
#error Requires C++ compiler
#endif

#ifdef BSILOG_INTEROP_EXPORTS
#define BSILOG_INTEROP_API              __declspec(dllexport)
#else
#define BSILOG_INTEROP_API              __declspec(dllimport)
#endif

#include <Logging/bentleylogging.h>

BEGIN_BENTLEY_LOGGING_NAMESPACE

namespace Provider {

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
class BSILOG_INTEROP_API ManagedInteropProvider : public ILogProvider
    {
    public:
        ManagedInteropProvider ( void );

        ~ManagedInteropProvider ( void );

        int __stdcall Initialize ();

        int __stdcall Uninitialize ();

        int __stdcall CreateLogger( wchar_t const* nameSpace, ILogProviderContext ** ppContext );

        int __stdcall DestroyLogger( ILogProviderContext * pContext );

        int __stdcall SetOption ( wchar_t const* attribName, wchar_t const* attribValue );

        int __stdcall GetOption ( wchar_t const* attribName, wchar_t * attribValue, unsigned int valueSize );

        int  __stdcall SetSeverity ( wchar_t const* _namespace, SEVERITY severity );

        bool __stdcall IsSeverityEnabled( ILogProviderContext * pContext, ::BentleyApi::NativeLogging::SEVERITY sev );

        void __stdcall LogMessage( ILogProviderContext * pContext, ::BentleyApi::NativeLogging::SEVERITY sev, wchar_t const* msg );
        void __stdcall LogMessage( ILogProviderContext * pContext, ::BentleyApi::NativeLogging::SEVERITY sev, char const* msg );
    };

EXTERN_C BSILOG_INTEROP_API ILogProvider* InstanceLoggingProvider( void );

} // End of Provider namespace

END_BENTLEY_LOGGING_NAMESPACE
