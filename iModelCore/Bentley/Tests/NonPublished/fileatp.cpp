/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/NonPublished/fileatp.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#if defined (_MSC_VER)
    #pragma warning (disable:4505)
#endif // defined (_MSC_VER)

#include <Bentley/BeTest.h>
#include <Logging/bentleylogging.h>

#if defined (WIP_INTERFERES_WITH_LOGGING_BY_TESTS)

USING_NAMESPACE_BENTLEY_LOGGING;

static BeFileName      s_logFileName;

BeFileName& getlogfilename ( WCharCP name )
    {
    BeTest::GetHost().GetOutputRoot (s_logFileName);
    s_logFileName.AppendToPath (BeFileName (NULL,NULL,name,L".log"));
    return s_logFileName;
    }

TEST (SimpleFileLogger, configSUCCESS)
        {
        ASSERT_TRUE ( false == LoggingConfig::IsProviderActive() );

        ASSERT_TRUE ( SUCCESS == LoggingConfig::SetOption( CONFIG_OPTION_OUTPUT_FILE, getlogfilename(L"configSUCCESS" ) ) );

    ASSERT_TRUE ( SUCCESS == LoggingConfig::ActivateProvider(SIMPLEFILE_LOGGING_PROVIDER) );

        ASSERT_TRUE ( SUCCESS == LoggingConfig::DeactivateProvider() );
        }

TEST (SimpleFileLogger, configFAILURE)
        {
        ASSERT_TRUE ( false == LoggingConfig::IsProviderActive() );

        ASSERT_TRUE ( ERROR == LoggingConfig::ActivateProvider(SIMPLEFILE_LOGGING_PROVIDER) );

        ASSERT_TRUE ( SUCCESS == LoggingConfig::DeactivateProvider() );
        }

TEST (SimpleFileLogger, SetOptionSUCCESS)
        {
        ASSERT_TRUE ( false == LoggingConfig::IsProviderActive() );

        ASSERT_TRUE ( SUCCESS == LoggingConfig::SetOption( CONFIG_OPTION_DEFAULT_SEVERITY, LOG_TEXT_FATAL ) );
        ASSERT_TRUE ( SUCCESS == LoggingConfig::SetOption( CONFIG_OPTION_DEFAULT_SEVERITY, LOG_TEXT_ERROR ) );
        ASSERT_TRUE ( SUCCESS == LoggingConfig::SetOption( CONFIG_OPTION_DEFAULT_SEVERITY, LOG_TEXT_WARNING ) );
        ASSERT_TRUE ( SUCCESS == LoggingConfig::SetOption( CONFIG_OPTION_DEFAULT_SEVERITY, LOG_TEXT_INFO ) );
        ASSERT_TRUE ( SUCCESS == LoggingConfig::SetOption( CONFIG_OPTION_DEFAULT_SEVERITY, LOG_TEXT_DEBUG ) );
        ASSERT_TRUE ( SUCCESS == LoggingConfig::SetOption( CONFIG_OPTION_DEFAULT_SEVERITY, LOG_TEXT_TRACE ) );

        ASSERT_TRUE ( SUCCESS == LoggingConfig::SetOption( CONFIG_OPTION_OUTPUT_FILE, getlogfilename(L"SetOptionSUCCESS" ) ) );

    ASSERT_TRUE ( SUCCESS == LoggingConfig::ActivateProvider(SIMPLEFILE_LOGGING_PROVIDER) );

        ASSERT_TRUE ( SUCCESS == LoggingConfig::DeactivateProvider() );
        }

TEST (SimpleFileLogger, SetOptionFAILURE)
        {
        ASSERT_TRUE ( false == LoggingConfig::IsProviderActive() );

        ASSERT_TRUE ( SUCCESS == LoggingConfig::SetOption( CONFIG_OPTION_DEFAULT_SEVERITY, L"SOMEINVALIDVALUE" ) );
        ASSERT_TRUE ( SUCCESS == LoggingConfig::SetOption( L"SOMEINVALIDOPTION", L"SOMEVALUE" ) );

        ASSERT_TRUE ( ERROR == LoggingConfig::ActivateProvider(SIMPLEFILE_LOGGING_PROVIDER) );

        ASSERT_TRUE ( SUCCESS == LoggingConfig::DeactivateProvider() );
        }

