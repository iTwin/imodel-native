/*--------------------------------------------------------------------------------------+
|
|  $Source: tests/FormattingTestsPch.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Bentley.h>
#include <Bentley/bmap.h>
#include <Bentley/BeTest.h>
#include <Bentley/BeTimeUtilities.h>
#include <Logging/bentleylogging.h>
#include <Formatting/FormattingApi.h>
#include "UnitsTestsPch.h"

#define LOG (*NativeLogging::LoggingManager::GetLogger (L"Format"))

#define BEGIN_BENTLEY_FORMATTEST_NAMESPACE BEGIN_BENTLEY_NAMESPACE namespace FormatTests {
#define END_BENTLEY_FORMATTEST_NAMESPACE } END_BENTLEY_NAMESPACE

using namespace std;
