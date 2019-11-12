//---------------------------------------------------------------------------------------------
// Copyright (c) Bentley Systems, Incorporated. All rights reserved.
// See COPYRIGHT.md in the repository root for full copyright notice.
//---------------------------------------------------------------------------------------------
#pragma once

/*----------------------------------------------------------------------------*/
/* Function prototypes                                                        */
/*----------------------------------------------------------------------------*/

int aecTable_insert         /* <= TRUE if error                    */
    (
    void **tblPP,                        /* <= caller must free          */
    int *nTblP,                          /* <=                                  */
    void *eleP,                          /* =>                                  */
    int eSize                            /* =>                                  */
    );

