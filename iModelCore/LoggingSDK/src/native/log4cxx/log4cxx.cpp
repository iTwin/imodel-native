/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma warning ( disable : 4786 )


#include "PublicAPI/log4cxx.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <map>

#include <log4cxx/logger.h>
#include <log4cxx/logmanager.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/xml/domconfigurator.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/helpers/filewatchdog.h>
#include <log4cxx/fileappender.h>
#include <log4cxx/ndc.h>
#include <log4cxx/mdc.h>
#include "utf8/checked.h"

#ifdef ANDROID
#include "CustomAppenders/androidlogprintappender.h"
#endif

using namespace log4cxx;
using namespace log4cxx::helpers;

BEGIN_BENTLEY_LOGGING_NAMESPACE;

namespace Provider {

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::map<ILogProviderContext *, log4cxx::Logger*> LoggerContextMap;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
class LoggerMap
    {
    protected:
        LoggerContextMap    m_map;

    public:
        LoggerMap ( void ) {};;

        virtual ~LoggerMap ( void )
            {
//            for ( LoggerContextMap::iterator it = m_map.begin() ; it != m_map.end() ; it++ )
//                {
//                Logger* pLogger = (*it).second;
//
//                pLogger->releaseRef();
//                }
            };

        ILogProviderContext * getProviderContext ( Logger* pLogger )
            {
            assert ( NULL != pLogger );
            if (NULL == pLogger)
                return NULL;
            return reinterpret_cast<ILogProviderContext *>(pLogger);
            };

        Logger* getLogger ( const wchar_t* nameSpace )
            {
            Logger* pLogger = Logger::getLogger ( nameSpace );

            assert ( NULL != pLogger );

            if ( NULL != pLogger )
                {
                m_map.insert ( LoggerContextMap::value_type(getProviderContext(pLogger),pLogger) );
                }

            return pLogger;
            };

