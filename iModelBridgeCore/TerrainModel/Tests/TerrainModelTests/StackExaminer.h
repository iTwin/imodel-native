/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
| Based on http://cplus.about.com/od/howtodothingsi2/a/timing.htm
|
+--------------------------------------------------------------------------------------*/
#pragma once
#define NOMINMAX
#include <windows.h>

void getCurrentThreadStackInfo
(
DWORD_PTR * pBase,   // <= base (highest) address value
DWORD64 *     pSize,   // <= max size of stack
DWORD64 *     pPeak    // <= peak usage of stack 
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                           GeorgeDulchinos      04/10
+---------------+---------------+---------------+---------------+---------------+------*/
void reclaimUnusedStackPages
(
);
