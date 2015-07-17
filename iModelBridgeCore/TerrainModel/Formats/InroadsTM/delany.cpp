//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* delany.c                                            tmi	  10-Apr-1990 */
/*----------------------------------------------------------------------------*/
/* Contains the functions necessary to achieve Delaunay triangles.            */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"

#define MAX_RECURSION_DEPTH 1000

extern boolean tinAltMoveMethod;

static int recursionDepth = 0;

/*----------------------------------------------------------------------------*/
/* Private static function prototypes                                         */
/*----------------------------------------------------------------------------*/
static int aecDTM_delaunayTriangleProcess(struct CIVdtmpnt *,struct CIVdtmtin *);

void aecDTM_delaunayTriangleInitRecursionDepthCheck()
{
    recursionDepth = 0;
}


/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_delaunayTriangle
 DESC: Main routine to achieve Delaunay triangles.
 HIST: Original - tmi 10-Apr-1990
 MISC:
 KEYW: DTM TRIANGLE DELAUNAY
-----------------------------------------------------------------------------%*/

int aecDTM_delaunayTriangle /* <= TRUE if error                    */
(
  struct CIVdtmsrf *srfP,              /* => surface to use                   */
  struct CIVdtmpnt *pP,                /* => point being added                */
  struct CIVdtmtin *aP,                /* => current triangle                 */
  long *ntinstkP,                      /* => # tins on tin stack              */
  long **tinstkPP                      /* => tin stack triangles              */
)
{
  struct CIVdtmtin *bP;
  int sts = SUCCESS, pntIndex;

  if( recursionDepth > MAX_RECURSION_DEPTH )
  {
    return sts;
  }

  recursionDepth++;

  if ( ( sts = aecDTM_getIndexOfTrianglePointByPointer ( &pntIndex, pP, aP ) ) == SUCCESS )
  {
    bP = (pntIndex == 2) ? aP->n12 : *(&aP->n12+pntIndex+1);

    if ( bP != aP )
    {
      if ( aecDTM_delaunayTriangleProcess ( pP, bP ) )
        if ( aecDTM_checkTrianglesForSwap ( aP, bP ) == SUCCESS )
          if ( ( sts = aecDTM_swapNeighboringTriangles ( srfP, aP, bP ) ) == SUCCESS )
            if ( ( sts = aecDTM_delaunayTriangle ( srfP, pP, aP, ntinstkP, tinstkPP ) ) == SUCCESS )
              if ( ( sts = aecDTM_delaunayTriangle ( srfP, pP, bP, ntinstkP, tinstkPP ) ) == SUCCESS )
                if ( tinstkPP )
                  sts = aecDTM_triangleStack ( ntinstkP, tinstkPP, bP );
    }
    else
    {
      aecDTM_triangulateSetErrorPoint ( aP->p1 );
      sts = DTM_M_TOLPRB;
    }
  }

  recursionDepth--;

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_delaunayTriangleCheck
 DESC: This routine just checks to determine how many triangles are
       Delaunaysized by adding the input point.  The actual configuration
       of the triangles remains unchanged.
 HIST: Original - tmi 10-Apr-1990
 MISC:
 KEYW: DTM TRIANGLE DELAUNAY CHECK
-----------------------------------------------------------------------------%*/

int aecDTM_delaunayTriangleCheck /* <= TRUE if error               */
(
  struct CIVdtmsrf *srfP,        /* => surface to use                         */
  struct CIVdtmpnt *pntP,        /* => point being added                      */
  struct CIVdtmtin *tinP,        /* => starting triangle                      */
  struct CIVdtmpnt *paP,         /* => triangle edge point to use             */
  struct CIVdtmpnt *pbP,         /* => 2nd triangle edge point to use         */
  long *ntinstkP,                /* => # tins on tin stack                    */
  long **tinstkPP                /* => triangle stack                         */
)
{
  struct CIVdtmpnt *p1P, *p2P, *p3P;
  int sts = SUCCESS, brk;

  if ( tinP->p1 == pbP  &&  tinP->p2 == paP )
    tinP = tinP->n12;
  else if ( tinP->p2 == pbP  &&  tinP->p3 == paP )
    tinP = tinP->n23;
  else
    tinP = tinP->n31;

  if ( tinP )
  {
    if ( tinP->p1 == paP  &&  tinP->p2 == pbP )
      p1P = tinP->p2,  p2P = tinP->p3,  p3P = tinP->p1,  brk = tinP->flg & DTM_C_TINB12;
    else if ( tinP->p2 == paP  &&  tinP->p3 == pbP )
      p1P = tinP->p3,  p2P = tinP->p1,  p3P = tinP->p2,  brk = tinP->flg & DTM_C_TINB23;
    else
      p1P = tinP->p1,  p2P = tinP->p2,  p3P = tinP->p3,  brk = tinP->flg & DTM_C_TINB31;

    if ( aecDTM_delaunayTriangleProcess ( pntP, tinP ) )
      if ( ! brk )
        if ( ( sts = aecDTM_triangleStack ( ntinstkP, tinstkPP, tinP ) ) == SUCCESS )
          if ( ( sts = aecDTM_delaunayTriangleCheck ( srfP, pntP, tinP, p2P, p1P, ntinstkP, tinstkPP ) ) == SUCCESS )
            sts = aecDTM_delaunayTriangleCheck ( srfP, pntP, tinP, p3P, p2P, ntinstkP, tinstkPP );
  }

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_delaunayTriangleProcess
 DESC: It computes to see if the triangle needs swapping.
 HIST: Original - tmi 01-Nov-1990
 MISC: static
 KEYW: DTM TRIANGLE DELAUNAY PROCESS
-----------------------------------------------------------------------------%*/

static int aecDTM_delaunayTriangleProcess
(
  struct CIVdtmpnt *pntP,
  struct CIVdtmtin *tinP
)
{
  int sts = FALSE;

  if ( tinP )
  {
    struct CIVdtmtin *altTinP = NULL;
    struct CIVdtmpnt *altTinPntsP = NULL;
    struct CIVdtmpnt *altPntP = NULL;
    double x, y, dx, dy, dx1, dy1, dx2, dy2, r, d, A, B, C, D, E, F, det;

    if ( tinAltMoveMethod )
    {
      DPoint3d moveOrigin;

      altTinP = (struct CIVdtmtin *)alloca ( sizeof ( struct CIVdtmtin ) );
      altTinPntsP = (struct CIVdtmpnt *)alloca ( sizeof ( struct CIVdtmpnt ) * 3 );
      altPntP = (struct CIVdtmpnt *)alloca ( sizeof ( struct CIVdtmpnt ) );

      memcpy ( altTinP, tinP, sizeof ( struct CIVdtmtin ) );
      memcpy ( &altTinPntsP[0], tinP->p1, sizeof ( struct CIVdtmpnt ) );
      memcpy ( &altTinPntsP[1], tinP->p2, sizeof ( struct CIVdtmpnt ) );
      memcpy ( &altTinPntsP[2], tinP->p3, sizeof ( struct CIVdtmpnt ) );
      memcpy ( altPntP, pntP, sizeof ( *altPntP ) );
      memcpy ( &moveOrigin, &pntP->cor, sizeof ( DPoint3d ) );

      VSUB ( altTinPntsP[0].cor, moveOrigin, altTinPntsP[0].cor );
      VSUB ( altTinPntsP[1].cor, moveOrigin, altTinPntsP[1].cor );
      VSUB ( altTinPntsP[2].cor, moveOrigin, altTinPntsP[2].cor );
      VSUB ( altPntP->cor, moveOrigin, altPntP->cor );

      altTinP->p1 = &altTinPntsP[0];
      altTinP->p2 = &altTinPntsP[1];
      altTinP->p3 = &altTinPntsP[2];

      tinP = altTinP;
      pntP = altPntP;
    }

    dx1 = tinP->p2->cor.x - tinP->p1->cor.x;
    dy1 = tinP->p2->cor.y - tinP->p1->cor.y;
    dx2 = tinP->p3->cor.x - tinP->p1->cor.x;
    dy2 = tinP->p3->cor.y - tinP->p1->cor.y;
    A = 2 * dx1;
    B = 2 * dy1;
    C = 2 * dx2;
    D = 2 * dy2;
    E = dx1 * dx1 + dy1 * dy1;
    F = dx2 * dx2 + dy2 * dy2;
    det = A * D - C * B;

    if ( det == 0.0 )
      sts = TRUE;
    else
    {
      dx = pntP->cor.x - tinP->p1->cor.x;
      dy = pntP->cor.y - tinP->p1->cor.y;
      x = ( D * E - B * F ) / det;
      y = ( A * F - C * E ) / det;
      r = x * x + y * y;
      x = x - dx;
      y = y - dy;
      d = x * x + y * y;
      if ( r > d )
        sts = TRUE;
    }
  }

  return ( sts );
}