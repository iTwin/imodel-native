/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/Client/BreakHelper.h>
BEGIN_BENTLEY_IMODELHUB_NAMESPACE

Breakpoints BreakHelper::s_breakpoint = Breakpoints::None;

//---------------------------------------------------------------------------------------
//@bsimethod                                    Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
void BreakHelper::SetBreakpoint(Breakpoints breakpoint)
    {
#if defined (ENABLE_BIM_CRASH_TESTS)
    s_breakpoint = breakpoint;
#endif
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
void BreakHelper::HitBreakpoint(Breakpoints breakpoint)
    {
#if defined (ENABLE_BIM_CRASH_TESTS)
    if (s_breakpoint == breakpoint)
        throw 1;
#endif
    }

END_BENTLEY_IMODELHUB_NAMESPACE
