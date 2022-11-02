/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#ifndef DGNPLATFORMTEST_COMMON_BUILD
    #define DGNPLATFORMTEST_COMMON_EXPORT __declspec(dllimport)
#else
    #define DGNPLATFORMTEST_COMMON_EXPORT __declspec(dllexport)
#endif
