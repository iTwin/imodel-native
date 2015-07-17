//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* intsct.h                                           aec    01-Jan-1994      */
/*----------------------------------------------------------------------------*/
/* General geometric intersection utility.                                    */
/*----------------------------------------------------------------------------*/
#pragma once

/*----------------------------------------------------------------------------*/
/* Function prototypes                                                        */
/*----------------------------------------------------------------------------*/

int aecIntersect_begin      /* <= TRUE if error                    */
    (
    void **thisPP,                       /* <= Intersect object                 */
    int (*intFuncP)(void*,void*,void*),  /* => Intersect function               */
    void *mdlDescP,                      /* => MDL Descriptor                   */
    void *userDataP,                     /* => user data pointer                */
    int preAlcSize                       /* => pre-alloc size ( or 0 )          */
    );

int aecIntersect_insert     /* <= TRUE if error                    */
    (
    void *thisP,                         /* => Intersect object                 */
    void *eleP,                          /* => Intersect element                */
    double xlo,                          /* => xlo of eleP                      */
    double ylo,                          /* => ylo of eleP                      */
    double xhi,                          /* => xhi of eleP                      */
    double yhi                           /* => yhi of eleP                      */
    );

int aecIntersect_end        /* <= TRUE if error                    */
    (
    void *thisP                          /* => Intersect object                 */
    );
