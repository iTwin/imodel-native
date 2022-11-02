/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
| Based on http://cplus.about.com/od/howtodothingsi2/a/timing.htm
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <windows.h>
#include <string>
#include "CommonExport.h"

namespace Bentley {

class StopWatch 
    {
private:
    std::wstring       m_description;
    LARGE_INTEGER m_start;
    LARGE_INTEGER m_stop;
    LARGE_INTEGER m_frequency;
    
    double ConvertLongIntegerToSeconds( LARGE_INTEGER & L);
    
public:
    DGNPLATFORMTEST_COMMON_EXPORT StopWatch(wchar_t * description = L"", bool startImmediately = false);
    DGNPLATFORMTEST_COMMON_EXPORT void         Start();
    DGNPLATFORMTEST_COMMON_EXPORT void         Stop();
    DGNPLATFORMTEST_COMMON_EXPORT double       GetElapsedSeconds();
    DGNPLATFORMTEST_COMMON_EXPORT std::wstring GetDescription();
    };
    
} // Bentley
