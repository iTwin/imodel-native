/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "../../UnitTests/NonPublished/ValuePrinter.h"
#include <Bentley/BeTest.h>

#define LOGGER_NAMESPACE_BEHTTP_TESTS "BeHttp.Tests"
#define TESTLOG (*BentleyApi::NativeLogging::LoggingManager::GetLogger (LOGGER_NAMESPACE_BEHTTP_TESTS))