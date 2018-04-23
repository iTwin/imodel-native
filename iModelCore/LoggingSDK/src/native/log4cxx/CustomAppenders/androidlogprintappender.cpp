/*--------------------------------------------------------------------------------------+
|
|     $Source: src/native/log4cxx/CustomAppenders/androidlogprintappender.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "androidlogprintappender.h"

#include <android/log.h>
#include <log4cxx/helpers/transcoder.h>

using namespace log4cxx;
using namespace log4cxx::helpers;

const size_t MESSAGE_SIZE_LIMIT = 1023;

IMPLEMENT_LOG4CXX_OBJECT(AndroidLogPrintAppender);

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 Robert.Lukasonok   03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void AndroidLogPrintAppender::append(const spi::LoggingEventPtr& event, Pool& pool)
    {
    int priority = ResolvePriority(event->getLevel());

    LogString buffer;
    this->layout->format(buffer, event, pool);
    LOG4CXX_ENCODE_CHAR(message, buffer);
    LOG4CXX_ENCODE_CHAR(loggerNamespace, event->getLoggerName());

    size_t messageSize = message.size();
    size_t startIndex = 0;
    while (startIndex < messageSize)
        {
        size_t length = GetNextChunkLength(message, startIndex);
        __android_log_print(priority, loggerNamespace.c_str(), "%.*s", static_cast<int>(length), startIndex + message.c_str());
        startIndex += length + 1;
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 Robert.Lukasonok   03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
int AndroidLogPrintAppender::ResolvePriority(const LevelPtr& level)
    {
    if (level->equals(Level::getFatal()))
        return ANDROID_LOG_FATAL;
    if (level->equals(Level::getError()))
        return ANDROID_LOG_ERROR;
    if (level->equals(Level::getWarn()))
        return ANDROID_LOG_WARN;
    if (level->equals(Level::getInfo()))
        return ANDROID_LOG_INFO;
    if (level->equals(Level::getDebug()))
        return ANDROID_LOG_DEBUG;
    if (level->equals(Level::getTrace()))
        return ANDROID_LOG_VERBOSE;

    if (level->equals(Level::getAll()))
        return ANDROID_LOG_VERBOSE;
    if (level->equals(Level::getOff()))
        return ANDROID_LOG_SILENT;

    return ANDROID_LOG_DEFAULT;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 Robert.Lukasonok   03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
size_t AndroidLogPrintAppender::GetNextChunkLength(const std::string& message, size_t startIndex)
    {
    size_t nextNewLine = message.find('\n', startIndex);
    if (std::string::npos == nextNewLine)
        return message.size() - startIndex;

    size_t chunkLength = nextNewLine - startIndex;
    while (chunkLength < MESSAGE_SIZE_LIMIT && startIndex + chunkLength < message.size())
        {
        nextNewLine = message.find('\n', nextNewLine + 1);
        if (std::string::npos == nextNewLine)
            nextNewLine = message.size();

        if (nextNewLine - startIndex > MESSAGE_SIZE_LIMIT)
            return chunkLength;

        chunkLength = nextNewLine - startIndex;
        }

    return chunkLength;
    }
