//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* bondry.c                                            tmi    14-Apr-1990     */
/*----------------------------------------------------------------------------*/
/* Several utilities to compute the perimeter of a triangulated DTM.	      */
/* The function returns an array of vertices and the number of points	      */
/* in that array.  The user must free the memory used to hold the	      */
/* vertices when they're all done.  The first and last point in the           */
/* returned polygon are the same.					      */
/*									      */
/* If the surface has an exterior boundary set of points, then they are       */
/* used as the perimeter.  Otherwise, the software finds the outeredge	      */
/* of the outermost non-deleted, non-range triangles.  These sides are	      */
/* then connected into a linstring which is returned to the caller.           */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"



/*----------------------------------------------------------------------------*/
/* Private static function prototypes                                         */
/*----------------------------------------------------------------------------*/
static int aecDTM_getSurfacePerimeterWithExterior(long *,DPoint3d **,struct CIVdtmsrf *);
static int aecDTM_getSurfacePerimeterWithoutExterior(long *,DPoint3d **,struct CIVdtmsrf *);
static int aecDTM_getSurfacePerimeterAddSegment(DPoint3d **,int,DPoint3d*,DPoint3d*,int*,long*);


/*----------------------------------------------------------------------------*/
/* Constants and macros                                                       */
/*----------------------------------------------------------------------------*/
#define     BONDRY_ALCSIZ     100






/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_getSurfacePerimeter
 DESC: Returns the surface perimeter
 HIST: Original - tmi 14-Apr-1990
 MISC: User must call free to free memory holding perimter
       coordinates, when done.
 KEYW: DTM PERIMETER BOUNDARY
-----------------------------------------------------------------------------%*/

