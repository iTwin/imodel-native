//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* jonfnc.h                                            aec    20-Dec-1992     */
/*----------------------------------------------------------------------------*/
/* Line segment joining function prototypes.                                  */
/*----------------------------------------------------------------------------*/
#pragma once

#include "aecuti.h"

/*----------------------------------------------------------------------------*/
/* Data structures                                                            */
/*----------------------------------------------------------------------------*/

struct AECjoinPoint
{
    struct AECjoinPoint *hshlnk;
    DPoint3d p;
    unsigned long lnk;
};

struct AECjoin
{
    struct AECjoinPoint **hsh;
    unsigned long nBuckets;
    int (*usrfnc)(void *,int,long,DPoint3d *);
    int opt;
    void *dat;
    void *mdlDescP;
    struct AECjoinPoint *frepntP;
    DPoint3d *vrtsP;
    unsigned long nvrt;
};

