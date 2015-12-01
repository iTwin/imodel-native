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


#define TIN_PROMPT_SIZE 1500
#define IDX_SRF_RANGEPOINTSET			14	// Range Point Set (child of SRF_SURFACE)
#define IDX_SRF_TRIANGLESET				16	// Triangle Set (child of SRF_SURFACE)

/*----------------------------------------------------------------------------*/
/* Structures                                                                 */
/*----------------------------------------------------------------------------*/
typedef struct CIVdiagproctin
{
  struct CIVdtmtin *tin;
  struct CIVdtmpnt **p1P;
  struct CIVdtmpnt **p2P;
  struct CIVdtmpnt **p3P;
  struct CIVdtmtin **n12P;
  struct CIVdtmtin **n23P;
  struct CIVdtmtin **n31P;
} CIVdiagproctin;


/*----------------------------------------------------------------------------*/
/* Constants and macros                                                       */
/*----------------------------------------------------------------------------*/
#define CIVSTATUS(a,b,c,d) { double zzz = (double)(a+=b)/(double)c * 100.; aecStatus_show ( zzz, d ); }
#define DISTANCE( px, py, qx, qy )	sqrt(((qx)-(px))*((qx)-(px))+((qy)-(py))*((qy)-(py)))

#define MAX_RECURSION_DEPTH 1000


/*----------------------------------------------------------------------------*/
/* Externals                                                                  */
/*----------------------------------------------------------------------------*/
static long npnt, cpnt, ntri, ctri;
static int recursionDepth = 0;
static struct CIVdtmpnt tinErrPnt;
static boolean tinErrPntSet = FALSE;

boolean tinAltMoveMethod = FALSE;

/*----------------------------------------------------------------------------*/
/* Private static function prototypes                                         */
/*----------------------------------------------------------------------------*/
static int  aecDTM_triangulatePointInside(struct CIVdtmsrf *,struct CIVdtmpnt *,struct CIVdtmtin *,int,struct CIVdtmpnt *,long *,long **);
static int  aecDTM_triangulatePointOnside(struct CIVdtmsrf *,struct CIVdtmpnt *,struct CIVdtmtin *,long,int,struct CIVdtmpnt *,long *,long **);



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_triangulatePoint
 DESC: Triangulates a single point.
 HIST: Original - tmi 17-Oct-1990
 MISC:
 KEYW: DTM TRIANGULATE POINT
-----------------------------------------------------------------------------%*/