int aecDTM_getSurfacePerimeter /* <= TRUE if error                 */
(
  long *nvrtP,                         /* <= # of perimeter vertices          */
  DPoint3d **vrtPP,                    /* <= per. verts. (free)        */
  struct CIVdtmsrf *srfP               /* => surface to use                   */
)
{
  int sts = SUCCESS;

  *nvrtP = srfP->extf->nrec - srfP->extf->ndel;

  if ( *nvrtP > 0 )
    sts = aecDTM_getSurfacePerimeterWithExterior ( nvrtP, vrtPP, srfP );
  else if ( srfP->tinf->nrec - srfP->tinf->ndel <= 0 )
    sts = DTM_M_NOTINF;
  else
    sts = aecDTM_getSurfacePerimeterWithoutExterior ( nvrtP, vrtPP, srfP );

  if( sts == SUCCESS )
    aecPolygon_removeDuplicatePoints( nvrtP, *vrtPP, AEC_C_TOL );

  return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_getSurfacePerimeterWithExterior
 DESC: Computes the model perimeter using the exterior point type points.
 HIST: Original - tmi 14-Apr-1990
 MISC: static
 KEYW: DTM PERIMETER BOUNDARY COMPUTE EXTERIOR
-----------------------------------------------------------------------------%*/

static int aecDTM_getSurfacePerimeterWithExterior /* <= TRUE if error         */
(
  long *nvrtP,                         /* <= # of perimeter vertices          */
  DPoint3d **vrtPP,                    /* <= per. verts. (free)        */
  struct CIVdtmsrf *srfP               /* => surface to use                   */
)
{
  struct CIVdtmblk *blkP;
  struct CIVdtmpnt *pP;
  DPoint3d *tmpP;
  int sts = SUCCESS;
  long i = 0;

  if ( ( tmpP = *vrtPP = (DPoint3d *) malloc ( (unsigned int)*nvrtP*sizeof(DPoint3d) ) ) == 0L )
    sts = DTM_M_MEMALF;
  else
    for ( blkP = srfP->extf->blk; blkP; blkP = blkP->nxt )
      for ( pP = blkP->rec.pnt; pP < blkP->rec.pnt + blkP->use; pP++ )
	if ( !aecDTM_isPointDeletedFlagSet(pP) )
        {
	  DTMPOINTTODPOINT ( srfP, pP, tmpP[i] );
          i++;
        }

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_getSurfacePerimeterWithoutExterior
 DESC: Computes the model perimeter when no exterior point type points are
       present in the mode.
 HIST: Original - tmi 14-Apr-1990
 MISC: static
 KEYW: DTM PERIMETER BOUNDARY COMPUTE
-----------------------------------------------------------------------------%*/

static int aecDTM_getSurfacePerimeterWithoutExterior
(
  long *nvrtP,                         /* <= # of perimeter vertices          */
  DPoint3d **vrtPP,                    /* <= per. verts. (free)        */
  struct CIVdtmsrf *srfP               /* => surface to use                   */
)
{
  struct CIVdtmtin *tinP, *strtinP, *prvtinP;
  struct CIVdtmpnt *p0P, *p1P;
  DPoint3d t0, t1;
  int sts = SUCCESS, state = 0, nalc = 0, sid;
  long lsid, jnk;

  *nvrtP = 0L;
  *vrtPP = (DPoint3d *)0;

  if ( ( sts = aecDTM_findOutsideTriangle ( &strtinP, &prvtinP, srfP ) ) == SUCCESS )
    if ( ( sts = aecDTM_getTriangleSideIndex ( &lsid, &jnk, strtinP, prvtinP, TRUE ) ) == SUCCESS )
    {
      tinP = prvtinP = strtinP;
      p0P = ( lsid == 3 ) ? tinP->p1 : *(&tinP->p1+lsid);
      DTMPOINTTODPOINT ( srfP, p0P, t0 );
      sid = lsid - 1;

      do
      {
        tinP = prvtinP;
        p1P = *(&tinP->p1+sid);
        DTMPOINTTODPOINT ( srfP, p1P, t1 );
        sts = aecDTM_getSurfacePerimeterAddSegment ( vrtPP, state, &t0, &t1, &nalc, nvrtP );
        state = 1;

        while ( !aecDTM_isTriangleDeletedFlagSet ( tinP )  &&  sts == SUCCESS )
        {
          if ( tinP )
          {
            prvtinP = tinP;
            sts = aecDTM_rotateAroundPoint ( NULL, &tinP, NULL, &sid, p1P, prvtinP, 0 );
          }
          else
            sts = DTM_M_TINCOR;
        }
      } while ( sts == SUCCESS  &&  ! VEQUAL((*vrtPP)[0],(*vrtPP)[*nvrtP-1],AEC_C_TOL) );

      state = 2;
      if ( sts == SUCCESS )
        sts = aecDTM_getSurfacePerimeterAddSegment ( vrtPP, state, &t0, &t1, &nalc, nvrtP );
    }

  return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_getSurfacePerimeterAddSegment
 DESC: Adds surface boundary segments to the hopper.
 HIST: Original - tmi 06-Mar-1991
 MISC: static
 KEYW: DTM PERIMETER BOUNDARY MEMORY ALLOC
-----------------------------------------------------------------------------%*/

static int aecDTM_getSurfacePerimeterAddSegment
(
  DPoint3d **vrtPP,
  int state,
  DPoint3d *p0P,
  DPoint3d *p1P,
  int *nalcP,
  long *nvrtP
)
{
  int sts = SUCCESS;

  switch ( state )
  {
    case ( 0 ) :
      *nalcP = 1;
      if ( ( *vrtPP = (DPoint3d *) malloc ( (unsigned int)(*nalcP*BONDRY_ALCSIZ)*sizeof(DPoint3d) ) ) == (DPoint3d *)0 )
        sts = DTM_M_MEMALF;
      else
        (*vrtPP)[(*nvrtP)++] = *p0P, (*vrtPP)[(*nvrtP)++] = *p1P;
      break;

    case ( 1 ) :
      if ( *nvrtP >= *nalcP * BONDRY_ALCSIZ )
        if ( ( *vrtPP = (DPoint3d *) realloc ( *vrtPP, (unsigned int)(++(*nalcP))*BONDRY_ALCSIZ*sizeof(DPoint3d) ) ) == (DPoint3d *)0 )
          sts = DTM_M_MEMALF;
      if ( sts == SUCCESS ) (*vrtPP)[(*nvrtP)++] = *p1P;
      break;

    case ( 2 ) :
      if ( ( *vrtPP = (DPoint3d *) realloc ( *vrtPP, (unsigned int)*nvrtP*sizeof(DPoint3d) ) ) == (DPoint3d *)0 )
        sts = DTM_M_MEMALF;
      break;
  }

  return ( sts );
}
