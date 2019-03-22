//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* prcpnt.c                                                tmi    28-Apr-1990 */
/*----------------------------------------------------------------------------*/
/* Loops through all points or, if a fence is active, only those points       */
/* that obey the 'opt' parameter setting.				      */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"


/*%-----------------------------------------------------------------------------
FUNC: aecDTM_sendAllPointsHonoringFence
DESC: Loops through all points or, if a fence is active, only those points
that obey the 'opt' parameter setting.
HIST: Original - tmi 28-Apr-1990
MISC:
KEYW: DTM POINT SEND ALL FENCE
-----------------------------------------------------------------------------%*/

int aecDTM_sendAllPointsHonoringFence /* <= TRUE if error          */
    (
    void *mdlDescP,                    /* => mdl app. desc (or NULL)            */
    struct CIVdtmsrf *srfP,            /* => surface to use                     */
    int opt,                           /* => options                            */
    int typmsk,                        /* => point type (zero for all)          */
    int (*usrfncP)(void *,int,long,    /* => your function                      */
    DPoint3d *,struct CIVdtmpnt *),
    void *usrdatP                      /* => your data                          */
    )
    {
    DPoint3d *vrt = NULL;
    int sts = SUCCESS;

    vrt = hmgrVertices_allocate (NULL, MAX_VERTICES);

    if ( !(opt & DTM_C_INSIDE)  &&  !(opt & DTM_C_OUTSIDE) )
        sts = aecDTM_sendAllPoints ( (void *)0, srfP, opt, typmsk, usrfncP, usrdatP );

    if (vrt)
        {
        hmgrVertices_free (vrt);
        vrt = NULL;
        }

    return ( sts );
    }
