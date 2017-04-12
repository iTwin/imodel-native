/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelHubClient/BreakHelper.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/DgnDbServerBreakHelper.h>
BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE

DgnDbServerBreakpoints DgnDbServerBreakHelper::s_breakpoint = DgnDbServerBreakpoints::None;

//---------------------------------------------------------------------------------------
//@bsimethod                                    Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
void DgnDbServerBreakHelper::SetBreakpoint(DgnDbServerBreakpoints breakpoint)
    {
#if defined (ENABLE_BIM_CRASH_TESTS)
    s_breakpoint = breakpoint;
#endif
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
void DgnDbServerBreakHelper::HitBreakpoint(DgnDbServerBreakpoints breakpoint)
    {
#if defined (ENABLE_BIM_CRASH_TESTS)
    if (s_breakpoint == breakpoint)
        throw 1;
#endif
    }

END_BENTLEY_DGNDBSERVER_NAMESPACE
