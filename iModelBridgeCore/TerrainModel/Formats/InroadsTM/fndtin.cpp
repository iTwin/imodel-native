//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* fndtin.c					tmi	10-Apr-1990           */
/*----------------------------------------------------------------------------*/
/* Contains routines which find triangles.                                    */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"

extern boolean tinAltMoveMethod;

/*----------------------------------------------------------------------------*/
/* Private data structures                                                    */
/*----------------------------------------------------------------------------*/
struct CIVfindOutsideTinDat
{
  struct CIVdtmtin **tin;
  struct CIVdtmtin **prvtin;
  struct CIVdtmpnt *firrng;
  struct CIVdtmpnt *lstrng;
};

struct CIVfindTinDat
{
  struct CIVdtmtin *tinP;
  int count;
};

/*----------------------------------------------------------------------------*/
/* Private static function prototypes                                         */
/*----------------------------------------------------------------------------*/
static int aecDTM_findOutsideTriangleProcess(void *,long,DPoint3d *,struct CIVdtmtin *,unsigned long);
static int aecDTM_findOutsideTriangleProcessAgain(void *,long,DPoint3d *,struct CIVdtmtin *,unsigned long);
static int aecDTM_findCompareTinDat(const void *,const void *);
static int aecDTM_isPointInsideTriangleAltMethod(struct CIVdtmtin**, int*, int*, int*, DPoint3d*, DPoint3d*);


/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_findTriangle
 DESC: Given a planimetric location, this routine returns a pointer to the
       triangle in which the point lies. If no triangle is found, then a
       status of DTM_M_PNTNIN is returned.  Arguments are as follows:

       srf : (struct CIVsrf *) surface which we are searching
       cor : (DPoint3d) planimetric location (real world)
       tri : (struct CIVtin **) pointer to found triangle, if no
             triangle is found, then a value of zero is returned.
       rng : (int *) true if triangle is a range triangle
       rpt : (int *) if different than -1, then its value is
             the index of the vertice that the point is equal to.
       sid : (int *) if different than zero, then its value is
             the index of the side of the returned triangle on
	     which the point falls.

       If you pass in NULL pointers for any of tin, rng, rpt, or sid, they
       will not be computed.
 HIST: Original - tmi 10-Apr-1990
 MISC:
 KEYW: DTM FIND TRIANGLE LOCATION
-----------------------------------------------------------------------------%*/

