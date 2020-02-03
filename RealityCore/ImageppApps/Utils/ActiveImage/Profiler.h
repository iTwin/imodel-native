/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/Profiler.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#define ENABLE_PROFILER

struct Profiler
    {
    inline Profiler(std::string const& name)
        {
#ifdef ENABLE_PROFILER
        LARGE_INTEGER frequency;
        QueryPerformanceFrequency(&frequency);

        m_frenqencyFactor = (1000.0 / (double)frequency.QuadPart) / 1000.0;

        m_name = name;
        m_MinTime = DBL_MAX;
        m_MaxTime = DBL_MIN;
        m_nbcall = 0;
        m_totalEllapsedTime = 0;
        m_StarTime.QuadPart = 0;
#endif
        }

    inline void Start()
        {
#ifdef ENABLE_PROFILER
        ++m_nbcall;
        QueryPerformanceCounter(&m_StarTime);
#endif
        }

    inline void End()
        {
#ifdef ENABLE_PROFILER
        LARGE_INTEGER StopTime;
        QueryPerformanceCounter(&StopTime);

        double ellapsedTime = (StopTime.QuadPart - m_StarTime.QuadPart) * m_frenqencyFactor;

        if(ellapsedTime < m_MinTime)
            m_MinTime = ellapsedTime;

        if(ellapsedTime > m_MaxTime)
            m_MaxTime = ellapsedTime;

        m_totalEllapsedTime += ellapsedTime;
#endif
        }

    inline void Print()
        {
#ifdef ENABLE_PROFILER
        printf(PrintToString().c_str());
#endif
        }

    string PrintToString()
        {
        char msg[2048] = "";
#ifdef ENABLE_PROFILER
            if(m_nbcall)
            {
                double averageTime = m_totalEllapsedTime / m_nbcall;

                sprintf(msg, "[%s] Average: %6.4f, Min: %6.4f, Max: %6.4f, Total: %6.4lf - Calls: %6ld \n", 
                    m_name.c_str(), averageTime,  m_MinTime, m_MaxTime, m_totalEllapsedTime, m_nbcall);
            }
            else
            {
                sprintf(msg, "[%s] Average: %6.4f, Min: %6.4f, Max: %6.4f, Total: %6.4lf - Calls: %6ld \n", 
                    m_name.c_str(), 0.0,  0.0, 0.0, 0.0, m_nbcall);

            }
#endif
            return msg;
        }

    std::string     m_name;
    double          m_frenqencyFactor;
    LARGE_INTEGER   m_StarTime;
    double          m_totalEllapsedTime; 
    double          m_MinTime;  
    double          m_MaxTime;
    ULONG           m_nbcall;
    };