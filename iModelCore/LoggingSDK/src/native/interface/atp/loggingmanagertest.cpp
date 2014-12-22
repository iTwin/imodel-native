/*--------------------------------------------------------------------------------------+
|
|     $Source: src/native/interface/atp/loggingmanagertest.cpp $
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

TEST (LoggingManager, CreateAndDestroyCachedSUCCESS)
        {
        ILogger* pIL = LoggingManager::GetLogger(L"bsilogTest");
        CHECK ( NULL != pIL );

        CHECK ( SUCCESS == LoggingManager::ReleaseLogger(pIL) );
        }

TEST (LoggingManager, CreateAndDestroySUCCESS)
        {
        ILogger* pIL = LoggingManager::CreateUncachedLogger(L"bsilogTest");
        CHECK ( NULL != pIL );

        CHECK ( SUCCESS == LoggingManager::DestroyUncachedLogger(pIL) );
        }

TEST (LoggingManager, CreateAndDestroyCachedFAILURE)
        {
    START_EXCEPT(L"LoggingManager::GetLogger - should throw invalid argument");
    CHECK( NULL == LoggingManager::GetLogger((WCharP)NULL) );
    END_WITH_EXCEPT(L"LoggingManager::GetLogger - should throw invalid argument");

        START_EXCEPT(L"LoggingManager::ReleaseLogger - should throw invalid argument");
        CHECK ( ERROR == LoggingManager::ReleaseLogger(NULL) );
        END_WITH_EXCEPT(L"LoggingManager::ReleaseLogger - should throw invalid argument");
        }

TEST (LoggingManager, CreateAndDestroyFAILURE)
        {
        START_EXCEPT(L"LoggingManager::CreateUncachedLogger - should throw invalid argument");
        CHECK( NULL == LoggingManager::CreateUncachedLogger((WCharP)NULL) );
        END_WITH_EXCEPT(L"LoggingManager::CreateUncachedLogger - should throw invalid argument");

        START_EXCEPT(L"LoggingManager::CreateUncachedLogger - should throw invalid argument");
        CHECK( NULL == LoggingManager::DestroyUncachedLogger(NULL));
        END_WITH_EXCEPT(L"LoggingManager::CreateUncachedLogger - should throw invalid argument");
        }

TEST (LoggingManager, CreateAndDestroyCachedSUCCESSProvider)
        {
        CHECK ( SUCCESS == LoggingConfig::ActivateProvider(CONSOLE_LOGGING_PROVIDER) );

    ILogger* pIL = LoggingManager::GetLogger(L"bsilogTest");
        CHECK ( NULL != pIL );

        CHECK ( SUCCESS == LoggingManager::ReleaseLogger(pIL) );

        CHECK ( SUCCESS == LoggingConfig::DeactivateProvider() );
    }

TEST (LoggingManager, CreateAndDestroySUCCESSProvider)
        {
        CHECK ( SUCCESS == LoggingConfig::ActivateProvider(CONSOLE_LOGGING_PROVIDER) );

    ILogger* pIL = LoggingManager::CreateUncachedLogger(L"CreateAndDestroySUCCESSProvider");
        CHECK ( NULL != pIL );

        CHECK ( SUCCESS == LoggingManager::DestroyUncachedLogger(pIL) );

    CHECK ( SUCCESS == LoggingConfig::DeactivateProvider() );
        }

TEST (LoggingManager, CreateAndDestroyCachedFAILUREProvider)
        {
        CHECK ( SUCCESS == LoggingConfig::ActivateProvider(CONSOLE_LOGGING_PROVIDER) );

    START_EXCEPT(L"LoggingManager::GetLogger - should throw invalid argument");
    CHECK( NULL == LoggingManager::GetLogger((WCharP)NULL) );
    END_WITH_EXCEPT(L"LoggingManager::GetLogger - should throw invalid argument");

        START_EXCEPT(L"LoggingManager::ReleaseLogger - should throw invalid argument");
        CHECK ( ERROR == LoggingManager::ReleaseLogger(NULL) );
        END_WITH_EXCEPT(L"LoggingManager::ReleaseLogger - should throw invalid argument");

    CHECK ( SUCCESS == LoggingConfig::DeactivateProvider() );
        }

TEST (LoggingManager, CreateAndDestroyFAILUREProvider)
        {
        CHECK ( SUCCESS == LoggingConfig::ActivateProvider(CONSOLE_LOGGING_PROVIDER) );

    START_EXCEPT(L"LoggingManager::CreateUncachedLogger - should throw invalid argument");
        CHECK( NULL == LoggingManager::CreateUncachedLogger((WCharP)NULL) );
        END_WITH_EXCEPT(L"LoggingManager::CreateUncachedLogger - should throw invalid argument");

        START_EXCEPT(L"LoggingManager::CreateUncachedLogger - should throw invalid argument");
        CHECK( NULL == LoggingManager::DestroyUncachedLogger(NULL));
        END_WITH_EXCEPT(L"LoggingManager::CreateUncachedLogger - should throw invalid argument");

    CHECK ( SUCCESS == LoggingConfig::DeactivateProvider() );
        }

