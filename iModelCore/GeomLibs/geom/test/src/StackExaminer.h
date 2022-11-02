/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
+--------------------------------------------------------------------------------------*/
// Based on http://cplus.about.com/od/howtodothingsi2/a/timing.htm
#pragma once
#include <windows.h>

void getCurrentThreadStackInfo
(
DWORD_PTR * pBase,   // <= base (highest) address value
DWORD64 *     pSize,   // <= max size of stack
DWORD64 *     pPeak    // <= peak usage of stack 
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void reclaimUnusedStackPages
(
);