int aecDTM_triangulatePoint /* <= TRUE if error                    */
(
  struct CIVdtmsrf *srfP,              /* => surface to use                   */
  struct CIVdtmfil *filP,              /* => dtm file containing point        */
  struct CIVdtmpnt *pntP,              /* => point to triangulate             */
  struct CIVdtmtin **tinPP,            /* => triangle to start process in     */
  long *ntinstkP,                      /* => # triangles on triangle stack    */
  long **tinstkPP                      /* => pointer to triangles on stack    */
)
{
  int sts, rpt, sid;

  // If the DTM_C_PNTTIN flag is set, do not triangulate this feature.
  if ( pntP->flg & DTM_C_PNTTIN )
  {
    sts = SUCCESS;
  }
  else
  {
    if ( ( sts = aecDTM_findTriangle ( tinPP, 0, &rpt, &sid, srfP, &pntP->cor ) ) == SUCCESS )
      if ( rpt != -1 )
        sts = aecDTM_deletePoint ( srfP, filP, pntP );
      else if ( sid )
        sts = aecDTM_triangulatePointOnside ( srfP, pntP, *tinPP, (long)sid, 0, (struct CIVdtmpnt *)0, ntinstkP, tinstkPP );
      else
        sts = aecDTM_triangulatePointInside ( srfP, pntP, *tinPP, 0, (struct CIVdtmpnt *)0, ntinstkP, tinstkPP );
  }

  return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_triangulatePointInside
 DESC: Triangulates regular points when they fall inside an existing
       triangles. This results in three new triangles being formed.
 HIST: Original - tmi 10-Apr-1990
 MISC: static
 KEYW: DTM TRIANGULATE POINT INSIDE
-----------------------------------------------------------------------------%*/

static int aecDTM_triangulatePointInside
(
  struct CIVdtmsrf *srf,
  struct CIVdtmpnt *pnt,
  struct CIVdtmtin *t1,
  int brk,
  struct CIVdtmpnt *prvpnt,
  long *ntinstk,
  long **tinstk
)
{
  struct CIVdtmpnt *p1, *p2, *p3;
  struct CIVdtmtin *t2, *t3, *n2, *n3, *zero;
  int sts;

  if ( srf->dis.confnc ) (*srf->dis.confnc)(srf,t1,0,srf->dis.consym);

  zero = (struct CIVdtmtin *)0;
  p1 = t1->p1;
  p2 = t1->p2;
  p3 = t1->p3;
  n2 = t1->n23;
  n3 = t1->n31;
  if ( ( sts = aecDTM_addTriangle ( &t2, srf, pnt, p2, p3, t1, n2, zero ) ) == SUCCESS )
    if ( ( sts = aecDTM_addTriangle ( &t3, srf, p1, pnt, p3, t1, t2, n3 ) ) == SUCCESS )
    {
      t1->p3  = pnt;
      t1->n23 = t2;
      t1->n31 = t3;
      t2->n31 = t3;

      if ( t1->flg & DTM_C_TINB23 ) t1->flg &= ~DTM_C_TINB23, t2->flg |= DTM_C_TINB23;
      if ( t1->flg & DTM_C_TINB31 ) t1->flg &= ~DTM_C_TINB31, t3->flg |= DTM_C_TINB31;

      aecDTM_updateTriangleNeighbor ( t1, n2, t2 );
      aecDTM_updateTriangleNeighbor ( t1, n3, t3 );

      if ( srf->dis.tinfnc ) (*srf->dis.tinfnc)(srf,t1,1,srf->dis.tinsym), (*srf->dis.tinfnc)(srf,t2,1,srf->dis.tinsym), (*srf->dis.tinfnc)(srf,t3,1,srf->dis.tinsym);
      if ( srf->dis.confnc ) (*srf->dis.confnc)(srf,t1,1,srf->dis.consym), (*srf->dis.confnc)(srf,t2,1,srf->dis.consym), (*srf->dis.confnc)(srf,t3,1,srf->dis.consym);

      if ( tinstk ) aecDTM_triangleStack ( ntinstk, tinstk, t1 ), aecDTM_triangleStack ( ntinstk, tinstk, t2 ), aecDTM_triangleStack ( ntinstk, tinstk, t3 );

      if ( brk )
      {
        aecDTM_setTriangleSideBreaklineFlag ( pnt, prvpnt, t1 );
        aecDTM_setTriangleSideBreaklineFlag ( pnt, prvpnt, t2 );
        aecDTM_setTriangleSideBreaklineFlag ( pnt, prvpnt, t3 );
      }

      if ( sts == SUCCESS )
        if ( ( sts = aecDTM_delaunayTriangle ( srf, pnt, t1, ntinstk, tinstk ) ) == SUCCESS )
          if ( ( sts = aecDTM_delaunayTriangle ( srf, pnt, t2, ntinstk, tinstk ) ) == SUCCESS )
            sts = aecDTM_delaunayTriangle ( srf, pnt, t3, ntinstk, tinstk );
    }

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_triangulatePointOnside
 DESC: Triangulates regular points when they fall on the edge of one or two
       existing triangles. This results in either two or four new triangles.
 HIST: Original - tmi 10-Apr-1990
 MISC: static
 KEYW: DTM TRIANGULATE POINT ONSIDE
-----------------------------------------------------------------------------%*/

static int aecDTM_triangulatePointOnside
(
  struct CIVdtmsrf *srf,
  struct CIVdtmpnt *pnt,
  struct CIVdtmtin *t1,
  long sid1,
  int brk,
  struct CIVdtmpnt *prvpnt,
  long *ntinstk,
  long **tinstk
)
{
  struct CIVdtmpnt *p1, *p2, *p3, *p4;
  struct CIVdtmtin *t2, *t3, *t4, *n1, *n2, *n3, *n4, *zero = (struct CIVdtmtin *)0;
  int sts, b0, b1, b2, b3, b4;
  long sid2;

  t2 = * ( &t1->n12 + sid1 - 1 );

  if ( srf->dis.tinfnc ) (*srf->dis.tinfnc)(srf,t1,0,srf->dis.tinsym), (*srf->dis.tinfnc)(srf,t2,0,srf->dis.tinsym);
  if ( srf->dis.confnc ) (*srf->dis.confnc)(srf,t1,0,srf->dis.consym), (*srf->dis.confnc)(srf,t2,0,srf->dis.consym);

  p1 = sid1 == 1 ? t1->p3  : * ( &t1->p1  + sid1 - 2 );
  p2 = * ( &t1->p1 + sid1 - 1 );
  p3 = p4 = (struct CIVdtmpnt *)0;

  n1 = sid1 == 1 ? t1->n31 : * ( &t1->n12 + sid1 - 2 );
  n2 = n3 = (struct CIVdtmtin *)0;
  n4 = sid1 == 3 ? t1->n12 : * ( &t1->n12 + sid1 );

  b0 = sid1 == 1 ? t1->flg & DTM_C_TINB12 : ( sid1 == 2 ? t1->flg & DTM_C_TINB23 : t1->flg & DTM_C_TINB31 );
  b1 = sid1 == 1 ? t1->flg & DTM_C_TINB31 : ( sid1 == 2 ? t1->flg & DTM_C_TINB12 : t1->flg & DTM_C_TINB23 );
  b2 = b3 = 0;
  b4 = sid1 == 1 ? t1->flg & DTM_C_TINB23 : ( sid1 == 2 ? t1->flg & DTM_C_TINB31 : t1->flg & DTM_C_TINB12 );
  t1->flg &= ~(DTM_C_TINB12 | DTM_C_TINB23 | DTM_C_TINB31);

  if ( t2  &&  ( sts = aecDTM_getTriangleSideIndex ( &sid1, &sid2, t1, t2, TRUE ) ) == SUCCESS )
  {
    p3 = sid2 == 1 ? t2->p3  : * ( &t2->p1  + sid2 - 2 );
    p4 = * ( &t2->p1 + sid2 - 1 );

    n2 = sid2 == 3 ? t2->n12 : * ( &t2->n12 + sid2 );
    n3 = sid2 == 1 ? t2->n31 : * ( &t2->n12 + sid2 - 2 );

    b2 = sid2 == 1 ? t2->flg & DTM_C_TINB23 : ( sid2 == 2 ? t2->flg & DTM_C_TINB31 : t2->flg & DTM_C_TINB12 );
    b3 = sid2 == 1 ? t2->flg & DTM_C_TINB31 : ( sid2 == 2 ? t2->flg & DTM_C_TINB12 : t2->flg & DTM_C_TINB23 );
    t2->flg &= ~(DTM_C_TINB12 | DTM_C_TINB23 | DTM_C_TINB31);
  }

  if ( ( sts = aecDTM_addTriangle ( &t3, srf, pnt, p3, p4, t2, n3, zero ) ) == SUCCESS )
    if ( ( sts = aecDTM_addTriangle ( &t4, srf, pnt, p4, p1, t3, n4, t1 ) ) == SUCCESS )
    {
      if ( b0 ) t1->flg |= DTM_C_TINB31, t2->flg |= DTM_C_TINB12, t3->flg |= DTM_C_TINB31, t4->flg |= DTM_C_TINB12;
      if ( b1 ) t1->flg |= DTM_C_TINB23;
      if ( b2 ) t2->flg |= DTM_C_TINB23;
      if ( b3 ) t3->flg |= DTM_C_TINB23;
      if ( b4 ) t4->flg |= DTM_C_TINB23;

      t1->p1  = t2->p1 = pnt;
      t1->p2  = p1;
      t1->p3  = t2->p2	= p2;
      t2->p3  = p3;

      t1->n12 = t4;
      t1->n23 = n1;
      t1->n31 = t2;
      t2->n12 = t1;
      t2->n23 = n2;
      t2->n31 = t3;
      t3->n31 = t4;

      if ( srf->dis.tinfnc ) (*srf->dis.tinfnc)(srf,t1,1,srf->dis.tinsym), (*srf->dis.tinfnc)(srf,t2,1,srf->dis.tinsym), (*srf->dis.tinfnc)(srf,t3,1,srf->dis.tinsym), (*srf->dis.tinfnc)(srf,t4,1,srf->dis.tinsym);
      if ( srf->dis.confnc ) (*srf->dis.confnc)(srf,t1,1,srf->dis.consym), (*srf->dis.confnc)(srf,t2,1,srf->dis.consym), (*srf->dis.confnc)(srf,t3,1,srf->dis.consym), (*srf->dis.confnc)(srf,t4,1,srf->dis.consym);

      if ( tinstk ) aecDTM_triangleStack ( ntinstk, tinstk, t1 ), aecDTM_triangleStack ( ntinstk, tinstk, t2 ), aecDTM_triangleStack ( ntinstk, tinstk, t3 ), aecDTM_triangleStack ( ntinstk, tinstk, t4 );

      aecDTM_updateTriangleNeighbor ( t1, n4, t4 );
      aecDTM_updateTriangleNeighbor ( t2, n3, t3 );

      if ( brk )
      {
        aecDTM_setTriangleSideBreaklineFlag ( pnt, prvpnt, t1 );
        aecDTM_setTriangleSideBreaklineFlag ( pnt, prvpnt, t2 );
        aecDTM_setTriangleSideBreaklineFlag ( pnt, prvpnt, t3 );
        aecDTM_setTriangleSideBreaklineFlag ( pnt, prvpnt, t4 );
      }

      if ( sts == SUCCESS )
        if ( ( sts = aecDTM_delaunayTriangle ( srf, pnt, t1, ntinstk, tinstk ) ) == SUCCESS )
          if ( ( sts = aecDTM_delaunayTriangle ( srf, pnt, t2, ntinstk, tinstk ) ) == SUCCESS )
            if ( ( sts = aecDTM_delaunayTriangle ( srf, pnt, t3, ntinstk, tinstk ) ) == SUCCESS )
              sts = aecDTM_delaunayTriangle ( srf, pnt, t4, ntinstk, tinstk );
    }

  return ( sts );
}

void aecDTM_triangulateClearErrorPoint
(
)
{
    memset ( &tinErrPnt, 0, sizeof ( struct CIVdtmpnt ) );
    tinErrPntSet = FALSE;
}

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
