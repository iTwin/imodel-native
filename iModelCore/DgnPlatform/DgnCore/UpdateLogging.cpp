/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/UpdateLogging.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

#include "UpdateLogging.h"

#if defined (VTUNE_LOGGING_ENABLED)
    #include "C:\Program Files (x86)\Intel\VTune Amplifier XE\include\ittnotify.h"
#endif

#if defined (VTUNE_LOGGING_ENABLED)
    static bool s_vtuneInited;
    static __itt_domain* queryViewDomain;
    static __itt_event startQueryEvent;
    static __itt_event startLoadingEvent;
    static __itt_event startDrawEvent;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   John.Gooding    04/2014
    //---------------------------------------------------------------------------------------
    static void InitVTune()
        {
        if (s_vtuneInited)
            return;

        s_vtuneInited = true;
        queryViewDomain = __itt_domain_create("bentley.queryView");
        startQueryEvent = __itt_event_create("Query", 5);
        startLoadingEvent = __itt_event_create("ElementLoad", (int)strlen("ElementLoad"));
        startDrawEvent = __itt_event_create("DrawView", 8);
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   John.Gooding    04/2014
    //---------------------------------------------------------------------------------------
    void PauseVTune()
        {
        __itt_pause();
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   John.Gooding    04/2014
    //---------------------------------------------------------------------------------------
    void ResumeVTune()
        {
        InitVTune();
        __itt_resume();
        }
#endif

//  This file does nothing unless WANT_QUERYVIEW_UPDATE_LOGGING is defined
#if defined (WANT_QUERYVIEW_UPDATE_LOGGING)
#include <DgnPlatform/DgnCore/QueryView.h>


USING_NAMESPACE_BENTLEY_DGN

static bool s_inLoadUpdateCyclePending;
static bool s_inLoadUpdateCycle;
static double s_startCycleTime;
static double s_finishQueryTime;
static double s_allowShowProgress;
static double s_finishLoadTime;
static double s_finishDrawTime;
static uint32_t s_numberElementsDrawn;
static uint32_t s_nAcceptCalls;
static uint32_t s_nScores;
static uint32_t s_sizeofQueryResult;

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    04/2014
//---------------------------------------------------------------------------------------
void UpdateLogging::RecordStartCycle()
    {
#if defined (VTUNE_LOGGING_ENABLED)
    InitVTune();
#endif
    //  CYCLE LOGGING DISABLED
    s_inLoadUpdateCyclePending = true;
    s_inLoadUpdateCycle = false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    04/2014
//---------------------------------------------------------------------------------------
void UpdateLogging::RecordStartQuery()
    {
    if (!s_inLoadUpdateCyclePending)
        return;

    s_inLoadUpdateCyclePending = false;
    s_inLoadUpdateCycle = true;
    s_startCycleTime = BeTimeUtilities::QuerySecondsCounter();
#if defined (VTUNE_LOGGING_ENABLED)
    __itt_event_start(startQueryEvent);
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    04/2014
//---------------------------------------------------------------------------------------
void UpdateLogging::RecordDoneQuery(uint32_t nAcceptCalls, uint32_t nScores, uint32_t numElements)
    {
    if (!s_inLoadUpdateCycle)
        return;

    s_finishQueryTime = BeTimeUtilities::QuerySecondsCounter();
    s_nAcceptCalls = nAcceptCalls;
    s_nScores = nScores;
    s_sizeofQueryResult = numElements;

#if defined (VTUNE_LOGGING_ENABLED)
    __itt_event_end(startQueryEvent);
    __itt_event_start(startLoadingEvent);
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    04/2014
//---------------------------------------------------------------------------------------
void UpdateLogging::RecordDoneLoad()
    {
    if (!s_inLoadUpdateCycle)
        return;

    s_finishLoadTime = BeTimeUtilities::QuerySecondsCounter();
#if defined (VTUNE_LOGGING_ENABLED)
    __itt_event_end(startLoadingEvent);
    __itt_resume();
    __itt_event_start(startDrawEvent);
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    04/2014
//---------------------------------------------------------------------------------------
void UpdateLogging::RecordAllowShowProgress()
    {
    if (!s_inLoadUpdateCycle)
        return;

    s_allowShowProgress = BeTimeUtilities::QuerySecondsCounter();
    }

extern "C" void OutputDebugStringA(CharCP);

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    04/2014
//---------------------------------------------------------------------------------------
void UpdateLogging::RecordDoneUpdate(uint32_t numDrawn, DrawPurpose drawPurpose)
    {
    if (!s_inLoadUpdateCycle)
        return;

#if defined (VTUNE_LOGGING_ENABLED)
    __itt_event_end(startDrawEvent);
    __itt_pause();
#endif
    switch (drawPurpose)
        {
        case DrawPurpose::Update:
        case DrawPurpose::UpdateHealing:
        case DrawPurpose::UpdateDynamic:
            break;
        default:
            s_inLoadUpdateCycle = false;
            return;
        }

    s_finishDrawTime = BeTimeUtilities::QuerySecondsCounter();
    s_numberElementsDrawn = numDrawn;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    04/2014
//---------------------------------------------------------------------------------------
void UpdateLogging::RecordDetach()
    {
    if (!s_inLoadUpdateCycle)
        return;

    s_inLoadUpdateCycle = false;
#if defined (VTUNE_LOGGING_ENABLED)
    __itt_pause();
#endif
    static int s_outputCounter;
    UPDATELOGGING_PRINT(BeSQLite::SqlPrintfString("(%d) Elements drawn = %d, query time = %lf, load time = %lf, element IDs return = %d, draw time = %lf, accept calls = %d, scores = %d\n", 
                    ++s_outputCounter, s_numberElementsDrawn, s_finishQueryTime - s_startCycleTime, s_finishLoadTime - s_finishQueryTime, 
                    s_sizeofQueryResult, 
                    s_finishDrawTime - s_finishLoadTime, s_nAcceptCalls, s_nScores).GetUtf8CP());
    }

#endif
