//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* mrkuti.c                                            tmi    22-Oct-1990     */
/*----------------------------------------------------------------------------*/
/* Contains various utilities to set and unset the delete bit in a set        */
/* of triangles.                                                              */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"


/*----------------------------------------------------------------------------*/
/* Constants and macros                                                       */
/*----------------------------------------------------------------------------*/
#define     CIVSTATUS(a,b,c,d) { double zzz = (double)(a+=b)/(double)c * 100.; aecStatus_show ( zzz, d ); }


/*----------------------------------------------------------------------------*/
/* Private data structures                                                    */
/*----------------------------------------------------------------------------*/
struct CIVmrkbnd
{
    struct CIVdtmsrf *srf;
    long nvrt;
    DPoint3d *vrtP;
    DPoint3d box[5];
    byte vld;
    byte del;
    byte inside;
    byte dntExtFtr;
};



/*----------------------------------------------------------------------------*/
/* Private static function prototypes                                         */
/*----------------------------------------------------------------------------*/
static int aecDTM_markBoundaryTrianglesProcess(void *,int,long,DPoint3d *,struct CIVdtmpnt *);
static int aecDTM_markBoundaryTrianglesProcessMore(void *,long,DPoint3d *,struct CIVdtmtin *,unsigned long);


/*----------------------------------------------------------------------------*/
/* Externals                                                                  */
/*----------------------------------------------------------------------------*/
static long ntin, ctin;
int tickerCount;




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_markRangeTriangles
 DESC: It marks as deleted all the triangles that have at least one vertex
       that is a range point as deleted.
 HIST: Original - tmi 11-Apr-1990
 MISC:
 KEYW: DTM TRIANGLES RANGE DELETE
-----------------------------------------------------------------------------%*/

