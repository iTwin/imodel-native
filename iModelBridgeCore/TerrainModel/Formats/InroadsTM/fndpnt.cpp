//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* fndpnt.c                                         tmi    19-Oct-1990        */
/*----------------------------------------------------------------------------*/
/* Finds a point in the surface given the point location.                     */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"



/*----------------------------------------------------------------------------*/
/* Private data structures                                                    */
/*----------------------------------------------------------------------------*/
struct CIVfndpnt
{
    int sts;                                      /* returned status          */
    long srfptr;                                  /* for dialog box           */
    int opt;                                      /* see above defines        */
    int typ;                                      /* type to look for         */
    struct CIVdtmsrf *srf;                        /* target surface           */
    DPoint3d loc;                                 /* input location           */
    struct CIVdtmpnt pnt;                         /* returned point           */
    struct CIVdtmpnt *pntptr;                     /* returned pointer         */
    double dis;                                   /* internal only            */
};




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_findPointType
 DESC: Given a surface and a point within that surface, this function
       returns the point's type and it's type mask.  If you don't want
       either one of these, pass in a null pointer.
 HIST: Original - tmi 20-Oct-1990
 MISC:
 KEYW: DTM POINT TYPE FIND
-----------------------------------------------------------------------------%*/

int aecDTM_findPointType    /* <= TRUE if error                    */
(
  long *typP,                          /* <= point's type                     */
  long *typmskP,                       /* <= point's type as masked value     */
  struct CIVdtmblk **inpblkP,          /* <= block where point is             */
  struct CIVdtmsrf *srfP,              /* => surface to use                   */
  struct CIVdtmpnt *pP                 /* => point to use                     */
)
{
  struct CIVdtmblk *blkP;
  struct CIVdtmpnt *startP, *stopP;
  int sts = DTM_M_NOPNTF, i;

  if ( typP ) *typP = 0;
  if ( typmskP ) *typmskP = 0;
  if ( inpblkP ) *inpblkP = (struct CIVdtmblk *)0;

  for ( i = 0; i < DTM_C_NMPNTF  &&  sts != SUCCESS; i++ )
    for ( blkP = srfP->pntf[i]->blk; blkP  &&  sts != SUCCESS; blkP = blkP->nxt )
    {
      startP = blkP->rec.pnt;
      stopP  = startP + blkP->use - 1;
      if ( pP >= startP  &&  pP <= stopP )
      {
	if ( typP ) *typP = i;
	if ( typmskP ) *typmskP = 1 << i;
	if ( inpblkP ) *inpblkP = blkP;
	sts = SUCCESS;
      }
    }

  if ( sts != SUCCESS )
    if ( aecDTM_isPointRangePoint ( srfP, pP ) )
    {
      if ( typP ) *typP = DTM_C_DTMRNG;
      if ( typmskP ) *typmskP = 0;                /* range points don't have a valid mask type */
      if ( inpblkP ) *inpblkP = srfP->rngf->blk;
      sts = SUCCESS;
    }


  return ( sts );
}
