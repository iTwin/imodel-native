//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* dtmfnd.h                                            aec    08-Feb-1994     */
/*----------------------------------------------------------------------------*/
/* Data structures and constants used with commands which find DTM entities.  */
/*----------------------------------------------------------------------------*/

#pragma once

/*----------------------------------------------------------------------------*/
/* Dependent include files                                                    */
/*----------------------------------------------------------------------------*/

#include <dtmstr.h>

/*----------------------------------------------------------------------------*/
/* Find elevation along linestring things                                     */
/*----------------------------------------------------------------------------*/

#define DTM_C_FIRST    0x1
#define DTM_C_LAST     0x2
#define DTM_C_VOID     0x4
#define DTM_C_DONTSKIP 0x8

typedef struct CIVlinsrf
{
    struct AECjoin *seg;
    DPoint3d *p;
    byte *flg;
    long ndat;
    unsigned long nalc;
} CIVlinsrf;