void aecDTM_markRangeTriangles
(
  struct CIVdtmsrf *srfP                    /* => surface to use              */
)
{
  if ( srfP != (struct CIVdtmsrf *)0 )
  {
    struct CIVdtmblk *blkP;
    struct CIVdtmtin *tP;

    for ( blkP = srfP->tinf->blk; blkP; blkP = blkP->nxt )
      for ( tP = blkP->rec.tin; tP < blkP->rec.tin + blkP->use; tP++ )
        if ( !aecDTM_isTriangleDeletedFlagSet ( tP ) )
          if ( aecDTM_isTriangleRangeTriangle ( srfP, tP ) )
            aecDTM_deleteTriangle ( srfP, tP, 2 );
  }
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_unmarkRangeTriangles
 DESC: It marks as undeleted all the triangles that have at least one
       vertex that is a range point as deleted.
 HIST: Original - tmi 11-Apr-1990
 MISC:
 KEYW: DTM TRIANGLE RANGE UNDELETE
-----------------------------------------------------------------------------%*/

void aecDTM_unmarkRangeTriangles
(
  struct CIVdtmsrf *srfP                   /* => surface to use               */
)
{
  if ( srfP != (struct CIVdtmsrf *)0 )
  {
    struct CIVdtmblk *blkP;
    struct CIVdtmtin *tP;

    for ( blkP = srfP->tinf->blk; blkP; blkP = blkP->nxt )
      for ( tP = blkP->rec.tin; tP < blkP->rec.tin + blkP->use; tP++ )
        if ( !aecDTM_isTriangleRemovedFlagSet ( tP ) )
          if ( aecDTM_isTriangleRangeTriangle ( srfP, tP ) )
            aecDTM_undeleteTriangle ( srfP, tP, 1 );
  }
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_markBoundaryTriangles
 DESC: It marks the triangles inside the interior areas and outside the
       exterior area as deleted.
 HIST: Original - tmi 22-Oct-1990
 MISC:
 KEYW:  DTM TRIANGLE BOUNDARY INTERIOR EXTERIOR DELETE
-----------------------------------------------------------------------------%*/

int aecDTM_markBoundaryTriangles /* <= TRUE if error               */
(
  struct CIVdtmsrf *srfP                    /* => surface to use              */
)
{
  struct CIVmrkbnd mrkbnd;
  int sts = SUCCESS;
  long nint, next;

  memset ( &mrkbnd, 0, sizeof(struct CIVmrkbnd) );
  mrkbnd.srf = srfP;
  mrkbnd.del = TRUE;
  mrkbnd.inside = TRUE;

  ctin = 0L;
  aecDTM_countSurfaceData ( (long *)0, &ntin, srfP );
  aecDTM_countBoundaries ( &nint, &next, srfP );
  ntin *= ( nint + next );
  aecStatus_initialize (TRUE);

  if ( ( sts = aecDTM_sendAllPoints ( (void *)0, srfP, (int)DTM_C_NOBREK|DTM_C_NOCONS|DTM_C_NOTINS, (int)DTM_C_INTMSK, aecDTM_markBoundaryTrianglesProcess, &mrkbnd ) ) == SUCCESS )
  {
    mrkbnd.inside = FALSE;

    CFeature feature;
    if ( feature.LoadFromDTMExteriorBoundary ( srfP ) == SUCCESS && feature.IsTriangulationEnabled() == FALSE )
        mrkbnd.dntExtFtr = TRUE;

    sts = aecDTM_sendAllPoints ( (void *)0, srfP, (int)DTM_C_NOBREK|DTM_C_NOCONS, (int)DTM_C_EXTMSK, aecDTM_markBoundaryTrianglesProcess, &mrkbnd );
  }

  aecStatus_close ();

  return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_markBoundaryTrianglesProcess
 DESC: Helps out the previous function.
 HIST: Original - tmi 22-Oct-1990
 MISC: static
 KEYW: DTM TRIANGLE BOUNDARY INTERIOR EXTERIOR DELETE PROCESS
-----------------------------------------------------------------------------%*/

static int aecDTM_markBoundaryTrianglesProcess
(
  void *datP,
  int,
  long npnt,
  DPoint3d *pntsP,
  struct CIVdtmpnt *
)
{
  aecPolygon_removeColinearPoints ( &npnt, pntsP );

  if ( npnt > 3L )
  {
    struct CIVmrkbnd *mrkP = (struct CIVmrkbnd *)datP;

    mrkP->nvrt = npnt;
    mrkP->vrtP = pntsP;

    if ( mrkP->inside == (byte)TRUE )
    {
      aecPolygon_computeRange ( &mrkP->box[0], &mrkP->box[2], npnt, pntsP );
      mrkP->box[1].x = mrkP->box[2].x;  mrkP->box[1].y = mrkP->box[0].y;
      mrkP->box[3].x = mrkP->box[0].x;  mrkP->box[3].y = mrkP->box[2].y;
      mrkP->box[4]   = mrkP->box[0];
      mrkP->box[1].z = mrkP->box[3].z = mrkP->box[0].z;
    }

    tickerCount = 0;

    aecDTM_sendAllTriangles ( (void *)0, mrkP->srf, 0, aecDTM_markBoundaryTrianglesProcessMore, mrkP );
  }

  return ( SUCCESS );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_markBoundaryTrianglesProcessMore
 DESC: Helps out the previous function.
 HIST: Original - tmi 22-Oct-1990
 MISC: static
 KEYW: DTM TRIANGLE BOUNDARY INTERIOR EXTERIOR DELETE PROCESS
-----------------------------------------------------------------------------%*/

static int aecDTM_markBoundaryTrianglesProcessMore
(
  void *datP,
  long,
  DPoint3d *tP,
  struct CIVdtmtin *tinP,
  unsigned long
)
{
  struct CIVmrkbnd *mrkP = (struct CIVmrkbnd *)datP;
  DPoint3d pnt;
  int insideBox;

  if ( !(++tickerCount % 1000) ) aecTicker_show(), tickerCount = 0;

  if ( ntin > 0 )
    CIVSTATUS ( ctin, 1, ntin, DTM_M_MRKBND );

  VCENTROIDXY ( tP[0], tP[1], tP[2], pnt );

  if ( mrkP->inside == (byte)TRUE )
    insideBox = aecPolygon_isPointInside ( 5L, mrkP->box, &pnt );
  else
    insideBox = TRUE;

  if ( insideBox )
  {
    mrkP->vld = (byte)aecPolygon_isPointInside ( mrkP->nvrt, mrkP->vrtP, &pnt );
    if ( mrkP->inside == FALSE ) mrkP->vld ^= TRUE;

    if ( mrkP->vld == (byte)TRUE )
    {
      if ( mrkP->del == (byte)TRUE )
      {
        aecDTM_deleteTriangle ( mrkP->srf, tinP, 2 );

        if ( mrkP->dntExtFtr == FALSE )
        {
            if ( !(tinP->flg & DTM_C_TINB12) ) aecDTM_deleteTriangle ( mrkP->srf, tinP->n12, 2 ), ctin++;
            if ( !(tinP->flg & DTM_C_TINB23) ) aecDTM_deleteTriangle ( mrkP->srf, tinP->n23, 2 ), ctin++;
            if ( !(tinP->flg & DTM_C_TINB31) ) aecDTM_deleteTriangle ( mrkP->srf, tinP->n31, 2 ), ctin++;
        }
      }
      else
      {
        aecDTM_undeleteTriangle ( mrkP->srf, tinP, 1 );

        if ( !(tinP->flg & DTM_C_TINB12) )
        {
          aecDTM_undeleteTriangle ( mrkP->srf, tinP->n12, 1 );
          ctin++;
        }
        if ( !(tinP->flg & DTM_C_TINB23) )
        {
          aecDTM_undeleteTriangle ( mrkP->srf, tinP->n23, 1 );
          ctin++;
        }
        if ( !(tinP->flg & DTM_C_TINB31) )
        {
          aecDTM_undeleteTriangle ( mrkP->srf, tinP->n31, 1 );
          ctin++;
        }
      }
    }
  }

  return ( SUCCESS );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_setTriangleSideBreaklineFlag
 DESC: It sets the breakline bit for two neighboring triangles given the
       two endpoints of the breakline (or interior or exterior boundary.)
 HIST: Original - tmi 22-Oct-1990
 MISC:
 KEYW: DTM TRIANGLE SIDE BREAKLINE SET
-----------------------------------------------------------------------------%*/

void aecDTM_setTriangleSideBreaklineFlag
(
  struct CIVdtmpnt *p1P,                /* => first breakline point           */
  struct CIVdtmpnt *p2P,                /* => second breakline point          */
  struct CIVdtmtin *tinP                /* => triangle                        */
)
{
  struct CIVdtmtin *neiP = (struct CIVdtmtin *)0;

  if ( tinP != (struct CIVdtmtin *)0 )
    if      ( (p1P == tinP->p1 && p2P == tinP->p2) || (p1P == tinP->p2 && p2P == tinP->p1) )
      tinP->flg |= DTM_C_TINB12, neiP = tinP->n12;
    else if ( (p1P == tinP->p2 && p2P == tinP->p3) || (p1P == tinP->p3 && p2P == tinP->p2) )
      tinP->flg |= DTM_C_TINB23, neiP = tinP->n23;
    else if ( (p1P == tinP->p3 && p2P == tinP->p1) || (p1P == tinP->p1 && p2P == tinP->p3) )
      tinP->flg |= DTM_C_TINB31, neiP = tinP->n31;

  if ( neiP != (struct CIVdtmtin *)0 )
    if      ( (p1P == neiP->p1 && p2P == neiP->p2) || (p1P == neiP->p2 && p2P == neiP->p1) )
      neiP->flg |= DTM_C_TINB12;
    else if ( (p1P == neiP->p2 && p2P == neiP->p3) || (p1P == neiP->p3 && p2P == neiP->p2) )
      neiP->flg |= DTM_C_TINB23;
    else if ( (p1P == neiP->p3 && p2P == neiP->p1) || (p1P == neiP->p1 && p2P == neiP->p3) )
      neiP->flg |= DTM_C_TINB31;
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_applyMaximumTriangleLength
 DESC: Deletes the input triangle if one or more sides has a length
       greator than or equal to the square root of the input length.
 HIST: Original - tmi 13-Dec-1993
 MISC:
 KEYW: DTM TRIANGLE SIZE MAXIMUM
-----------------------------------------------------------------------------%*/

void aecDTM_applyMaximumTriangleLength
(
  struct CIVdtmsrf *srfP,             /* => surface we're using              */
  double sizeSquared,                 /* => length to check, squared.        */
  struct CIVdtmtin *tP                /* => triangle to check                */
)
{
  if ( sizeSquared > 0. )
    if ( !aecDTM_isTriangleDeletedFlagSet ( tP ) )
    {
      DPoint3d tmp;

      VSUBXY ( tP->p2->cor, tP->p1->cor, tmp );
      if ( VDOTXY ( tmp, tmp ) >= sizeSquared )
      {
        aecDTM_deleteTriangle ( srfP, tP, 2 );
        if ( tP->n12 ) aecDTM_deleteTriangle ( srfP, tP->n12, 2 );
      }
      else
      {
        VSUBXY ( tP->p3->cor, tP->p2->cor, tmp );
        if ( VDOTXY ( tmp, tmp ) >= sizeSquared )
        {
          aecDTM_deleteTriangle ( srfP, tP, 2 );
          if ( tP->n23 ) aecDTM_deleteTriangle ( srfP, tP->n23, 2 );
        }
        else
        {
          VSUBXY ( tP->p3->cor, tP->p1->cor, tmp );
          if ( VDOTXY ( tmp, tmp ) >= sizeSquared )
          {
            aecDTM_deleteTriangle ( srfP, tP, 2 );
            if ( tP->n31 ) aecDTM_deleteTriangle ( srfP, tP->n31, 2 );
          }
        }
      }
    }
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_applyMaximumTriangleLengthAndMarkRangeTriangles
 DESC: It deletes any triangles that have at least one size whose length
       is longer than the maximum triangle length parameter and also marks
       any range triangles.  Combining these two processes into one saves
       time.
 HIST: Original - tmi 31-Mar-1994
 MISC:
 KEYW: DTM TRIANGLE SIZE MAXIMUM RANGE MARK
-----------------------------------------------------------------------------%*/

void aecDTM_applyMaximumTriangleLengthAndMarkRangeTriangles
(
  struct CIVdtmsrf *srfP,                    /* => surface to use             */
  double maxTriLength                        /* => max. tri. side length      */
)
{
  if ( srfP != (struct CIVdtmsrf *)0 )
  {
    struct CIVdtmblk *blkP;
    struct CIVdtmtin *tP;

    maxTriLength *= maxTriLength;
    ctin = 0L;
    aecDTM_countSurfaceData ( (long *)0, &ntin, srfP );
    aecStatus_initialize (TRUE);

    for ( blkP = srfP->tinf->blk; blkP; blkP = blkP->nxt )
      for ( tP = blkP->rec.tin; tP < blkP->rec.tin + blkP->use; tP++ )
        if ( !aecDTM_isTriangleDeletedFlagSet ( tP ) )
        {
          aecTicker_show ();
          if ( ntin > 0 )
            CIVSTATUS ( ctin, 1, ntin, DTM_M_MRKRNG );

          if ( aecDTM_isTriangleRangeTriangle ( srfP, tP ) )
            aecDTM_deleteTriangle ( srfP, tP, 2 );
          else if ( maxTriLength > 0. )
            aecDTM_applyMaximumTriangleLength ( srfP, maxTriLength, tP );
        }

    aecStatus_close ();
  }
}
