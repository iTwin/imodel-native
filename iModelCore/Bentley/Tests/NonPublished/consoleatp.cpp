/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#if defined (_MSC_VER)
    #pragma warning (disable:4505)
#endif // defined (_MSC_VER)

#include <Bentley/BeTest.h>
#include <Logging/bentleylogging.h>

#if defined (WIP_INTERFERES_WITH_LOGGING_BY_TESTS)

USING_NAMESPACE_BENTLEY_LOGGING;

//---------------------------------------------------------------------------------------
// @betest                                      Sam.Wilson                    4/13
//---------------------------------------------------------------------------------------
TEST (ConsoleLogger, configSUCCESS)
        {
        ASSERT_TRUE ( false == LoggingConfig::IsProviderActive() );

        ASSERT_TRUE ( SUCCESS == LoggingConfig::ActivateProvider(CONSOLE_LOGGING_PROVIDER) );

        ASSERT_TRUE ( SUCCESS == LoggingConfig::DeactivateProvider() );
        }

//---------------------------------------------------------------------------------------
// @betest                                      Sam.Wilson                    4/13
//---------------------------------------------------------------------------------------
TEST (ConsoleLogger, SetOptionSUCCESS)
        {
        ASSERT_TRUE ( false == LoggingConfig::IsProviderActive() );

        ASSERT_TRUE ( SUCCESS == LoggingConfig::SetOption( CONFIG_OPTION_DEFAULT_SEVERITY, LOG_TEXT_FATAL ) );
        ASSERT_TRUE ( SUCCESS == LoggingConfig::SetOption( CONFIG_OPTION_DEFAULT_SEVERITY, LOG_TEXT_ERROR ) );
        ASSERT_TRUE ( SUCCESS == LoggingConfig::SetOption( CONFIG_OPTION_DEFAULT_SEVERITY, LOG_TEXT_WARNING ) );
        ASSERT_TRUE ( SUCCESS == LoggingConfig::SetOption( CONFIG_OPTION_DEFAULT_SEVERITY, LOG_TEXT_INFO ) );
        ASSERT_TRUE ( SUCCESS == LoggingConfig::SetOption( CONFIG_OPTION_DEFAULT_SEVERITY, LOG_TEXT_DEBUG ) );
        ASSERT_TRUE ( SUCCESS == LoggingConfig::SetOption( CONFIG_OPTION_DEFAULT_SEVERITY, LOG_TEXT_TRACE ) );

        ASSERT_TRUE ( SUCCESS == LoggingConfig::ActivateProvider(CONSOLE_LOGGING_PROVIDER) );

        ASSERT_TRUE ( SUCCESS == LoggingConfig::DeactivateProvider() );
        }

//---------------------------------------------------------------------------------------
// @betest                                      Sam.Wilson                    4/13
//---------------------------------------------------------------------------------------
TEST (ConsoleLogger, SetOptionFAILURE)
        {
        ASSERT_TRUE ( false == LoggingConfig::IsProviderActive() );

        ASSERT_TRUE ( SUCCESS == LoggingConfig::SetOption( CONFIG_OPTION_DEFAULT_SEVERITY, L"SOMEINVALIDVALUE" ) );
        ASSERT_TRUE ( SUCCESS == LoggingConfig::SetOption( L"SOMEINVALIDOPTION", L"SOMEVALUE" ) );

        ASSERT_TRUE ( SUCCESS == LoggingConfig::ActivateProvider(CONSOLE_LOGGING_PROVIDER) );

        ASSERT_TRUE ( SUCCESS == LoggingConfig::DeactivateProvider() );
        }

//---------------------------------------------------------------------------------------
// @betest                                      Sam.Wilson                    4/13
//---------------------------------------------------------------------------------------
TEST (ConsoleLogger, SetSeveritySUCCESS)
        {
        ASSERT_TRUE ( false == LoggingConfig::IsProviderActive() );

        ASSERT_TRUE ( SUCCESS == LoggingConfig::ActivateProvider(CONSOLE_LOGGING_PROVIDER) );

        ASSERT_TRUE ( SUCCESS == LoggingConfig::SetSeverity( L"", LOG_FATAL ) );
        ASSERT_TRUE ( SUCCESS == LoggingConfig::SetSeverity( L"", LOG_ERROR ) );
        ASSERT_TRUE ( SUCCESS == LoggingConfig::SetSeverity( L"", LOG_WARNING ) );
        ASSERT_TRUE ( SUCCESS == LoggingConfig::SetSeverity( L"", LOG_INFO ) );
        ASSERT_TRUE ( SUCCESS == LoggingConfig::SetSeverity( L"", LOG_DEBUG ) );
        ASSERT_TRUE ( SUCCESS == LoggingConfig::SetSeverity( L"", LOG_TRACE ) );

        ASSERT_TRUE ( SUCCESS == LoggingConfig::DeactivateProvider() );
        }

//---------------------------------------------------------------------------------------
// @betest                                      Sam.Wilson                    4/13
//---------------------------------------------------------------------------------------
TEST (ConsoleLogger, SetSeverityFAILURE)
        {
        ASSERT_TRUE ( false == LoggingConfig::IsProviderActive() );

        ASSERT_TRUE ( SUCCESS == LoggingConfig::ActivateProvider(CONSOLE_LOGGING_PROVIDER) );

        ASSERT_TRUE ( ERROR == LoggingConfig::SetSeverity( NULL, LOG_FATAL ) );

    ASSERT_TRUE ( ERROR == LoggingConfig::SetSeverity( L"", (SEVERITY)(LOG_FATAL+1) ) );
        ASSERT_TRUE ( ERROR == LoggingConfig::SetSeverity( L"", (SEVERITY)(LOG_TRACE-1) ) );

        ASSERT_TRUE ( ERROR == LoggingConfig::SetSeverity( L"", (SEVERITY)(45) ) );
        ASSERT_TRUE ( ERROR == LoggingConfig::SetSeverity( L"", (SEVERITY)(-35) ) );

        ASSERT_TRUE ( SUCCESS == LoggingConfig::DeactivateProvider() );
        }

//---------------------------------------------------------------------------------------
// @betest                                      Sam.Wilson                    4/13
//---------------------------------------------------------------------------------------
TEST (ConsoleLogger, SetMaxMessageSize)
        {
        ASSERT_TRUE ( false == LoggingConfig::IsProviderActive() );

        ASSERT_TRUE ( SUCCESS == LoggingConfig::ActivateProvider(CONSOLE_LOGGING_PROVIDER) );

        LoggingConfig::SetMaxMessageSize( 0 );
        ASSERT_TRUE ( 0 == LoggingConfig::SetMaxMessageSize( 256 ) );
        ASSERT_TRUE ( 256 == LoggingConfig::SetMaxMessageSize( 1024 ) );
        ASSERT_TRUE ( 1024 == LoggingConfig::SetMaxMessageSize( DEFAULT_MESSAGE_SIZE ) );

        ASSERT_TRUE ( SUCCESS == LoggingConfig::DeactivateProvider() );
        }

#endif // defined (WIP_INTERFERES_WITH_LOGGING_BY_TESTS)
