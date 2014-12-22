/*--------------------------------------------------------------------------------------+
|
|     $Source: src/managed/interop/atp/atp.cpp $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "cppunittest\TestHarness.h"
#include <windows.h>
#include <bentley.h>
#include <Logging/bentleylogging.h>

USING_NAMESPACE_CPPUNITTEST;
USING_NAMESPACE_BENTLEY_LOGGING;

TEST (LoggingConfig, configFAILURE)
        {
        CHECK ( false == LoggingConfig::IsProviderActive() );

        CHECK ( ERROR == LoggingConfig::ActivateProvider(NULL_LOGGING_PROVIDER) );

        CHECK ( false == LoggingConfig::IsProviderActive() );

        CHECK ( ERROR == LoggingConfig::DeactivateProvider() );
        }

TEST (LoggingConfig, configSUCCESS)
        {
        CHECK ( false == LoggingConfig::IsProviderActive() );

        CHECK ( SUCCESS == LoggingConfig::ActivateProvider(MANAGED_LOGGING_PROVIDER) );

        CHECK ( true == LoggingConfig::IsProviderActive() );

        CHECK ( SUCCESS == LoggingConfig::DeactivateProvider() );
        }


TEST (LoggingConfig, LogToAnyProvider)
    {
        CHECK ( false == LoggingConfig::IsProviderActive() );

    CHECK ( SUCCESS == LoggingConfig::ActivateProvider(MANAGED_LOGGING_PROVIDER) );

    ILogger* pLogger = LoggingManager::GetLogger( L"LogToAnyProvider" );
    CHECK ( NULL != pLogger );

    pLogger->fatal( L"Fatal Message" );
    pLogger->error( L"Error Message" );
    pLogger->warning( L"Warning Message" );
    pLogger->info( L"Information Message" );
    pLogger->debug(L"Debug Message" );
    pLogger->trace(L"Trace Message" );

        CHECK ( SUCCESS == LoggingConfig::DeactivateProvider() );
    }

TEST (LoggingConfig, SetMaxMessageSize)
        {
        CHECK ( false == LoggingConfig::IsProviderActive() );

        CHECK ( SUCCESS == LoggingConfig::ActivateProvider(MANAGED_LOGGING_PROVIDER) );

        CHECK ( true == LoggingConfig::IsProviderActive() );

        CHECK ( 3000 != LoggingConfig::SetMaxMessageSize ( 3000 ) );
        CHECK ( 3000 == LoggingConfig::SetMaxMessageSize ( 0 ) );
        CHECK ( 2147483647 != LoggingConfig::SetMaxMessageSize ( 2147483647 ) );

        CHECK ( SUCCESS == LoggingConfig::DeactivateProvider() );
        }

TEST (LoggingConfig, SetSeverityFatal)
    {
        CHECK ( false == LoggingConfig::IsProviderActive() );

    CHECK ( SUCCESS == LoggingConfig::ActivateProvider(MANAGED_LOGGING_PROVIDER) );

    ILogger* pLogger = LoggingManager::GetLogger( L"SetSeverityFatal" );
    CHECK ( NULL != pLogger );

    CHECK ( SUCCESS == LoggingConfig::SetSeverity ( L"SetSeverityFatal", LOG_FATAL ) );

    pLogger->fatal( L"Fatal Message" );
    pLogger->error( L"Error Message" );
    pLogger->warning( L"Warning Message" );
    pLogger->info( L"Information Message" );
    pLogger->debug(L"Debug Message" );
    pLogger->trace(L"Trace Message" );

        CHECK ( SUCCESS == LoggingConfig::DeactivateProvider() );
    }

TEST (LoggingConfig, SetSeverityError)
    {
        CHECK ( false == LoggingConfig::IsProviderActive() );

    CHECK ( SUCCESS == LoggingConfig::ActivateProvider(MANAGED_LOGGING_PROVIDER) );

    ILogger* pLogger = LoggingManager::GetLogger( L"SetSeverityError" );
    CHECK ( NULL != pLogger );

    CHECK ( SUCCESS == LoggingConfig::SetSeverity ( L"SetSeverityError", LOG_ERROR ) );

    pLogger->fatal( L"Fatal Message" );
    pLogger->error( L"Error Message" );
    pLogger->warning( L"Warning Message" );
    pLogger->info( L"Information Message" );
    pLogger->debug(L"Debug Message" );
    pLogger->trace(L"Trace Message" );

        CHECK ( SUCCESS == LoggingConfig::DeactivateProvider() );
    }

TEST (LoggingConfig, SetSeverityWarning)
    {
        CHECK ( false == LoggingConfig::IsProviderActive() );

    CHECK ( SUCCESS == LoggingConfig::ActivateProvider(MANAGED_LOGGING_PROVIDER) );

    ILogger* pLogger = LoggingManager::GetLogger( L"SetSeverityWarning" );
    CHECK ( NULL != pLogger );

    CHECK ( SUCCESS == LoggingConfig::SetSeverity ( L"SetSeverityWarning", LOG_WARNING ) );

    pLogger->fatal( L"Fatal Message" );
    pLogger->error( L"Error Message" );
    pLogger->warning( L"Warning Message" );
    pLogger->info( L"Information Message" );
    pLogger->debug(L"Debug Message" );
    pLogger->trace(L"Trace Message" );

        CHECK ( SUCCESS == LoggingConfig::DeactivateProvider() );
    }

TEST (LoggingConfig, SetSeverityInfo)
    {
        CHECK ( false == LoggingConfig::IsProviderActive() );

    CHECK ( SUCCESS == LoggingConfig::ActivateProvider(MANAGED_LOGGING_PROVIDER) );

    ILogger* pLogger = LoggingManager::GetLogger( L"SetSeverityInfo" );
    CHECK ( NULL != pLogger );

    CHECK ( SUCCESS == LoggingConfig::SetSeverity ( L"SetSeverityInfo", LOG_INFO ) );

    pLogger->fatal( L"Fatal Message" );
    pLogger->error( L"Error Message" );
    pLogger->warning( L"Warning Message" );
    pLogger->info( L"Information Message" );
    pLogger->debug(L"Debug Message" );
    pLogger->trace(L"Trace Message" );

        CHECK ( SUCCESS == LoggingConfig::DeactivateProvider() );
    }

TEST (LoggingConfig, SetSeverityDebug)
    {
        CHECK ( false == LoggingConfig::IsProviderActive() );

    CHECK ( SUCCESS == LoggingConfig::ActivateProvider(MANAGED_LOGGING_PROVIDER) );

    ILogger* pLogger = LoggingManager::GetLogger( L"SetSeverityDebug" );
    CHECK ( NULL != pLogger );

    CHECK ( SUCCESS == LoggingConfig::SetSeverity ( L"SetSeverityDebug", LOG_DEBUG ) );

    pLogger->fatal( L"Fatal Message" );
    pLogger->error( L"Error Message" );
    pLogger->warning( L"Warning Message" );
    pLogger->info( L"Information Message" );
    pLogger->debug(L"Debug Message" );
    pLogger->trace(L"Trace Message" );

        CHECK ( SUCCESS == LoggingConfig::DeactivateProvider() );
    }

TEST (LoggingConfig, SetSeverityTrace)
    {
        CHECK ( false == LoggingConfig::IsProviderActive() );

    CHECK ( SUCCESS == LoggingConfig::ActivateProvider(MANAGED_LOGGING_PROVIDER) );

    ILogger* pLogger = LoggingManager::GetLogger( L"SetSeverityTrace" );
    CHECK ( NULL != pLogger );

    CHECK ( SUCCESS == LoggingConfig::SetSeverity ( L"SetSeverityTrace", LOG_TRACE ) );

    pLogger->fatal( L"Fatal Message" );
    pLogger->error( L"Error Message" );
    pLogger->warning( L"Warning Message" );
    pLogger->info( L"Information Message" );
    pLogger->debug(L"Debug Message" );
    pLogger->trace(L"Trace Message" );

        CHECK ( SUCCESS == LoggingConfig::DeactivateProvider() );
    }

TEST (LoggingConfig, SetOptionDefaultLevelWarning)
    {
        CHECK ( false == LoggingConfig::IsProviderActive() );

    CHECK ( SUCCESS == LoggingConfig::ActivateProvider(MANAGED_LOGGING_PROVIDER) );

    ILogger* pLogger = LoggingManager::GetLogger( L"SetOptionDefaultLevelWarning" );
    CHECK ( NULL != pLogger );

    CHECK ( SUCCESS == LoggingConfig::SetOption ( CONFIG_OPTION_DEFAULT_SEVERITY, LOG_TEXT_WARNING ) );

    pLogger->fatal( L"Fatal Message" );
    pLogger->error( L"Error Message" );
    pLogger->warning( L"Warning Message" );
    pLogger->info( L"Information Message" );
    pLogger->debug(L"Debug Message" );
    pLogger->trace(L"Trace Message" );

        CHECK ( SUCCESS == LoggingConfig::DeactivateProvider() );
    }

