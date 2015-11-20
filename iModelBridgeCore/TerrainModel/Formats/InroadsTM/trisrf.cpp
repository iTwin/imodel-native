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
static int  aecDTM_triangulateStart(struct CIVdtmsrf *,long,int);
static int  aecDTM_triangulateProcess(struct CIVdtmsrf *,int,int);
static int  aecDTM_triangulatePointProcess(void *,int,long,DPoint3d *,struct CIVdtmpnt *);
static int  aecDTM_triangulatePointInside(struct CIVdtmsrf *,struct CIVdtmpnt *,struct CIVdtmtin *,int,struct CIVdtmpnt *,long *,long **);
static int  aecDTM_triangulatePointOnside(struct CIVdtmsrf *,struct CIVdtmpnt *,struct CIVdtmtin *,long,int,struct CIVdtmpnt *,long *,long **);
static int  aecDTM_triangulateLinearProcess(struct CIVdtmpnt *,struct CIVdtmpnt **,struct CIVdtmtin *,struct CIVdtmtin **,struct CIVdtmfil *,struct CIVtinLineData *,int,int *,struct CIVdtmpnt *,struct CIVdtmpnt *);
static int  aecDTM_triangulateLinearAffectedTriangles(struct CIVdtmpnt *,struct CIVdtmpnt *,struct CIVdtmtin *,struct CIVdtmtin *,struct CIVtinLineData *);
static int  aecDTM_triangulateLinearAffectedTrianglesProcess(struct CIVdtmtin *,DPoint3d *,DPoint3d *,int,int,void *);
static void aecDTM_triangulateLinearDirect(struct CIVdtmpnt *,struct CIVdtmpnt *,struct CIVdtmtin *,long *);
static int  aecDTM_triangulateLinearLists(struct CIVdtmpnt *,struct CIVdtmpnt *,struct CIVtinLineData *);
static int  aecDTM_triangulateCheckAreaTolerance(struct CIVtinLineData *);
static int  aecDTM_triangulateLinearPatch(struct CIVtinLineData *);
static int  aecDTM_triangulateLinearFindNeighbor(long,long *,struct CIVdtmtin **,int *);
static int  aecDTM_triangulateLinearAllocateMore(long,long *,long **,long **);
static int  aecDTM_triangulateLinearAllocate(struct CIVtinLineData *);
static int  aecDTM_alignTriangleDiagonals(struct CIVdtmsrf *);
static int  aecDTM_alignTriangleDiagonalsProcess(void *,long,DPoint3d *,struct CIVdtmtin*,unsigned long);
static int  aecDTM_isRectangle ( struct CIVdtmpnt *rectPnts[4] );
static int aecDTM_nullInvalidateTinPointers(void*, long, DPoint3d*, struct CIVdtmtin*, unsigned long);



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_triangulate
 DESC: Triangulates a surface.
 HIST: Original - tmi 10-Apr-1990
 MISC:
 KEYW: DTM TRIANGULATE
-----------------------------------------------------------------------------%*/

