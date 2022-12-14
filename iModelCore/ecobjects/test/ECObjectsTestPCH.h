/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Bentley.h>
#include <ECObjects/ECObjectsAPI.h>
#include <ECObjects/DesignByContract.h>
#include <Bentley/WString.h>
#include <Bentley/BeTest.h>
#include <Bentley/BeFileName.h>
#include <Bentley/Logging.h>
#include <cmath>

#define LOG (NativeLogging::CategoryLogger("ECObjectsNative"))

#define BEGIN_BENTLEY_ECN_TEST_NAMESPACE  BEGIN_BENTLEY_NAMESPACE namespace ECNTests {
#define END_BENTLEY_ECN_TEST_NAMESPACE    } END_BENTLEY_NAMESPACE

//define DesignByContract macros here locally in a simplified form until it is available from the API.
#ifndef PRECONDITION

#define ASSERT_FALSE_IF_NOT_DISABLED(_Message)    (void)((AssertDisabler::AreAssertsDisabled()) || (BeAssert(_Message), 0))

#define LOG_ASSERT_FAILURE(_LogMessage, ...) LogFailureMessage(_LogMessage, ## __VA_ARGS__)
BEGIN_BENTLEY_ECOBJECT_NAMESPACE
ECOBJECTS_EXPORT void LogFailureMessage (WCharCP message, ...);
END_BENTLEY_ECOBJECT_NAMESPACE

#define LOG_ASSERT_RETURN(_Expression, _ErrorStatus, _LogMessage, ...)  \
        {                                                               \
        LOG_ASSERT_FAILURE (_LogMessage, ## __VA_ARGS__);               \
        ASSERT_FALSE_IF_NOT_DISABLED(_Expression);                      \
        return _ErrorStatus;                                            \
        }

#define EXPECT_CONDITION_LOG_ASSERT_RETURN(_Expression, _ErrorStatus, _LogMessage, ...) \
    {                                                                                   \
    if (!(_Expression))                                                                 \
        {                                                                               \
        LOG_ASSERT_RETURN(_Expression, _ErrorStatus, _LogMessage, ## __VA_ARGS__)       \
        }                                                                               \
    }

#define PRECONDITION(_Expression, _ErrorStatus)                                                                                 \
        EXPECT_CONDITION_LOG_ASSERT_RETURN(_Expression, _ErrorStatus,                                                           \
        L"The following method precondition check has failed:\n  precondition: %hs\n  method: %hs\n  file: %hs\n  line: %i\n",  \
        #_Expression, __FUNCTION__, __FILE__, __LINE__)

#define EXPECTED_CONDITION(_Expression)     ( (_Expression)                                                                                                                                             \
    || (LOG_ASSERT_FAILURE(L"The following expected condition has failed:\n  expected condition: %hs\n  method: %hs\n  file: %hs\n  line: %i\n", #_Expression, __FUNCTION__, __FILE__, __LINE__), 0)    \
    || (ASSERT_FALSE_IF_NOT_DISABLED (_Expression), 0) )

#define POSTCONDITION(_Expression, _ErrorStatus)                                                                                        \
        EXPECT_CONDITION_LOG_ASSERT_RETURN(_Expression, _ErrorStatus,                                                                   \
            L"The following method postcondition check has failed:\n  postcondition: %hs\n  method: %hs\n  file: %hs\n  line: %i\n",    \
            #_Expression, __FUNCTION__, __FILE__, __LINE__)

#endif
