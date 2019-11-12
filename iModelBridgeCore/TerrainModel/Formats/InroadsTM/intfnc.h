//---------------------------------------------------------------------------------------------
// Copyright (c) Bentley Systems, Incorporated. All rights reserved.
// See COPYRIGHT.md in the repository root for full copyright notice.
//---------------------------------------------------------------------------------------------
/*----------------------------------------------------------------------------*/
/* intfnc.h                                            aec    09-Dec-1992     */
/*----------------------------------------------------------------------------*/
/* Prototypes for functions used to interrupt processing.                     */
/*----------------------------------------------------------------------------*/
#pragma once

/*----------------------------------------------------------------------------*/
/* Function prototypes.                                                       */
/*----------------------------------------------------------------------------*/

int aecInterrupt_check      /* <= TRUE if interrupt requested      */
    (
    void
    );

void aecInterrupt_initialize
    (
    void
    );

void aecInterrupt_stop
    (
    void
    );
