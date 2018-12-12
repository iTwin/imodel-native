#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <Logging/bentleylogging.h>

BEGIN_BENTLEY_NAMESPACE

struct BeConsole
    {
    static int Printf(CharCP fmt, ...) 
        {
        va_list args;
        va_start(args, fmt);
        NativeLogging::LoggingManager::GetLogger("BeConsole")->messageva(NativeLogging::LOG_DEBUG, fmt, args);
        va_end(args);
        return 1;
        }

    static int WPrintf(WCharCP fmt, ...) 
        {
        va_list args;
        va_start(args, fmt);
        NativeLogging::LoggingManager::GetLogger("BeConsole")->messageva(NativeLogging::LOG_DEBUG, fmt, args);
        va_end(args);
        return 1;
        }

    };

END_BENTLEY_NAMESPACE
