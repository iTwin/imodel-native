//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* stsuti.c                                              tmi    16-Nov-1990   */
/*----------------------------------------------------------------------------*/
/* Displays the status dialog box.                                            */
/*----------------------------------------------------------------------------*/

#include "stdafx.h"
#include <stsfnc.h>

static BOOL s_bSuppress = FALSE;

/*%-----------------------------------------------------------------------------
FUNC: aecStatus_initialize
DESC: It initializes some things for displaying the status bar.
HIST: Original - tmi 03-Jan-1993
MISC:
KEYW: STATUS PERCENT COMPLETE INITIALIZE
-----------------------------------------------------------------------------%*/

void aecStatus_initialize
    (
    int bHonorStatusBarToggle             /* => TRUE: honor status lock          */
    )
    {
    if( !s_bSuppress )
        {
        aecOutput_blankAllFields( );
        }
    }

/*%-----------------------------------------------------------------------------
FUNC: aecStatus_show
DESC: Call this one with the percentage data.  aecStatus_initialize must
be called first.  The status is only redisplayed every 2 percent.
HIST: Original - tmi 03-Jan-1993
MISC:
KEYW: STATUS PERCENT COMPLETE SHOW
-----------------------------------------------------------------------------%*/

void aecStatus_show
    (
    double dPercent,                      /* => percent complete (0 to 100)      */
    ULONG nString              /* => ID of string (or 0 for default)  */
    )
    {
    }

/*%-----------------------------------------------------------------------------
FUNC: aecStatus_close
DESC: It closes the status dialog box.
HIST: Original - tmi 03-Jan-1993
MISC:
KEYW: STATUS PERCENT COMPLETE CLOSE CLEANUP
-----------------------------------------------------------------------------%*/

void aecStatus_close
    (
    void
    )
    {
    }

