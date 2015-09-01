//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCProfiler.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Start counting
//-----------------------------------------------------------------------------
__forceinline LARGE_INTEGER HFCProfiler::Start(int pi_CounterID)
    {
    // Increment call count
    m_Counters[pi_CounterID].m_CounterKey.ClaimKey();
    ++m_Counters[pi_CounterID].m_Calls;
    m_Counters[pi_CounterID].m_CounterKey.ReleaseKey();

    // Allocate per-thread call stack if necessary
    if (s_pFunctionStack == 0)
        {
        s_pFunctionStack = new IntVector;
        s_pFunctionStack->reserve(30);
        s_pOverheads = new IntVector;
        s_pOverheads->reserve(30);
        }

    // This function is officially starting.
    s_pFunctionStack->push_back(pi_CounterID);
    s_pOverheads->push_back(0);

    // *** This must be the last task ***
    LARGE_INTEGER Start;
    QueryPerformanceCounter(&Start);
    return Start;
    }


//-----------------------------------------------------------------------------
// Stop counting
//-----------------------------------------------------------------------------
__forceinline void HFCProfiler::Stop(int pi_CounterID, LARGE_INTEGER pi_Start)
    {
    // *** This must be the first task ***
    LARGE_INTEGER Finish;
    QueryPerformanceCounter(&Finish);

    HASSERT(!s_pFunctionStack->empty());
    HASSERT(s_pFunctionStack->back() == pi_CounterID);

#if 1
    // Unstack this function call
    int OverheadCount = s_pOverheads->back();
    s_pFunctionStack->pop_back();
    s_pOverheads->pop_back();

#else

    // Use this code to troubleshoot mismatched START/STOP pairs.

    int OverheadCount = 0;
    if (s_pFunctionStack->empty())
        {
        OutputDebugStringW("STOP without matching START!\n");
        if (m_BreakOnError)
            DebugBreak();
        }
    else
        {
        if (s_pFunctionStack->back() != pi_CounterID)
            {
            WChar Message[512];
            swprintf(Message, L"Missing STOP call in %s\n", m_Counters[s_pFunctionStack->back()].m_Name.c_str());
            OutputDebugStringW(Message);
            if (m_BreakOnError)
                DebugBreak();
            }

        OverheadCount = s_pOverheads->back();
        s_pFunctionStack->pop_back();
        s_pOverheads->pop_back();
        }
#endif

    Counter& rCurrentCounter = m_Counters[pi_CounterID];

    // Compute total time in function
    double FunctionTime = ((double)Finish.QuadPart) - ((double)pi_Start.QuadPart);
    FunctionTime *= m_FrequencyInverseForMillis;

    // Remove overhead inside children calls
//    FunctionTime -= OverheadCount * m_Overhead;

    rCurrentCounter.m_CounterKey.ClaimKey();
    rCurrentCounter.m_TotalMilliSeconds += FunctionTime;
    rCurrentCounter.m_Overheads += OverheadCount;
    rCurrentCounter.m_CounterKey.ReleaseKey();

    if (!s_pFunctionStack->empty())
        {
        // Remove our time from our parent
        Counter& rParentCounter = m_Counters[s_pFunctionStack->back()];

        rParentCounter.m_CounterKey.ClaimKey();
        rParentCounter.m_ChildrenMilliSeconds += FunctionTime;
        rParentCounter.m_CounterKey.ReleaseKey();

        s_pOverheads->back() += (OverheadCount + 1);
        }
    }


#define HFCPROFILER_START(Name) \
    static int HFC_PROFILER_Counter_ID = HFCProfiler::GetInstance()->RegisterCounter(Name); \
    LARGE_INTEGER HFC_PROFILER_Counter_Start = HFCProfiler::GetInstance()->Start(HFC_PROFILER_Counter_ID); \
    try {


#define HFCPROFILER_STOP \
    HFCProfiler::GetInstance()->Stop(HFC_PROFILER_Counter_ID, HFC_PROFILER_Counter_Start); \
    } catch(...) { \
        HFCProfiler::GetInstance()->Stop(HFC_PROFILER_Counter_ID, HFC_PROFILER_Counter_Start); \
        throw; \
    }


#define HFCPROFILER_STOP_RETURN(x) \
    HFCProfiler::GetInstance()->Stop(HFC_PROFILER_Counter_ID, HFC_PROFILER_Counter_Start); \
    return (x); \
    } catch(...) { \
        HFCProfiler::GetInstance()->Stop(HFC_PROFILER_Counter_ID, HFC_PROFILER_Counter_Start); \
        throw; \
    }


#define HFCPROFILER_RETURN_INTERNAL(x) \
    HFCProfiler::GetInstance()->Stop(HFC_PROFILER_Counter_ID, HFC_PROFILER_Counter_Start); \
    return (x);

END_IMAGEPP_NAMESPACE