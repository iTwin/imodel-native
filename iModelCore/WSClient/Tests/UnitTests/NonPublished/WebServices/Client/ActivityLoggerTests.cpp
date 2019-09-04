/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "../../Utils/WebServicesTestsHelper.h"
#include "../../../../../Client/WebApi/WebApiV2Utils/ActivityLogger.h"
#include "../../../../../Client/Logging.h"
#include "../../Utils/StubLogger.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

struct ActivityLoggerTests : WSClientBaseTest {};

void TestActivityLoggerWithActivityId
(
Utf8StringCR activityName,
Utf8StringCR activityId,
std::function<void(ActivityLoggerCR)> onAct,
NativeLogging::SEVERITY expectedSeverity,
Utf8StringCR expectedMessage,
Utf8StringCR expectedNamespace = Utf8String()
)
    {
    StubLogger stubLogger;
    ActivityLogger logger(stubLogger, activityName, "Mas-Request-Id", activityId);

    onAct(logger);

    ASSERT_TRUE(stubLogger.HasUtf8CPLogs());
    EXPECT_EQ(expectedSeverity, stubLogger.GetLastUtf8CPLog().GetSeverity());
    EXPECT_STREQ(expectedMessage.c_str(), stubLogger.GetLastUtf8CPLog().GetMessage().c_str());
    EXPECT_STREQ(expectedNamespace.c_str(), stubLogger.GetLastUtf8CPLog().GetNameSpace().c_str());
    EXPECT_EQ(1, stubLogger.GetLogsCount());
    }

void TestActivityLoggerWithActivityId
(
Utf8StringCR activityName,
Utf8StringCR activityId,
std::function<void(ActivityLoggerCR)> onAct,
NativeLogging::SEVERITY expectedSeverity,
WStringCR expectedMessage,
WStringCR expectedNamespace = WString()
)
    {
    StubLogger stubLogger;
    ActivityLogger logger(stubLogger, activityName, "Mas-Request-Id", activityId);

    onAct(logger);

    ASSERT_TRUE(stubLogger.HasWCharCPLogs());
    EXPECT_EQ(expectedSeverity, stubLogger.GetLastWCharCPLog().GetSeverity());
    EXPECT_STREQ(expectedMessage.c_str(), stubLogger.GetLastWCharCPLog().GetMessage().c_str());
    EXPECT_STREQ(expectedNamespace.c_str(), stubLogger.GetLastWCharCPLog().GetNameSpace().c_str());
    EXPECT_EQ(1, stubLogger.GetLogsCount());
    }

