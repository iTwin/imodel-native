//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* rnguti.c                                            tmi    10-Apr-1990     */
/*----------------------------------------------------------------------------*/
/* Contains all functions which compute ranges of civil data.                 */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"



/*----------------------------------------------------------------------------*/
/* Private static function prototypes                                         */
/*----------------------------------------------------------------------------*/
static void aecDTM_computeSurfacePointRange(struct CIVdtmfil *,struct CIVdtmpnt *);
static void aecDTM_computeSurfaceRangeProcess(struct CIVdtmfil *,struct CIVdtmfil *);






/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_computeSurfaceRange
 DESC: Exercises data contained within all files.
 HIST: Original - tmi 10-Apr-1990
 MISC:
 KEYW: DTM RANGE COMPUTE
-----------------------------------------------------------------------------%*/

void aecDTM_computeSurfaceRange
(
  struct CIVdtmsrf *srfP                /* => surface to use                  */
)
{
  if ( srfP != (struct CIVdtmsrf *)0 )
    if ( !(srfP->rngf->flg & DTM_C_NORANG) )
      aecDTM_computeSurfaceRangeForce ( srfP );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_computeSurfaceRangeForce
 DESC: Same as aecDTM_computeSurfaceRange except does not honor DTM_C_NORANG flag.
 HIST: Original - tmi 12-Aug-1994
 MISC:
 KEYW: DTM RANGE COMPUTE
-----------------------------------------------------------------------------%*/

void aecDTM_computeSurfaceRangeForce
(
  struct CIVdtmsrf *srfP                /* => surface to use                  */
)
{
  if ( srfP != (struct CIVdtmsrf *)0 )
  {
    struct CIVdtmfil *rngfP = srfP->rngf;
    int i;

    rngfP->min.x = rngfP->min.y = rngfP->min.z = AEC_C_MAXDBL;
    rngfP->max.x = rngfP->max.y = rngfP->max.z = AEC_C_MINDBL;

    for ( i = 0; i < DTM_C_NMPNTF; i++ )
    {
      aecDTM_computeSurfaceFileRange ( srfP->pntf[i] );
      aecDTM_computeSurfaceRangeProcess ( rngfP, srfP->pntf[i] );
    }
  }
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_computeSurfaceFileRange
 DESC: Exercises the data contained within a single point file.
 HIST: Original - tmi 10-Apr-1990
 MISC:
 KEYW: DTM RANGE FILE COMPUTE
-----------------------------------------------------------------------------%*/

void aecDTM_computeSurfaceFileRange
(
  struct CIVdtmfil *filP                /* => surface file to use             */
)
{
  if ( filP != (struct CIVdtmfil *)0 )
  {
    struct CIVdtmblk *blkP;
    struct CIVdtmpnt *pP;

    filP->min.x = filP->min.y = filP->min.z = AEC_C_MAXDBL;
    filP->max.x = filP->max.y = filP->max.z = AEC_C_MINDBL;

    for ( blkP = filP->blk; blkP; blkP = blkP->nxt )
      for ( pP = blkP->rec.pnt; pP < blkP->rec.pnt + blkP->use; pP++ )
        if ( !aecDTM_isPointDeletedFlagSet ( pP ) )
          aecDTM_computeSurfacePointRange ( filP, pP );

    filP->flg |= DTM_C_FILEXR;
  }
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_computeSurfacePointRange
 DESC: Computes the point range.
 HIST: Original - tmi 26-Feb-1991
 MISC: static
 KEYW: DTM RANGE POINT COMPUTE
-----------------------------------------------------------------------------%*/

static void aecDTM_computeSurfacePointRange
(
  struct CIVdtmfil *filP,
  struct CIVdtmpnt *pP
)
{
  if ( pP->cor.x < filP->min.x ) filP->min.x = pP->cor.x;
  if ( pP->cor.y < filP->min.y ) filP->min.y = pP->cor.y;
  if ( pP->cor.z < filP->min.z ) filP->min.z = pP->cor.z;
  if ( pP->cor.x > filP->max.x ) filP->max.x = pP->cor.x;
  if ( pP->cor.y > filP->max.y ) filP->max.y = pP->cor.y;
  if ( pP->cor.z > filP->max.z ) filP->max.z = pP->cor.z;
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_computeSurfaceRangeProcess
 DESC: Updates the surface's range file limits.
 HIST: Original - tmi 26-Feb-1991
 MISC: static
 KEYW: DTM RANGE SURFACE COMPUTE PROCESS
-----------------------------------------------------------------------------%*/

static void aecDTM_computeSurfaceRangeProcess
(
  struct CIVdtmfil *rngfP,
  struct CIVdtmfil *filP
)
{
  if ( filP->min.x < rngfP->min.x ) rngfP->min.x = filP->min.x;
  if ( filP->min.y < rngfP->min.y ) rngfP->min.y = filP->min.y;
  if ( filP->min.z < rngfP->min.z ) rngfP->min.z = filP->min.z;
  if ( filP->max.x > rngfP->max.x ) rngfP->max.x = filP->max.x;
  if ( filP->max.y > rngfP->max.y ) rngfP->max.y = filP->max.y;
  if ( filP->max.z > rngfP->max.z ) rngfP->max.z = filP->max.z;
}





/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_updateSurfaceRange
 DESC: Checks to determine if the input point is outside of the surface
       range.  If it is, it updates the range rectangle to account for the
       points location.
 HIST: Original - tmi 17-Oct-1990
 MISC:
 KEYW: DTM RANGE SURFACE UPDATE
-----------------------------------------------------------------------------%*/

void aecDTM_updateSurfaceRange
(
  struct CIVdtmsrf *srfP,                /* => surface to use                 */
  struct CIVdtmpnt *pntP                 /* => point to check                 */
)
{
  if ( srfP != (struct CIVdtmsrf *)0  &&  pntP != (struct CIVdtmpnt *)0 )
  {
    double deltaX, deltaY;

    deltaX = deltaY = AEC_C_MINDBL;

    if ( pntP->cor.x < srfP->rngf->min.x )
      deltaX = srfP->rngf->min.x - pntP->cor.x;

    if ( pntP->cor.x > srfP->rngf->max.x )
      deltaX = pntP->cor.x - srfP->rngf->max.x;

    if ( pntP->cor.y < srfP->rngf->min.y )
      deltaY = srfP->rngf->min.y - pntP->cor.y;

    if ( pntP->cor.y > srfP->rngf->max.y )
      deltaY = pntP->cor.y - srfP->rngf->max.y;

    if ( deltaX != AEC_C_MINDBL  ||  deltaY != AEC_C_MINDBL )
    {
      if ( !(srfP->rngf->flg & DTM_C_NORANG) )
      {
        DPoint3d cor[4];
        double dx, dy;

        dx = fabs(srfP->rngf->max.x - srfP->rngf->min.x);
        dy = fabs(srfP->rngf->max.y - srfP->rngf->min.y);

        if ( deltaX >= deltaY )
          deltaY = ( fabs(dx) < AEC_C_TOL ) ? deltaX : ( dy / dx ) * ( deltaX + dx ) - dy;
        else if ( deltaY > deltaX )
          deltaX = ( fabs(dy) < AEC_C_TOL ) ? deltaY : ( dx / dy ) * ( deltaY + dy ) - dx;

        srfP->rngf->min.x -= deltaX;
        srfP->rngf->min.y -= deltaY;
        srfP->rngf->max.x += deltaX;
        srfP->rngf->max.y += deltaY;

        aecDTM_setRangePointCoordinates ( cor, &srfP->rngf->min );
        srfP->rngf->blk->rec.pnt[0].cor = cor[0];
        srfP->rngf->blk->rec.pnt[1].cor = cor[1];
        srfP->rngf->blk->rec.pnt[2].cor = cor[2];
        srfP->rngf->blk->rec.pnt[3].cor = cor[3];
      }

      aecDTM_unmarkRangeTriangles ( srfP );
      aecDTM_markRangeTriangles ( srfP );
    }
  }
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_setRangePointCoordinates
 DESC: It sets the coordinates of the four range points used to define
       the convex hull of the triangulated surface.
 HIST: Original - tmi 03-Nov-1993
 MISC:
 KEYW: DTM POINT RANGE COORDINATES SET
-----------------------------------------------------------------------------%*/

void aecDTM_setRangePointCoordinates
(
  DPoint3d *pP,                        /* <=> array of 4 coordinates          */
  DPoint3d *rangeP                     /*  => lower left & upper right coord. */
)
{
  pP[0].x = rangeP[0].x - DTM_C_RNGTOL;
  pP[0].y = rangeP[0].y - DTM_C_RNGTOL;
  pP[0].z = rangeP[0].z - DTM_C_RNGTOL;

  pP[2].x = rangeP[1].x + DTM_C_RNGTOL;
  pP[2].y = rangeP[1].y + DTM_C_RNGTOL;
  pP[2].z = rangeP[1].z + DTM_C_RNGTOL;

  pP[1].x = pP[0].x;
  pP[1].y = pP[2].y;
  pP[1].z = pP[0].z;

  pP[3].x = pP[2].x;
  pP[3].y = pP[0].y;
  pP[3].z = pP[2].z;
}



