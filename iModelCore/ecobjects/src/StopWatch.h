/*--------------------------------------------------------------------------------------+
|
|     $Source: src/StopWatch.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
| Based on http://cplus.about.com/od/howtodothingsi2/a/timing.htm
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/
#include <windows.h>
#include <string>

BEGIN_BENTLEY_EC_NAMESPACE

class StopWatch 
    {
private:
    WString       m_description;
    LARGE_INTEGER m_start;
    LARGE_INTEGER m_stop;
    LARGE_INTEGER m_frequency;
    
    double ConvertLongIntegerToSeconds( LARGE_INTEGER & L);
    
public:
    StopWatch(WCharP description = L"", bool startImmediately = false);
    void         Start();
    void         Stop();
    double       GetElapsedSeconds();
    WString GetDescription();
    };
    
END_BENTLEY_EC_NAMESPACE