//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* patch.c                                            tmi    29-Oct-1990      */
/*----------------------------------------------------------------------------*/
/* Given a surface and a point in that surface, this function deletes         */
/* the point from the surface.  Then, if the surface contains                 */
/* triangles, then the existing triangles connect to the point are            */
/* deleted and the resulting whole is patched with a new set of               */
/* triangles which meet the Delaunay criteria.                                */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"

extern boolean tinAltMoveMethod;

/*----------------------------------------------------------------------------*/
/* Private static function prototypes                                         */
/*----------------------------------------------------------------------------*/
static int aecDTM_patchInit(long,long **,long **);
static int aecDTM_patchUpdateNeighbor(struct CIVdtmtin *,struct CIVdtmpnt *,struct CIVdtmtin *);
static int aecDTM_patchVerify(struct CIVdtmsrf *,struct CIVdtmpnt *,struct CIVdtmpnt *,struct CIVdtmpnt *,long,long *);
static int aecDTM_patchVerifyAltMethod(struct CIVdtmsrf *,struct CIVdtmpnt *,struct CIVdtmpnt *,struct CIVdtmpnt *,long,long *);
static int aecDTM_patchVerifyClip(void *,DPoint3d *,DPoint3d *);





/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_patch
 DESC: Given a surface and a point in that surface, this function deletes
       the point from the surface.  Then, if the surface contains
       triangles, then the existing triangles connect to the point are
       deleted and the resulting whole is patched with a new set of
       triangles which meet the Delaunay criteria.
 HIST: Original - tmi 29-Oct-1990
 MISC:
 KEYW: DTM PATCH HOLE
-----------------------------------------------------------------------------%*/

int aecDTM_patch            /* <= TRUE if error                    */
(
  struct CIVdtmsrf *srfP,              /* => surface to use                   */
  long npntlst,                        /* => # of points in hole perimeter    */
  long *pntlstP,                       /* => list of points                   */
  long *nbrlstP,                       /* => list of neighboring triangles    */
  long *nnewlstP,                      /* => # of new triangles in hole       */
  long **newlstPP                      /* => list of new triangle in hole     */
)
{
  int sts = SUCCESS;

  *nnewlstP = 0L;

  aecDTM_patchCheck ( &npntlst, pntlstP, nbrlstP );
  if ( ( sts = aecDTM_patchHole ( srfP, npntlst, pntlstP, nbrlstP, nnewlstP, newlstPP ) ) == SUCCESS )
    sts = aecDTM_patchDelaunay ( srfP, npntlst, pntlstP, *nnewlstP, *newlstPP );

  return ( sts );
}





/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_patchHole
 DESC: It fills in holes in a terrain model created by deleting a point
       and all triangles connected to that point.  It should handle cases
       where the resultant polygon to be filled in is either convex or
       concave.
 HIST: Original - tmi 29-Oct-1990
 MISC:
 KEYW: DTM PATCH HOLE
-----------------------------------------------------------------------------%*/

