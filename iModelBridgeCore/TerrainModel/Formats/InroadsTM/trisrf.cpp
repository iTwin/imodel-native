//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* trisrf.c                                            tmi        10-Apr-1990 */
/*----------------------------------------------------------------------------*/
/* Contains all functions necessary to triangulate a surface.                 */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"


/*----------------------------------------------------------------------------*/
/* Externals                                                                  */
/*----------------------------------------------------------------------------*/
static struct CIVdtmpnt tinErrPnt;
static boolean tinErrPntSet = FALSE;

boolean tinAltMoveMethod = FALSE;

void aecDTM_triangulateSetErrorPoint
(
    struct CIVdtmpnt *dtmPntP
)
{
    memcpy ( &tinErrPnt, dtmPntP, sizeof ( struct CIVdtmpnt ) );
    tinErrPntSet = TRUE;
}

int aecDTM_validateTinPtr   /* <= TRUE if error                    */
(
    struct CIVdtmblk **inpblkP,        /* <= block where triangle is          */
    struct CIVdtmsrf *srfP,            /* => surface to use                   */
    struct CIVdtmtin *tinP             /* => triangle to use                  */
)
{
    struct CIVdtmblk *blkP;
    struct CIVdtmtin *startP, *stopP;
    int sts = DTM_M_NOTINF;

    if ( inpblkP ) *inpblkP = (struct CIVdtmblk *)0;

    for ( blkP = srfP->tinf->blk; blkP  &&  sts != SUCCESS; blkP = blkP->nxt )
    {
        startP = blkP->rec.tin;
        stopP  = startP + blkP->use - 1;
        if ( tinP >= startP  &&  tinP <= stopP )
        {
            if ( inpblkP ) *inpblkP = blkP;
                sts = SUCCESS;
        }
    }

    return ( sts );
}