// void InvokeActionWithVaList(std::function<void(va_list)> action, ... )
//     {
//     va_list args;
//     va_start(args, action);
//     action(args);
//     va_end(args);
//     }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, HasValidActivityInfo_WithoutActivityIdAndHeaderName_ReturnsFalse)
    {
    StubLogger stubLogger;
    ActivityLogger logger(stubLogger, "WithoutActivityIdAndHeaderName");
    EXPECT_FALSE(logger.HasValidActivityInfo());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, HasValidActivityInfo_WithoutActivityId_ReturnsFalse)
    {
    StubLogger stubLogger;
    ActivityLogger logger(stubLogger, "WithoutActivityId", "Mas-Request-Id");
    EXPECT_FALSE(logger.HasValidActivityInfo());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, HasValidActivityInfo_WithoutHeaderName_ReturnsFalse)
    {
    StubLogger stubLogger;
    ActivityLogger logger(stubLogger, "WithoutHeaderName", Utf8String() ,"1E51");
    EXPECT_FALSE(logger.HasValidActivityInfo());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, HasValidActivityInfo_WithActivityIdAndHeaderName_ReturnsTrue)
    {
    StubLogger stubLogger;
    ActivityLogger logger(stubLogger, "WithActivityIdAndHeaderName", "Mas-Request-Id", "1E51");
    EXPECT_TRUE(logger.HasValidActivityInfo());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, GetActivityId_WithoutActivityId_ReturnsEmptyString)
    {
    StubLogger stubLogger;
    ActivityLogger logger(stubLogger, "WithoutActivityId");
    EXPECT_TRUE(logger.GetActivityId().empty());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, GetActivityId_WithActivityId_ReturnsActivityId)
    {
    StubLogger stubLogger;
    ActivityLogger logger(stubLogger, "WithActivityId", "Mas-Request-Id", "1E51");
    EXPECT_STREQ("1E51", logger.GetActivityId().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, GetHeaderName_WithoutHeaderName_ReturnsEmptyString)
    {
    StubLogger stubLogger;
    ActivityLogger logger(stubLogger, "WithoutHeaderName");
    EXPECT_TRUE(logger.GetHeaderName().empty());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, GetHeaderName_WithHeaderName_ReturnsHeaderName)
    {
    StubLogger stubLogger;
    ActivityLogger logger(stubLogger, "WithHeaderName", "Mas-Request-Id");
    EXPECT_STREQ("Mas-Request-Id", logger.GetHeaderName().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, isSeverityEnabled_InfoSeverity_ReturnsIsSeverityEnabledFromLogger)
    {
    StubLogger stubLogger;
    stubLogger.OnIsSeverityEnabled([=] (NativeLogging::SEVERITY sev)
        {
        return sev >= NativeLogging::SEVERITY::LOG_ERROR;
        });
    ActivityLogger logger(stubLogger, "InfoSeverityEnabled");

    EXPECT_TRUE(logger.isSeverityEnabled(NativeLogging::SEVERITY::LOG_FATAL));
    EXPECT_TRUE(logger.isSeverityEnabled(NativeLogging::SEVERITY::LOG_ERROR));
    EXPECT_FALSE(logger.isSeverityEnabled(NativeLogging::SEVERITY::LOG_WARNING));
    EXPECT_FALSE(logger.isSeverityEnabled(NativeLogging::SEVERITY::LOG_INFO));
    EXPECT_FALSE(logger.isSeverityEnabled(NativeLogging::SEVERITY::LOG_DEBUG));
    EXPECT_FALSE(logger.isSeverityEnabled(NativeLogging::SEVERITY::LOG_TRACE));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, message_WrappedLoggerReturnsSeverityDisabled_DoesNotLog)
    {
    StubLogger stubLogger;
    stubLogger.OnIsSeverityEnabled([=] (NativeLogging::SEVERITY severity)
        {
        return false;
        });
    ActivityLogger logger(stubLogger, "severityIsDisabled");

    logger.message("Testing.Namespace", NativeLogging::SEVERITY::LOG_WARNING, "Something is logged");

    EXPECT_EQ(0, stubLogger.GetLogsCount());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, message_WrappedLoggerReturnsSeverityEnabled_LogsExpectedMessage)
    {
    StubLogger stubLogger;
    stubLogger.OnIsSeverityEnabled([=] (NativeLogging::SEVERITY severity)
        {
        return true;
        });
    ActivityLogger logger(stubLogger, "severityIsEnabled");

    logger.message("Testing.Namespace", NativeLogging::SEVERITY::LOG_WARNING, "Something is logged");

    EXPECT_EQ(1, stubLogger.GetLogsCount());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, message_WithoutActivityId_LogsExpectedMessage)
    {
    StubLogger stubLogger;
    ActivityLogger logger(stubLogger, "withoutActivityId");

    logger.message("Testing.Namespace", NativeLogging::SEVERITY::LOG_DEBUG, "Something is logged");

    ASSERT_TRUE(stubLogger.HasUtf8CPLogs());
    EXPECT_STREQ("withoutActivityId: Something is logged", stubLogger.GetLastUtf8CPLog().GetMessage().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, message_WithActivityIdAndWCharCPMessageAndWithoutNameSpace_LogsExpectedMessage)
    {
    auto activityName = "messageWCharCP";
    auto activityId = "1E51";

    auto onAct = [=] (ActivityLogger logger)
        {
        logger.message(NativeLogging::SEVERITY::LOG_TRACE, L"Something is logged");
        };

    auto expectedSeverity = NativeLogging::SEVERITY::LOG_TRACE;
    auto expectedMessage = L"messageWCharCP: Something is logged (ActivityId: 1E51)";

    TestActivityLoggerWithActivityId(activityName, activityId, onAct, expectedSeverity, expectedMessage);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, message_WithActivityIdAndUtf8CPMessageAndWithoutNameSpace_LogsExpectedMessage)
    {
    auto activityName = "messageUtf8CP";
    auto activityId = "1E51";
    
    auto onAct = [=] (ActivityLogger logger)
        {
        logger.message(NativeLogging::SEVERITY::LOG_WARNING, "Something is logged");
        };
    
    auto expectedSeverity = NativeLogging::SEVERITY::LOG_WARNING;
    auto expectedMessage = "messageUtf8CP: Something is logged (ActivityId: 1E51)";

    TestActivityLoggerWithActivityId(activityName, activityId, onAct, expectedSeverity, expectedMessage);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, messagev_WithActivityIdAndWCharCPMessageAndWithoutNameSpace_LogsExpectedMessage)
    {
    auto activityName = "messagevWCharCP";
    auto activityId = "1E51";

    auto onAct = [=] (ActivityLogger logger)
        {
        logger.messagev(NativeLogging::SEVERITY::LOG_INFO, L"Something %ls logged", L"is");
        };
    
    auto expectedSeverity = NativeLogging::SEVERITY::LOG_INFO;
    auto expectedMessage = L"messagevWCharCP: Something is logged (ActivityId: 1E51)";

    TestActivityLoggerWithActivityId(activityName, activityId, onAct, expectedSeverity, expectedMessage);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, messagev_WithActivityIdAndUtf8CPMessageAndWithoutNameSpace_LogsExpectedMessage)
    {
    auto activityName = "messagevUtf8CP";
    auto activityId = "1E51";
    
    auto onAct = [=] (ActivityLogger logger)
        {
        logger.messagev(NativeLogging::SEVERITY::LOG_TRACE, "Something %s logged", "is");
        };
    
    auto expectedSeverity = NativeLogging::SEVERITY::LOG_TRACE;
    auto expectedMessage = "messagevUtf8CP: Something is logged (ActivityId: 1E51)";

    TestActivityLoggerWithActivityId(activityName, activityId, onAct, expectedSeverity, expectedMessage);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
// WIP_BIM02DEV_MERGE
// TEST_F(ActivityLoggerTests, messageva_WithActivityIdAndWCharCPMessageAndWithoutNameSpace_LogsExpectedMessage)
//     {
//     auto activityName = "messagevaWCharCP";
//     auto activityId = "1E51";
    
//     auto onAct = [=] (ActivityLogger logger)
//         {
//         InvokeActionWithVaList([=, &logger] (va_list args)
//             {
//             logger.messageva(NativeLogging::SEVERITY::LOG_WARNING, L"Something %ls logged", args);
//             }, L"is");
//         };
    
//     auto expectedSeverity = NativeLogging::SEVERITY::LOG_WARNING;
//     auto expectedMessage = L"messagevaWCharCP: Something is logged (ActivityId: 1E51)";

//     TestActivityLoggerWithActivityId(activityName, activityId, onAct, expectedSeverity, expectedMessage);
//     }

// /*--------------------------------------------------------------------------------------+
// * @bsimethod                                                    Mantas.Smicius    10/2018
// +---------------+---------------+---------------+---------------+---------------+------*/
// TEST_F(ActivityLoggerTests, messageva_WithActivityIdAndUtf8CPMessageAndWithoutNameSpace_LogsExpectedMessage)
//     {
//     auto activityName = "messagevaUtf8CP";
//     auto activityId = "1E51";
    
//     auto onAct = [=] (ActivityLogger logger)
//         {
//         InvokeActionWithVaList([=, &logger] (va_list args)
//             {
//             logger.messageva(NativeLogging::SEVERITY::LOG_ERROR, "Something %s logged", args);
//             }, "is");
//         };
    
//     auto expectedSeverity = NativeLogging::SEVERITY::LOG_ERROR;
//     auto expectedMessage = "messagevaUtf8CP: Something is logged (ActivityId: 1E51)";

//     TestActivityLoggerWithActivityId(activityName, activityId, onAct, expectedSeverity, expectedMessage);
//     }
// WIP_BIM02DEV_MERGE

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, message_WithActivityIdAndWCharCPMessageAndNameSpace_LogsExpectedMessage)
    {
    auto activityName = "messageWCharCP";
    auto activityId = "1E51";
    
    auto onAct = [=] (ActivityLogger logger)
        {
        logger.message(L"Testing.Namespace", NativeLogging::SEVERITY::LOG_WARNING, L"Something is logged");
        };
    
    auto expectedSeverity = NativeLogging::SEVERITY::LOG_WARNING;
    auto expectedMessage = L"messageWCharCP: Something is logged (ActivityId: 1E51)";
    auto expectedNameSpace = L"Testing.Namespace";

    TestActivityLoggerWithActivityId(activityName, activityId, onAct, expectedSeverity, expectedMessage, expectedNameSpace);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, message_WithActivityIdAndUtf8CPMessageAndNameSpace_LogsExpectedMessage)
    {
    auto activityName = "messageUtf8CP";
    auto activityId = "1E51";
    
    auto onAct = [=] (ActivityLogger logger)
        {
        logger.message("Testing.Namespace", NativeLogging::SEVERITY::LOG_DEBUG, "Something is logged");
        };
    
    auto expectedSeverity = NativeLogging::SEVERITY::LOG_DEBUG;
    auto expectedMessage = "messageUtf8CP: Something is logged (ActivityId: 1E51)";
    auto expectedNameSpace = "Testing.Namespace";

    TestActivityLoggerWithActivityId(activityName, activityId, onAct, expectedSeverity, expectedMessage, expectedNameSpace);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, messagev_WithActivityIdAndWCharCPMessageAndNameSpace_LogsExpectedMessage)
    {
    auto activityName = "messagevWCharCP";
    auto activityId = "1E51";
    
    auto onAct = [=] (ActivityLogger logger)
        {
        logger.messagev(L"Testing.Namespace", NativeLogging::SEVERITY::LOG_ERROR, L"Something %ls logged", L"is");
        };
    
    auto expectedSeverity = NativeLogging::SEVERITY::LOG_ERROR;
    auto expectedMessage = L"messagevWCharCP: Something is logged (ActivityId: 1E51)";
    auto expectedNameSpace = L"Testing.Namespace";

    TestActivityLoggerWithActivityId(activityName, activityId, onAct, expectedSeverity, expectedMessage, expectedNameSpace);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, messagev_WithActivityIdAndUtf8CPMessageAndNameSpace_LogsExpectedMessage)
    {
    auto activityName = "messagevUtf8CP";
    auto activityId = "1E51";
    
    auto onAct = [=] (ActivityLogger logger)
        {
        logger.messagev("Testing.Namespace", NativeLogging::SEVERITY::LOG_FATAL, "Something %s logged", "is");
        };
    
    auto expectedSeverity = NativeLogging::SEVERITY::LOG_FATAL;
    auto expectedMessage = "messagevUtf8CP: Something is logged (ActivityId: 1E51)";
    auto expectedNameSpace = "Testing.Namespace";

    TestActivityLoggerWithActivityId(activityName, activityId, onAct, expectedSeverity, expectedMessage, expectedNameSpace);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
// WIP_BIM02DEV_MERGE
// TEST_F(ActivityLoggerTests, messageva_WithActivityIdAndWCharCPMessageAndNameSpace_LogsExpectedMessage)
//     {
//     auto activityName = "messagevaWCharCP";
//     auto activityId = "1E51";
    
//     auto onAct = [=] (ActivityLogger logger)
//         {
//         InvokeActionWithVaList([=, &logger] (va_list args)
//             {
//             logger.messageva(L"Testing.Namespace", NativeLogging::SEVERITY::LOG_WARNING, L"Something %ls logged", args);
//             }, L"is");
//         };
    
//     auto expectedSeverity = NativeLogging::SEVERITY::LOG_WARNING;
//     auto expectedMessage = L"messagevaWCharCP: Something is logged (ActivityId: 1E51)";
//     auto expectedNameSpace = L"Testing.Namespace";

//     TestActivityLoggerWithActivityId(activityName, activityId, onAct, expectedSeverity, expectedMessage, expectedNameSpace);
//     }

// /*--------------------------------------------------------------------------------------+
// * @bsimethod                                                    Mantas.Smicius    10/2018
// +---------------+---------------+---------------+---------------+---------------+------*/
// TEST_F(ActivityLoggerTests, messageva_WithActivityIdAndUtf8CPMessageAndNameSpace_LogsExpectedMessage)
//     {
//     auto activityName = "messagevaUtf8CP";
//     auto activityId = "1E51";
    
//     auto onAct = [=] (ActivityLogger logger)
//         {
//         InvokeActionWithVaList([=, &logger] (va_list args)
//             {
//             logger.messageva("Testing.Namespace", NativeLogging::SEVERITY::LOG_ERROR, "Something %s logged", args);
//             }, "is");
//         };
    
//     auto expectedSeverity = NativeLogging::SEVERITY::LOG_ERROR;
//     auto expectedMessage = "messagevaUtf8CP: Something is logged (ActivityId: 1E51)";
//     auto expectedNameSpace = "Testing.Namespace";

//     TestActivityLoggerWithActivityId(activityName, activityId, onAct, expectedSeverity, expectedMessage, expectedNameSpace);
//     }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, fatal_WithActivityIdAndWCharCPMessage_LogsExpectedFatalMessage)
    {
    auto activityName = "fatalWCharCP";
    auto activityId = "1E51";
    
    auto onAct = [=] (ActivityLogger logger)
        {
        logger.fatal(L"Something is logged");
        };
    
    auto expectedSeverity = NativeLogging::SEVERITY::LOG_FATAL;
    auto expectedMessage = L"fatalWCharCP: Something is logged (ActivityId: 1E51)";

    TestActivityLoggerWithActivityId(activityName, activityId, onAct, expectedSeverity, expectedMessage);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, fatal_WithActivityIdAndUtf8CPMessage_LogsExpectedFatalMessage)
    {
    auto activityName = "fatalUtf8CP";
    auto activityId = "1E51";
    
    auto onAct = [=] (ActivityLogger logger)
        {
        logger.fatal("Something is logged");
        };
    
    auto expectedSeverity = NativeLogging::SEVERITY::LOG_FATAL;
    auto expectedMessage = "fatalUtf8CP: Something is logged (ActivityId: 1E51)";

    TestActivityLoggerWithActivityId(activityName, activityId, onAct, expectedSeverity, expectedMessage);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, fatalv_WithActivityIdAndWCharCPMessage_LogsExpectedFatalMessage)
    {
    auto activityName = "fatalvWCharCP";
    auto activityId = "1E51";
    
    auto onAct = [=] (ActivityLogger logger)
        {
        logger.fatalv(L"Something %ls logged", L"is");
        };
    
    auto expectedSeverity = NativeLogging::SEVERITY::LOG_FATAL;
    auto expectedMessage = L"fatalvWCharCP: Something is logged (ActivityId: 1E51)";

    TestActivityLoggerWithActivityId(activityName, activityId, onAct, expectedSeverity, expectedMessage);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, fatalv_WithActivityIdAndUtf8CPMessage_LogsExpectedFatalMessage)
    {
    auto activityName = "fatalvUtf8CP";
    auto activityId = "1E51";
    
    auto onAct = [=] (ActivityLogger logger)
        {
        logger.fatalv("Something %s logged", "is");
        };
    
    auto expectedSeverity = NativeLogging::SEVERITY::LOG_FATAL;
    auto expectedMessage = "fatalvUtf8CP: Something is logged (ActivityId: 1E51)";

    TestActivityLoggerWithActivityId(activityName, activityId, onAct, expectedSeverity, expectedMessage);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, error_WithActivityIdAndWCharCPMessage_LogsExpectedErrorMessage)
    {
    auto activityName = "errorWCharCP";
    auto activityId = "1E51";
    
    auto onAct = [=] (ActivityLogger logger)
        {
        logger.error(L"Something is logged");
        };
    
    auto expectedSeverity = NativeLogging::SEVERITY::LOG_ERROR;
    auto expectedMessage = L"errorWCharCP: Something is logged (ActivityId: 1E51)";

    TestActivityLoggerWithActivityId(activityName, activityId, onAct, expectedSeverity, expectedMessage);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, error_WithActivityIdAndUtf8CPMessage_LogsExpectedErrorMessage)
    {
    auto activityName = "errorUtf8CP";
    auto activityId = "1E51";
    
    auto onAct = [=] (ActivityLogger logger)
        {
        logger.error("Something is logged");
        };
    
    auto expectedSeverity = NativeLogging::SEVERITY::LOG_ERROR;
    auto expectedMessage = "errorUtf8CP: Something is logged (ActivityId: 1E51)";

    TestActivityLoggerWithActivityId(activityName, activityId, onAct, expectedSeverity, expectedMessage);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, errorv_WithActivityIdAndWCharCPMessage_LogsExpectedErrorMessage)
    {
    auto activityName = "errorvWCharCP";
    auto activityId = "1E51";
    
    auto onAct = [=] (ActivityLogger logger)
        {
        logger.errorv(L"Something %ls logged", L"is");
        };
    
    auto expectedSeverity = NativeLogging::SEVERITY::LOG_ERROR;
    auto expectedMessage = L"errorvWCharCP: Something is logged (ActivityId: 1E51)";

    TestActivityLoggerWithActivityId(activityName, activityId, onAct, expectedSeverity, expectedMessage);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, errorv_WithActivityIdAndUtf8CPMessage_LogsExpectedErrorMessage)
    {
    auto activityName = "errorvUtf8CP";
    auto activityId = "1E51";
    
    auto onAct = [=] (ActivityLogger logger)
        {
        logger.errorv("Something %s logged", "is");
        };
    
    auto expectedSeverity = NativeLogging::SEVERITY::LOG_ERROR;
    auto expectedMessage = "errorvUtf8CP: Something is logged (ActivityId: 1E51)";

    TestActivityLoggerWithActivityId(activityName, activityId, onAct, expectedSeverity, expectedMessage);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, warning_WithActivityIdAndWCharCPMessage_LogsExpectedWarningMessage)
    {
    auto activityName = "warningWCharCP";
    auto activityId = "1E51";
    
    auto onAct = [=] (ActivityLogger logger)
        {
        logger.warning(L"Something is logged");
        };
    
    auto expectedSeverity = NativeLogging::SEVERITY::LOG_WARNING;
    auto expectedMessage = L"warningWCharCP: Something is logged (ActivityId: 1E51)";

    TestActivityLoggerWithActivityId(activityName, activityId, onAct, expectedSeverity, expectedMessage);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, warning_WithActivityIdAndUtf8CPMessage_LogsExpectedWarningMessage)
    {
    auto activityName = "warningUtf8CP";
    auto activityId = "1E51";
    
    auto onAct = [=] (ActivityLogger logger)
        {
        logger.warning("Something is logged");
        };
    
    auto expectedSeverity = NativeLogging::SEVERITY::LOG_WARNING;
    auto expectedMessage = "warningUtf8CP: Something is logged (ActivityId: 1E51)";

    TestActivityLoggerWithActivityId(activityName, activityId, onAct, expectedSeverity, expectedMessage);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, warningv_WithActivityIdAndWCharCPMessage_LogsExpectedWarningMessage)
    {
    auto activityName = "warningvWCharCP";
    auto activityId = "1E51";
    
    auto onAct = [=] (ActivityLogger logger)
        {
        logger.warningv(L"Something %ls logged", L"is");
        };
    
    auto expectedSeverity = NativeLogging::SEVERITY::LOG_WARNING;
    auto expectedMessage = L"warningvWCharCP: Something is logged (ActivityId: 1E51)";

    TestActivityLoggerWithActivityId(activityName, activityId, onAct, expectedSeverity, expectedMessage);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, warningv_WithActivityIdAndUtf8CPMessage_LogsExpectedWarningMessage)
    {
    auto activityName = "warningvUtf8CP";
    auto activityId = "1E51";
    
    auto onAct = [=] (ActivityLogger logger)
        {
        logger.warningv("Something %s logged", "is");
        };
    
    auto expectedSeverity = NativeLogging::SEVERITY::LOG_WARNING;
    auto expectedMessage = "warningvUtf8CP: Something is logged (ActivityId: 1E51)";

    TestActivityLoggerWithActivityId(activityName, activityId, onAct, expectedSeverity, expectedMessage);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, info_WithActivityIdAndWCharCPMessage_LogsExpectedInfoMessage)
    {
    auto activityName = "infoWCharCP";
    auto activityId = "1E51";
    
    auto onAct = [=] (ActivityLogger logger)
        {
        logger.info(L"Something is logged");
        };
    
    auto expectedSeverity = NativeLogging::SEVERITY::LOG_INFO;
    auto expectedMessage = L"infoWCharCP: Something is logged (ActivityId: 1E51)";

    TestActivityLoggerWithActivityId(activityName, activityId, onAct, expectedSeverity, expectedMessage);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, info_WithActivityIdAndUtf8CPMessage_LogsExpectedInfoMessage)
    {
    auto activityName = "infoUtf8CP";
    auto activityId = "1E51";
    
    auto onAct = [=] (ActivityLogger logger)
        {
        logger.info("Something is logged");
        };
    
    auto expectedSeverity = NativeLogging::SEVERITY::LOG_INFO;
    auto expectedMessage = "infoUtf8CP: Something is logged (ActivityId: 1E51)";

    TestActivityLoggerWithActivityId(activityName, activityId, onAct, expectedSeverity, expectedMessage);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, infov_WithActivityIdAndWCharCPMessage_LogsExpectedInfoMessage)
    {
    auto activityName = "infovWCharCP";
    auto activityId = "1E51";
    
    auto onAct = [=] (ActivityLogger logger)
        {
        logger.infov(L"Something %ls logged", L"is");
        };
    
    auto expectedSeverity = NativeLogging::SEVERITY::LOG_INFO;
    auto expectedMessage = L"infovWCharCP: Something is logged (ActivityId: 1E51)";

    TestActivityLoggerWithActivityId(activityName, activityId, onAct, expectedSeverity, expectedMessage);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, infov_WithActivityIdAndUtf8CPMessage_LogsExpectedInfoMessage)
    {
    auto activityName = "infovUtf8CP";
    auto activityId = "1E51";
    
    auto onAct = [=] (ActivityLogger logger)
        {
        logger.infov("Something %s logged", "is");
        };
    
    auto expectedSeverity = NativeLogging::SEVERITY::LOG_INFO;
    auto expectedMessage = "infovUtf8CP: Something is logged (ActivityId: 1E51)";

    TestActivityLoggerWithActivityId(activityName, activityId, onAct, expectedSeverity, expectedMessage);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, debug_WithActivityIdAndWCharCPMessage_LogsExpectedDebugMessage)
    {
    auto activityName = "debugWCharCP";
    auto activityId = "1E51";
    
    auto onAct = [=] (ActivityLogger logger)
        {
        logger.debug(L"Something is logged");
        };
    
    auto expectedSeverity = NativeLogging::SEVERITY::LOG_DEBUG;
    auto expectedMessage = L"debugWCharCP: Something is logged (ActivityId: 1E51)";

    TestActivityLoggerWithActivityId(activityName, activityId, onAct, expectedSeverity, expectedMessage);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, debug_WithActivityIdAndUtf8CPMessage_LogsExpectedDebugMessage)
    {
    auto activityName = "debugUtf8CP";
    auto activityId = "1E51";
    
    auto onAct = [=] (ActivityLogger logger)
        {
        logger.debug("Something is logged");
        };
    
    auto expectedSeverity = NativeLogging::SEVERITY::LOG_DEBUG;
    auto expectedMessage = "debugUtf8CP: Something is logged (ActivityId: 1E51)";

    TestActivityLoggerWithActivityId(activityName, activityId, onAct, expectedSeverity, expectedMessage);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, debugv_WithActivityIdAndWCharCPMessage_LogsExpectedDebugMessage)
    {
    auto activityName = "debugvWCharCP";
    auto activityId = "1E51";
    
    auto onAct = [=] (ActivityLogger logger)
        {
        logger.debugv(L"Something %ls logged", L"is");
        };
    
    auto expectedSeverity = NativeLogging::SEVERITY::LOG_DEBUG;
    auto expectedMessage = L"debugvWCharCP: Something is logged (ActivityId: 1E51)";

    TestActivityLoggerWithActivityId(activityName, activityId, onAct, expectedSeverity, expectedMessage);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, debugv_WithActivityIdAndUtf8CPMessage_LogsExpectedDebugMessage)
    {
    auto activityName = "debugvUtf8CP";
    auto activityId = "1E51";
    
    auto onAct = [=] (ActivityLogger logger)
        {
        logger.debugv("Something %s logged", "is");
        };
    
    auto expectedSeverity = NativeLogging::SEVERITY::LOG_DEBUG;
    auto expectedMessage = "debugvUtf8CP: Something is logged (ActivityId: 1E51)";

    TestActivityLoggerWithActivityId(activityName, activityId, onAct, expectedSeverity, expectedMessage);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, trace_WithActivityIdAndWCharCPMessage_LogsExpectedTraceMessage)
    {
    auto activityName = "traceWCharCP";
    auto activityId = "1E51";
    
    auto onAct = [=] (ActivityLogger logger)
        {
        logger.trace(L"Something is logged");
        };
    
    auto expectedSeverity = NativeLogging::SEVERITY::LOG_TRACE;
    auto expectedMessage = L"traceWCharCP: Something is logged (ActivityId: 1E51)";

    TestActivityLoggerWithActivityId(activityName, activityId, onAct, expectedSeverity, expectedMessage);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, trace_WithActivityIdAndUtf8CPMessage_LogsExpectedTraceMessage)
    {
    auto activityName = "traceUtf8CP";
    auto activityId = "1E51";
    
    auto onAct = [=] (ActivityLogger logger)
        {
        logger.trace("Something is logged");
        };
    
    auto expectedSeverity = NativeLogging::SEVERITY::LOG_TRACE;
    auto expectedMessage = "traceUtf8CP: Something is logged (ActivityId: 1E51)";

    TestActivityLoggerWithActivityId(activityName, activityId, onAct, expectedSeverity, expectedMessage);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, tracev_WithActivityIdAndWCharCPMessage_LogsExpectedTraceMessage)
    {
    auto activityName = "tracevWCharCP";
    auto activityId = "1E51";
    
    auto onAct = [=] (ActivityLogger logger)
        {
        logger.tracev(L"Something %ls logged", L"is");
        };
    
    auto expectedSeverity = NativeLogging::SEVERITY::LOG_TRACE;
    auto expectedMessage = L"tracevWCharCP: Something is logged (ActivityId: 1E51)";

    TestActivityLoggerWithActivityId(activityName, activityId, onAct, expectedSeverity, expectedMessage);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityLoggerTests, tracev_WithActivityIdAndUtf8CPMessage_LogsExpectedTraceMessage)
    {
    auto activityName = "tracevUtf8CP";
    auto activityId = "1E51";
    
    auto onAct = [=] (ActivityLogger logger)
        {
        logger.tracev("Something %s logged", "is");
        };
    
    auto expectedSeverity = NativeLogging::SEVERITY::LOG_TRACE;
    auto expectedMessage = "tracevUtf8CP: Something is logged (ActivityId: 1E51)";

    TestActivityLoggerWithActivityId(activityName, activityId, onAct, expectedSeverity, expectedMessage);
    }