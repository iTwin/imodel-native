/*--------------------------------------------------------------------------------------+
|
|     $Source: src/native/log4cxx/log4cxx.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*__BENTLEY_INTERNAL_ONLY__*/
#pragma once

#include <windows.h>
#include <xstring>
#include <vector>
#include <Logging/bentleylogging.h>

#pragma warning(push)
#pragma warning (disable:4251)  

#ifdef BSILOG_LOG4CXX_EXPORTS
#define BSILOG_LOG4CXX_API __declspec(dllexport)
#else
#define BSILOG_LOG4CXX_API __declspec(dllimport)
#endif

BEGIN_BENTLEY_LOGGING_NAMESPACE

namespace Provider {

// Map between logger context handle and logger to use
class LoggerMap;

class BSILOG_LOG4CXX_API Log4cxxProvider : public ILogProvider
    {
    public:
        Log4cxxProvider ( wchar_t const* name = NULL );

        virtual ~Log4cxxProvider ( void );

        void PushThreadContext ( wchar_t const* context );

        void PopThreadContext ( void );

        void ClearThreadContext ( void );

        void AddContext ( wchar_t const* key, wchar_t const* context );

        void RemoveContext ( wchar_t const* key );

        void ClearContext ( void );

        int  GetLogfileNames (std::vector<std::wstring>& fileNames);

        enum
            {
            DEFAULT_DELAY = 60000
            };


    // ILogProvider methods
    public:
        int __stdcall Initialize ();

        int __stdcall Uninitialize ();

        int __stdcall CreateLogger ( wchar_t const* nameSpace, ILogProviderContext ** pContext );

        int __stdcall DestroyLogger ( ILogProviderContext * pContext );

        void __stdcall LogMessage ( ILogProviderContext * context, SEVERITY sev, wchar_t const* msg );
        void __stdcall LogMessage ( ILogProviderContext * context, SEVERITY sev, Utf8CP msg ) {LogMessage(context,sev,WString(msg, true).c_str());}

        bool __stdcall IsSeverityEnabled ( ILogProviderContext * context, SEVERITY sev );

        int __stdcall SetSeverity ( wchar_t const* nameSpace, SEVERITY sev );

        int __stdcall SetOption ( wchar_t const* attribName, wchar_t const* attribValue );

        int __stdcall GetOption ( wchar_t const* attribName, wchar_t * attribValue, unsigned int valueSize );

    public:
        int BasicConfiguration ( void );

        int LoadConfiguration ( wchar_t const* configFile );

        int LoadAndWatchConfiguration ( wchar_t const* configFile, long delay );

    protected:
        // This is being declared here as a pointer instead of the actual instance
        // to avoid creating a direct linkage between the callers of this class and
        // the log4cxx implemenetation.
        LoggerMap*      m_pMap;
//        SeverityMap     m_severity;
        std::wstring    m_configFile;
        unsigned int          m_configRefresh;
    };

EXTERN_C BSILOG_LOG4CXX_API ILogProvider* InstanceLoggingProvider( void );

EXTERN_C BSILOG_LOG4CXX_API DWORD log4cxx_createLog4cxxProvider(HANDLE* pProvider);

EXTERN_C BSILOG_LOG4CXX_API void log4cxx_destroyLog4cxxProvider(HANDLE* pProvider);

EXTERN_C BSILOG_LOG4CXX_API DWORD log4cxx_initializeLog4cxxProvider(HANDLE pProvider, wchar_t const* const configFile, int watchInterval);

}
#pragma warning(pop)

END_BENTLEY_LOGGING_NAMESPACE;