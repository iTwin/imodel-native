/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once


#include <Bentley/Logging.h>

BEGIN_BENTLEY_NAMESPACE

struct BeConsole
    {
    static int Printf(CharCP fmt, ...)
        {
        va_list args;
        va_start(args, fmt);
        NativeLogging::Logging::LogMessageV("BeConsole", NativeLogging::LOG_DEBUG, fmt, args);
        va_end(args);
        return 1;
        }

    static int WPrintf(WCharCP fmt, ...)
        {
        va_list args;
        va_start(args, fmt);
        NativeLogging::Logging::LogMessageVW("BeConsole", NativeLogging::LOG_DEBUG, fmt, args);
        va_end(args);
        return 1;
        }

    };

END_BENTLEY_NAMESPACE
