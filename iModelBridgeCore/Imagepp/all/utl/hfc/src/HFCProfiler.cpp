//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCProfiler.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>
    //:> must be first for PreCompiledHeader Option

#include <Imagepp/all/h/HFCProfiler.h>


HFC_IMPLEMENT_SINGLETON(HFCProfiler)


__declspec(thread) HFCProfiler::IntVector* HFCProfiler::s_pFunctionStack = 0;
__declspec(thread) HFCProfiler::IntVector* HFCProfiler::s_pOverheads = 0;

/** ---------------------------------------------------------------------------
    Constructor.
    ---------------------------------------------------------------------------
 */
HFCProfiler::HFCProfiler()
    {
    m_BreakOnError = false;
    m_Overhead = 0.0;

    LARGE_INTEGER Frequency;
    QueryPerformanceFrequency(&Frequency);
    m_FrequencyInverseForMillis = 1000.0 / (double)Frequency.QuadPart;

    // Compute the profiler overhead (start & stop functions)

    LARGE_INTEGER StartStamp;
    LARGE_INTEGER StopStamp;
    RegisterCounter(_T("StartupParent"));
    RegisterCounter(_T("StartupChild"));

    LARGE_INTEGER AValue = Start(0);
    LARGE_INTEGER BValue;
    double MinimumOverhead = 1000.0;  // 1 second :-)
    double CurrentOverhead;
    for (int i = 0 ; i < 1000 ; ++i)
        {
        QueryPerformanceCounter(&StartStamp);
        BValue = Start(1);
        Stop(1, BValue);
        QueryPerformanceCounter(&StopStamp);
        CurrentOverhead = ((double)StopStamp.QuadPart - (double)StartStamp.QuadPart) * m_FrequencyInverseForMillis;
        MinimumOverhead = MIN(MinimumOverhead, CurrentOverhead);
        }
    Stop(0, AValue);

    m_Overhead = MinimumOverhead;

    m_Counters.pop_back();
    m_Counters.pop_back();
    }


/** ---------------------------------------------------------------------------
    Destructor.
    ---------------------------------------------------------------------------
 */
HFCProfiler::~HFCProfiler()
    {
    DumpStatistics();
    }


//-----------------------------------------------------------------------------
// Add a function
//-----------------------------------------------------------------------------
size_t HFCProfiler::RegisterCounter(const WString& pi_rName)
    {
    HFCMonitor Monitor(m_ProfilerKey);

    size_t Position = m_Counters.size();
    m_Counters.push_back(Counter(pi_rName));
    return Position;
    }


//-----------------------------------------------------------------------------
// Printout statistics for the current run
//-----------------------------------------------------------------------------
void HFCProfiler::DumpStatistics()
    {
    double TotalTime = 0.0;
    int TotalCalls = 0;

    Counters::iterator Itr(m_Counters.begin());
    while (Itr != m_Counters.end())
        {
        TotalTime  += Itr->m_TotalMilliSeconds - Itr->m_ChildrenMilliSeconds;
        TotalCalls += Itr->m_Calls;

        ++Itr;
        }

    // template for cout and wout. ex. _tout ?
    char Line[1024];
    cout << "================================================================" << endl;
    sprintf(Line, "Total Time:     %12.3lf ms", TotalTime);
    cout << Line << endl;
    sprintf(Line, "Total Calls: %11ld", TotalCalls);
    cout << Line << endl;
    sprintf(Line, "Computed overhead:   %10lf ms", m_Overhead);
    cout << Line << endl;

    cout << endl;
    cout << endl;

    cout << " FuncTime      %     FuncTotal    %     Calls     SubCalls  Name" << endl;
    cout << "----------------------------------------------------------------" << endl;

    sort(m_Counters.begin(), m_Counters.end());

    Counters::reverse_iterator ReverseItr = m_Counters.rbegin();
    while (ReverseItr != m_Counters.rend())
        {
        double FunctionLocalTime = ReverseItr->m_TotalMilliSeconds - ReverseItr->m_ChildrenMilliSeconds;
        sprintf(Line, "%12.3lf %5.1lf %12.3lf %5.1lf %10ld %10ld %s\n",
                FunctionLocalTime,
                FunctionLocalTime / TotalTime * 100.0,
                ReverseItr->m_TotalMilliSeconds,
                ReverseItr->m_TotalMilliSeconds / TotalTime * 100.0,
                ReverseItr->m_Calls,
                ReverseItr->m_Overheads,
                ReverseItr->m_Name.c_str());

        cout << Line;

        ++ReverseItr;
        }
    }
