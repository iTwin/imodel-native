/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#if defined (_MSC_VER)
#pragma warning (disable:4505)
#endif // defined (_MSC_VER)

#include <Bentley/BeTest.h>
#include <Logging/bentleylogging.h>

//#if defined (WIP_INTERFERES_WITH_LOGGING_BY_TESTS)
struct ProviderActivator
    {
    ProviderActivator(BeFileName const& logfile)
        {
        BeFileName::BeDeleteFile(logfile);
        NativeLogging::LoggingConfig::SetOption(CONFIG_OPTION_OUTPUT_FILE, logfile);
        NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::SIMPLEFILE_LOGGING_PROVIDER);
        }

    ~ProviderActivator()
        {
        NativeLogging::LoggingConfig::DeactivateProvider();
        }
    };

class BentleyLoggingTests :public testing::Test
    {
    public:

        virtual void SetUp()
            {
            }
        virtual void TearDown()
            {
            }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                        12/16
//---------------------------------------------------------------------------------------
TEST_F(BentleyLoggingTests, LogggerCheck)
    {
    BeFileName logfile;
    BeTest::GetHost().GetOutputRoot(logfile);
    logfile.AppendToPath(L"bentleylogging_test.log");

    ProviderActivator loggerActivatorInScope(logfile);
    NativeLogging::LoggingConfig::SetSeverity(L"bentleylogging_test", NativeLogging::LOG_TRACE);

    // We should see both messages in console
    NativeLogging::ILogger* logger = NativeLogging::LoggingManager::GetLogger(L"bentleylogging_test");

    //testing log messages
    logger->message(NativeLogging::LOG_TRACE, L"This is a trace log message");
    logger->messagev(NativeLogging::LOG_TRACE, L"This is a formatted trace log message with decimal %d", 787);
    logger->message(NativeLogging::LOG_DEBUG, L"This is a debug log message");
    logger->message(NativeLogging::LOG_INFO, L"This is a info log message");
    logger->messagev(NativeLogging::LOG_INFO, L"This is a formatted info log message with decimal %10d", 1977);
    logger->message(NativeLogging::LOG_WARNING, L"This is a warning log message");
    logger->message(NativeLogging::LOG_ERROR, L"This is a error log message");
    logger->message(NativeLogging::LOG_FATAL, L"This is a fatal log message");
    logger->messagev(NativeLogging::LOG_FATAL, L"This is a formatted fatal log message with character %c", 65);
    //logger = NativeLogging::LoggingManager::GetLogger (L"bentleylogging_test2");
    //logger->message(L"bentleylogging_test2",NativeLogging::LOG_TRACE, L"This is a trace log message");
    EXPECT_TRUE(logger->isSeverityEnabled(NativeLogging::LOG_TRACE));
    EXPECT_TRUE(logger->isSeverityEnabled(NativeLogging::LOG_ERROR));
    EXPECT_TRUE(NativeLogging::LoggingConfig::IsProviderActive());
    logger->info(L"info (wide)");
    logger->trace("trace (UTF-8)");
    // Now we should see messages truncated to "tr"
    NativeLogging::LoggingConfig::SetMaxMessageSize(3);
    logger->trace(L"trace (wide)");
    logger->trace("trace (UTF-8)");
    NativeLogging::LoggingConfig::SetMaxMessageSize(1024);


    //Checking different severity level with all possible logs
    bvector<NativeLogging::SEVERITY> severity;
    severity.push_back(NativeLogging::LOG_FATAL);
    severity.push_back(NativeLogging::LOG_ERROR);
    severity.push_back(NativeLogging::LOG_WARNING);
    severity.push_back(NativeLogging::LOG_INFO);
    severity.push_back(NativeLogging::LOG_DEBUG);
    severity.push_back(NativeLogging::LOG_TRACE);

    bvector<NativeLogging::SEVERITY>::iterator it = severity.begin();
    while (it != severity.end())
        {
        logger->message(severity.back(), "//////////////////////////////////////////////////////");
        NativeLogging::LoggingConfig::SetSeverity("bentleylogging_test", severity.back());
        logger->trace(L"Trace (wide)");
        logger->trace("Trace (UTF-8)");
        logger->tracev(L"Style arguments Trace (wide) %c", 68);
        logger->tracev("Style arguments Trace (UTF-8) %c", 68);

        logger->error(L"Error log (wide)");
        logger->error("Error log (UTF-8)");
        logger->errorv(L"Style arguments Error log (wide) %d", 68);
        logger->errorv("Style arguments Error log (UTF-8) %d", 68);

        logger->debug(L"Debug log (wide)");
        logger->debug("Debug log (UTF-8)");
        logger->debugv(L"Style arguments Debug log (wide) %d", 68);
        logger->debugv("Style arguments Debug log (UTF-8) %d", 68);

        logger->fatal(L"Fatal log (wide)");
        logger->fatal("Fatal log (UTF-8)");
        logger->fatalv(L"Style arguments Fatal log (wide) %d", 68);
        logger->fatalv("Style arguments Fatal log (UTF-8) %d", 68);

        logger->info(L"Info log (wide)");
        logger->info("Info log (UTF-8)");
        logger->infov(L"Style arguments Info log (wide) %d", 68);
        logger->infov("Style arguments Info log (UTF-8) %d", 68);

        logger->warning(L"Warning log (wide)");
        logger->warning("Warning log (UTF-8)");
        logger->warningv(L"Style arguments Warning log (wide) %d", 68);
        logger->warningv("Style arguments Warning log (UTF-8) %d", 68);
        logger->message(severity.back(), "//////////////////////////////////////////////////////");
        severity.pop_back();

        }
    NativeLogging::LoggingConfig::SetSeverity(L"bentleylogging_test", NativeLogging::LOG_DEBUG);
    logger->message(L"bentleylogging_test", NativeLogging::LOG_ERROR, L"Error Log (wide) with namespace");
    logger->message("bentleylogging_test", NativeLogging::LOG_DEBUG, "Debug Log (Utf-8) with namespace");
    logger->messagev(L"bentleylogging_test2", NativeLogging::LOG_FATAL, L"Fatal Log (wide) with namespace %c", 67);
    logger->messagev(L"bentleylogging_test3", NativeLogging::LOG_ERROR, L"Error Log (wide) with namespace %c", 67);
    logger->messagev(NativeLogging::LOG_ERROR, L"Error Log (wide) %c", 67);
    logger->messagev(NativeLogging::LOG_ERROR, "Error Log (Utf-8) %c", 67);

    //ilooger = NativeLogging::LoggingManager::CreateUncachedLogger(L"bentleyloging_test4");
    //ilooger->message(NativeLogging::LOG_ERROR, "Error message from looger2 call 2");
    //ilooger = NativeLogging::LoggingManager::CreateUncachedLogger(L"bentleyloging_test4");
    NativeLogging::ILogger* ilooger = NativeLogging::LoggingManager::CreateUncachedLogger(L"bentleyloging_test4");
    ilooger->message(NativeLogging::LOG_ERROR, "Error message from looger2 call 1");
    EXPECT_TRUE(0 == NativeLogging::LoggingManager::DestroyUncachedLogger(ilooger));
#ifdef COMMENT_OUT // You just freed "ilooger". You must not try to use it.
    ilooger->message(NativeLogging::LOG_ERROR, "Error message from looger2 call 3");
#endif

    EXPECT_TRUE(NativeLogging::LoggingConfig::IsProviderActive());
    EXPECT_TRUE(0 == NativeLogging::LoggingConfig::DeactivateProvider());
    EXPECT_FALSE(NativeLogging::LoggingConfig::IsProviderActive());
    //will not work because provider is inactive
    logger->messagev(NativeLogging::LOG_ERROR, "Error Log (Utf-8) %c", 67);
    // *** TBD: read file logfile and check results
    loggerActivatorInScope.~ProviderActivator();
    }

//#endif // defined (WIP_INTERFERES_WITH_LOGGING_BY_TESTS)
