/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/BeHttp/Tests.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "../../UnitTests/Published/ValuePrinter.h"
#include <Bentley/BeTest.h>

#define LOGGER_NAMESPACE_BEHTTP_TESTS "BeHttp.Tests"
#define TESTLOG (*BentleyApi::NativeLogging::LoggingManager::GetLogger (LOGGER_NAMESPACE_BEHTTP_TESTS))