int aecDTM_findTriangle   /* <=  TRUE if error                     */
(
  struct CIVdtmtin **tinPP,          /* <=> starting/found triangle           */
  int *rngP,                         /* <=  TRUE: range triangle              */
  int *rptP,                         /* <=  TRUE: input pnt is tin pnt        */
  int *sidP,                         /* <=  TRUE: input pnt on tin side       */
  struct CIVdtmsrf *srfP,            /*  => surface to use                    */
  DPoint3d *corP                     /*  => coordinates to use                */
)
{
  struct CIVdtmtin *triP;
  struct CIVdtmtin *prevtriP = NULL;
  struct CIVdtmtin *prev2triP = NULL;  // Previous, previous triangle.
  struct CIVdtmtin *firstTinP = NULL;
  struct CIVfindTinDat *tinsDatP = NULL;
  DPoint3d t[3];
  boolean bFirst = TRUE;
  int sts = SUCCESS, fnd = 0, trpt = -1, tsid = 0;
  long ctr = 0;
  long numTins = 0;
#if defined (DEBUG)
int  dbg = 0;
#endif

  triP = *tinPP;
  *tinPP = (struct CIVdtmtin *)0;
  if ( rptP ) *rptP = -1;
  if ( sidP ) *sidP = 0;
  if ( rngP ) *rngP = 0;

  if ( srfP == (struct CIVdtmsrf *)0 )
    return ( DTM_M_NOSRFS );
  else if ( triP == (struct CIVdtmtin *)0 )
    if ( ( sts = aecDTM_findFirstTriangle ( &triP, srfP ) ) != SUCCESS )
      return ( DTM_M_NOTINF );

  for ( ctr = 0, bFirst = TRUE; sts == SUCCESS && !fnd  &&  trpt == -1  &&  !tsid  &&  triP; ctr++ )
  {
    if ( bFirst )
    {
      firstTinP = triP;
      bFirst = FALSE;
    }
    else if ( triP == firstTinP )
    {
      sts = DTM_M_PNTNIN;

      // If anyone hits this assert statement, please let me know about it.
      // twl 8/13/2001
      assert ( sts == SUCCESS );
    }

    if ( sts == SUCCESS )
    {

      DTMTINTODPOINT ( srfP, triP, t );

      prev2triP = prevtriP;
      prevtriP = triP;

      aecDTM_isPointInsideTriangle ( &triP, &fnd, &trpt, &tsid, corP, t );

      if ( aecDTM_validateTinPtr ( NULL, srfP, triP) == SUCCESS )
      {
        struct CIVfindTinDat tinDat;
        struct CIVfindTinDat *tinDatP = NULL;

        memset ( &tinDat, 0, sizeof ( tinDat ) );
        tinDat.tinP = triP;
        tinDatP = (struct CIVfindTinDat *)bsearch( &tinDat, tinsDatP, numTins, sizeof ( struct CIVfindTinDat ), aecDTM_findCompareTinDat );

        if ( tinDatP )
        {
          if ( tinDatP->count > 0 )
            sts = DTM_M_PNTNIN;
          else
            tinDatP->count++;
        }
      }
      else
        sts = DTM_M_PNTNIN;

      if( ctr == 1000 )
      {
        ctr = 0;
        sts = aecInterrupt_check();

        if ( sts == SUCCESS )
        {
            if ( triP && prev2triP && triP == prev2triP )
                sts = DTM_M_PNTNIN;

            // If anyone hits this assert statement, please let me know about it.
            // twl 11/13/2001
            assert ( sts == SUCCESS );
        }

        if ( sts == SUCCESS )
        {
          struct CIVfindTinDat tinDat;

          memset ( &tinDat, 0, sizeof ( tinDat ) );
          tinDat.tinP = triP;
          aecTable_insert ( (void **)&tinsDatP, (int *)&numTins, (void *)&tinDat, sizeof ( struct CIVfindTinDat ) );
          qsort ( tinsDatP, numTins, sizeof ( struct CIVfindTinDat ), aecDTM_findCompareTinDat );
        }
      }
    }
  }

  if ( tinsDatP )
    free ( tinsDatP );

  if( sts == SUCCESS )
  {
    if ( fnd || trpt != -1 || tsid )
    {
      if ( tinPP ) *tinPP = triP;
      if ( rptP  ) *rptP  = trpt;
      if ( sidP  ) *sidP  = tsid;
      if ( rngP  &&  fnd  &&  aecDTM_isTriangleRangeTriangle ( srfP, triP ) ) *rngP = 1;
    }
    else
      sts = DTM_M_PNTNIN;
  }

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_isPointInsideTriangle
 DESC: Checks to determine if the point fell inside the input triangle.
 HIST: Original - tmi 26-Feb-1991
 MISC:
 KEYW: DTM FIND TRIANGLE INSIDE
-----------------------------------------------------------------------------%*/

int aecDTM_isPointInsideTriangle /* <= TRUE if point inside        */
(
  struct CIVdtmtin **triPP,          /* <=> starting/found triangle           */
  int *fndP,                         /* <=  TRUE: range triangle              */
  int *trptP,                        /* <=  TRUE: input pnt is tin pnt        */
  int *tsidP,                        /* <=  TRUE: input pnt on tin side       */
  DPoint3d *pntP,                    /* =>  point to use                      */
  DPoint3d *tP                       /* =>  triangle coordinates              */
)
{
  DPoint3d *altTinPntsP = NULL, *altPntP = NULL;
  DPoint3d a, b, c, acor, bcor;
  struct CIVdtmtin *nxttriP = (struct CIVdtmtin *)0;
  int i;
  double tmp;

  if ( tinAltMoveMethod )
  {
    DPoint3d moveOrigin;

    altTinPntsP = (DPoint3d *)alloca ( sizeof ( DPoint3d ) * 3 );
    altPntP = (DPoint3d *)alloca ( sizeof ( DPoint3d ) );

    memcpy ( altTinPntsP, tP, 3 * sizeof ( DPoint3d ) );
    memcpy ( altPntP, pntP, sizeof ( DPoint3d ) );
    memcpy ( &moveOrigin, pntP, sizeof ( DPoint3d ) );

    for ( i = 0; i < 3; i++ )
    {
        VSUB ( altTinPntsP[i], moveOrigin, altTinPntsP[i] );
    }

    VSUB ( *altPntP, moveOrigin, *altPntP );
    tP = altTinPntsP;
    pntP = altPntP;
  }

  memset (&a, 0, sizeof (a));
  memset (&b, 0, sizeof (b));

  for ( i = 1; i < 5; i++ )
  {
    switch ( i )
    {
      case ( 1 ) :  a = tP[0];  b = tP[1];  nxttriP = (*triPP)->n12;  break;
      case ( 2 ) :  a = tP[1];  b = tP[2];  nxttriP = (*triPP)->n23;  break;
      case ( 3 ) :  a = tP[2];  b = tP[0];  nxttriP = (*triPP)->n31;  break;
      case ( 4 ) :  *fndP = 1;  continue;
    }

    VSUBXY ( *pntP, a, acor );
    VSUBXY ( b, a, bcor );
    VCROSSXY ( acor, bcor, c );
    tmp = VDOTXY ( acor, bcor ) / VDOTXY ( bcor, bcor );
    VSCALEXY ( bcor, tmp, bcor );
    VSUBXY ( acor, bcor, bcor );
    tmp = VDOTXY ( bcor, bcor );

    if ( c.z < 0.  &&  tmp > AEC_C_TOL1SQR )
    {
      *triPP = nxttriP;
      break;
    }
    else if ( tmp <= AEC_C_TOL1SQR )
      if ( VEQUALXY ( a, *pntP, AEC_C_TOL1 ) )
        *trptP = i - 1;
      else if ( VEQUALXY ( b, *pntP, AEC_C_TOL1 ) )
        *trptP = (i == 3) ? 0 : i;
      else if ( VINSIDEXY ( a, b, *pntP, AEC_C_TOL1 ) )
        *tsidP = i;
  }

  return ( *fndP );
}





/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_findFirstTriangle
 DESC: Returns a pointer to the first triangle in the network. If there are
       no triangles, then a status of DTM_M_NOTRIS is returned.
 HIST: Original - tmi 10-Apr-1990
 MISC:
 KEYW: DTM FIND TRIANGLE FIRST
-----------------------------------------------------------------------------%*/

int aecDTM_findFirstTriangle /* <= TRUE if error                   */
(
  struct CIVdtmtin **tinPP,             /* <= found triangle                  */
  struct CIVdtmsrf *srfP                /* => surface to use                  */
)
{
  struct CIVdtmblk *blkP;
  struct CIVdtmtin *tP;

  *tinPP = 0;
  if (srfP && srfP->tinf && srfP->tinf->blk)
  {
      for ( blkP = srfP->tinf->blk; blkP; blkP = blkP->nxt )
        for ( tP = blkP->rec.tin; tP < blkP->rec.tin + blkP->use; tP++ )
          if ( !aecDTM_isTriangleRemovedFlagSet ( tP ) )
          {
            *tinPP = tP;
            return ( SUCCESS );
          }
  }

  return ( DTM_M_NOTINF );
}





/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_findOutsideTriangle
 DESC: It finds a triangle which is on the outside of the model (excluding
       any range or outside deleted triangles.)  I'm assumming that at
       least one non-deleted triangle is the neighbor of a range point
       triangle.
 HIST: Original - tmi 06-Mar-1991
 MISC:
 KEYW: DTM FIND TRIANGLE OUTSIDE
-----------------------------------------------------------------------------%*/

int aecDTM_findOutsideTriangle /* <= TRUE if error                 */
(
  struct CIVdtmtin **tinPP,            /* <= found triangle                   */
  struct CIVdtmtin **prvtinPP,         /* <= previous triangle                */
  struct CIVdtmsrf *srfP               /* => surface to use                   */
)
{
  struct CIVfindOutsideTinDat dat;

  *tinPP = *prvtinPP = (struct CIVdtmtin *)0;
  memset ( &dat, 0, sizeof(dat) );
  dat.firrng = srfP->rngf->blk->rec.pnt;
  dat.lstrng = srfP->rngf->blk->rec.pnt + srfP->rngf->blk->use - 1;
  dat.tin = tinPP;
  dat.prvtin = prvtinPP;

  /*
    First, look for a non-deleted triangle connected to a range point
  */

  aecDTM_sendAllTriangles ( (void *)0, srfP, DTM_C_NOBREK, aecDTM_findOutsideTriangleProcess, &dat );


  /*
    If we didn't find one, there is a chance that the maximum triangle
    parameter is masking all triangles connected to range points.  Therefore,
    we must perform another type of search for an outer edge triangle.
  */

  if ( *tinPP == (struct CIVdtmtin *)0 )
    aecDTM_sendAllTriangles ( (void *)0, srfP, DTM_C_NOBREK, aecDTM_findOutsideTriangleProcessAgain, &dat );

  return ( ( *tinPP == (struct CIVdtmtin *)0 ) ? DTM_M_NOTINF : SUCCESS );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_findOutsideTriangleProcess
 DESC: Helps out the previous function.
 HIST: Original - tmi 06-Mar-1991
 MISC: static
 KEYW: DTM FIND TRIANGLE OUTSIDE PROCESS
-----------------------------------------------------------------------------%*/

static int aecDTM_findOutsideTriangleProcess
(
  void *tmp,
  long,
  DPoint3d *,
  struct CIVdtmtin *t,
  unsigned long
)
{
  struct CIVfindOutsideTinDat *dat = (struct CIVfindOutsideTinDat *)tmp;
  struct CIVdtmtin *n = (struct CIVdtmtin *)0;
  int sts = SUCCESS, cnt;

  cnt = 0;
  if ( t->n12 )
  {
    n = t->n12;
    if ( n->p1 >= dat->firrng  &&  n->p1 <= dat->lstrng ) cnt++;
    if ( n->p2 >= dat->firrng  &&  n->p2 <= dat->lstrng ) cnt++;
    if ( n->p3 >= dat->firrng  &&  n->p3 <= dat->lstrng ) cnt++;
  }

  if ( cnt == 0  &&  t->n23 )
  {
    n = t->n23;
    if ( n->p1 >= dat->firrng  &&  n->p1 <= dat->lstrng ) cnt++;
    if ( n->p2 >= dat->firrng  &&  n->p2 <= dat->lstrng ) cnt++;
    if ( n->p3 >= dat->firrng  &&  n->p3 <= dat->lstrng ) cnt++;
  }

  if ( cnt == 0  &&  t->n31 )
  {
    n = t->n31;
    if ( n->p1 >= dat->firrng  &&  n->p1 <= dat->lstrng ) cnt++;
    if ( n->p2 >= dat->firrng  &&  n->p2 <= dat->lstrng ) cnt++;
    if ( n->p3 >= dat->firrng  &&  n->p3 <= dat->lstrng ) cnt++;
  }

  if ( cnt == 1 )
  {
    if      ( n == t->n12  &&  t->n23  &&  aecDTM_isTriangleDeletedFlagSet(t->n23) ) n = t->n23;
    else if ( n == t->n23  &&  t->n31  &&  aecDTM_isTriangleDeletedFlagSet(t->n31) ) n = t->n31;
    else if ( n == t->n31  &&  t->n12  &&  aecDTM_isTriangleDeletedFlagSet(t->n12) ) n = t->n12;

    *dat->tin = t;
    *dat->prvtin = n;
    sts = DTM_M_STPPRC;
  }

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_findOutsideTriangleProcessAgain
 DESC: Helps out the previous function.
 HIST: Original - tmi 06-Mar-1991
 MISC: static
 KEYW: DTM FIND TRIANGLE OUTSIDE PROCESS AGAIN
-----------------------------------------------------------------------------%*/

static int aecDTM_findOutsideTriangleProcessAgain
(
  void *tmp,
  long,
  DPoint3d *,
  struct CIVdtmtin *t,
  unsigned long 
)
{
  struct CIVdtmtin *n = (struct CIVdtmtin *)0;
  int sts = SUCCESS;

  if      ( t->n12  &&  !(t->flg & DTM_C_TINB12)  &&  aecDTM_isTriangleDeletedFlagSet(t->n12) )
    n = t->n12;
  else if ( t->n23  &&  !(t->flg & DTM_C_TINB23)  &&  aecDTM_isTriangleDeletedFlagSet(t->n23) )
    n = t->n23;
  else if ( t->n31  &&  !(t->flg & DTM_C_TINB31)  &&  aecDTM_isTriangleDeletedFlagSet(t->n31) )
    n = t->n31;

  if ( n != (struct CIVdtmtin *)0 )
  {
    struct CIVfindOutsideTinDat *dat = (struct CIVfindOutsideTinDat *)tmp;

    if      ( n == t->n12  &&  t->n23  &&  aecDTM_isTriangleDeletedFlagSet(t->n23) ) n = t->n23;
    else if ( n == t->n23  &&  t->n31  &&  aecDTM_isTriangleDeletedFlagSet(t->n31) ) n = t->n31;
    else if ( n == t->n31  &&  t->n12  &&  aecDTM_isTriangleDeletedFlagSet(t->n12) ) n = t->n12;

    *dat->tin = t;
    *dat->prvtin = n;
    sts = DTM_M_STPPRC;
  }

  return ( sts );
}





/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_findNonDeletedNeighborTriangle
 DESC: This logic checks to see if point falls on side of deleted triangle
       and the deleted triangle's neighbor is not deleted.  If so, then
       we send the non-deleted neighbor back.
 HIST: Original - tmi 05-Nov-1993
 MISC:
 KEYW: DTM FIND TRIANGLE NEIGHBOR NONDELETED
-----------------------------------------------------------------------------%*/

void aecDTM_findNonDeletedNeighborTriangle
(
  struct CIVdtmtin **inputTinPP,        /* <=> current/neighbor trian.        */
  int rpt,                              /*  => TRUE: repeat point             */
  int side                              /*  => TRUE: point falls onside       */
)
{
  if ( inputTinPP != (struct CIVdtmtin **)0  &&  *inputTinPP != (struct CIVdtmtin *)0 )
  {
    struct CIVdtmtin *tinP = *inputTinPP;

    if ( aecDTM_isTriangleDeletedFlagSet ( tinP ) )
    {
      if ( rpt != -1 )
      {
        struct CIVdtmpnt *ckptP;
        struct CIVdtmtin *orgtinP, *prvtinP = (struct CIVdtmtin *)0;
        int sts = SUCCESS;

        ckptP = *(&tinP->p1+rpt);

        orgtinP = prvtinP = tinP;
        sts = aecDTM_rotateAroundPoint ( NULL, &tinP, NULL, NULL, ckptP, prvtinP, 0 );

        while ( tinP != orgtinP  &&  sts == SUCCESS  &&  aecDTM_isTriangleDeletedFlagSet ( tinP ) )
        {
          prvtinP = tinP;
          sts = aecDTM_rotateAroundPoint ( NULL, &tinP, NULL, NULL, ckptP, prvtinP, 0 );
        }

        if ( sts != SUCCESS  ||  tinP == (struct CIVdtmtin *)0 ) tinP = orgtinP;
      }
      else if ( side )
      {
        struct CIVdtmtin *neiP = *(&tinP->n12 + side - 1 );
        if ( neiP && !aecDTM_isTriangleDeletedFlagSet ( neiP ) ) tinP = neiP;
      }
    }

    *inputTinPP = tinP;
  }
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_findCompareTinDat
 DESC: 
 HIST: Original - twl 20-May-2002
 MISC: static
 KEYW: 
-----------------------------------------------------------------------------%*/

static int aecDTM_findCompareTinDat
(
  const void *elm1,
  const void *elm2
)
{
  struct CIVfindTinDat *tinDat1P = (struct CIVfindTinDat *) elm1;
  struct CIVfindTinDat *tinDat2P = (struct CIVfindTinDat *) elm2;

  if ( tinDat1P->tinP < tinDat2P->tinP )
    return -1;
  
  if ( tinDat1P->tinP > tinDat2P->tinP )
    return 1;

  return 0;
}