TEST (SimpleFileLogger, SetSeveritySUCCESS)
        {
        ASSERT_TRUE ( false == LoggingConfig::IsProviderActive() );

        ASSERT_TRUE ( SUCCESS == LoggingConfig::SetOption( CONFIG_OPTION_OUTPUT_FILE, getlogfilename(L"SetSeveritySUCCESS" ) ) );

    ASSERT_TRUE ( SUCCESS == LoggingConfig::ActivateProvider(SIMPLEFILE_LOGGING_PROVIDER) );

        ASSERT_TRUE ( SUCCESS == LoggingConfig::SetSeverity( L"", LOG_FATAL ) );
        ASSERT_TRUE ( SUCCESS == LoggingConfig::SetSeverity( L"", LOG_ERROR ) );
        ASSERT_TRUE ( SUCCESS == LoggingConfig::SetSeverity( L"", LOG_WARNING ) );
        ASSERT_TRUE ( SUCCESS == LoggingConfig::SetSeverity( L"", LOG_INFO ) );
        ASSERT_TRUE ( SUCCESS == LoggingConfig::SetSeverity( L"", LOG_DEBUG ) );
        ASSERT_TRUE ( SUCCESS == LoggingConfig::SetSeverity( L"", LOG_TRACE ) );

        ASSERT_TRUE ( SUCCESS == LoggingConfig::DeactivateProvider() );
        }

TEST (SimpleFileLogger, SetSeverityFAILURE)
        {
        ASSERT_TRUE ( false == LoggingConfig::IsProviderActive() );

        ASSERT_TRUE ( SUCCESS == LoggingConfig::SetOption( CONFIG_OPTION_OUTPUT_FILE, getlogfilename(L"SetSeverityFAILURE" ) ) );

    ASSERT_TRUE ( SUCCESS == LoggingConfig::ActivateProvider(SIMPLEFILE_LOGGING_PROVIDER) );

        ASSERT_TRUE ( ERROR == LoggingConfig::SetSeverity( NULL, LOG_FATAL ) );

    ASSERT_TRUE ( ERROR == LoggingConfig::SetSeverity( L"", (SEVERITY)(LOG_FATAL+1) ) );
        ASSERT_TRUE ( ERROR == LoggingConfig::SetSeverity( L"", (SEVERITY)(LOG_TRACE-1) ) );

        ASSERT_TRUE ( ERROR == LoggingConfig::SetSeverity( L"", (SEVERITY)(45) ) );
        ASSERT_TRUE ( ERROR == LoggingConfig::SetSeverity( L"", (SEVERITY)(-35) ) );

        ASSERT_TRUE ( SUCCESS == LoggingConfig::DeactivateProvider() );
        }

TEST (SimpleFileLogger, SetMaxMessageSize)
        {
        ASSERT_TRUE ( false == LoggingConfig::IsProviderActive() );

        ASSERT_TRUE ( SUCCESS == LoggingConfig::SetOption( CONFIG_OPTION_OUTPUT_FILE, getlogfilename(L"SetMaxMessageSize" ) ) );

    ASSERT_TRUE ( SUCCESS == LoggingConfig::ActivateProvider(SIMPLEFILE_LOGGING_PROVIDER) );

        LoggingConfig::SetMaxMessageSize( 0 );
        ASSERT_TRUE ( 0 == LoggingConfig::SetMaxMessageSize( 256 ) );
        ASSERT_TRUE ( 256 == LoggingConfig::SetMaxMessageSize( 1024 ) );
        ASSERT_TRUE ( 1024 == LoggingConfig::SetMaxMessageSize( DEFAULT_MESSAGE_SIZE ) );

        ASSERT_TRUE ( SUCCESS == LoggingConfig::DeactivateProvider() );
        }


TEST (SimpleFileLogger, FatalMessage)
        {
        ASSERT_TRUE ( false == LoggingConfig::IsProviderActive() );

        ASSERT_TRUE ( SUCCESS == LoggingConfig::SetOption( CONFIG_OPTION_OUTPUT_FILE, getlogfilename(L"FatalMessage" ) ) );

    ASSERT_TRUE ( SUCCESS == LoggingConfig::ActivateProvider(SIMPLEFILE_LOGGING_PROVIDER) );

    ILogger* pLogger = LoggingManager::GetLogger( L"TEST" );

    ASSERT_TRUE ( NULL != pLogger );

    pLogger->fatal ( L"Some Message" );

        ASSERT_TRUE ( SUCCESS == LoggingConfig::DeactivateProvider() );
        }


#endif // defined (WIP_INTERFERES_WITH_LOGGING_BY_TESTS)
