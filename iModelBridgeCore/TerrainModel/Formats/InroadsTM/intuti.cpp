//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* intuti.c                                        tmi    25-Apr-1990         */
/*----------------------------------------------------------------------------*/
/* These functions are used to determine if the user has requested that       */
/* the command which is currently processing be interrupted.                  */
/*----------------------------------------------------------------------------*/

#include "stdafx.h"


/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include <intfnc.h>

#define CHECKINTERVAL  255  // must be less than 255

//---------------------------------------------------------------------------
// Statics
//---------------------------------------------------------------------------

static int  s_nInterruptCount = 0;
static int  s_nCheckInterval = CHECKINTERVAL;
static BOOL s_bInterruptRequested = FALSE;


/*%-----------------------------------------------------------------------------
FUNC: aecInterrupt_check
DESC: Stick this one in your loop to determine if user wants to interrupt.
If the user has interrupted processing this function will return a
status of AEC_E_PRCINT.  Otherwise, a status of 0 is returned.
HIST: Original - tmi 09-Dec-1992
MISC:
KEYW: INTERRUPT CHECK
-----------------------------------------------------------------------------%*/

int aecInterrupt_check
    (
    void
    )
    {
#ifdef DHTODO
    int nStatus = SUCCESS;

    if ( s_bInterruptRequested ) 
        {
        nStatus = AEC_E_PRCINT;
        s_bInterruptRequested = FALSE;
        }
    else
        {
        // Added PostMessage calls to prevent hangup in autocad when processing, and user ALT-TABS out
        // of autocad. djs 3/24/00
        if ( nStatus == SUCCESS && ( ++s_nInterruptCount % s_nCheckInterval ) == 0 )
            {
            MSG msg;

            if ( PeekMessage ( &msg, NULL,                      WM_KEYFIRST, WM_KEYLAST, PM_REMOVE )  ||
                PeekMessage ( &msg, NULL /*GetCadWindowHandle()*/, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE ) )
                {
                if ( msg.message == WM_KEYUP    || msg.message == WM_KEYDOWN ||
                    msg.message == WM_SYSKEYUP || msg.message == WM_SYSKEYDOWN )
                    {
                    if ( msg.wParam == VK_ESCAPE )
                        nStatus = AEC_E_PRCINT;
                    else if ( msg.wParam == 'C' && GetKeyState ( VK_CONTROL ) )
                        nStatus = AEC_E_PRCINT;
                    else // Need to post message back if not being processed (Someone else may want it!!)
                        PostMessage( msg.hwnd, msg.message, msg.wParam, msg.lParam );
                    }
                else // Need to post message back if not being processed (Someone else may want it!!)
                    PostMessage( msg.hwnd, msg.message, msg.wParam, msg.lParam );
                }
            }
        }

    if( nStatus == AEC_E_PRCINT )
        s_nInterruptCount = 0;

    return nStatus;
#endif
    return 0;
    }



/*%-----------------------------------------------------------------------------
FUNC: aecInterrupt_initialize
DESC: MDL loops need this function.
HIST: Original - tmi 19-Feb-1992
MISC:
KEYW: INTERRUPT INITIALIZE
-----------------------------------------------------------------------------%*/

void aecInterrupt_initialize
    (
    void
    )
    {
    s_nInterruptCount = 0;
    s_bInterruptRequested = FALSE;
    }



/*%-----------------------------------------------------------------------------
FUNC: aecInterrupt_stop
DESC: MDL loops need this function.
HIST: Original - tmi 19-Feb-1992
MISC:
KEYW: INTERRUPT STOP
-----------------------------------------------------------------------------%*/

void aecInterrupt_stop
    (
    void
    )
    {
    s_nCheckInterval = CHECKINTERVAL;
    }