        Logger* getLogger ( ILogProviderContext * logContext )
            {
            LoggerContextMap::iterator it = m_map.find(logContext);

            Logger* pLogger = NULL;

            if ( it != m_map.end() )
                {
                pLogger = (*it).second;
                }

            assert ( NULL != pLogger );

            return pLogger;
            };

    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
const LevelPtr translateSeverity
(
SEVERITY    sev
)
    {
    switch ( sev )
        {
        case LOG_FATAL:
            {
            return Level::getFatal();
            break;
            }
        case LOG_ERROR:
            {
            return Level::getError();
            break;
            }
        case LOG_WARNING:
            {
            return Level::getWarn();
            break;
            }
        case LOG_INFO:
            {
            return Level::getInfo();
            break;
            }
        case LOG_DEBUG:
            {
            return Level::getDebug();
            break;
            }
        case LOG_TRACE:
            {
            return Level::getTrace();
            break;
            }
        }

    return Level::getAll();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
Log4cxxProvider::Log4cxxProvider
(
const wchar_t* name
) : m_pMap ( NULL ), m_configRefresh(0)
    {
#ifdef ANDROID
    // Shared library linking might use --no-whole-archive to strip unused symbols, thus making
    // AndroidLogPrintAppender not register statically, as this is common issue with such linking
    // and classes that register to frameworks statically. Force registration here.
    AndroidLogPrintAppender::registerClass();
#endif

    m_pMap = new LoggerMap;
    assert ( NULL != m_pMap );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
Log4cxxProvider::~Log4cxxProvider ( void )
    {
    Uninitialize();
    assert ( NULL != m_pMap );
    delete m_pMap;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
int __stdcall Log4cxxProvider::Initialize ( void )
    {
    int status = SUCCESS;

    if ( 0 != m_configRefresh && m_configFile.length() > 0 )
        {
        status = LoadAndWatchConfiguration ( m_configFile.c_str(), m_configRefresh );
        }
    else if ( m_configFile.length() > 0 )
        {
        status = LoadConfiguration ( m_configFile.c_str() );
        }
    else
        {
        status = BasicConfiguration();
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
int __stdcall Log4cxxProvider::Uninitialize ( void )
    {
    LogManager::shutdown();
    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
int Log4cxxProvider::BasicConfiguration ( void )
    {
    BasicConfigurator::configure();
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
int Log4cxxProvider::LoadConfiguration
(
const wchar_t* configFile
)
    {
    if ( NULL == configFile || L'\0' == configFile[0] )
        {
        throw std::invalid_argument ( "configFile" );
        }

#ifdef BENTLEY_WIN32
    assert ( !IsBadStringPtr ( configFile, 256 ) );
#endif

    try
        {
        xml::DOMConfigurator::configure(configFile);
        }
    catch ( log4cxx::helpers::Exception& )
        {
        return BSIERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
int Log4cxxProvider::LoadAndWatchConfiguration
(
wchar_t const* configFile,
long    delay
)
    {
    if ( NULL == configFile || L'\0' == configFile[0] )
        {
        throw std::invalid_argument ( "configFile" );
        }

#ifdef BENTLEY_WIN32
    assert ( !IsBadStringPtr ( configFile, 256 ) );
#endif
    assert ( DEFAULT_DELAY == helpers::FileWatchdog::DEFAULT_DELAY );
    try
        {
        xml::DOMConfigurator::configureAndWatch(configFile,delay);
        }
    catch ( log4cxx::helpers::Exception& )
        {
        return BSIERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* get FileAppender output log filenames
* @bsimethod                                                    MichaelMcCarty  10/05
+---------------+---------------+---------------+---------------+---------------+------*/
int Log4cxxProvider::GetLogfileNames
(
std::vector<std::wstring>&  fileNames
)
    {
    LoggerPtr logger = Logger::getRootLogger();
    if (LoggerPtr(0) == logger)
        return -1;

    AppenderList appenders = logger->getAllAppenders();

    for (AppenderList::const_iterator it = appenders.begin();
            appenders.end() != it;
            it++)
        {
        Appender const* appender = *it;
        FileAppender const* fileAppender = dynamic_cast<FileAppender const*> (appender);
        if (NULL != fileAppender)
            {
            fileNames.push_back (fileAppender->getFile());
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
void Log4cxxProvider::PushThreadContext
(
const wchar_t* context
)
    {
    NDC::push(context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
void Log4cxxProvider::PopThreadContext ( void )
    {
    NDC::pop();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
void Log4cxxProvider::ClearThreadContext ( void )
    {
    NDC::remove();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
void Log4cxxProvider::AddContext
(
const wchar_t* key,
const wchar_t* context
)
    {
    MDC::put(key,context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
void Log4cxxProvider::RemoveContext
(
const wchar_t* key
)
    {
    MDC::remove(key);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
void Log4cxxProvider::ClearContext ( void )
    {
    MDC::clear();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
int __stdcall Log4cxxProvider::CreateLogger
(
const wchar_t* nameSpace,
ILogProviderContext** ppContext
)
    {
    assert ( NULL != ppContext );
    assert ( NULL != m_pMap );
    if (NULL == m_pMap)
        return ERROR;

    LoggerPtr logger = m_pMap->getLogger ( nameSpace );

    assert ( NULL != logger );

    *ppContext = m_pMap->getProviderContext ( logger );

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
int __stdcall Log4cxxProvider::DestroyLogger
(
ILogProviderContext * context
)
    {
    // Because of the log4cxx implementation we don't have to do anything here.
    // Just do an assert to catch any stupid programmer tricks.

    assert ( NULL != context );

    assert ( NULL != m_pMap && NULL != m_pMap->getLogger ( context ) );

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
void __stdcall Log4cxxProvider::LogMessage
(
ILogProviderContext *  context,
SEVERITY            sev,
const wchar_t*             msg
)
    {
    assert ( NULL != context );
    assert ( NULL != msg );

    assert ( NULL != m_pMap );
    if (NULL == m_pMap)
        return;

    LoggerPtr logger = m_pMap->getLogger ( context );

    assert ( NULL != logger );
    if (NULL == logger)
        return;

    logger->log ( translateSeverity(sev), msg );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void __stdcall Log4cxxProvider::LogMessage
(
ILogProviderContext * context,
SEVERITY sev,
Utf8CP msg
)
    {
    const wchar_t* converted = nullptr;
    int length = (int)strlen (msg); 
    
#if defined (BENTLEY_WIN32)
    int outStrCount = ::MultiByteToWideChar (CP_UTF8, 0, msg, length, NULL, 0);
    std::wstring outStr; outStr.resize (outStrCount); // (allocates space for outStrCount + 1 wchar_t's)
    ::MultiByteToWideChar (CP_UTF8, 0, msg, length, &outStr[0], outStrCount);
    outStr[outStrCount] = '\0';
    converted = outStr.c_str();
#else
    std::vector<char32_t> outBuff;
    try 
        {
        utf8::utf8to32(msg, (msg + length), std::back_inserter(outBuff));
        }
    catch (utf8::exception e)
        {
        std::wstring error(L"LOG4CXX PROVIDER - UTF8 CONVERSION FAILED");
        outBuff.insert(outBuff.end(), error.begin(), error.end());
        }
    std::wstring ws(outBuff.begin(), outBuff.end());
    converted = ws.c_str();
#endif

    LogMessage(context, sev, converted);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
bool __stdcall Log4cxxProvider::IsSeverityEnabled
(
ILogProviderContext *  context,
SEVERITY            sev
)
    {
    assert ( NULL != context );
    if (NULL == context)
        return false;

    assert ( NULL != m_pMap );
    if (NULL == m_pMap)
        return false;

    LoggerPtr logger = m_pMap->getLogger ( context );

    assert ( NULL != logger );
    if (NULL == logger)
        return false;

    return logger->isEnabledFor ( translateSeverity(sev) );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
int __stdcall Log4cxxProvider::SetSeverity
(
const wchar_t*     nameSpace,
SEVERITY    sev
)
    {
    assert ( NULL != m_pMap );

    LoggerPtr logger = m_pMap->getLogger ( nameSpace );

    assert ( logger != NULL );

    logger->setLevel(translateSeverity(sev));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
int __stdcall Log4cxxProvider::SetOption
(
const wchar_t* attribName,
const wchar_t* attribValue
)
    {
    assert ( NULL != attribName );
    assert ( NULL != attribValue );

//    if ( 0 == wcscmp ( attribName, CONFIG_OPTION_DEFAULT_SEVERITY ) )
//        {
//        m_severity.SetDefaultSeverity ( GetSeverityFromText(attribValue) );
//        }
//    else
    if ( 0 == wcscmp ( attribName, CONFIG_OPTION_CONFIG_FILE ) )
        {
        m_configFile = attribValue;
        }
    else if ( 0 == wcscmp ( attribName, CONFIG_OPTION_CONFIG_REFRESH ) )
        {
        m_configRefresh = wcstol ( attribValue, NULL, 10 );
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
int __stdcall Log4cxxProvider::GetOption
(
const wchar_t* attribName,
wchar_t*  attribValue,
unsigned int  valueSize
)
    {
    assert ( NULL != attribName );
    assert ( NULL != attribValue );

    if (NULL == attribName)
        return ERROR;
    if (NULL == attribValue)
        return ERROR;

    if ( 0 == wcscmp ( attribName, CONFIG_OPTION_CONFIG_FILE ) )
        {
        wcsncpy ( attribValue,  m_configFile.c_str(), valueSize-1 );
        }
    else if ( 0 == wcscmp ( attribName, CONFIG_OPTION_CONFIG_REFRESH ) )
        {
        wprintf( attribValue, "%d", m_configRefresh );
        }
    else if ( 0 == wcscmp ( attribName, CONFIG_OPTION_OUTPUT_FILE ) )
        {
        std::vector<std::wstring> fileNames;

        if ( SUCCESS != GetLogfileNames(fileNames) )
            {
            return ERROR;
            }

        if ( fileNames.empty() )
            {
            return ERROR;
            }

        wcsncpy ( attribValue, (*fileNames.begin()).c_str(), valueSize-1 );
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
ILogProvider* InstanceLoggingProvider( void )
    {
    return new Log4cxxProvider();
    }

}

END_BENTLEY_LOGGING_NAMESPACE;
