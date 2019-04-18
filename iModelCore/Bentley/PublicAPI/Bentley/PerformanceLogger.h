/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <Logging/bentleylogging.h>

#define PERFLOG (*NativeLogging::LoggingManager::GetLogger (L"Performance"))
#define PERFLOG_START(application,feature) PERFLOG.infov("%s,%s,Start,%" PRIu64, (application), (feature), BeTimeUtilities::GetCurrentTimeAsUnixMillis())
#define PERFLOG_FINISH(application,feature) PERFLOG.infov("%s,%s,Finish,%" PRIu64, (application), (feature), BeTimeUtilities::GetCurrentTimeAsUnixMillis())

/*
 * Usage Examples:
 *
 * PERFLOG_START("Core", "Opening the DgnDb")
 * PERFLOG_FINISH("Core", "Opening the DgnDb")
 *
 * PERFLOG_START("Navigator", "Opening the DgnDb")
 * PERFLOG_FINISH("Navigator", "Opening the DgnDb")
 *
 */

