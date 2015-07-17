//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* dalloc.c                                            tmi	10-Apr-1990   */
/*----------------------------------------------------------------------------*/
/* Contains all the functions which deallocate memory for surfaces.           */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_deallocateFile
 DESC: Deallocates memory used by a file within a dtm surface.
 HIST: Original - tmi 10-Apr-1990
 MISC:
 KEYW: DTM FILE MEMORY FREE DEALLOCATE
-----------------------------------------------------------------------------%*/

int aecDTM_deallocateFile /* <= TRUE if error                      */
(
  struct CIVdtmfil *filP             /* => DTM file to deallocate             */
)
{
  struct CIVdtmblk *blkP;
  int sts = SUCCESS;

  if ( filP != (struct CIVdtmfil *)0 )
  {
    while ( filP->blk  &&  sts == SUCCESS )
    {
      blkP = filP->blk;
      filP->blk = blkP->nxt;
      sts = aecDTM_deallocateBlock ( blkP );
    }

    if ( sts == SUCCESS )
    {
      filP->blk   = 0;
      filP->nrec  = filP->ndel = 0;
      if ( filP->flg & DTM_C_NORANG )
        filP->flg = DTM_C_NORANG;
      else
      {
        filP->flg = 0;
        filP->min.x = filP->min.y = filP->min.z = AEC_C_MAXDBL;
        filP->max.x = filP->max.y = filP->max.z = AEC_C_MINDBL;
      }
    }
  }

  return ( sts );
}





/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_deallocateBlock
 DESC: Deallocates memory used by a block of data within a surface file.
 HIST: Original - tmi 10-Apr-1990
 MISC:
 KEYW: DTM BLOCK MEMORY FREE DEALLOCATE
-----------------------------------------------------------------------------%*/

int aecDTM_deallocateBlock /* <= TRUE if error                     */
(
  struct CIVdtmblk *blkP              /* => block of memory to dealloc.       */
)
{
  free ( (void *)blkP->rec.pnt );
  free ( (void *)blkP );
  return ( SUCCESS );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_deallocateTriangleStack
 DESC: It removes the surface triangle stack.
 HIST: Original - tmi 03-Nov-1990
 MISC:
 KEYW: DTM TRIANGLE STACK MEMORY FREE DEALLOCATE
-----------------------------------------------------------------------------%*/

int aecDTM_deallocateTriangleStack /* <= TRUE of error             */
(
  struct CIVdtmsrf *srfP                /* => srf with stack to free          */
)
{
  int sts = SUCCESS;

  if ( srfP->ntinstk > 0L )
      free ( srfP->tinstk );
  srfP->ntinstk = 0L;
  srfP->tinstk = (long *)0;

  return ( sts );
}
