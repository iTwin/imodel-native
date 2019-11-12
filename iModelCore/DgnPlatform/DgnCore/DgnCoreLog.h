/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
// Should only be included from .cpp files in DgnCore that wish to log with DgnCore namespace

#include <Logging/bentleylogging.h>
#define LOG (*NativeLogging::LoggingManager::GetLogger(L"DgnCore"))
#define THREADLOG (*NativeLogging::LoggingManager::GetLogger(DgnDb::GetThreadIdName()))
