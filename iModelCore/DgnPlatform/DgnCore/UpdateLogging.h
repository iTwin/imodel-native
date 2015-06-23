/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/UpdateLogging.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//  #define WANT_QUERYVIEW_UPDATE_LOGGING 1
//  #define VTUNE_LOGGING_ENABLED 1
//  #define UPDATELOGGING_USE_PRINTF

#if defined (WANT_QUERYVIEW_UPDATE_LOGGING)
    #if UPDATELOGGING_USE_PRINTF
        #define UPDATELOGGING_PRINT(message) printf("%s\n", message)
    #elif _WIN32
        extern "C" void OutputDebugStringA(CharCP);
        #define UPDATELOGGING_PRINT(message) OutputDebugStringA(message)
    #else
        #include <Bentley/BeDebugLog.h>
        #define UPDATELOGGING_PRINT(message) (BeDebugLogFunctions::PerformBeDebugLog (message, __FILE__, __LINE__))
    #endif
#endif

#if defined(VTUNE_LOGGING_ENABLED)
    //  NOTE -- you must also remove the comment mark from the line in DgnPlatform.mke that adds in libittnotify.
    DGNPLATFORM_EXPORT void PauseVTune();
    DGNPLATFORM_EXPORT void ResumeVTune();
#endif

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

struct UpdateLogging
    {
#if defined (WANT_QUERYVIEW_UPDATE_LOGGING)
    static void RecordStartCycle();
    static void RecordStartQuery();
    static void RecordDoneQuery(uint32_t nAcceptCalls, uint32_t nScores, uint32_t numElements);
    static void RecordDoneLoad();
    static void RecordDetach();
    static void RecordAllowShowProgress();
    static void RecordDoneUpdate(uint32_t numDrawn, Dgn::DrawPurpose drawPurpose);
#else
    static void RecordStartCycle() {}
    static void RecordStartQuery() {}
    static void RecordDoneQuery(uint32_t nAcceptCalls, uint32_t nScores, uint32_t numElements) {}
    static void RecordDoneLoad() {}
    static void RecordDetach() {}
    static void RecordAllowShowProgress() {}
    static void RecordDoneUpdate(uint32_t numDrawn, Dgn::DrawPurpose drawPurpose) {}
#endif
    };

END_BENTLEY_DGNPLATFORM_NAMESPACE
