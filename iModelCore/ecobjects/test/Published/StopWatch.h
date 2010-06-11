/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/nativeatp/Published/StopWatch.h $
|
|  $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
| Based on http://cplus.about.com/od/howtodothingsi2/a/timing.htm
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <windows.h>
#include <string>

namespace Bentley {

class StopWatch 
    {
private:
    bwstring       m_description;
    LARGE_INTEGER m_start;
    LARGE_INTEGER m_stop;
    LARGE_INTEGER m_frequency;
    
    double ConvertLongIntegerToSeconds( LARGE_INTEGER & L);
    
public:
    StopWatch(wchar_t * description = L"", bool startImmediately = false);
    void         Start();
    void         Stop();
    double       GetElapsedSeconds();
    bwstring GetDescription();
    };
    
} // Bentley