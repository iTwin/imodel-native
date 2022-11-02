/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
// Should only be included from .cpp files in DgnCore that wish to log with DgnCore namespace

#include <Bentley/Logging.h>
#define LOG NativeLogging::CategoryLogger("DgnCore")
#define THREADLOG NativeLogging::CategoryLogger("DgnCore")
