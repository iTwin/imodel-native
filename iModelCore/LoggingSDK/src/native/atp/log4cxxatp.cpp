/*--------------------------------------------------------------------------------------+
|
|     $Source: src/native/atp/log4cxxatp.cpp $
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

wchar_t* getlogfilename ( LPCWSTR name );

TEST (Log4cxxLogger, configSUCCESS)
        {
        CHECK ( false == LoggingConfig::IsProviderActive() );

        CHECK ( SUCCESS == LoggingConfig::SetOption( CONFIG_OPTION_OUTPUT_FILE, getlogfilename(L"configSUCCESS" ) ) );

    CHECK ( SUCCESS == LoggingConfig::ActivateProvider(LOG4CXX_LOGGING_PROVIDER) );

        CHECK ( SUCCESS == LoggingConfig::DeactivateProvider() );
        }

TEST (Log4cxxLogger, configFAILURE)
        {
        CHECK ( false == LoggingConfig::IsProviderActive() );

        // For this provider, it will default to concole mode with no configuration and return success
        CHECK ( SUCCESS == LoggingConfig::ActivateProvider(LOG4CXX_LOGGING_PROVIDER) );

        CHECK ( SUCCESS == LoggingConfig::DeactivateProvider() );
        }

TEST (Log4cxxLogger, SetOptionSUCCESS)
        {
        CHECK ( false == LoggingConfig::IsProviderActive() );

        CHECK ( SUCCESS == LoggingConfig::SetOption( CONFIG_OPTION_DEFAULT_SEVERITY, LOG_TEXT_FATAL ) );
        CHECK ( SUCCESS == LoggingConfig::SetOption( CONFIG_OPTION_DEFAULT_SEVERITY, LOG_TEXT_ERROR ) );
        CHECK ( SUCCESS == LoggingConfig::SetOption( CONFIG_OPTION_DEFAULT_SEVERITY, LOG_TEXT_WARNING ) );
        CHECK ( SUCCESS == LoggingConfig::SetOption( CONFIG_OPTION_DEFAULT_SEVERITY, LOG_TEXT_INFO ) );
        CHECK ( SUCCESS == LoggingConfig::SetOption( CONFIG_OPTION_DEFAULT_SEVERITY, LOG_TEXT_DEBUG ) );
        CHECK ( SUCCESS == LoggingConfig::SetOption( CONFIG_OPTION_DEFAULT_SEVERITY, LOG_TEXT_TRACE ) );

        CHECK ( SUCCESS == LoggingConfig::SetOption( CONFIG_OPTION_OUTPUT_FILE, getlogfilename(L"SetOptionSUCCESS" ) ) );

    CHECK ( SUCCESS == LoggingConfig::ActivateProvider(LOG4CXX_LOGGING_PROVIDER) );

        CHECK ( SUCCESS == LoggingConfig::DeactivateProvider() );
        }

TEST (Log4cxxLogger, SetOptionFAILURE)
        {
        CHECK ( false == LoggingConfig::IsProviderActive() );

        CHECK ( SUCCESS == LoggingConfig::SetOption( CONFIG_OPTION_DEFAULT_SEVERITY, L"SOMEINVALIDVALUE" ) );
        CHECK ( SUCCESS == LoggingConfig::SetOption( L"SOMEINVALIDOPTION", L"SOMEVALUE" ) );

        CHECK ( SUCCESS == LoggingConfig::ActivateProvider(LOG4CXX_LOGGING_PROVIDER) );

        CHECK ( SUCCESS == LoggingConfig::DeactivateProvider() );
        }

TEST (Log4cxxLogger, SetSeveritySUCCESS)
        {
        CHECK ( false == LoggingConfig::IsProviderActive() );

        CHECK ( SUCCESS == LoggingConfig::SetOption( CONFIG_OPTION_OUTPUT_FILE, getlogfilename(L"SetSeveritySUCCESS" ) ) );

    CHECK ( SUCCESS == LoggingConfig::ActivateProvider(LOG4CXX_LOGGING_PROVIDER) );

        CHECK ( SUCCESS == LoggingConfig::SetSeverity( L"", LOG_FATAL ) );
        CHECK ( SUCCESS == LoggingConfig::SetSeverity( L"", LOG_ERROR ) );
        CHECK ( SUCCESS == LoggingConfig::SetSeverity( L"", LOG_WARNING ) );
        CHECK ( SUCCESS == LoggingConfig::SetSeverity( L"", LOG_INFO ) );
        CHECK ( SUCCESS == LoggingConfig::SetSeverity( L"", LOG_DEBUG ) );
        CHECK ( SUCCESS == LoggingConfig::SetSeverity( L"", LOG_TRACE ) );

        CHECK ( SUCCESS == LoggingConfig::DeactivateProvider() );
        }

TEST (Log4cxxLogger, SetSeverityFAILURE)
        {
        CHECK ( false == LoggingConfig::IsProviderActive() );

        CHECK ( SUCCESS == LoggingConfig::SetOption( CONFIG_OPTION_OUTPUT_FILE, getlogfilename(L"SetSeverityFAILURE" ) ) );

    CHECK ( SUCCESS == LoggingConfig::ActivateProvider(LOG4CXX_LOGGING_PROVIDER) );

        CHECK ( ERROR == LoggingConfig::SetSeverity( NULL, LOG_FATAL ) );

    CHECK ( ERROR == LoggingConfig::SetSeverity( L"", (SEVERITY)(LOG_FATAL+1) ) );
        CHECK ( ERROR == LoggingConfig::SetSeverity( L"", (SEVERITY)(LOG_TRACE-1) ) );

        CHECK ( ERROR == LoggingConfig::SetSeverity( L"", (SEVERITY)(45) ) );
        CHECK ( ERROR == LoggingConfig::SetSeverity( L"", (SEVERITY)(-35) ) );

        CHECK ( SUCCESS == LoggingConfig::DeactivateProvider() );
        }

TEST (Log4cxxLogger, SetMaxMessageSize)
        {
        CHECK ( false == LoggingConfig::IsProviderActive() );

        CHECK ( SUCCESS == LoggingConfig::SetOption( CONFIG_OPTION_OUTPUT_FILE, getlogfilename(L"SetMaxMessageSize" ) ) );

    CHECK ( SUCCESS == LoggingConfig::ActivateProvider(LOG4CXX_LOGGING_PROVIDER) );

        LoggingConfig::SetMaxMessageSize( 0 );
        CHECK ( 0 == LoggingConfig::SetMaxMessageSize( 256 ) );
        CHECK ( 256 == LoggingConfig::SetMaxMessageSize( 1024 ) );
        CHECK ( 1024 == LoggingConfig::SetMaxMessageSize( DEFAULT_MESSAGE_SIZE ) );

        CHECK ( SUCCESS == LoggingConfig::DeactivateProvider() );
        }


TEST (Log4cxxLogger, FatalMessage)
        {
        CHECK ( false == LoggingConfig::IsProviderActive() );

        CHECK ( SUCCESS == LoggingConfig::SetOption( CONFIG_OPTION_OUTPUT_FILE, getlogfilename(L"FatalMessage" ) ) );

    CHECK ( SUCCESS == LoggingConfig::ActivateProvider(LOG4CXX_LOGGING_PROVIDER) );

    ILogger* pLogger = LoggingManager::GetLogger( L"TEST" );

    CHECK ( NULL != pLogger );

    pLogger->fatal ( L"Some Message" );

        CHECK ( SUCCESS == LoggingConfig::DeactivateProvider() );
        }



