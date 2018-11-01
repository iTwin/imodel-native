/*--------------------------------------------------------------------------------------+
|
|     $Source: src/native/atp/consoleatp.cpp $
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

TEST (ConsoleLogger, configSUCCESS)
        {
        CHECK ( false == LoggingConfig::IsProviderActive() );

        CHECK ( SUCCESS == LoggingConfig::ActivateProvider(CONSOLE_LOGGING_PROVIDER) );

        CHECK ( SUCCESS == LoggingConfig::DeactivateProvider() );
        }

TEST (ConsoleLogger, SetOptionSUCCESS)
        {
        CHECK ( false == LoggingConfig::IsProviderActive() );

        CHECK ( SUCCESS == LoggingConfig::SetOption( CONFIG_OPTION_DEFAULT_SEVERITY, LOG_TEXT_FATAL ) );
        CHECK ( SUCCESS == LoggingConfig::SetOption( CONFIG_OPTION_DEFAULT_SEVERITY, LOG_TEXT_ERROR ) );
        CHECK ( SUCCESS == LoggingConfig::SetOption( CONFIG_OPTION_DEFAULT_SEVERITY, LOG_TEXT_WARNING ) );
        CHECK ( SUCCESS == LoggingConfig::SetOption( CONFIG_OPTION_DEFAULT_SEVERITY, LOG_TEXT_INFO ) );
        CHECK ( SUCCESS == LoggingConfig::SetOption( CONFIG_OPTION_DEFAULT_SEVERITY, LOG_TEXT_DEBUG ) );
        CHECK ( SUCCESS == LoggingConfig::SetOption( CONFIG_OPTION_DEFAULT_SEVERITY, LOG_TEXT_TRACE ) );

        CHECK ( SUCCESS == LoggingConfig::ActivateProvider(CONSOLE_LOGGING_PROVIDER) );

        CHECK ( SUCCESS == LoggingConfig::DeactivateProvider() );
        }

TEST (ConsoleLogger, SetOptionFAILURE)
        {
        CHECK ( false == LoggingConfig::IsProviderActive() );

        CHECK ( SUCCESS == LoggingConfig::SetOption( CONFIG_OPTION_DEFAULT_SEVERITY, L"SOMEINVALIDVALUE" ) );
        CHECK ( SUCCESS == LoggingConfig::SetOption( L"SOMEINVALIDOPTION", L"SOMEVALUE" ) );

        CHECK ( SUCCESS == LoggingConfig::ActivateProvider(CONSOLE_LOGGING_PROVIDER) );

        CHECK ( SUCCESS == LoggingConfig::DeactivateProvider() );
        }

TEST (ConsoleLogger, SetSeveritySUCCESS)
        {
        CHECK ( false == LoggingConfig::IsProviderActive() );

        CHECK ( SUCCESS == LoggingConfig::ActivateProvider(CONSOLE_LOGGING_PROVIDER) );

        CHECK ( SUCCESS == LoggingConfig::SetSeverity( L"", LOG_FATAL ) );
        CHECK ( SUCCESS == LoggingConfig::SetSeverity( L"", LOG_ERROR ) );
        CHECK ( SUCCESS == LoggingConfig::SetSeverity( L"", LOG_WARNING ) );
        CHECK ( SUCCESS == LoggingConfig::SetSeverity( L"", LOG_INFO ) );
        CHECK ( SUCCESS == LoggingConfig::SetSeverity( L"", LOG_DEBUG ) );
        CHECK ( SUCCESS == LoggingConfig::SetSeverity( L"", LOG_TRACE ) );

        CHECK ( SUCCESS == LoggingConfig::DeactivateProvider() );
        }

TEST (ConsoleLogger, SetSeverityFAILURE)
        {
        CHECK ( false == LoggingConfig::IsProviderActive() );

        CHECK ( SUCCESS == LoggingConfig::ActivateProvider(CONSOLE_LOGGING_PROVIDER) );

        CHECK ( ERROR == LoggingConfig::SetSeverity( NULL, LOG_FATAL ) );

    CHECK ( ERROR == LoggingConfig::SetSeverity( L"", (SEVERITY)(LOG_FATAL+1) ) );
        CHECK ( ERROR == LoggingConfig::SetSeverity( L"", (SEVERITY)(LOG_TRACE-1) ) );

        CHECK ( ERROR == LoggingConfig::SetSeverity( L"", (SEVERITY)(45) ) );
        CHECK ( ERROR == LoggingConfig::SetSeverity( L"", (SEVERITY)(-35) ) );

        CHECK ( SUCCESS == LoggingConfig::DeactivateProvider() );
        }

TEST (ConsoleLogger, SetMaxMessageSize)
        {
        CHECK ( false == LoggingConfig::IsProviderActive() );

        CHECK ( SUCCESS == LoggingConfig::ActivateProvider(CONSOLE_LOGGING_PROVIDER) );

        LoggingConfig::SetMaxMessageSize( 0 );
        CHECK ( 0 == LoggingConfig::SetMaxMessageSize( 256 ) );
        CHECK ( 256 == LoggingConfig::SetMaxMessageSize( 1024 ) );
        CHECK ( 1024 == LoggingConfig::SetMaxMessageSize( DEFAULT_MESSAGE_SIZE ) );

        CHECK ( SUCCESS == LoggingConfig::DeactivateProvider() );
        }





