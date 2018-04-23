/*--------------------------------------------------------------------------------------+
|
|     $Source: src/native/log4cxx/CustomAppenders/androidlogprintappender.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <log4cxx/appenderskeleton.h>

namespace log4cxx
{
/*--------------------------------------------------------------------------------------+
* @bsiclass                                                  Robert.Lukasonok   03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct AndroidLogPrintAppender : AppenderSkeleton
    {
    private:
        static int ResolvePriority(const LevelPtr& level);
        static size_t GetNextChunkLength(const std::string& message, size_t startIndex);

    public:
        DECLARE_LOG4CXX_OBJECT(AndroidLogPrintAppender)
        BEGIN_LOG4CXX_CAST_MAP()
            LOG4CXX_CAST_ENTRY(AndroidLogPrintAppender)
            LOG4CXX_CAST_ENTRY_CHAIN(AppenderSkeleton)
        END_LOG4CXX_CAST_MAP()

        AndroidLogPrintAppender() {}

        bool requiresLayout() const override { return true; }

        void close() override {}

        void append(const spi::LoggingEventPtr& event, helpers::Pool& p) override;
    };
}
