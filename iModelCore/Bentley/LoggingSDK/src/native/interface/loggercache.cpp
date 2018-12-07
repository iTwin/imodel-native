/*--------------------------------------------------------------------------------------+
|
|     $Source: LoggingSDK/src/native/interface/loggercache.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "bsilogprivate.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_LOGGING

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
LoggerCache::LoggerCache ( void )
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
LoggerCache::~LoggerCache ( void )
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
bool LoggerCache::IsLoggerCached
(
WCharCP nameSpace
)
    {
    if ( NULL == nameSpace )
        {
        throw std::invalid_argument ( "nameSpace" );
        }

    BeMutexHolder lock(m_lock);

    LoggerMap::const_iterator it = m_loggers.find ( nameSpace );

    bool retValue = true;

    if ( it == m_loggers.end() )
        {
        retValue = false;
        }

    return retValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
bool LoggerCache::IsLoggerCached
(
ILogger* pLogger
)
    {
    if ( NULL == pLogger )
        {
        throw std::invalid_argument ( "pLogger" );
        }

    BeMutexHolder lock(m_lock);

    bool retValue = false;

    for ( LoggerMap::iterator it = m_loggers.begin() ; it != m_loggers.end() ; it++ )
        {
        if ( (*it).second == pLogger )
            {
            retValue = true;
            break;
            }
        }

    return retValue;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
ILogger* LoggerCache::CreateLogger
(
WCharCP nameSpace
)
    {
    Provider::ILogProviderContext * context = NULL;

    if ( NULL == nameSpace )
        {
        throw std::invalid_argument ( "nameSpace" );
        }

    GetLogProvider().CreateLogger ( nameSpace, &context );

    BsiLogger* pLog = new BsiLogger ( nameSpace, context );

    return pLog;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
int LoggerCache::DestroyLogger
(
ILogger* pLogger
)
    {
    if ( NULL == pLogger )
        {
        throw std::invalid_argument ( "pLogger" );
        }

    BsiLogger* pLog = dynamic_cast<BsiLogger*>(pLogger);

    if ( NULL == pLog )
        {
        throw std::invalid_argument ( "pLogger" );
        }

    // Let the provider know we're destroying the logger
    GetLogProvider().DestroyLogger ( pLog->getContext() );

    // Remove the logger from the cache
    BeAssert ( false == GetLoggerCache().IsLoggerCached(pLogger) );

    GetLoggerCache().RemoveLogger ( pLogger );

    delete pLog;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
ILogger* LoggerCache::GetLogger
(
WCharCP nameSpace
)
    {
    if ( NULL == nameSpace )
        {
        throw std::invalid_argument ( "nameSpace" );
        }

    BeMutexHolder lock(m_lock);

    LoggerMap::const_iterator it = m_loggers.find ( nameSpace );

    ILogger*    pLogger = NULL;

    if ( it == m_loggers.end() )
        {
        pLogger = CreateLogger ( nameSpace );

        m_loggers.insert ( LoggerMap::value_type(nameSpace,pLogger) );
        }
    else
        {
        pLogger = (*it).second;
        }

    return pLogger;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
ILogger* LoggerCache::RemoveLogger
(
WCharCP nameSpace
)
    {
    if ( NULL == nameSpace )
        {
        throw std::invalid_argument ( "nameSpace" );
        }

    BeMutexHolder lock(m_lock);

    LoggerMap::iterator it = m_loggers.find ( nameSpace );

    ILogger*    pLogger = NULL;

    if ( it != m_loggers.end() )
        {
        pLogger = (*it).second;

        m_loggers.erase ( it );
        }

    return pLogger;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
int LoggerCache::RemoveLogger
(
ILogger* pLogger
)
    {
    if ( NULL == pLogger )
        {
        throw std::invalid_argument ( "pLogger" );
        }

    BeMutexHolder lock(m_lock);

    for ( LoggerMap::iterator it = m_loggers.begin() ; it != m_loggers.end() ; it++ )
        {
        if ( (*it).second == pLogger )
            {
            m_loggers.erase ( it );
            break;
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
void LoggerCache::RemoveProvider
(
Provider::ILogProvider* pProvider
)
    {
    if ( NULL == pProvider )
        {
        throw std::invalid_argument ( "pProvider" );
        }

    BeMutexHolder lock(m_lock);

    for ( LoggerMap::iterator it = m_loggers.begin() ; it != m_loggers.end() ; it++ )
        {
        BsiLogger*  pLog = dynamic_cast<BsiLogger*>((*it).second);

        pProvider->DestroyLogger ( pLog->getContext() );

        pLog->setContext ( NULL );
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
int LoggerCache::AddProvider
(
Provider::ILogProvider* pProvider
)
    {
    if ( NULL == pProvider )
        {
        throw std::invalid_argument ( "pProvider" );
        }

    Provider::ILogProviderContext* pContext = NULL;

    BeMutexHolder lock(m_lock);

    for ( LoggerMap::iterator it = m_loggers.begin() ; it != m_loggers.end() ; it++ )
        {
        BsiLogger*  pLog = dynamic_cast<BsiLogger*>((*it).second);

        pProvider->CreateLogger ( pLog->getNamespace().c_str(), &pContext );

        pLog->setContext ( pContext );
        }

    return SUCCESS;
    }


