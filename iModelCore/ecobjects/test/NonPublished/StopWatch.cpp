/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublished/StopWatch.cpp $
|
|  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
| Based on http://cplus.about.com/od/howtodothingsi2/a/timing.htm
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"
#include "StopWatch.h"

namespace Bentley {

double StopWatch::ConvertLongIntegerToSeconds(LARGE_INTEGER & L) 
    {
    return ((double)L.QuadPart / (double)m_frequency.QuadPart) ;
    }

StopWatch::StopWatch(wchar_t * description, bool startImmediately) : m_description (description)
    {
    m_start.QuadPart=0;
    m_stop.QuadPart=0; 
    QueryPerformanceFrequency(&m_frequency);
    
    if (startImmediately)
        Start();
    }

void StopWatch::Start() 
    {
    QueryPerformanceCounter(&m_start);
    }

void StopWatch::Stop() 
    {
    QueryPerformanceCounter(&m_stop);
    }

double StopWatch::GetElapsedSeconds()
    {
    LARGE_INTEGER time;
    time.QuadPart = m_stop.QuadPart - m_start.QuadPart;
    return ConvertLongIntegerToSeconds (time) ;
    }

WString StopWatch::GetDescription()
    {
    return m_description;
    }    

} // Bentley
