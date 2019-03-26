/*--------------------------------------------------------------------------------------+
|
|     $Source: src/native/interface/atp/bsilogTest.cpp $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <cppunittest/TestHarness.h>
#include <windows.h>
#include <bentley/bentley.h>
#include <Logging/bentleylogging.h>

USING_NAMESPACE_CPPUNITTEST;
USING_NAMESPACE_BENTLEY_LOGGING;

TEST (LoggerConfig, configFAILURE)
        {
        CHECK ( false == LoggingConfig::IsProviderActive() );

        START_EXCEPT(L"LoggingConfig::ActivateProvider - should throw invalid argument");
        CHECK ( ERROR == LoggingConfig::ActivateProvider(NULL) );
        END_WITH_EXCEPT(L"LoggingConfig::ActivateProvider - should throw invalid argument");

        CHECK ( false == LoggingConfig::IsProviderActive() );

        CHECK ( 3000 != LoggingConfig::SetMaxMessageSize ( 3000 ) );
        CHECK ( 3000 == LoggingConfig::SetMaxMessageSize ( 0 ) );
        CHECK ( 2147483647 != LoggingConfig::SetMaxMessageSize ( 2147483647 ) );

        CHECK ( ERROR == LoggingConfig::DeactivateProvider() );
        }

TEST (LoggerConfig, configSUCCESS)
        {
        CHECK ( false == LoggingConfig::IsProviderActive() );

//      HANDLE* pProvider = new HANDLE;
//      Provider::ILogProvider* provider = reinterpret_cast<Provider::ILogProvider*>(pProvider);
//      CHECK ( SUCCESS == LoggerConfig::registerLogInterface(provider) );

//      CHECK ( true == LoggerConfig::isInterfaceRegistered() );

//      CHECK ( 3000 != LoggerConfig::setMaxMessageSize ( 3000 ) );
//      CHECK ( 3000 == LoggerConfig::setMaxMessageSize ( 0 ) );
//      CHECK ( 2147483647 != LoggerConfig::setMaxMessageSize ( 2147483647 ) );

//      CHECK ( SUCCESS == LoggerConfig::unregisterLogInterface() );

//      delete pProvider;
        }

// These have all been deprecated

//TEST (LoggerFactory, CreateAndDestroySUCCESS)
//        {
//        ILogger* pIL = LoggerFactory::createLogger(L"bsilogTest");
//        CHECK ( NULL != pIL );
//
//        CHECK ( SUCCESS == LoggerFactory::destroyLogger(pIL) );
//        }
//
//TEST (LoggerFactory, CreateAndDestroyFAILURE)
//        {
//    START_EXCEPT(L"LoggingManager::CreateUncachedLogger - should throw invalid argument");
//    CHECK( NULL == LoggingManager::CreateUncachedLogger(NULL) );
//    END_WITH_EXCEPT(L"LoggingManager::CreateUncachedLogger - should throw invalid argument");
//
//        START_EXCEPT(L"LoggingManager::DestroyUncachedLogger - should throw invalid argument");
//        CHECK ( ERROR == LoggingManager::DestroyUncachedLogger(NULL) );
//        END_WITH_EXCEPT(L"LoggingManager::DestroyUncachedLogger - should throw invalid argument");
//        }
//
//TEST (LoggerRegistry, CreateAndDestroyFAILURE)
//        {
//        START_EXCEPT(L"LoggingManager::GetLogger - should throw invalid argument");
//        CHECK( NULL == LoggingManager::GetLogger(NULL) );
//        END_WITH_EXCEPT(L"LoggingManager::GetLogger - should throw invalid argument");
//
//        START_EXCEPT(L"LoggerRegistry::removeLogger - should throw invalid argument");
//        CHECK( NULL == LoggerRegistry::removeLogger(NULL));
//        END_WITH_EXCEPT(L"LoggerRegistry::removeLogger - should throw invalid argument");
//        }
//
//TEST (LoggerRegistry, CreateAndDestroySUCCESS)
//        {
//        LPCWSTR nameSpace = L"bsilogTest";
//        ILogger* pIL = LoggerRegistry::getLogger(nameSpace);
//        CHECK ( NULL != pIL );
//
//        pIL = LoggerRegistry::removeLogger(nameSpace);
//    // This API has a flwed design should never return the allocated logger to the caller
//    // CHECK ( NULL != pIL );
//    CHECK ( NULL == pIL ); // New design
//        }
//
//TEST (BSILOG_API, CreateAndDestroyFAILURE)
//        {
//        START_EXCEPT(L"BSILOG_API - shoud not throw any exceptions");
//
//        LPCWSTR nameSpace = L"BSILOG_APITest";
//
//        CHECK ( ERROR == logger_createLogger(nameSpace, NULL) );
//        CHECK ( ERROR == logger_destroyLogger(NULL) );
//        CHECK ( ERROR == logger_getLogger(nameSpace, NULL) );
//        CHECK ( ERROR == logger_removeLogger(NULL) );
//
//        CHECK ( 3000 != logger_setMaxMessageSize ( 3000 ) );
//        CHECK ( 3000 == logger_setMaxMessageSize ( 0 ) );
//        CHECK ( 2147483647 != logger_setMaxMessageSize ( 2147483647 ) );
//
//        CHECK ( false == logger_isLoggingEnabled() );
//
//        END_NO_EXCEPT(L"END BSILOG_API - shoud not throw any exceptions");
//        }
//
//TEST (BSILOG_API, CreateAndDestroySUCCESS)
//        {
//        START_EXCEPT(L"BSILOG_API - shoud not throw any exceptions");
//
//        LPCWSTR nameSpace = L"BSILOG_APITest";
//        HANDLE pHandle = {0};
//
//        CHECK ( SUCCESS == logger_createLogger(nameSpace, &pHandle) );
//        CHECK ( NULL != pHandle );
//
//        CHECK ( SUCCESS == logger_destroyLogger(&pHandle) );
//        CHECK ( NULL == pHandle );
//
//        CHECK ( SUCCESS == logger_getLogger(nameSpace, &pHandle) );
//
//        CHECK ( 3000 != logger_setMaxMessageSize ( 3000 ) );
//        CHECK ( 3000 == logger_setMaxMessageSize ( 0 ) );
//        CHECK ( 2147483647 != logger_setMaxMessageSize ( 2147483647 ) );
//
//        CHECK ( SUCCESS == logger_removeLogger(nameSpace) );
//
//        CHECK ( SUCCESS == logger_isLoggingEnabled() );
//
//        END_NO_EXCEPT(L"END BSILOG_API - shoud not throw any exceptions");
//        }
//
//TEST (BSILOG_API, Provider)
//        {
//        START_EXCEPT(L"BSILOG_API - shoud not throw any exceptions");
//
//        CHECK ( ERROR == logger_registerProvider(NULL));
//
//        END_NO_EXCEPT(L"END BSILOG_API - shoud not throw any exceptions");
//        }
//
//TEST (BSILOG_API, Message)
//        {
//        START_EXCEPT(L"BSILOG_API - shoud not throw any exceptions");
//
//        LPCWSTR nameSpace = L"BSILOG_APITest";
//        HANDLE pHandle = {0};
//        LPCWSTR msg = L"Message from BSILOG_API Message test.";
//
//        CHECK ( SUCCESS == logger_createLogger( nameSpace, &pHandle ) );
//
//        logger_message( NULL, LOG_FATAL, msg );
//        logger_message( pHandle, (SEVERITY)-10000, msg );
//        logger_message( pHandle, LOG_FATAL, NULL );
//
//        logger_messagev( NULL, LOG_FATAL, msg, L"%d is an integer", 10 );
//        logger_messagev( pHandle, (SEVERITY)-10000, msg, L"%d is an integer", 10 );
//        logger_messagev( pHandle, LOG_FATAL, NULL, L"%d is an integer", 10 );
//
//        logger_messageva( NULL, LOG_FATAL, msg, NULL );
//        logger_messageva( pHandle, (SEVERITY)-10000, msg, NULL );
//        logger_messageva( pHandle, LOG_FATAL, NULL, NULL );
//
//        CHECK ( SUCCESS == logger_destroyLogger(&pHandle));
//
//        END_NO_EXCEPT(L"END BSILOG_API - shoud not throw any exceptions");
//        }
//
//TEST (BSILOG_API, Fatal)
//        {
//        START_EXCEPT(L"BSILOG_API - shoud not throw any exceptions");
//
//        LPCWSTR nameSpace = L"BSILOG_APITest";
//        HANDLE pHandle = {0};
//
//        CHECK ( SUCCESS == logger_createLogger( nameSpace, &pHandle ) );
//
//        logger_fatal ( NULL, NULL );
//        logger_fatal ( pHandle, NULL);
//        logger_fatalv ( NULL, NULL, NULL );
//        logger_fatalv ( pHandle, NULL, NULL );
//
//        CHECK ( SUCCESS == logger_destroyLogger(&pHandle));
//
//        END_NO_EXCEPT(L"END BSILOG_API - shoud not throw any exceptions");
//        }
//
//TEST (BSILOG_API, Error)
//        {
//        START_EXCEPT(L"BSILOG_API - shoud not throw any exceptions");
//
//        LPCWSTR nameSpace = L"BSILOG_APITest";
//        HANDLE pHandle = {0};
//
//        CHECK ( SUCCESS == logger_createLogger( nameSpace, &pHandle ) );
//
//        logger_error ( NULL, NULL );
//        logger_error ( pHandle, NULL);
//        logger_errorv ( NULL, NULL, NULL );
//        logger_errorv ( pHandle, NULL, NULL );
//
//        CHECK ( SUCCESS == logger_destroyLogger(&pHandle));
//
//        END_NO_EXCEPT(L"END BSILOG_API - shoud not throw any exceptions");
//        }
//
//TEST (BSILOG_API, Warning)
//        {
//        START_EXCEPT(L"BSILOG_API - shoud not throw any exceptions");
//
//        LPCWSTR nameSpace = L"BSILOG_APITest";
//        HANDLE pHandle = {0};
//
//        CHECK ( SUCCESS == logger_createLogger( nameSpace, &pHandle ) );
//
//        logger_warning ( NULL, NULL );
//        logger_warning ( pHandle, NULL);
//        logger_warningv ( NULL, NULL, NULL );
//        logger_warningv ( pHandle, NULL, NULL );
//
//        CHECK ( SUCCESS == logger_destroyLogger(&pHandle));
//
//        END_NO_EXCEPT(L"END BSILOG_API - shoud not throw any exceptions");
//        }
//
//TEST (BSILOG_API, Info)
//        {
//        START_EXCEPT(L"BSILOG_API - shoud not throw any exceptions");
//
//        HANDLE pHandle = {0};
//
//        LPCWSTR nameSpace = L"BSILOG_APITest";
//        CHECK ( SUCCESS == logger_createLogger( nameSpace, &pHandle ) );
//
//        logger_info ( NULL, NULL );
//        logger_info ( pHandle, NULL);
//        logger_infov ( NULL, NULL, NULL );
//        logger_infov ( pHandle, NULL, NULL );
//
//        CHECK ( SUCCESS == logger_destroyLogger(&pHandle));
//
//        END_NO_EXCEPT(L"END BSILOG_API - shoud not throw any exceptions");
//        }
//
//TEST (BSILOG_API, Debug)
//        {
//        START_EXCEPT(L"BSILOG_API - shoud not throw any exceptions");
//
//        HANDLE pHandle = {0};
//
//        LPCWSTR nameSpace = L"BSILOG_APITest";
//        CHECK ( SUCCESS == logger_createLogger( nameSpace, &pHandle ) );
//
//        logger_debug ( NULL, NULL );
//        logger_debug ( pHandle, NULL);
//        logger_debugv ( NULL, NULL, NULL );
//        logger_debugv ( pHandle, NULL, NULL );
//
//        CHECK ( SUCCESS == logger_destroyLogger(&pHandle));
//
//        END_NO_EXCEPT(L"END BSILOG_API - shoud not throw any exceptions");
//        }
//
//TEST (BSILOG_API, Trace)
//        {
//        START_EXCEPT(L"BSILOG_API - shoud not throw any exceptions");
//
//        LPCWSTR nameSpace = L"BSILOG_APITest";
//        HANDLE pHandle = {0};
//
//        CHECK ( SUCCESS == logger_createLogger( nameSpace, &pHandle ) );
//
//        logger_trace ( NULL, NULL );
//        logger_trace ( pHandle, NULL);
//        logger_tracev ( NULL, NULL, NULL );
//        logger_tracev ( pHandle, NULL, NULL );
//
//        CHECK ( SUCCESS == logger_destroyLogger(&pHandle));
//
//        END_NO_EXCEPT(L"END BSILOG_API - shoud not throw any exceptions");
//        }


