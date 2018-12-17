//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* pntuti.c                                               tmi    24-Oct-1990  */
/*----------------------------------------------------------------------------*/
/* Various utilities to do things to and with points.                         */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"


/*----------------------------------------------------------------------------*/
/* Private static function prototypes                                         */
/*----------------------------------------------------------------------------*/

static int _pntFileIndexStart[DTM_C_NMPNTF];
static struct CIVdtmsrf *_indexSrfP = NULL;


/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_removeConstructionPoints
 DESC: Removes the construction points from each block.  The points in
       the block are then compressed.  This function should only be used
       by the triangularization routine.
 HIST: Original - tmi 24-Oct-1990
 MISC: Construction points can only be random points, so that is the only
       point file type that is processed.
 KEYW: DTM POINT CONSTRUCTION DELETE REMOVE
-----------------------------------------------------------------------------%*/

void aecDTM_removeConstructionPoints
(
  struct CIVdtmsrf *srfP            /* => surface to process                  */
)
{
  struct CIVdtmblk *blkP;
  struct CIVdtmpnt *pP;
  long use;

  for ( blkP = srfP->regf->blk; blkP; blkP = blkP->nxt )
  {
    use = blkP->use;

    for ( pP = blkP->rec.pnt; pP < blkP->rec.pnt + use; pP++ )
      if ( aecDTM_isPointConstructionFlagSet ( pP ) )
        aecDTM_deletePoint ( srfP, srfP->regf, pP );
  }
  aecDTM_countSurfacePointsInFile ( srfP->regf );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_isPointRangePoint
 DESC: Returns TRUE if input point is a range point.
 HIST: Original - tmi 23-Oct-1993
 MISC:
 KEYW: DTM POINT TYPE RANGE
-----------------------------------------------------------------------------%*/

int aecDTM_isPointRangePoint    /* <= TRUE if point is range point */
(
  struct CIVdtmsrf *srfP,                  /* => surface to use               */
  struct CIVdtmpnt *pntP                   /* => point to check               */
)
{
  return ( pntP >= srfP->rngf->blk->rec.pnt  &&  pntP <= srfP->rngf->blk->rec.pnt + srfP->rngf->blk->use - 1 );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_getIndexOfTrianglePointByCoordinate
 DESC: Given a point's coordinates and a set of triangle coordinates, this
       functions determines which point in the triangle the point is.
 HIST: Original - tmi 12-Jun-1991
 MISC:
 KEYW: DTM POINT TRIANGLE INDEX LOCATION COORDINATES
-----------------------------------------------------------------------------%*/

int aecDTM_getIndexOfTrianglePointByCoordinates /* <= TRUE if error */
(
  int *pntIndexP,                       /* <= point's index (0, 1, 2, 3)      */
  DPoint3d *pntP,                       /* => point coordinates to use        */
  DPoint3d *tP                          /* => array of all triangle coords.   */
)
{
  int sts = DTM_M_NOPNTF;

  if ( pntIndexP != (int *)0 )
  {
    if      ( VEQUALXY ( *pntP, tP[0], AEC_C_TOL ) ) *pntIndexP = 0;
    else if ( VEQUALXY ( *pntP, tP[1], AEC_C_TOL ) ) *pntIndexP = 1;
    else if ( VEQUALXY ( *pntP, tP[2], AEC_C_TOL ) ) *pntIndexP = 2;
    else                                             *pntIndexP = 3;

    if ( *pntIndexP != 3 ) sts = SUCCESS;
  }

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_getIndexOfTrianglePointByPointer
 DESC: Given a point's pointer and a triangle, this function returns the
       index of the point within the triangle.
 HIST: Original - tmi 30-Jul-1991
 MISC:
 KEYW: DTM POINT TRIANGLE INDEX POINTER
-----------------------------------------------------------------------------%*/

int aecDTM_getIndexOfTrianglePointByPointer
(
  int *pntIndexP,                       /* <= point's index (0, 1, 2, 3)      */
  struct CIVdtmpnt *pP,                 /* => point to use                    */
  struct CIVdtmtin *tP                  /* => triangle to use                 */
)
{
  int sts = DTM_M_NOPNTF;

  if ( pntIndexP != (int *)0 )
  {
    if      ( pP == tP->p1 ) *pntIndexP = 0;
    else if ( pP == tP->p2 ) *pntIndexP = 1;
    else if ( pP == tP->p3 ) *pntIndexP = 2;
    else                   *pntIndexP = 3;

    if ( *pntIndexP != 3 ) sts = SUCCESS;
  }

  return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_isPointFirstLinearPoint
 DESC: Determines if the input linear point is the first one in a linear feature.
 HIST: Original - tmi 23-Jun-1994
 MISC: NOTE: No check is made to ensure input point is not a random point.
 KEYW: DTM POINT LINEAR IS FIRST POINT
-----------------------------------------------------------------------------%*/

int aecDTM_isPointFirstLinearPoint /* <= TRUE if first linear pnt. */
(
  struct CIVdtmpnt *pntP               /* => point to check                   */
)
{
  int sts = FALSE;

  if ( pntP != (struct CIVdtmpnt *)0 )
    if ( !(pntP->flg & DTM_C_PNTPUD) )
      sts = TRUE;

  return ( sts );
}


void aecDTM_removeDuplicateDTMPoints
(
  long *nvrtP,                         /* <=> # of vertices in line.          */
  CIVdtmpnt *vrtP,                     /* <=> linestring vertices             */
  double tolerance                     /*  => tolerance to use                */
)
{

  int i, j = *nvrtP;

  if (*nvrtP > 1)
  {
    for( i = 1, j = 1; i < *nvrtP; i++ )
    {
      if(!(VEQUAL( vrtP[i-1].cor, vrtP[i].cor, tolerance )) )
      {
        vrtP[j] = vrtP[i];
        ++j;
      }
    }
  }
  *nvrtP = j;
}

