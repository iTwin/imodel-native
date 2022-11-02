/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/Logging.h>

#define PERFLOG (NativeLogging::CategoryLogger("Performance"))
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

