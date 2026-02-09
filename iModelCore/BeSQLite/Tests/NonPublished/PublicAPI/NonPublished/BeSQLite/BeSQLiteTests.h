/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#pragma once

#include <BeSQLite/BeSQLite.h>
#include <Bentley/BeTest.h>
#include <Bentley/BeTimeUtilities.h>
#include <Bentley/Logging.h>

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_LOGGING

#define LOG (*BentleyApi::NativeLogging::LoggingManager::GetLogger(L"BeSQLiteTest"))
