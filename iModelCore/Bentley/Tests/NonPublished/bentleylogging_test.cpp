/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/bentleylogging_test.cpp $
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

struct ProviderActivator
{
ProviderActivator (BeFileName const& logfile)  
    {
    BeFileName::BeDeleteFile (logfile);
    NativeLogging::LoggingConfig::SetOption( CONFIG_OPTION_OUTPUT_FILE, logfile);
    NativeLogging::LoggingConfig::ActivateProvider (NativeLogging::SIMPLEFILE_LOGGING_PROVIDER);
    }

~ProviderActivator() 
    {
    NativeLogging::LoggingConfig::DeactivateProvider ();
    } 
};

TEST(bentleylogging_test, LogUtf8)
{
    BeFileName logfile;
    BeTest::GetHost().GetOutputRoot (logfile);
    logfile.AppendToPath (L"bentleylogging_test.log");

    ProviderActivator loggerActivatorInScope (logfile);

    NativeLogging::LoggingConfig::SetSeverity (L"bentleylogging_test", NativeLogging::LOG_TRACE);
    // We should see both messages in console
    NativeLogging::ILogger* logger = NativeLogging::LoggingManager::GetLogger (L"bentleylogging_test");
    logger->trace (L"trace (wide)");
    logger->trace ("trace (UTF-8)");
    // Now we should see messages truncated to "tr"
    NativeLogging::LoggingConfig::SetMaxMessageSize (3);
    logger->trace (L"trace (wide)");
    logger->trace ("trace (UTF-8)");
    NativeLogging::LoggingConfig::SetMaxMessageSize (1024);
    // Now we should see nothing
    NativeLogging::LoggingConfig::SetSeverity (L"bentleylogging_test", NativeLogging::LOG_ERROR);
    logger->trace (L"trace (wide)");
    logger->trace ("trace (UTF-8)");


    // *** TBD: read file logfile and check results
}

#endif // defined (WIP_INTERFERES_WITH_LOGGING_BY_TESTS)
