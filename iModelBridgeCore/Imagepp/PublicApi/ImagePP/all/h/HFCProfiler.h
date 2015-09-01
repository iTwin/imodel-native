//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCProfiler.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "winbase.h"

#include <Imagepp/all/h/HFCExclusiveKey.h>
#include "HFCMacros.h"
#include "HFCMonitor.h"

BEGIN_IMAGEPP_NAMESPACE
/**

  This class is a basic profiler that reports the same kind of
  information as Microsoft's built-in profiler. However, it is
  possible to instrument inline methods or method portions. Also,
  this profiler works in multithreaded applications.

  Instrumentation must be done by hand using the macros at the end
  of the .hpp file.

*/
class HNOVTABLEINIT HFCProfiler
    {
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HFCProfiler)

public:

    HFCProfiler();
    ~HFCProfiler();


    size_t          RegisterCounter(const WString& pi_rName);

    LARGE_INTEGER   Start(int pi_CounterID);
    void            Stop(int pi_CounterID, LARGE_INTEGER);

    void            DumpStatistics();

    typedef vector<int> IntVector;

private:

    struct Counter
        {
        Counter(const WString& pi_rName)
            : m_Name(pi_rName)
            {
            m_Calls = 0;
            m_Overheads = 0;
            m_TotalMilliSeconds = 0.0;
            m_ChildrenMilliSeconds = 0.0;
            }
        Counter(const Counter& pi_rObj)
            : m_Name(pi_rObj.m_Name)
            {
            m_Calls = pi_rObj.m_Calls;
            m_Overheads = pi_rObj.m_Overheads;
            m_TotalMilliSeconds = pi_rObj.m_TotalMilliSeconds;
            m_ChildrenMilliSeconds = pi_rObj.m_ChildrenMilliSeconds;
            }

        Counter& operator=(const Counter& pi_rObj)
            {
            m_Name = pi_rObj.m_Name;
            m_Calls = pi_rObj.m_Calls;
            m_Overheads = pi_rObj.m_Overheads;
            m_TotalMilliSeconds = pi_rObj.m_TotalMilliSeconds;
            m_ChildrenMilliSeconds = pi_rObj.m_ChildrenMilliSeconds;

            return *this;
            }

        bool operator<(const Counter& pi_rObj)
            {
            return (m_TotalMilliSeconds - m_ChildrenMilliSeconds) <
                   (pi_rObj.m_TotalMilliSeconds - pi_rObj.m_ChildrenMilliSeconds);
            }

        WString m_Name;
        int     m_Calls;
        int     m_Overheads;
        double  m_TotalMilliSeconds;
        double  m_ChildrenMilliSeconds;

        HFCExclusiveKey m_CounterKey;
        };

    // We keep one counter for each registered function
    typedef vector<Counter> Counters;
    Counters        m_Counters;

    // These represent per-thread call-stack and overhead count
    // for each function.
    __declspec(thread) static IntVector*
    s_pFunctionStack;
    __declspec(thread) static IntVector*
    s_pOverheads;

    HFCExclusiveKey m_ProfilerKey;

    // Used to debug start/stop mismatches
    bool           m_BreakOnError;

    // This is (1 / QueryPerformanceFrequency()) * 1000,
    // so that (stop-start) * ThisVariable gives milliseconds.
    double          m_FrequencyInverseForMillis;

    // The overhead is the time spent in the Start and Stop methods.
    double          m_Overhead;
    };

END_IMAGEPP_NAMESPACE
#include "HFCProfiler.hpp"