int aecDTM_triangulate        /* <= TRUE if error                  */
(
  long *numPointsP,                      /* <= # points triangulated (or NULL)*/
  long *numTrianglesP,                   /* <= # triangles generated (or NULL)*/
  long *timeP,                           /* <= triang. time (secs) (or NULL)  */
  struct CIVdtmsrf *srfP,                /* => surface to triangulate         */
  int options,                           /* => options                        */
  double *maxTriangleLengthP,            /* => max. tri length (or NULL)      */
  byte *extDataChecksP,                  /* => TRUE, FALSE (or NULL)          */
  double *ftrFilterToleranceP,           /* => feature filter tol. (or NULL)  */
  struct CIVdistri *distriP,             /* => triangle display pars (or NULL)*/
  boolean useAltMoveMethod // = FALSE     /* => user alternate move to 0,0,0 method */
)
{
  if ( aecDTM_isLockTrianglesFlagSet(srfP) )
    return DTM_M_TINLOCKED;

  tinAltMoveMethod = useAltMoveMethod;

  int sts = SUCCESS;
  time_t start = 0, stop;
  int opt = DTM_C_NOBREK;

  aecDTM_clearAllPointsIgnoreSegmentFlag ( srfP );
  aecDTM_triangulateClearErrorPoint ( );

  if ( options & TRISRF_INT ) opt = 0;

  npnt = cpnt = 0L;
  srfP->dis.tinsym = distriP;
  if ( timeP != (long *)0 ) time ( &start );

  aecTicker_initialize();

  if( sts == SUCCESS )
    sts = aecDTM_crossingCheckExterior( srfP );

  if( sts == SUCCESS && ((extDataChecksP == (byte *)0 ) ? srfP->par.extDatChk : *extDataChecksP) )
    sts = aecDTM_crossingCheck ( srfP, DTM_C_NOTINS, NULL, NULL, NULL );

  if( sts == SUCCESS )
  {
    aecDTM_removeConstructionPoints ( srfP );
    if ( ( sts = aecDTM_deallocateFile ( srfP->tinf ) ) == SUCCESS )
      if ( ( sts = aecDTM_deallocateTriangleStack ( srfP ) ) == SUCCESS )
      {
        aecDTM_countSurfaceData ( &npnt, (long *)0, srfP );
        if ( npnt > 0L )
          if ( ( sts = aecDTM_deallocateFile ( srfP->rngf ) ) == SUCCESS )
            if ( ( sts = aecDTM_triangulateStart ( srfP, npnt, options & TRISRF_BLK ) ) == SUCCESS )
            {
              aecDTM_delaunayTriangleInitRecursionDepthCheck();
              aecStatus_initialize(TRUE);
              sts = aecDTM_triangulateProcess ( srfP, opt, ((extDataChecksP == (byte *)0 ) ? srfP->par.extDatChk : *extDataChecksP) );
              aecStatus_close();

              if ( sts == SUCCESS ) sts = aecDTM_markBoundaryTriangles ( srfP );
              aecDTM_applyMaximumTriangleLengthAndMarkRangeTriangles ( srfP, (maxTriangleLengthP == (double *)0 ) ? srfP->par.maxsid : *maxTriangleLengthP );

              if ( srfP->ptrIndexTableP != (void *)0 ) 
                if ( sts == SUCCESS )
                  sts = aecDTM_indexTableBuild ( srfP );
            }
      }
  }

  if ( sts == SUCCESS && ((extDataChecksP == (byte *)0 ) ? srfP->par.extDatChk : *extDataChecksP) )
     sts = aecDTM_alignTriangleDiagonals ( srfP );

  // Removing duplicate points during the triangulation process may result in
  // features that have no points.  Let's get rid of those here.
  aecDTM_removeChildlessFtrs ( srfP );

  if ( numPointsP    != (long *)0 ) aecDTM_countSurfaceData ( numPointsP, (long *)0, srfP );
  if ( numTrianglesP != (long *)0 ) aecDTM_countSurfaceData ( (long *)0, numTrianglesP, srfP );

  srfP->dis.tinfnc = (int (*)(void *,struct CIVdtmtin *,int,void *))0;

  aecTicker_stop();
  if ( timeP != (long *)0 ) time ( &stop ), *timeP = (long)(stop - start);
  memset ( &srfP->dis, 0, sizeof(struct CIVdtmdis) );

  if ( sts == SUCCESS )
  {
    aecDTM_clearSurfaceTinOutOfDateFlag ( srfP );
  }

  tinAltMoveMethod = FALSE;

  if ( sts == DTM_M_TOLPRB && !useAltMoveMethod )
  {
    sts = aecDTM_triangulate ( numPointsP, numTrianglesP, timeP, srfP, options, maxTriangleLengthP,
                               extDataChecksP, ftrFilterToleranceP, distriP, TRUE );
  }

  if ( sts != SUCCESS && !useAltMoveMethod )
  {
    aecDTM_sendAllTriangles ( (void *)0, srfP, DTM_C_NOBREK|DTM_C_DELETE, aecDTM_nullInvalidateTinPointers, srfP );
  }

  return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_triangulateStart
 DESC: Allocates as a first guess twice the number of points as the number
       of triangles and then forms the initial two triangles from the range
       of the surface points.
 HIST: Original - tmi 10-Apr-1990
 MISC: static
 KEYW: DTM TRIANGULATE START
-----------------------------------------------------------------------------%*/

static int aecDTM_triangulateStart
(
  struct CIVdtmsrf *srf,
  long npnt,
  int useBlockSize
)
{
  struct CIVdtmblk *blk;
  struct CIVdtmtin *t1, *t2;
  struct CIVdtmpnt *p;
  DPoint3d cor[4];
  int sts;
  long nrec = useBlockSize ? DTM_C_BLKSIZ : 2 * ( npnt + 100 );

  if (nrec > DTM_C_BLKSIZTIN)
      nrec = DTM_C_BLKSIZTIN;

  aecDTM_computeSurfaceRange ( srf );
  aecDTM_setRangePointCoordinates ( cor, &srf->rngf->min );

  if ( ( sts = aecDTM_allocateBlock ( &blk, srf->tinf, nrec, (int)0 ) ) == SUCCESS )
    if ( ( sts = aecDTM_addPointsExt ( &p, srf, (int)(DTM_C_DTMRNG | ADDPNT_DTM), 4L, cor ) ) == SUCCESS )
      if ( ( sts = aecDTM_addTriangle ( &t1, srf, &p[0], &p[1], &p[2], (struct CIVdtmtin *)0, (struct CIVdtmtin *)0, (struct CIVdtmtin *)0 ) ) == SUCCESS )
        if ( ( sts = aecDTM_addTriangle ( &t2, srf, &p[2], &p[3], &p[0], (struct CIVdtmtin *)0, (struct CIVdtmtin *)0, (struct CIVdtmtin *)0 ) ) == SUCCESS )
	{
          t1->n31 = t2;
          t2->n31 = t1;
          if ( srf->dis.tinfnc ) (*srf->dis.tinfnc)(srf,t1,1,srf->dis.tinsym), (*srf->dis.tinfnc)(srf,t2,1,srf->dis.tinsym);
          if ( srf->dis.confnc ) (*srf->dis.confnc)(srf,t1,1,srf->dis.consym), (*srf->dis.confnc)(srf,t2,1,srf->dis.consym);
        }

  return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_triangulateProcess
 DESC: Triangulates the model.
 HIST: Original - tmi 10-Apr-1990
 MISC: static
 KEYW: DTM TRIANGULATE PROCESS
-----------------------------------------------------------------------------%*/

static int aecDTM_triangulateProcess
(
  struct CIVdtmsrf *srf,
  int opt,
  int robust
)
{
  struct CIVtinLineData dat;
  int sts;

  memset ( &dat, 0, sizeof(dat) );
  dat.srf = srf;
  dat.robust = robust;

  opt |= DTM_C_PNTWEL;

  if ( ( sts = aecDTM_sendAllPoints ( (void *)0, srf, opt, DTM_C_BRKMSK|DTM_C_INFMSK|DTM_C_CTRMSK|DTM_C_INTMSK|DTM_C_EXTMSK, aecDTM_triangulateLinear, &dat ) ) == SUCCESS )
    if ( ( sts = aecDTM_sendAllPoints ( (void *)0, srf, opt, DTM_C_REGMSK, aecDTM_triangulatePointProcess, &dat ) ) == SUCCESS )
    {
      opt &= ~DTM_C_PNTWEL;
      opt |=  DTM_C_PNTWOE;

      if ( ( sts = aecDTM_sendAllPoints ( (void *)0, srf, opt, DTM_C_BRKMSK|DTM_C_INFMSK|DTM_C_CTRMSK|DTM_C_INTMSK|DTM_C_EXTMSK, aecDTM_triangulateLinear, &dat ) ) == SUCCESS )
        sts = aecDTM_triangulateLinearFree ( &dat );
    }

  return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_triangulatePointProcess
 DESC: Receives each regulat point and processes it.
 HIST: Original - tmi 03-Aug-1991
 MISC: static
 KEYW: DTM TRIANGULATE POINT PROCESS
-----------------------------------------------------------------------------%*/

static int aecDTM_triangulatePointProcess
(
  void *tmp,
  int,
  long np,
  DPoint3d *,
  struct CIVdtmpnt *pnt
)
{
  struct CIVtinLineData *dat = (struct CIVtinLineData *)tmp;

  if ( pnt->flg & DTM_C_PNTTIN )
    return ( SUCCESS );

  aecTicker_show ();

  if( npnt > 0 )
    CIVSTATUS ( cpnt, np, npnt, DTM_M_TRIREG );

  return ( aecDTM_triangulatePoint ( dat->srf, dat->srf->regf, pnt, &dat->tmp, (long *)0, (long **)0 ) );
}




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




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_triangulateLinear
 DESC: Handles the triangularization for a single linear feature.
 HIST: Original - tmi 22-Oct-1990
 MISC:
 KEYW: DTM TRIANGULATE LINEAR
-----------------------------------------------------------------------------%*/

int aecDTM_triangulateLinear /* <= TRUE if error                   */
(
  void *tmp,                           /* => triangulate data structure       */
  int typ,                             /* => type of input points             */
  long np,                             /* => # points in string               */
  DPoint3d *,                          /* => array of point coordinates       */
  struct CIVdtmpnt *pnt                /* => pointer to first linear point    */
)
{
  struct CIVtinLineData *dat = (struct CIVtinLineData *)tmp;
  struct CIVdtmfil *fil = dat->srf->pntf[typ];
  struct CIVdtmpnt *startPnt, *endPnt;
  int sts, startRpt, endRpt;
  long i;


  // If the DTM_C_PNTTIN flag is set, do not triangulate this feature.
  if ( pnt[0].flg & DTM_C_PNTTIN )
  {
    sts = SUCCESS;
  }
  else
  {
    aecTicker_show ();

    if( npnt > 0 )
      CIVSTATUS ( cpnt, 1, npnt, DTM_M_TRILIN );

    startPnt = &pnt[0];
    if ( startPnt->flg & DTM_C_PNTELV ) aecDTM_getElevationAtXY ( &startPnt->cor, dat->srf, &dat->startTin );

    if ( ( sts = aecDTM_findTriangle ( &dat->startTin, (int *)0, &startRpt, (int *)0, dat->srf, &startPnt->cor ) ) == SUCCESS )
      if ( startRpt != -1 )
        startPnt = *(&dat->startTin->p1+startRpt);
      else
        sts = aecDTM_triangulatePoint ( dat->srf, fil, startPnt, &dat->startTin, (long *)0, (long **)0 );

    for ( i = 1; i < np  &&  sts == SUCCESS; i++ )
    {
      aecTicker_show ();

      if( npnt > 0 )
        CIVSTATUS ( cpnt, 1, npnt, DTM_M_TRILIN );

      endPnt = &pnt[i];
      if ( endPnt->flg & DTM_C_PNTELV ) aecDTM_getElevationAtXY ( &endPnt->cor, dat->srf, &dat->endTin );

      recursionDepth = 0;

      sts = aecDTM_triangulateLinearProcess ( startPnt, &endPnt, dat->startTin, &dat->endTin, fil, dat, startRpt, &endRpt, &pnt[i-1], &pnt[i] );

      startPnt = endPnt;
      dat->startTin = dat->endTin;
      startRpt = endRpt;
    }
  }

  return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_triangulateLinearProcess
 DESC: It processes a single linear line segment.
 HIST: Original - tmi 24-Oct-1990
 MISC: static
 KEYW: DTM TRIANGULATE LINEAR PROCESS
-----------------------------------------------------------------------------%*/

static int aecDTM_triangulateLinearProcess
(
  struct CIVdtmpnt *p0,
  struct CIVdtmpnt **p1,
  struct CIVdtmtin *startTin,
  struct CIVdtmtin **endTin,
  struct CIVdtmfil *fil,
  struct CIVtinLineData *dat,
  int startRpt,
  int *endRpt,
  struct CIVdtmpnt *inputP0,
  struct CIVdtmpnt *inputP1
)
{
  int sts;

  sts = aecInterrupt_check();

  if ( sts == SUCCESS )
    if( recursionDepth > MAX_RECURSION_DEPTH )
      sts = DTM_M_TOLPRB;

  if ( sts == SUCCESS )
    if ( ( sts = aecDTM_findTriangle ( endTin, (int *)0, endRpt, (int *)0, dat->srf, &(*p1)->cor ) ) == SUCCESS )
      if ( *endRpt != -1 )
        *p1 = *(&(*endTin)->p1 + *endRpt);
      else
        sts = aecDTM_triangulatePoint ( dat->srf, fil, *p1, endTin, (long *)0, (long **)0 );

  if ( sts == SUCCESS )
    if ( inputP0->flg & DTM_C_PNTIGN )
      sts = DTM_M_STPPRC;

  if ( sts == SUCCESS )
    if( VEQUALXY( inputP0->cor, inputP1->cor, AEC_C_TOL ) )
      sts = DTM_M_STPPRC;

  if ( sts == SUCCESS )
    if ( ( sts = aecDTM_triangulateLinearAffectedTriangles ( p0, *p1, startTin, *endTin, dat ) ) == SUCCESS )
      if ( ! dat->direct )
        if ( dat->conPnt != (struct CIVdtmpnt *)0  &&  dat->conPnt != *p1 )
        {
          struct CIVdtmpnt *conPnt = dat->conPnt;
          int tmpRpt;

          ++recursionDepth;

          if ( ( sts = aecDTM_triangulateLinearProcess ( p0, &conPnt, startTin, endTin, fil, dat, startRpt, &tmpRpt, inputP0, inputP1 ) ) == SUCCESS )
            sts = aecDTM_triangulateLinearProcess ( conPnt, p1, startTin, endTin, fil, dat, tmpRpt, endRpt, inputP0, inputP1 );

          --recursionDepth;
        }
        else if ( dat->tinNum > 0L )
          if ( ( sts = aecDTM_triangulateLinearLists ( p0, *p1, dat ) ) == SUCCESS )
            sts = aecDTM_triangulateLinearPatch ( dat );

  if ( sts == DTM_M_STPPRC ) sts = SUCCESS;

  if ( sts == SUCCESS )
    sts = aecDTM_triangulateLinearFree ( dat );
  else
    aecDTM_triangulateLinearFree ( dat );

  return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_triangulateLinearAffectedTriangles
 DESC: Generates a list of triangles affected by the current linear segment.
       But if linear segment passes over another point, this function
       immediately returns so that that point can be processed.  And, if
       this linear segment passes over another, a construction point is
       generated and the function immediately returns so that the two new
       segments can be generated.
 HIST: Original - tmi 01-Aug-1991
 MISC: static
 KEYW: DTM TRIANGULATE LINEAR AFFECTED TRIANGLES FIND
-----------------------------------------------------------------------------%*/

static int aecDTM_triangulateLinearAffectedTriangles
(
  struct CIVdtmpnt *p0,
  struct CIVdtmpnt *p1,
  struct CIVdtmtin *startTin,
  struct CIVdtmtin *endTin,
  struct CIVtinLineData *dat
)
{
  int sts = SUCCESS;

  dat->tinNum = 0L;
  dat->conPnt = (struct CIVdtmpnt *)0;
  dat->direct = 0;

  if ( VEQUALXY ( p0->cor, p1->cor, AEC_C_TOL ) )
    dat->direct = 1;
  else if ( ( sts = aecDTM_sendTrianglesAlongLineFirst ( &startTin, (struct CIVdtmtin **)0, dat->srf, p0, p1 ) ) == SUCCESS )
    if ( ( sts = aecDTM_sendTrianglesAlongLineFirst ( &endTin, (struct CIVdtmtin **)0, dat->srf, p1, p0 ) ) == SUCCESS )
    {
      aecDTM_triangulateLinearDirect ( p0, p1, startTin, &dat->direct );
      if ( ! dat->direct )
        sts = aecDTM_sendTrianglesAlongLine ( (void *)0, dat->srf, &p0->cor, &p1->cor, startTin, endTin, aecDTM_triangulateLinearAffectedTrianglesProcess, (void *)dat );
    }

  if ( sts == DTM_M_STPPRC ) dat->tinNum = 0L, sts = SUCCESS;

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_triangulateLinearAffectedTrianglesProcess
 DESC: Helps out the previous function.
 HIST: Original - tmi 01-Aug-1991
 MISC: static
 KEYW: DTM TRIANGULATE LINEAR AFFECTED TRIANGLES FIND PROCESS
-----------------------------------------------------------------------------%*/

static int aecDTM_triangulateLinearAffectedTrianglesProcess
(
  struct CIVdtmtin *tin,
  DPoint3d *,
  DPoint3d *p,
  int pntIndex,
  int sidIndex,
  void *tmp
)
{
  struct CIVtinLineData *dat = (struct CIVtinLineData *)tmp;
  int sts;

  if ( ( sts = aecDTM_triangulateLinearAllocateMore ( dat->tinNum, &dat->tinAlc, &dat->tin, (long **)0 ) ) == SUCCESS )
  {
    dat->tin[dat->tinNum++] = (long)tin;

    if ( pntIndex != 3 )
    {
      dat->conPnt = *(&tin->p1+pntIndex);
      sts = DTM_M_STPPRC;
    }
    else if ( sidIndex != 3  &&  tin->flg & (DTM_C_TINB12<<sidIndex) )
      if ( ( sts = aecDTM_addPointsToFile ( &dat->conPnt, dat->srf, dat->srf->regf, DTM_C_DTMREG, 0, 1L, p ) ) == SUCCESS )
      {
        aecDTM_setPointConstructionFlag ( dat->conPnt );
        sts = DTM_M_STPPRC;
      }
  }

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_triangulateLinearDirect
 DESC: Checks to see if the two input points are directly connected.
 HIST: Original - tmi 02-Aug-1991
 MISC: static
 KEYW: DTM TRIANGULATE LINEAR DIRECT
-----------------------------------------------------------------------------%*/

static void aecDTM_triangulateLinearDirect
(
  struct CIVdtmpnt *p0,
  struct CIVdtmpnt *p1,
  struct CIVdtmtin *a,
  long *direct
)
{
  struct CIVdtmtin *b = (struct CIVdtmtin *)0;
  long sida, sidb;

  if ( a )
  {
    if      ( a->p1 == p0  &&  a->p2 == p1 ) b = a->n12;
    else if ( a->p2 == p0  &&  a->p3 == p1 ) b = a->n23;
    else if ( a->p3 == p0  &&  a->p1 == p1 ) b = a->n31;
    else if ( a->p1 == p1  &&  a->p2 == p0 ) b = a->n12;
    else if ( a->p2 == p1  &&  a->p3 == p0 ) b = a->n23;
    else if ( a->p3 == p1  &&  a->p1 == p0 ) b = a->n31;

    if ( aecDTM_getTriangleSideIndex ( &sida, &sidb, a, b, FALSE ) == SUCCESS )
    {
      a->flg |= DTM_C_TINB12 << (sida-1);
      if( b ) b->flg |= DTM_C_TINB12 << (sidb-1);
      *direct = 1;
    }
  }
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_triangulateLinearLists
 DESC: Generates the left and right point and triangle neighbor lists.
 HIST: Original - tmi 01-Aug-1991
 MISC: static
 KEYW: DTM TRIANGULATE LINEAR LISTS
-----------------------------------------------------------------------------%*/

static int aecDTM_triangulateLinearLists
(
  struct CIVdtmpnt *p0,
  struct CIVdtmpnt *p1,
  struct CIVtinLineData *dat
)
{
  struct CIVdtmtin *t;
  int sts, pntIndex, side = 0;
  long i;

  dat->leftNum = dat->rightNum = 0L;

  t = (struct CIVdtmtin *)dat->tin[0];
  if ( ( sts = aecDTM_getIndexOfTrianglePointByPointer ( &pntIndex, p0, t ) ) == SUCCESS )
  {
    if ( ( sts = aecDTM_triangulateLinearAllocate ( dat ) ) == SUCCESS )
    {
        dat->leftPnt[dat->leftNum++] = dat->rightPnt[dat->rightNum++] = (long)p0;
        if ( ( sts = aecDTM_triangulateLinearAllocate ( dat ) ) == SUCCESS )
        {
        dat->leftNei[dat->leftNum-1]   = (long)*(&t->n12+pntIndex);
        dat->rightNei[dat->rightNum-1] = (pntIndex == 0) ? (long)t->n31 : (long)*(&t->n12+pntIndex-1);
        dat->leftPnt[dat->leftNum++]   = (pntIndex == 2) ? (long)t->p1  : (long)*(&t->p1+pntIndex+1);
        dat->rightPnt[dat->rightNum++] = (pntIndex == 0) ? (long)t->p3  : (long)*(&t->p1+pntIndex-1);
        }
    }

    for ( i = 1; i < dat->tinNum - 1  &&  sts == SUCCESS; i++ )
    {
        t = (struct CIVdtmtin *)dat->tin[i];

        if (      (long)t->n12 == dat->tin[i-1]  &&  (long)t->n23 == dat->tin[i+1] )
        pntIndex = 2, side = 1;
        else if ( (long)t->n23 == dat->tin[i-1]  &&  (long)t->n31 == dat->tin[i+1] )
        pntIndex = 0, side = 1;
        else if ( (long)t->n31 == dat->tin[i-1]  &&  (long)t->n12 == dat->tin[i+1] )
        pntIndex = 1, side = 1;
        else if ( (long)t->n12 == dat->tin[i+1]  &&  (long)t->n23 == dat->tin[i-1] )
        pntIndex = 0, side = 0;
        else if ( (long)t->n23 == dat->tin[i+1]  &&  (long)t->n31 == dat->tin[i-1] )
        pntIndex = 1, side = 0;
        else if ( (long)t->n31 == dat->tin[i+1]  &&  (long)t->n12 == dat->tin[i-1] )
        pntIndex = 2, side = 0;
        else
        sts = DTM_M_NOSIDE;

        if ( sts == SUCCESS )
        if ( ( sts = aecDTM_triangulateLinearAllocate ( dat ) ) == SUCCESS )
            if ( side )
            {
            dat->rightNei[dat->rightNum-1] = (long)*(&t->n12+pntIndex);
            dat->rightPnt[dat->rightNum++] = (long)*(&t->p1+pntIndex);
            }
            else
            {
            dat->leftNei[dat->leftNum-1] = (pntIndex == 0) ? (long)t->n31 : (long)*(&t->n12+pntIndex-1);
            dat->leftPnt[dat->leftNum++] = (long)*(&t->p1+pntIndex);
            }
    }

    if ( sts == SUCCESS )
    {
        t = (struct CIVdtmtin *)dat->tin[dat->tinNum-1];
        aecDTM_getIndexOfTrianglePointByPointer ( &pntIndex, p1, t );

        if ( ( sts = aecDTM_triangulateLinearAllocate ( dat ) ) == SUCCESS )
        {
        dat->leftNei[dat->leftNum-1]   = (pntIndex == 0) ? (long)t->n31 : (long)*(&t->n12+pntIndex-1);
        dat->rightNei[dat->rightNum-1] = (long)*(&t->n12+pntIndex);
        dat->leftPnt[dat->leftNum++]   = dat->rightPnt[dat->rightNum++] = (long)p1;

        if ( ( sts = aecDTM_triangulateLinearAllocate ( dat ) ) == SUCCESS )
        {
            dat->leftNei[dat->leftNum-1] = dat->rightNei[dat->rightNum-1] = -1;
            dat->leftPnt[dat->leftNum++] = dat->rightPnt[dat->rightNum++] = (long)p0;

            if ( dat->robust )
            if ( ( sts = aecDTM_triangulateCheckAreaTolerance ( dat ) ) == DTM_M_STPPRC )
                sts = aecDTM_triangulateLinearFree ( dat );
        }
        }
    }
  }

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_triangulateCheckAreaTolerance
 DESC: The following block of code was added because of tolerance
       problems with breaklines sitting ontop of breaklines.  It forces
       skipping adding of a linear feature if patching process would have
       tolerance problems.  The PATCH_TOL value is currently just guessed
       at.
 HIST: Original - tmi 05-Oct-1993
 MISC: static
 KEYW: DTM TRIANGULATE AREA TOLERANCE CHECK
-----------------------------------------------------------------------------%*/

#define PATCH_TOL  0.01

static int aecDTM_triangulateCheckAreaTolerance
(
  struct CIVtinLineData *dat
)
{
  int sts = SUCCESS;
  DPoint3d *leftP, *rightP;

  leftP = rightP = (DPoint3d *)0;

  if ( ( leftP  = (DPoint3d *) calloc ( dat->leftNum,  sizeof(DPoint3d) ) ) != (DPoint3d *)0 )
    if ( ( rightP = (DPoint3d *) calloc ( dat->rightNum, sizeof(DPoint3d) ) ) != (DPoint3d *)0 )
    {
      long i;
      double leftArea, rightArea;

      for ( i = 0; i < dat->leftNum; i++  ) leftP[i]  = ((struct CIVdtmpnt *)dat->leftPnt[i])->cor;
      for ( i = 0; i < dat->rightNum; i++ ) rightP[i] = ((struct CIVdtmpnt *)dat->rightPnt[i])->cor;

      aecPolygon_planarArea ( &leftArea,  dat->leftNum,  leftP );
      aecPolygon_planarArea ( &rightArea, dat->rightNum, rightP );

      if ( leftArea < PATCH_TOL  ||  rightArea < PATCH_TOL )
        sts = DTM_M_STPPRC;
    }
    else
      sts = DTM_M_MEMALF;
  else
    sts = DTM_M_MEMALF;

  if ( leftP  != (DPoint3d *)0  )
      free ( leftP  );
  if ( rightP != (DPoint3d *)0 )
      free ( rightP );

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_triangulateLinearPatch
 DESC: It fills in the holes on the left and right side of the segment.
 HIST: Original - tmi 01-Aug-1991
 MISC: static
 KEYW: DTM TRIANGULATE LINEAR PATCH
-----------------------------------------------------------------------------%*/

static int aecDTM_triangulateLinearPatch
(
  struct CIVtinLineData *dat
)
{
  int sts = SUCCESS;

  if ( dat->leftNum > 0L  &&  dat->rightNum > 0L )
  {
    struct CIVdtmtin *leftTin, *rightTin;
    int leftSide, rightSide;
    long i, leftNum, rightNum;

    leftNum  = dat->leftNum;
    rightNum = dat->rightNum;

    for ( i = 0; i < dat->tinNum  &&  sts == SUCCESS; i++ )
      sts = aecDTM_triangleStackPut ( dat->srf, (struct CIVdtmtin *)dat->tin[i], 0 );

    aecPolygon_reverseLongArray ( dat->rightNum, dat->rightPnt );
    aecPolygon_reverseLongArray ( dat->rightNum-1, dat->rightNei );

    aecDTM_patchCheck ( &dat->leftNum, dat->leftPnt, dat->leftNei );
    if ( ( sts = aecDTM_patchHole ( dat->srf, dat->leftNum, dat->leftPnt, dat->leftNei, &dat->leftTinNum, &dat->leftTin ) ) == SUCCESS )
    {
      aecDTM_patchCheck ( &dat->rightNum, dat->rightPnt, dat->rightNei );
      if ( ( sts = aecDTM_patchHole ( dat->srf, dat->rightNum, dat->rightPnt, dat->rightNei, &dat->rightTinNum, &dat->rightTin ) ) == SUCCESS )
        if ( ( sts = aecDTM_triangulateLinearFindNeighbor ( dat->leftTinNum, dat->leftTin, &leftTin, &leftSide ) ) == SUCCESS )
          if ( ( sts = aecDTM_triangulateLinearFindNeighbor ( dat->rightTinNum, dat->rightTin, &rightTin, &rightSide ) ) == SUCCESS )
          {
            *(&leftTin->n12+leftSide)   = rightTin;
            *(&rightTin->n12+rightSide) = leftTin;
            if ( ( sts = aecDTM_patchDelaunay ( dat->srf, dat->leftNum, dat->leftPnt, dat->leftTinNum, dat->leftTin ) ) == SUCCESS )
              if ( ( sts = aecDTM_patchDelaunay ( dat->srf, dat->rightNum, dat->rightPnt, dat->rightTinNum, dat->rightTin ) ) == SUCCESS )
              {
                dat->tmp = (struct CIVdtmtin *)0;
                for ( i = dat->leftNum; i < leftNum  &&  sts == SUCCESS; i += 2 )
                  sts = aecDTM_triangulatePoint ( dat->srf, dat->srf->regf, (struct CIVdtmpnt *)dat->leftPnt[i], &dat->tmp, (long *)0, (long **)0 );
                for ( i = dat->rightNum; i < rightNum  &&  sts == SUCCESS; i += 2 )
                  sts = aecDTM_triangulatePoint ( dat->srf, dat->srf->regf, (struct CIVdtmpnt *)dat->rightPnt[i], &dat->tmp, (long *)0, (long **)0 );
              }
          }
    }
  }

  return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_triangulateLinearFindNeighbor
 DESC: It finds the triangle in the input list that has a neighbor = -1.
 HIST: Original - tmi 02-Aug-1991
 MISC: static
 KEYW: DTM TRIANGULATE LINEAR NEIGHBOR FIND
-----------------------------------------------------------------------------%*/

static int aecDTM_triangulateLinearFindNeighbor
(
  long tinNum,
  long *tins,
  struct CIVdtmtin **tin,
  int *side
)
{
  int sts = SUCCESS;
  long i;

  for ( *side = -1, i = 0; i < tinNum  &&  *side == -1; i++ )
  {
    *tin = (struct CIVdtmtin *)tins[i];

    if ( (long)(*tin)->n12 == -1 )
      *side = 0, (*tin)->flg |= DTM_C_TINB12;
    else if ( (long)(*tin)->n23 == -1 )
      *side = 1, (*tin)->flg |= DTM_C_TINB23;
    else if ( (long)(*tin)->n31 == -1 )
      *side = 2, (*tin)->flg |= DTM_C_TINB31;
  }

  if ( *side == -1 ) sts = DTM_M_NOSIDE;

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_triangulateLinearAllocateMore
 DESC: This is the memory allocator.
 HIST: Original - tmi 01-Aug-1991
 MISC: static
 KEYW: DTM TRIANGULATE LINEAR ALLOCATE MEMORY MORE
-----------------------------------------------------------------------------%*/

#define ALCSIZ 1000

static int aecDTM_triangulateLinearAllocateMore
(
  long num,
  long *nalc,
  long **ptr1,
  long **ptr2
)
{
  int sts = SUCCESS;

  if ( num >= *nalc * ALCSIZ )
  {
    unsigned int siz;

    *nalc += 1;
    siz = (unsigned int) (*nalc * ALCSIZ);

    *ptr1 = (num == 0L) ? (long *) calloc ( siz, sizeof(long) ) : (long *) realloc ( *ptr1, siz * sizeof(long) );
    if ( *ptr1 == (long *)0 )
      sts = DTM_M_MEMALF;

    if ( ptr2 != (long **)0 )
    {
      *ptr2 = (num == 0L) ? (long *) calloc ( siz, sizeof(long) ) : (long *) realloc ( *ptr2, siz * sizeof(long) );
      if ( *ptr2 == (long *)0 )
        sts = DTM_M_MEMALF;
    }
  }

  return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_triangulateLinearAllocate
 DESC: Allocates memory for linear feature triangularization.
 HIST: Original - tmi 01-Aug-1991
 MISC: static
 KEYW: DTM TRIANGULATE LINEAR ALLOCATE MEMORY
-----------------------------------------------------------------------------%*/

static int aecDTM_triangulateLinearAllocate
(
  struct CIVtinLineData *dat
)
{
  int sts;

  if ( ( sts = aecDTM_triangulateLinearAllocateMore ( dat->leftNum, &dat->leftAlc, &dat->leftPnt, &dat->leftNei ) ) == SUCCESS )
    sts = aecDTM_triangulateLinearAllocateMore ( dat->rightNum, &dat->rightAlc, &dat->rightPnt, &dat->rightNei );

  return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_triangulateLinearFree
 DESC: It frees up the memory used when triangulating linear features.
 HIST: Original - tmi 01-Aug-1991
 MISC:
 KEYW: DTM TRIANGULATE LINEAR MEMORY DEALLOCATE FREE
-----------------------------------------------------------------------------%*/

int aecDTM_triangulateLinearFree /* <= TRUE if error               */
(
  struct CIVtinLineData *dat           /* => internal data structure          */
)
{
  int sts = SUCCESS;

  if ( dat->tin      != (long *)0)
      free ( dat->tin );
  if ( dat->leftPnt  != (long *)0 )
      free ( dat->leftPnt );
  if ( dat->leftNei  != (long *)0)
      free ( dat->leftNei );
  if ( dat->leftTin  != (long *)0 )
      free ( dat->leftTin );
  if ( dat->rightPnt != (long *)0 )
      free ( dat->rightPnt );
  if ( dat->rightNei != (long *)0 )
      free ( dat->rightNei );
  if ( dat->rightTin != (long *)0 )
      free ( dat->rightTin );

  dat->tin = dat->leftPnt = dat->leftNei = dat->leftTin = dat->rightPnt = dat->rightNei = dat->rightTin = (long *)0;
  dat->tinNum = dat->tinAlc = dat->leftNum = dat->leftAlc = dat->leftTinNum = dat->rightNum = dat->rightAlc = dat->rightTinNum = 0L;

  return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_alignTriangleDiagonals
 DESC: Post process that searches for rectangular triangle pairs and reassigns 
       their points so that diaglonals are formed in a consistent manner.  If a
       rectangular pair is found, points will be arragned so that the diagonal
       appears from the lower left to the upper right of the rectangle.  If a 
       non-orthogonal rectanglular pair is found, points will be arranged so that
       the diagonal appears from the lowest corner to the corner opposite the 
       lowest in the rectangle.
 HIST: Original - twl 16-Aug-1998
 MISC:
 KEYW: DTM TRIANGULATE ALIGN DIAGONALS
-----------------------------------------------------------------------------%*/

static int aecDTM_alignTriangleDiagonals
(
  struct CIVdtmsrf *srfP                /* => surface to triangulate         */
)
{
  int sts = SUCCESS;

  ctri = 0L;
  ntri = 0L;
  aecDTM_countSurfaceData ( (long *)0, &ntri, srfP );
  aecTicker_initialize();
  aecStatus_initialize(TRUE);
  aecDTM_clearAllTrianglesProcessedFlag ( srfP );
  sts = aecDTM_sendAllTriangles ( (void *)0, srfP, 0, aecDTM_alignTriangleDiagonalsProcess, (void *)srfP );
  aecDTM_clearAllTrianglesProcessedFlag ( srfP );
  aecStatus_close();
  aecTicker_stop();

  return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_isRectangle
 DESC: Triangulates a surface.
 HIST: Original - tmi 10-Apr-1990
 MISC:
 KEYW: DTM TRIANGULATE
-----------------------------------------------------------------------------%*/

static int aecDTM_isRectangle
(
  struct CIVdtmpnt *rectPnts[4]
)
{
    DPoint3d vec1;
    DPoint3d vec2;

    memset ( &vec1, 0, sizeof ( DPoint3d ) );
    memset ( &vec2, 0, sizeof ( DPoint3d ) );

    VSUBXY ( rectPnts[0]->cor, rectPnts[1]->cor, vec1 );
    VSUBXY ( rectPnts[2]->cor, rectPnts[1]->cor, vec2 );

    if ( fabs ( VDOT ( vec1, vec2 ) ) > AEC_C_TOL )
        return FALSE;

    VSUBXY ( rectPnts[1]->cor, rectPnts[2]->cor, vec1 );
    VSUBXY ( rectPnts[3]->cor, rectPnts[2]->cor, vec2 );

    if ( fabs ( VDOT ( vec1, vec2 ) ) > AEC_C_TOL )
        return FALSE;


    VSUBXY ( rectPnts[2]->cor, rectPnts[3]->cor, vec1 );
    VSUBXY ( rectPnts[0]->cor, rectPnts[3]->cor, vec2 );

    if ( fabs ( VDOT ( vec1, vec2 ) ) > AEC_C_TOL )
        return FALSE;

    return TRUE;
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_alignTriangleDiagonalsProcess
 DESC: Compares the input triangle with each of it's neighbors to see if a 
       rectangular pair is formed.  If a rectangular pair is found, the diagonal
       formed by the triangle pair is checked to see if it runs from the lowest
       corner of the rectangle to the corner opposite the lowest, or from the 
       lower left corner to the upper right corner in the case of an orthogonal
       rectangle.  If not, triangle points are rearranged and neibors reassigned
       so that it does.
 HIST: Original - twl 16-Aug-1998
 MISC:
 KEYW: DTM TRIANGULATE ALIGN DIAGONALS PROCESS
-----------------------------------------------------------------------------%*/

static int aecDTM_alignTriangleDiagonalsProcess
(
  void *ud,
  long,
  DPoint3d *,
  struct CIVdtmtin *tin,
  unsigned long
)
{
  struct CIVdtmsrf *srfP = (struct CIVdtmsrf *) ud;
  struct CIVdtmpnt *rectPnts[4];
  struct CIVdiagproctin tin1;
  struct CIVdiagproctin tin2;
  boolean bStop = FALSE;
  double length1;
  double length2;
  int lowRectPntNdx;
  int i;
  int j;
  int sts = SUCCESS;

  memset (&tin1, 0, sizeof (tin1));
  memset (&tin2, 0, sizeof (tin2));

  aecTicker_show();

  if( ntri > 0 )
    CIVSTATUS ( ctri, 1, ntri, DTM_M_TRIREG );

  tin1.tin = tin;


  // If the process bit is set, this triangle has already been processed
  // in a rectangular triangle pair.  Since a triangle can only have one
  // neighbor that forms a rectangle, we can skip it.  If the triangle is
  // deleted or removed from the network, we don't process it.

  if ( aecDTM_isTriangleProcessedFlagSet ( tin1.tin )   ||
       aecDTM_isTriangleDeletedFlagSet   ( tin1.tin )   || 
       aecDTM_isTriangleRemovedFlagSet   ( tin1.tin ) )
  {
    bStop = TRUE;
  }
  

  // Loop through the triangle's neigbors. We'll set up pointers to the
  // triangle's points and neibors so that the triangle corner opposite 
  // the diagonal will always be pointed to by p3P and; p1P and p2P will
  // point to the endpoints of the diagonal.  
 
  for ( i = 0; i < 3 && !bStop; i++ )
  {
    switch ( i )
    {
    case 0:  // Neigbor on side 12.


      // If side 12 is a breakline we can't touch it.

      if ( tin1.tin->flg & DTM_C_TINB12 )  
        continue;

      tin1.p1P  = &tin->p1;
      tin1.p2P  = &tin->p2;
      tin1.p3P  = &tin->p3;
      tin1.n12P = &tin->n12;
      tin1.n23P = &tin->n23;
      tin1.n31P = &tin->n31;
      tin2.tin  = tin->n12;
      break;

    case 1: // Neigbor on side 23.


      // If side 23 is a breakline we can't touch it.

      if ( tin1.tin->flg & DTM_C_TINB23 )
        continue;

      tin1.p1P  = &tin->p2;
      tin1.p2P  = &tin->p3;
      tin1.p3P  = &tin->p1;
      tin1.n12P = &tin->n23;
      tin1.n23P = &tin->n31;
      tin1.n31P = &tin->n12;
      tin2.tin  = tin->n23;
      break;

    case 2: // Neigbor on side 31.


      // If side 31 is a breakline we can't touch it.

      if ( tin1.tin->flg & DTM_C_TINB31 )
        continue;

      tin1.p1P  = &tin->p3;
      tin1.p2P  = &tin->p1;
      tin1.p3P  = &tin->p2;
      tin1.n12P = &tin->n31;
      tin1.n23P = &tin->n12;
      tin1.n31P = &tin->n23;
      tin2.tin  = tin->n31;
      break;
    }


    // If the process bit is, set this triangle has already been processed
    // in a rectangular triangle pair.  Since a triangle can only have one
    // neighbor that forms a rectangle, we can skip it.  If the triangle is
    // deleted or removed from the network, we don't process it.

    if ( aecDTM_isTriangleProcessedFlagSet ( tin2.tin )   ||
         aecDTM_isTriangleDeletedFlagSet   ( tin2.tin )   || 
         aecDTM_isTriangleRemovedFlagSet   ( tin2.tin ) )
    {
      continue;
    }


    // Setup the neigboring triangle's pointers.  We'll set up pointers to the
    // triangle's points and neibors so that the triangle corner opposite 
    // the diagonal will always be pointed to by p3P and; p1P and p2P will
    // point to the endpoints of the diagonal.   

    if ( tin == tin2.tin->n12 )
    {
      tin2.p1P  = &tin2.tin->p1;
      tin2.p2P  = &tin2.tin->p2;
      tin2.p3P  = &tin2.tin->p3;
      tin2.n12P = &tin2.tin->n12;
      tin2.n23P = &tin2.tin->n23;
      tin2.n31P = &tin2.tin->n31;
    }
    else if ( tin == tin2.tin->n23 )
    {
      tin2.p1P  = &tin2.tin->p2;
      tin2.p2P  = &tin2.tin->p3;
      tin2.p3P  = &tin2.tin->p1;
      tin2.n12P = &tin2.tin->n23;
      tin2.n23P = &tin2.tin->n31;
      tin2.n31P = &tin2.tin->n12;
    }
    else if ( tin == tin2.tin->n31 )
    {
      tin2.p1P  = &tin2.tin->p3;
      tin2.p2P  = &tin2.tin->p1;
      tin2.p3P  = &tin2.tin->p2;
      tin2.n12P = &tin2.tin->n31;
      tin2.n23P = &tin2.tin->n12;
      tin2.n31P = &tin2.tin->n23;
    }
    else
    {
        // This condition will only occur if the triangle data is not
        // self-consistent.  To avoid a crash, bypass further processing
        continue;
    }


    //  If p1P and p2P on tin1 and tin2 are coincident, swap p1P and p2P on
    //  tin2.  We want p3P on both triangles to point to the corners of the 
    //  polygon opposite the diagonal.  We want p1P and p2P on both triangles
    //  to point to the endpoints of the diagonal, but we want p1P and p2P of
    //  each triangle to be on opposite ends of the diagonal.  This will have
    //  all our pointers lined up so we can process and swap points without
    //  having to check for, and process a lot of different cases.

    length1 = mdlVec_distance ( &((*(tin1.p1P))->cor), &((*(tin2.p1P))->cor) );
    length2 = mdlVec_distance ( &((*(tin1.p1P))->cor), &((*(tin2.p2P))->cor) );

    if ( length1 < length2 )
    {
      struct CIVdtmpnt **tmpPntP;
      struct CIVdtmtin **tmpTinP;

      tmpPntP  = tin2.p1P;
      tin2.p1P = tin2.p2P;
      tin2.p2P = tmpPntP;

      tmpTinP   = tin2.n23P;
      tin2.n23P = tin2.n31P;
      tin2.n31P = tmpTinP;
    }


    // Store pointers to the triangle points that form the polygon.
    // Because we store tin1.p1P in [0] and tin1.p2P in [2], we know
    // the diagonal runs from [0] to [2].

    rectPnts[0] = *(tin1.p1P);
    rectPnts[1] = *(tin2.p3P);
    rectPnts[2] = *(tin1.p2P);
    rectPnts[3] = *(tin1.p3P);


    // Check the length of each possible diagonal of the triangle pair polygon
    // to see if a rectangle is formed.
 
    length1 = DISTANCE ( rectPnts[0]->cor.x, rectPnts[0]->cor.y, rectPnts[2]->cor.x, rectPnts[2]->cor.y );
    length2 = DISTANCE ( rectPnts[1]->cor.x, rectPnts[1]->cor.y, rectPnts[3]->cor.x, rectPnts[3]->cor.y );

    if ( fabs ( ( length1 - length2 ) ) < AEC_C_TOL3 && aecDTM_isRectangle ( rectPnts ) )  // If true, it is a rectangle.
    {

      //  Find the lowest corner of the rectangle, or the lower left in the
      //  case of an orthogonal rectangle.

      for ( j = 1, lowRectPntNdx = 0; j < 4; j++ )
      {
        if ( rectPnts[j]->cor.y < rectPnts[lowRectPntNdx]->cor.y ||
             ( rectPnts[j]->cor.y == rectPnts[lowRectPntNdx]->cor.y && rectPnts[j]->cor.x < rectPnts[lowRectPntNdx]->cor.x ) )
        {
          lowRectPntNdx = j;
        }
      }

      
      // If the lowest or lower left corner is not rectPnts[0] or rectPnts[2],
      // the diagonal runs between the wrong corners of the rectangle.  We have
      // some work to do to change it.

      if ( lowRectPntNdx != 0 && lowRectPntNdx != 2 )
      {
         struct CIVdtmtin *tmpN23;


         // If view triangles is turned on, erase tin1 and tin2.

         if ( srfP->dis.tinfnc )
         {
           (*srfP->dis.tinfnc)(srfP,tin1.tin,0,srfP->dis.tinsym);
           (*srfP->dis.tinfnc)(srfP,tin2.tin,0,srfP->dis.tinsym);
         }

         
         // Swap appropriate points and neigbors so that diagonal is
         // properly formed.

         *(tin1.p2P) = *(tin2.p3P);
         *(tin2.p2P) = *(tin1.p3P);

         *(tin1.n12P) = *(tin2.n23P);
         tmpN23 = *(tin1.n23P);
         *(tin1.n23P) = tin2.tin;

         if ( (*(tin1.n12P))->n12 == tin2.tin )
           (*(tin1.n12P))->n12 = tin1.tin;
         else if ( (*(tin1.n12P))->n23 == tin2.tin )
           (*(tin1.n12P))->n23 = tin1.tin;
         else if ( (*(tin1.n12P))->n31 == tin2.tin )
           (*(tin1.n12P))->n31 = tin1.tin;

         *(tin2.n12P) = tmpN23;
         *(tin2.n23P) = tin1.tin;

         if ( (*(tin2.n12P))->n12 == tin1.tin )
           (*(tin2.n12P))->n12 = tin2.tin;
         else if ( (*(tin2.n12P))->n23 == tin1.tin )
           (*(tin2.n12P))->n23 = tin2.tin;
         else if ( (*(tin2.n12P))->n31 == tin1.tin )
           (*(tin2.n12P))->n31 = tin2.tin;


         // If view triangles is turned on, redraw tin1 and tin2.

         if ( srfP->dis.tinfnc )
         {
           (*srfP->dis.tinfnc)(srfP,tin1.tin,1,srfP->dis.tinsym);
           (*srfP->dis.tinfnc)(srfP,tin2.tin,1,srfP->dis.tinsym);
         }
      }

   
      // Set the processed bit on both triangles to prevent 
      // unnecessary processing later on.

      aecDTM_setTriangleProcessedFlag ( tin1.tin );
      aecDTM_setTriangleProcessedFlag ( tin2.tin );

      bStop = TRUE;
    }
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

#ifdef NOTUSED
static BOOL aecDTM_triangulateGetErrorPoint
(
    struct CIVdtmpnt *dtmPntP
)
{
    if ( tinErrPntSet )
        memcpy ( dtmPntP, &tinErrPnt, sizeof ( struct CIVdtmpnt ) );

    return ( tinErrPntSet );    
}
#endif

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

static int aecDTM_nullInvalidateTinPointers
(
  void *tmp,
  long nPnts,
  DPoint3d *tinPnts,
  struct CIVdtmtin *tin,
  unsigned long
)
{
  struct CIVdtmsrf *srfP = (struct CIVdtmsrf *)tmp;

  if ( aecDTM_validateTinPtr ( NULL, srfP, tin->n12 ) != SUCCESS )
    tin->n12 = NULL;

  if ( aecDTM_validateTinPtr ( NULL, srfP, tin->n23 ) != SUCCESS )
    tin->n23 = NULL;

  if ( aecDTM_validateTinPtr ( NULL, srfP, tin->n31 ) != SUCCESS )
    tin->n31 = NULL;

  return SUCCESS;
}