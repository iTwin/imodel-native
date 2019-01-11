//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
#pragma once

/*----------------------------------------------------------------------------*/
/* Function prototypes.                                                       */
/*----------------------------------------------------------------------------*/

void aecStatus_initialize
    (
    int honorStatusBarToggle             /* => TRUE: honor status lock          */
    );

void aecStatus_show
    (
    double percent,                      /* => percent complete (0 to 100)      */
    unsigned long stringNum              /* => ID of string in stringlist       */
    );

void aecStatus_close
    (
    void
    );