int aecDTM_patchHole        /* <= TRUE if error                    */
(
  struct CIVdtmsrf *srfP,              /* => surface to use                   */
  long npnt,                           /* => # of points in hole perimeter    */
  long *pntlstP,                       /* => list of points                   */
  long *neilstP,                       /* => list of neighboring triangles    */
  long *nnewlstP,                      /* => # of new triangles in hole       */
  long **newlstPP                      /* => list of new triangle in hole     */
)
{
  struct CIVdtmpnt *p1P, *p2P, *p3P;
  struct CIVdtmtin *tinP, *n1P, *n2P, *n3P = (struct CIVdtmtin *)0;
  int sts = SUCCESS;
  long i, j, k, *pntstkP, *neistkP, loopCnt = 0;
  short bExitWhile = FALSE;


  // When npnt is less than 2 aecDTM_patchInit allocates blocks of memory
  // that zero bytes in size or -4 bytes in size.  This is causing
  // MicroStation exception errors.  07-08-98  twl.

  if ( npnt < 3 )
    return ( sts );

  sts = aecDTM_patchInit ( npnt, &pntstkP, &neistkP );
  memcpy ( pntstkP, pntlstP, (unsigned int)npnt*sizeof(long) );
  memcpy ( neistkP, neilstP, (unsigned int)(npnt-1)*sizeof(long) );

  while ( npnt > 2  &&  sts == SUCCESS && !bExitWhile )
  {
    for ( i = j = k = 0; j < (npnt-k-1)/2  &&  sts == SUCCESS; j++, i = j*2 )
    {
      p1P = (struct CIVdtmpnt *) pntstkP[i+k];
      p2P = (struct CIVdtmpnt *) pntstkP[i+k+1];
      p3P = (struct CIVdtmpnt *) pntstkP[i+k+2];
      if ( p1P == p2P || p1P == p3P || p2P == p3P )
      {
        bExitWhile = TRUE;
        sts = 0;
        break;
      }

      while ( j < (npnt-k-1)/2  &&  aecDTM_patchVerify ( srfP, p1P, p2P, p3P, npnt-1, pntstkP ) )
      {
        if ( loopCnt > 1000 && j == 0 && j >= (npnt-k-2)/2 )
            break;

        neistkP[j+k] = neistkP[i+k];
        pntstkP[j+k] = (long) p1P;
        k++;
        p1P = (struct CIVdtmpnt *) pntstkP[i+k];
        p2P = (struct CIVdtmpnt *) pntstkP[i+k+1];
        p3P = (struct CIVdtmpnt *) pntstkP[i+k+2];
      }
      if ( j >= (npnt-k-1)/2 ) break;

      n1P = (struct CIVdtmtin *) neistkP[i+k];
      n2P = (struct CIVdtmtin *) neistkP[i+k+1];

      if ( j < (npnt-1)/2 )
        if ( ( sts = aecDTM_addTriangle ( &tinP, srfP, p1P, p2P, p3P, n1P, n2P, n3P ) ) == SUCCESS )
        {
          aecDTM_patchUpdateNeighbor ( n1P, p1P, tinP );
          aecDTM_patchUpdateNeighbor ( n2P, p2P, tinP );

          pntstkP[j+k] = (long) p1P;
          neistkP[j+k] = (long) tinP;
          sts = aecDTM_triangleStack ( nnewlstP, newlstPP, tinP );
          if ( srfP->dis.tinfnc ) (*srfP->dis.tinfnc) ( srfP, tinP, 1, srfP->dis.tinsym );
          if ( srfP->dis.confnc ) (*srfP->dis.confnc) ( srfP, tinP, 1, srfP->dis.consym );
        }
    }
    if ( !bExitWhile )
    {
      pntstkP[j+k] = pntstkP[i+k];

      if ( (npnt+k) % 2 == 0 )
      {
        neistkP[j+k] = neistkP[i+k];
        pntstkP[j+k+1] = pntstkP[i+k+1];
      }

      npnt = ( npnt + k + 1 ) / 2 + ( npnt + k + 1 ) % 2;

      if( sts == SUCCESS )
        sts = aecInterrupt_check();

      loopCnt++;
    }
  }

  if ( sts == SUCCESS )
  {
    p1P = (struct CIVdtmpnt *) pntstkP[0];
    p2P = (struct CIVdtmpnt *) pntstkP[1];
    n1P = (struct CIVdtmtin *) neistkP[0];
    n2P = (struct CIVdtmtin *) neistkP[1];

    aecDTM_patchUpdateNeighbor ( n1P, p1P, n2P );
    aecDTM_patchUpdateNeighbor ( n2P, p2P, n1P );
  }

  free ( pntstkP );
  free ( neistkP );

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_patchInit
 DESC: Initializes the data structures used for this command.
 HIST: Original - tmi 29-Oct-1990
 MISC: static
 KEYW: DTM PATCH HOLE INITIALIZE
-----------------------------------------------------------------------------%*/

static int aecDTM_patchInit
(
  long npnt,
  long **pntstk,
  long **neistk
)
{
  long allocSize = npnt + 2;
  int sts = SUCCESS;

  if ( ( *pntstk = (long *) malloc ( (unsigned int)allocSize*sizeof(long) ) ) == (long *)0 )
    sts = DTM_M_MEMALF;
  else if ( ( *neistk = (long *) malloc ( (unsigned int)(allocSize-1)*sizeof(long) ) ) == (long *)0 )
    sts = DTM_M_MEMALF;

  return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_patchUpdateNeighbor
 DESC: Updates the neigboring triangle to have the correct neighbor.
 HIST: Original - tmi 29-Oct-1990
 MISC: static
 KEYW: DTM PATCH HOLE NEIGHBOR UPDATE
-----------------------------------------------------------------------------%*/

static int aecDTM_patchUpdateNeighbor
(
  struct CIVdtmtin *nei,
  struct CIVdtmpnt *pnt,
  struct CIVdtmtin *tin
)
{
  int sts = SUCCESS, pntIndex;

  if ( nei != (struct CIVdtmtin *)0  &&  nei != (struct CIVdtmtin *)-1 )
    if ( ( sts = aecDTM_getIndexOfTrianglePointByPointer ( &pntIndex, pnt, nei ) ) == SUCCESS )
    {
      pntIndex = ( pntIndex == 0 ) ? 2 : pntIndex - 1;
      *(&nei->n12+pntIndex) = tin;
      if ( nei->flg & (DTM_C_TINB12 << pntIndex) )
        if ( ( sts = aecDTM_getIndexOfTrianglePointByPointer ( &pntIndex, pnt, tin ) ) == SUCCESS )
          tin->flg |= DTM_C_TINB12 << pntIndex;
    }

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_patchDelaunay
 DESC: Makes sure the triangles in and around the patched whole meet the
       Delaunay criteria.
 HIST: Original - tmi 20-Oct-1990
 MISC:
 KEYW: DTM PATCH HOLE DELAUNAY
-----------------------------------------------------------------------------%*/

int aecDTM_patchDelaunay    /* <= TRUE if error                    */
(
  struct CIVdtmsrf *srfP,              /* => surface to use                   */
  long npntlst,                        /* => # points in perimter polygon     */
  long *pntlstP,                       /* => list of ptrs to perimeter pnts   */
  long nnewlst,                        /* => # triangles in new list          */
  long *newlstP                        /* => list of new triangles ptrs       */
)
{
  struct CIVdtmpnt *pntP;
  struct CIVdtmtin *tinP;
  int sts = SUCCESS;
  long i, j;

  for ( i = 0; i < npntlst  &&  sts == SUCCESS; i++ )
    for ( pntP = (struct CIVdtmpnt *)pntlstP[i], j = 0; j < nnewlst  &&  sts == SUCCESS; j++ )
    {
      tinP = (struct CIVdtmtin *) newlstP[j];
      if ( pntP == tinP->p1  ||  pntP == tinP->p2  ||  pntP == tinP->p3 )
        sts = aecDTM_delaunayTriangle ( srfP, pntP, tinP, (long *)0, (long **)0 );
    }

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_patchVerify
 DESC: Verifies that the triangle formed from the input three points is
       a valid triangle in that no other point in the point stack falls
       within or on the edge of the current triangle.  It returns true if
       the triangle cannot be formed, otherwise it returns false if the
       triangle is valid.
 HIST: Original - tmi 30-Oct-1990
 MISC: static
 KEYW: DTM PATCH HOLE VERIFY
-----------------------------------------------------------------------------%*/

struct CIVpatch_dat { int tru; };

static int aecDTM_patchVerify
(
  struct CIVdtmsrf *,
  struct CIVdtmpnt *p1,
  struct CIVdtmpnt *p2,
  struct CIVdtmpnt *p3,
  long npntstk,
  long *pntstk
)
{
  struct CIVdtmpnt *pnt;
  struct CIVpatch_dat dat;
  DPoint3d p[3], tmp[2];
  struct AECclip *clp = (struct AECclip *)0;
  long i;
  DPoint3d moveOrigin;

  memset ( &moveOrigin, 0, sizeof ( moveOrigin ) );
  dat.tru = 0;

  DTMPOINTTODPOINT ( srf, p1, p[0] );
  DTMPOINTTODPOINT ( srf, p2, p[1] );
  DTMPOINTTODPOINT ( srf, p3, p[2] );

  if ( tinAltMoveMethod )
  {
    memcpy ( &moveOrigin, &p[0], sizeof ( moveOrigin ) );
    VSUB ( p[0], moveOrigin, p[0] );
    VSUB ( p[1], moveOrigin, p[1] );
    VSUB ( p[2], moveOrigin, p[2] );
  }

  VSUBXY ( p[1], p[0], tmp[0] );
  VSUBXY ( p[2], p[0], tmp[1] );
  VCROSSXY ( tmp[1], tmp[0], tmp[0] );

  if ( tmp[0].z <= AEC_C_TOL1 )
    dat.tru = 1;
  else
  {
    p[0].z = p[1].z = p[2].z = 0.;
    if ( ( clp = aecClip_create ( 3L, p, 0 ) ) != (struct AECclip *)0 )
    {
      for ( i = 0; i < npntstk  &&  dat.tru == 0; i++ )
      {
        pnt = (struct CIVdtmpnt *) pntstk[i];
        if ( pnt != p1  &&  pnt != p2  &&  pnt != p3 )
        {
          DTMPOINTTODPOINT ( srf, pnt, p[0] );

          if ( tinAltMoveMethod )
            VSUB ( p[0], moveOrigin, p[0] );

          p[1] = p[0];
          aecClip_string ( (void *)0, clp, 2L, p, 0, &dat, (void *)0, (void (*)())aecDTM_patchVerifyClip, (void (*)())0 );
        }
      }
    }

    if ( clp ) aecClip_free ( clp );
  }

  return ( dat.tru );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_patchVerifyClip
 DESC: Helps out the previous function.
 HIST: Original - tmi 30-Oct-1990
 MISC: static
 KEYW: DTM PATCH HOLE VERIFY CLIP
-----------------------------------------------------------------------------%*/

static int aecDTM_patchVerifyClip
(
  void *tmp,
  DPoint3d *,
  DPoint3d *
)
{
  struct CIVpatch_dat *dat = (struct CIVpatch_dat *) tmp;
  dat->tru = 1;
  return ( SUCCESS );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_patchCheck
 DESC: Checks to ensure that there are no points in the input polygon that
       project out into the middle of the polygon as a single line.
 HIST: Original - tmi 03-Aug-1991
 MISC:
 KEYW: DTM PATCH HOLE CHECK
-----------------------------------------------------------------------------%*/

void aecDTM_patchCheck
(
  long *npntlstP,                    /* => # points in list                   */
  long *pntlstP,                     /* => point list                         */
  long *nbrlstP                      /* => neighbor triangle list             */
)
{
  long i, tmp, fnd = 0;

  for ( i = 0; i < *npntlstP - 2; i++ )
    if ( pntlstP[i] == pntlstP[i+2] )
    {
      *npntlstP -= 2;
      tmp = pntlstP[i+1];
      memcpy ( &pntlstP[i], &pntlstP[i+2], (unsigned int)(*npntlstP-i)*sizeof(long) );
      memcpy ( &nbrlstP[i], &nbrlstP[i+2], (unsigned int)(*npntlstP-i-1)*sizeof(long) );
      pntlstP[*npntlstP] = tmp;
      i -= 2;
      fnd = 1;
    }

  if ( *npntlstP > 3 && pntlstP[1] == pntlstP[*npntlstP-2] )
  {
    *npntlstP -= 2;
    tmp = pntlstP[0];
    memcpy ( &pntlstP[0], &pntlstP[1], (unsigned int)(*npntlstP)*sizeof(long) );
    memcpy ( &nbrlstP[0], &nbrlstP[1], (unsigned int)(*npntlstP-1)*sizeof(long) );
    pntlstP[*npntlstP] = tmp;
    fnd = 1;
  }

  if ( fnd ) aecDTM_patchCheck ( npntlstP, pntlstP, nbrlstP );
}
