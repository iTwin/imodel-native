/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#ifndef DGNPLATFORMTEST_COMMON_BUILD
    #define DGNPLATFORMTEST_COMMON_EXPORT __declspec(dllimport)
#else
    #define DGNPLATFORMTEST_COMMON_EXPORT __declspec(dllexport)
#endif
