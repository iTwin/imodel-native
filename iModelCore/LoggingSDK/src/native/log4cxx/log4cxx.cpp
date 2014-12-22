/*--------------------------------------------------------------------------------------+
|
|     $Source: logging/native/log4cxx/log4cxx.cpp $
|
|  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma warning ( disable : 4786 )


#include "log4cxx.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <bentley/bentley.h>

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
            return reinterpret_cast<ILogProviderContext *>(pLogger);
            };

        Logger* getLogger ( LPCWSTR nameSpace )
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
LPCWSTR name
) : m_pMap ( NULL ), m_configRefresh(0)
    {
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
LPCWSTR configFile
)
    {
    if ( NULL == configFile || NULL == configFile[0] )
        {
        throw std::invalid_argument ( "configFile" );
        }

    assert ( !IsBadStringPtr ( configFile, 256 ) );

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
LPCWSTR configFile,
long    delay
)
    {
    if ( NULL == configFile || NULL == configFile[0] )
        {
        throw std::invalid_argument ( "configFile" );
        }

    assert ( !IsBadStringPtr ( configFile, 256 ) );
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
    if (LoggerPtr(NULL) == logger)
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
LPCWSTR context
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
LPCWSTR key,
LPCWSTR context
)
    {
    MDC::put(key,context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
void Log4cxxProvider::RemoveContext
(
LPCWSTR key
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
LPCWSTR nameSpace,
ILogProviderContext** ppContext
)
    {
    assert ( NULL != ppContext );
    assert ( NULL != m_pMap );

    LoggerPtr logger = m_pMap->getLogger ( nameSpace );

    assert ( logger != NULL );

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
LPCWSTR             msg
)
    {
    assert ( NULL != context );
    assert ( NULL != msg );

    assert ( NULL != m_pMap );

    LoggerPtr logger = m_pMap->getLogger ( context );

    assert ( logger != NULL );

    logger->log ( translateSeverity(sev), msg );
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

    assert ( NULL != m_pMap );

    LoggerPtr logger = m_pMap->getLogger ( context );

    assert ( logger != NULL );

    return logger->isEnabledFor ( translateSeverity(sev) );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
int __stdcall Log4cxxProvider::SetSeverity
(
LPCWSTR     nameSpace,
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
LPCWSTR attribName,
LPCWSTR attribValue
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
        m_configRefresh = _wtol ( attribValue );
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
LPCWSTR attribName,
LPWSTR  attribValue,
unsigned int  valueSize
)
    {
    assert ( NULL != attribName );
    assert ( NULL != attribValue );

    if ( 0 == wcscmp ( attribName, CONFIG_OPTION_CONFIG_FILE ) )
        {
        wcsncpy ( attribValue,  m_configFile.c_str(), valueSize-1 );
        }
    else if ( 0 == wcscmp ( attribName, CONFIG_OPTION_CONFIG_REFRESH ) )
        {
        _ltow ( m_configRefresh, attribValue, 10 );
        }
    else if ( 0 == wcscmp ( attribName, CONFIG_OPTION_OUTPUT_FILE ) )
        {
        std::vector<std::wstring> fileNames;

        if ( SUCCESS != GetLogfileNames(fileNames) && fileNames.begin() != fileNames.end() )
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