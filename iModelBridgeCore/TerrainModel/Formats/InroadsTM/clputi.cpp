//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* clputi.c                                           tmi    28-Feb-1994      */
/*----------------------------------------------------------------------------*/
/* Set of utility routines to provide clipping capabilities.		      */
/*----------------------------------------------------------------------------*/

#include "stdafx.h"


/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include <clpfnc.h>



/*----------------------------------------------------------------------------*/
/* Private data structures                                                    */
/*----------------------------------------------------------------------------*/

struct AECclipVertices
{
  DPoint3d v;
  struct AECclipVertices *flnk, *blnk, *vlnk;
};

struct AECclipPolygon
{
  DPoint3d area;
  struct AECclipVertices *vrts;
  size_t nvrt;
};

struct AECclipdat
{
  DPoint3d *nrm;
  struct AECclip *clp;
#if defined (mdl)  ||  defined (hp700)  ||  defined (sparc)
  void (*insfnc)();
  void (*outfnc)();
#else
  void (*insfnc)(void *,DPoint3d *,DPoint3d *);
  void (*outfnc)(void *,DPoint3d *,DPoint3d *);
#endif
  void *udat1, *udat2;
  void *inMdlDescP;
  void *outMdlDescP;
};



/*----------------------------------------------------------------------------*/
/* Private static function prototypes                                         */
/*----------------------------------------------------------------------------*/

static struct AECclip *aecClip_buildPlane(DPoint3d *,DPoint3d *);
static struct AECclipPolygon *newply(size_t,DPoint3d *);
static struct AECclip *bldply(struct AECclipPolygon *,int,int);
static struct AECclip *newpln(void);
static void   oknrm(DPoint3d *,struct AECclipPolygon *);
static void   plyare(DPoint3d *,struct AECclipPolygon *);
static struct AECclipPolygon *hulply(struct AECclipPolygon *);
static struct AECclipVertices *newvrt(void);
static void   aecClip_segment(void *,DPoint3d *,DPoint3d *);
static void   clpseg(int,DPoint3d *,DPoint3d *,void *,void *,DPoint3d *,DPoint3d *,
                     void (*)(void *,DPoint3d *,DPoint3d *),void *,
                     void (*)(void *,DPoint3d *,DPoint3d *),void *,int);
static void   frepln(struct AECclip *);
static void   freply(struct AECclipPolygon *);
static void   frevrt(struct AECclipVertices *);
static void   hulseg(struct AECclipPolygon *,struct AECclipVertices *,struct AECclipVertices *);
static void   xadd(struct AECclipPolygon *);
static void   xbck(struct AECclipVertices *);
static void   xfwd(struct AECclipVertices *);
static void   xrem(struct AECclipPolygon *);



/*%-----------------------------------------------------------------------------
 FUNC: aecClip_create
 DESC: This function takes a polygon and compiles it into a plane
       expression representing the projection of the polygon along its
       normal.  A pointer to the plane expression is returned.
 DESC: The "plane expression" above is actually a nested series of convex
       hulls. This is realized by the following algorithm:
       [1] If the clipper is not simple, it is broken down into pieces that
           are simple. This will almost always require shuffling the points
           and indices.
       [2] Then the convex hull of the simple clipper is found. Each segment
           on the hull may not be an actual segment on the clipper. In such
           cases the clipper will have a "nest (nst)" w/r to the hull segment.
           Then the convex hull of the nest is found and the process repeated.
           There could be nests within nests.
       [3] The process of finding the convex hull of a polygon is accomplished
           by finding (separately) the UPPER and LOWER hull, then combining.
 HIST: Original - tmi 12-Apr-1990
       Added comments [1],[2],[3] - mah 19-Apr-2005
 MISC:
 KEYW: CLIP CREATE
-----------------------------------------------------------------------------%*/

struct AECclip *aecClip_create /* <= NULL if error                 */
(
  size_t numVrts,                        /* => number of polygon pnts           */
  DPoint3d *vrtsP,                     /* => polygon coordinates              */
  int dontAllowParallelSides           /* => TRUE: no par. sides              */
)
{
  struct AECclip *r = (struct AECclip *)0;
  struct AECclipPolygon *p;

  if ( numVrts > 2 )
  {
    p = newply ( numVrts, vrtsP );
    xrem ( p );
    r = bldply ( p, 0, 0 );

    if( r )
    {
      r->prvPar = r->noPar;
      r->noPar = (byte) dontAllowParallelSides;
    }

    freply ( p );
  }

  return ( r );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecClip_free
 DESC: Frees the memory used to hold a single plane expression.
 HIST: Original - tmi 12-Apr-1990
 MISC:
 KEYW: CLIP FREE MEMORY
-----------------------------------------------------------------------------%*/

void aecClip_free
(
   struct AECclip *clpP                /* => clipping descriptor              */
)
{
  if (clpP)
  {
    clpP->noPar = clpP->prvPar;
    clpP->prvPar = 0;

    while ( clpP )
    {
      struct AECclip *nxt = clpP->nxt;

      if ( clpP->nst )
          aecClip_free ( clpP->nst );
      frepln ( clpP );
      clpP = nxt;
    }
  }
}



/*%-----------------------------------------------------------------------------
 FUNC: aecClip_string
 DESC: This function clips a line string with the plane expression given by
       the first argument 'clp'.

       The string is specified by 'nvrt' (the number of vertices), 'vrts'
       (an array of type 'DPoint3d'), and 'type' which is either 0,
       indicating an open string, or 1, indicating a closed string.

       The input string is cut into one or more resultant line segments
       which are sent to the two specified output functions.  'insfnc'
       points to the function which will be called with segments which are
       inside the volume defined by 'clp'.  'outfnc' points to the
       function which will be called with segments which are outside the
       volume defined by 'clp'. If either 'insfnc' or 'outfnc' is zero, the
       corresponding segments will be discarded.

       The output functions must be of the form:

           void fun ( void *dat, DPoint3d *v1, DPoint3d *v2 )

       The first parameter to the output functions is a pointer to user-
       define data passed through from 'udat1' or 'udat2'.  This provides a
       mechanism by which level, color or other attributes may be passed
       through.  The user data parameters can be zero if no such data is
       required.  If the user data gives enough information to determine
       what action is to be taken, 'insfnc' and 'outfnc' can be the same.
 HIST: Original - tmi 12-Apr-1990
 MISC:
 KEYW: CLIP STRING
-----------------------------------------------------------------------------%*/

void aecClip_string
(
  void *mdlDescP,                      /* => mdl desc. (or NULL)              */
  struct AECclip *clpP,                /* => clipping descriptor              */
  size_t numVrt,                         /* => # points in string               */
  DPoint3d *vrtsP,                     /* => coordinates of string            */
  int type,                            /* => 0: open, 1: closed               */
  void *insideUserDataP,               /* => user data for inside             */
  void *outsideUserDataP,              /* => user data for outside            */
  void (*insideFunctionP)(),           /* => function for inside              */
  void (*outsideFunctionP)()           /* => function for outside             */
)
{
  struct AECclipdat dat;

  dat.inMdlDescP = dat.outMdlDescP = mdlDescP;
  dat.udat1 = insideUserDataP;
  dat.udat2 = outsideUserDataP;
  dat.insfnc = (void (*)(void *,DPoint3d *,DPoint3d *))insideFunctionP;
  dat.outfnc = (void (*)(void *,DPoint3d *,DPoint3d *))outsideFunctionP;
  dat.clp = clpP;

  for ( size_t i = 1; i < numVrt; ++i )
    aecClip_segment ( (void *)&dat, &vrtsP[i-1], &vrtsP[i] );

  if ( type )
    aecClip_segment ( (void *)&dat, &vrtsP[numVrt-1], &vrtsP[0] );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecClip_copy
 DESC: Returns a copy of the input 'clp'.
 HIST: Original - tmi 12-Apr-1990
 MISC:
 KEYW: CLIP CREATE COPY
-----------------------------------------------------------------------------%*/

static struct AECclip *aecClip_copy /* <= copy of clip desc.              */
(
  struct AECclip *clpP                 /* => original clip. desc.             */
)
{
  struct AECclip *p2;

  if ( ! clpP )
    p2 = 0;
  else
  {
    p2 = newpln ();
    *p2 = *clpP;
    p2->nxt = aecClip_copy ( clpP->nxt );
    p2->nst = aecClip_copy ( clpP->nst );
  }

  return ( p2 );
}


/*%-----------------------------------------------------------------------------
 FUNC: aecClip_buildPlane
 DESC: Returns an expression for the volume on the positive side of the plane.
 HIST: Original - tmi 12-Apr-1990
 MISC: static
 KEYW: CLIP PLANE BUILD
-----------------------------------------------------------------------------%*/

static struct AECclip *aecClip_buildPlane
(
  DPoint3d *pnt,
  DPoint3d *nrm
)
{
  struct AECclip *r = newpln ();
  r->p = *pnt;
  r->nrm = *nrm;
  return ( r );
}



/*%-----------------------------------------------------------------------------
 FUNC: plyare
 DESC: Returns the magnitude of a polygon as the normal vector to the polygon.
 HIST: Original - tmi 12-Apr-1990
 MISC: static
 KEYW: CLIP POLYGON AREA COMPUTE
-----------------------------------------------------------------------------%*/

static void plyare
(
  DPoint3d *sumP,
  struct AECclipPolygon *p
)
{
  struct AECclipVertices *v;

  VIDENTITY ( sumP[0] );

  v = p->vrts;
  do
  {
    DPoint3d vt;
    struct AECclipVertices *nxt_v = v->flnk;
    VCROSS ( v->v, nxt_v->v, vt );
    VADD ( vt, sumP[0], sumP[0] );
    v = nxt_v;
  } while ( v != p->vrts );

  VSCALE ( sumP[0], .5, sumP[0] );

  return;
}



/*%-----------------------------------------------------------------------------
 FUNC: oknrm
 DESC: Returns vector normal to polygon -- even if the polygon has segments
       that cross.
 HIST: Original - tmi 12-Apr-1990
 MISC: static
 KEYW: CLIP POLYGON NORMAL COMPUTE
-----------------------------------------------------------------------------%*/

static void oknrm
(
  DPoint3d *sumP,
  struct AECclipPolygon *p
)
{
  struct AECclipVertices *v;
  double scale;

  VIDENTITY ( sumP[0] );

  for ( v = p->vrts->flnk; v != p->vrts; v = v->flnk )
  {
    DPoint3d vt, v1, v2;

    VSUB ( v->v, p->vrts->v, v1 );
    VSUB ( v->flnk->v, p->vrts->v, v2 );
    VCROSS ( v1, v2, vt );

    if ( VDOT ( sumP[0], vt ) < 0. )
    {
      VSUB ( sumP[0], vt, sumP[0] );
    }
    else
    {
      VADD ( sumP[0], vt, sumP[0] );
    }
  }

  scale = VLEN ( sumP[0] );
  if ( scale > 1e-30 )
  {
    scale = 1.0 / scale;
    VSCALE ( sumP[0], scale, sumP[0] );
  }

  return;
}



/*%-----------------------------------------------------------------------------
 FUNC: bldply
 DESC: Lower level function which helps compile a polygon into a plane expression.
 HIST: Original - tmi 12-Apr-1990
 MISC: static
 KEYW: CLIP POLYGON BUILD
-----------------------------------------------------------------------------%*/

static struct AECclip *bldply
(
  struct AECclipPolygon *p,
  int inhibit,
  int nestLevel
)
{
  struct AECclip *plst = 0;
  struct AECclipPolygon *hul = hulply (p);
  struct AECclipVertices *vrts = hul->vrts;
  struct AECclipVertices *v;

  if ( hul->nvrt < 3 )
  {
    return ( (struct AECclip *)0 );
  }

  v = vrts;
  do
  {
    struct AECclipVertices *nxt_v = v->flnk;

    if ( ! inhibit || v->vlnk != p->vrts )
    {
      struct AECclip *pp;
      DPoint3d vt, tmp;
      double len;

      VSUB ( nxt_v->v, v->v, vt );
      VCROSS ( p->area, vt, vt );
      if ( ( len = VLEN ( vt ) ) != 0. ) VSCALE ( vt, (1.0/len), vt );

      tmp = v->v;
      pp = aecClip_buildPlane ( &tmp, &vt );
      pp->nxt = plst;
      pp->nestLevel = (short)nestLevel;
      plst = pp;

      if ( v->vlnk->flnk == nxt_v->vlnk )
        pp->nst = 0;
      else
      {
        struct AECclipVertices *savflnkf = nxt_v->vlnk->flnk;
        struct AECclipVertices *savblnkf = v->vlnk->blnk;

        v->vlnk->blnk = nxt_v->vlnk;
        nxt_v->vlnk->flnk = v->vlnk;

        hul->vrts = nxt_v->vlnk;
        VNEG ( p->area, hul->area );
        pp->nst = bldply ( hul, 1, (int)(!nestLevel) );

        v->vlnk->blnk = savblnkf;
        nxt_v->vlnk->flnk = savflnkf;
      }
    }
    v = nxt_v;
  } while ( v != vrts );

  hul->vrts = vrts;
  freply ( hul );

  return ( plst );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecClip_segment
 DESC: Higher level function to clip a line segment with the plane expression.
 HIST: Original - tmi 12-Apr-1990
 MISC: static
 KEYW: CLIP SEGMENT PROCESS
-----------------------------------------------------------------------------%*/

static void aecClip_segment
(
  void *tdat,
  DPoint3d *v1,
  DPoint3d *v2
)
{
  struct AECclipdat *dat = (struct AECclipdat *)tdat;

  if ( ! dat->clp )
  {
    if ( dat->insfnc )
      (*dat->insfnc) ( dat->udat1, v1, v2 );
  }
  else if ( dat->clp->nst )
  {
    struct AECclipdat rdat, rdat2;

    rdat.clp	     = dat->clp->nst;
    rdat.udat1	     = dat->udat2;
    rdat.insfnc      = dat->outfnc;
    rdat.inMdlDescP  = dat->outMdlDescP;
    rdat.udat2	     = (void *)&rdat2;
    rdat.outfnc      = aecClip_segment;
    rdat.outMdlDescP = (void *)0;

    rdat2 = *dat;
    rdat2.clp = dat->clp->nxt;
    aecClip_segment ( (void *)&rdat, v1, v2 );
  }
  else
  {
    struct AECclipdat rdat2;

    rdat2 = *dat;
    rdat2.clp = dat->clp->nxt;
    clpseg ( dat->clp->nestLevel, v1, v2, (void *)&rdat2, dat->udat2, &dat->clp->nrm, &dat->clp->p, aecClip_segment, (void *)0, dat->outfnc, dat->outMdlDescP, dat->clp->noPar );
  }
}



/*%-----------------------------------------------------------------------------
 FUNC: clpseg
 DESC: Lower level function to clip a line segment with a plane expression.
 HIST: Original - tmi 12-Apr-1990
 MISC: static
 KEYW: CLIP SEGMENT PROCESS
-----------------------------------------------------------------------------%*/

static void clpseg
(
  int nestLevel,
  DPoint3d *p1,
  DPoint3d *p2,
  void *in_dat,
  void *out_dat,
  DPoint3d *nrm,
  DPoint3d *pnt,
  void (*insfnc)(void *,DPoint3d *,DPoint3d *),
  void *inMdlDescP,
  void (*outfnc)(void *,DPoint3d *,DPoint3d *),
  void *outMdlDescP,
  int noPar
)
{
  DPoint3d tmpP1 = *p1, tmpP2 = *p2;
  double plnzer = VDOT (*nrm, *pnt);
  double t1 = VDOT (*nrm, *p1);
  double t2 = VDOT (*nrm, *p2);

  t1 -= plnzer;
  t2 -= plnzer;

  if ( ISSMALL( t1, AEC_C_TOL1 ) ) t1 = 0.;        /*** changed from TOL2 to TOL1 tmi 5-28-92 ***/
  if ( ISSMALL( t2, AEC_C_TOL1 ) ) t2 = 0.;        /*** changed from TOL2 to TOL1 tmi 5-28-92 ***/

  if ( t1 == 0.  &&  t2 == 0. )
  {
    if ( ! noPar )
    {
      if ( nestLevel )
      {
        if ( outfnc )
          (*outfnc) ( out_dat, &tmpP1, &tmpP2 );
      }
      else
      {
        if ( insfnc )
          (*insfnc) ( in_dat, &tmpP1, &tmpP2 );
      }
    }
  }
  else if ( t1 >= 0.  &&  t2 >= 0. )
  {
    if ( insfnc )
      (*insfnc) (  in_dat, &tmpP1, &tmpP2 );
  }
  else if ( t1 <= 0.  &&  t2 <= 0. )
  {
    if ( outfnc )
      (*outfnc) ( out_dat, &tmpP1, &tmpP2 );
  }
  else
  {
    DPoint3d v;
    double t;

    double dNumer, dDenom;
    DPoint3d deltaP1Pnt, deltaP1P2;

    if ( t1 < 0. )
    {
      DPoint3d p = tmpP1;
      tmpP1 = tmpP2;
      tmpP2 = p;
    }


    VSUB ( *pnt, tmpP1, deltaP1Pnt );
    dNumer = VDOT ( *nrm, deltaP1Pnt );

    VSUB ( tmpP2, tmpP1, deltaP1P2 );
    dDenom = VDOT ( *nrm, deltaP1P2 );

    t = dNumer / dDenom;

    VSCALE ( deltaP1P2, t, deltaP1P2 );
    VADD ( tmpP1, deltaP1P2, v );

    if ( insfnc )
      (*insfnc) (  in_dat, &tmpP1, &v );

    if ( outfnc )
      (*outfnc) ( out_dat, &v, &tmpP2 );
  }
}



/*%-----------------------------------------------------------------------------
 FUNC: hulply
 DESC: Returns polygon which is the convex hull of the input polygon.
 HIST: Original - tmi 12-Apr-1990
 MISC: static
 KEYW: CLIP POLYGON HULL COMPUTE
-----------------------------------------------------------------------------%*/

static struct AECclipPolygon *hulply
(
  struct AECclipPolygon *p
)
{
  struct AECclipPolygon *np = (struct AECclipPolygon *) malloc (sizeof (*np));
  struct AECclipVertices *v, *min, *max, *miny, *maxy;

  v = min = max = miny = maxy = p->vrts;
  do
  {
    if ( v->v.x < min->v.x ) min = v;
    if ( v->v.x > max->v.x ) max = v;
    if ( v->v.y < miny->v.y ) miny = v;
    if ( v->v.y > maxy->v.y ) maxy = v;
    v = v->flnk;
  } while ( v != p->vrts );

  if ( min == max )
  {
    min = miny;
    max = maxy;
  }

  v = newvrt ();
  v->vlnk = min;
  v->v = min->v;
  v->flnk = v->blnk = newvrt ();
  v->flnk->vlnk = max;
  v->flnk->v = max->v;
  v->flnk->blnk = v;
  v->flnk->flnk = v;

  np->nvrt = 2;
  np->vrts = v;
  np->area = p->area;

  v = np->vrts->flnk;
  hulseg ( np, np->vrts, v );
  hulseg ( np, v, np->vrts );

  plyare ( &np->area, np );

  return ( np );
}



/*%-----------------------------------------------------------------------------
 FUNC: hulseg
 DESC: Lower level routine which helps compute the convex hull of a polygon
 HIST: Original - tmi 12-Apr-1990
 MISC: static
 KEYW: CLIP POLYGON HULL COMPUTE PROCESS
-----------------------------------------------------------------------------%*/

static void hulseg
(
  struct AECclipPolygon *p,
  struct AECclipVertices *v1,
  struct AECclipVertices *v2
)
{
  struct AECclipVertices *v, *maxv = 0;
  DPoint3d n;
  double maxdot = 0.;

  VSUB ( v2->v, v1->v, n );
  VCROSS ( n, p->area, n );

  for ( v = v1->vlnk->flnk; v != v2->vlnk; v = v->flnk )
  {
    DPoint3d tv;
    double dot;

    VSUB ( v->v, v1->v, tv );
    dot = VDOT ( n, tv );

    if ( dot >= maxdot )
    {
      maxdot = dot;
      maxv = v;
    }
  }

  if ( maxv )
  {
    ++p->nvrt;
    v = newvrt ();
    v->flnk = v2;
    v->blnk = v1;
    v1->flnk = v;
    v2->blnk = v;
    v->v = maxv->v;
    v->vlnk = maxv;
    hulseg ( p, v1, v );
    hulseg ( p, v, v2 );
   }
}



/*%-----------------------------------------------------------------------------
 FUNC: xadd
 DESC: Used to fix-up polygons which have segments that cross-over.
 HIST: Original - tmi 12-Apr-1990
 MISC: static
 KEYW: CLIP POLYGON CROSS CLEANUP ADD
-----------------------------------------------------------------------------%*/

static void xadd
(
  struct AECclipPolygon *p
)
{
  struct AECclipVertices *av, *clst = 0;
  DPoint3d nrm;

  oknrm ( &nrm, p );

  av = p->vrts;
  do
  {
    DPoint3d va, van;
    double van0;
    struct AECclipVertices *nxtav = av->flnk, *bv = nxtav;

    VSUB ( av->v, nxtav->v, va );
    VCROSS ( nrm, va, van );
    van0 = VDOT ( van, av->v );

    if ( bv != p->vrts ) bv = bv->flnk;

    while ( bv != p->vrts )
    {
      struct AECclipVertices *nxtbv = bv->flnk;
      double dotb0, dotb1;

      dotb0  = VDOT ( van, bv->v );
      dotb0 -= van0;
      dotb1  = VDOT ( van, nxtbv->v );
      dotb1 -= van0;
      if ( ISSMALL( dotb0, AEC_C_TOL2 ) ) dotb0 = 0.;
      if ( ISSMALL( dotb1, AEC_C_TOL2 ) ) dotb1 = 0.;

      if ( ( (dotb0 < 0.) != (dotb1 < 0.) )  &&  dotb0 != 0.  &&  dotb1 != 0. )
      {
	DPoint3d vb, vbn;
        double dota0, dota1, vbn0;

	VSUB ( nxtbv->v, bv->v, vb );
	VCROSS ( nrm, vb, vbn );
	vbn0 = VDOT ( vbn, bv->v );

	dota0  = VDOT ( vbn, av->v );
	dota0 -= vbn0;
	dota1  = VDOT ( vbn, nxtav->v );
	dota1 -= vbn0;
	if ( ISSMALL( dota0, AEC_C_TOL2 ) ) dota0 = 0.;
	if ( ISSMALL( dota1, AEC_C_TOL2 ) ) dota1 = 0.;

	if ( ( (dota0 < 0.) != (dota1 < 0.) )  &&  dota0 != 0.	&&  dota1 != 0. )
	{
      double t = dotb0 / (dotb0 - dotb1);
	  struct AECclipVertices *vpa = newvrt (), *vpb = newvrt ();

	  p->nvrt += 2;

	  VSCALE ( vb, t, vpb->v );
	  VADD ( bv->v, vpb->v, vpb->v );

      vpa->v = vpb->v;

	  vpb->vlnk = vpa;
	  vpa->vlnk = clst;
	  clst = vpb;

	  vpa->flnk  = nxtav;
	  vpa->blnk  = av;
	  vpb->flnk  = nxtbv;
	  vpb->blnk  = bv;
	  vpa->flnk->blnk = vpa;
	  vpa->blnk->flnk = vpa;
	  vpb->flnk->blnk = vpb;
	  vpb->blnk->flnk = vpb;

	  nxtav = vpa;
	  VSUB ( av->v, nxtav->v, va );
	}
      }
      bv = nxtbv;
    }
    av = nxtav;
  } while ( av != p->vrts );

  while ( clst )
  {
    struct AECclipVertices *t, *bv, *cv;

    bv = clst;
    cv = bv->vlnk;
    clst = cv->vlnk;

    t = cv->flnk;
    cv->flnk = bv->flnk;
    bv->flnk = t;
    cv->flnk->blnk = cv;
    bv->flnk->blnk = bv;
    cv->vlnk = bv;
  }
}



/*%-----------------------------------------------------------------------------
 FUNC: xbck
 DESC: Helps clean up cross-over polygons.
 HIST: Original - tmi 12-Apr-1990
 MISC: static
 KEYW: CLIP POLYGON CROSS CLEANUP BACKWARDS
-----------------------------------------------------------------------------%*/

static void xbck
(
  struct AECclipVertices *vlst
)
{
  struct AECclipVertices *v = vlst;

  do
  {
    struct AECclipVertices *nxt = v->blnk;
    v->blnk = v->flnk;
    v->flnk = nxt;
    v = nxt;
  } while ( v != vlst );

  do
  {
    struct AECclipVertices *nxt = v->flnk;
    if ( v->vlnk )
    {
      v->vlnk->vlnk = 0;
      xfwd ( v->vlnk );
      v->flnk = v->vlnk->flnk;
      v->vlnk->flnk = nxt;
      v->flnk->blnk = v;
      v->vlnk->flnk->blnk = v;
      v->vlnk = 0;
    }
    v = nxt;
  } while ( v != vlst );
}



/*%-----------------------------------------------------------------------------
 FUNC: xfwd
 DESC: Helps clean up cross-over polygons.
 HIST: Original - tmi 12-Apr-1990
 MISC: static
 KEYW: CLIP POLYGON CROSS CLEANUP FORWARD
-----------------------------------------------------------------------------%*/

static void xfwd
(
  struct AECclipVertices *vlst
)
{
  struct AECclipVertices *v = vlst;

  do
  {
    struct AECclipVertices *nxt = v->flnk;
    if ( v->vlnk )
    {
      v->vlnk->vlnk = 0;
      xbck ( v->vlnk );
      v->flnk = v->vlnk->flnk;
      v->vlnk->flnk = nxt;
      v->flnk->blnk = v;
      v->vlnk->flnk->blnk = v;
      v->vlnk = 0;
    }
    v = nxt;
  } while ( v != vlst );
}



/*%-----------------------------------------------------------------------------
 FUNC: xrem
 DESC: Helps clean up cross-over polygons.
 HIST: Original - tmi 12-Apr-1990
 MISC: static
 KEYW: CLIP POLYGON CROSS CLEANUP REMOVE
-----------------------------------------------------------------------------%*/

static void xrem
(
  struct AECclipPolygon *p
)
{
  xadd ( p );
  xfwd ( p->vrts );
  plyare ( &p->area, p );
}



/*%-----------------------------------------------------------------------------
 FUNC: newvrt
 DESC: It allocates memory for an array of vertices.
 HIST: Original - tmi 12-Apr-1990
 MISC: static
 KEYW: CLIP VERTICES MEMORY ALLOCATE
-----------------------------------------------------------------------------%*/

static struct AECclipVertices *newvrt
(
  void
)
{
  return ( (struct AECclipVertices *) malloc (sizeof (struct AECclipVertices)) );
}



/*%-----------------------------------------------------------------------------
 FUNC: newpln
 DESC: It allocates the memory for a new plane.
 HIST: Original - tmi 12-Apr-1990
 MISC: static
 KEYW: CLIP PLANE MEMORY ALLOCATE
-----------------------------------------------------------------------------%*/

static struct AECclip *newpln
(
  void
)
{
  struct AECclip *p = (struct AECclip *) malloc (sizeof (struct AECclip));
  p->nst = p->nxt = 0;
  p->noPar = p->prvPar = (byte)0;
  return ( p );
}



/*%-----------------------------------------------------------------------------
 FUNC: newply
 DESC: It allocates the memory for another polygon.
 HIST: Original - tmi 12-Apr-1990
 MISC: static
 KEYW: CLIP POLYGON MEMORY ALLOCATE
-----------------------------------------------------------------------------%*/

static struct AECclipPolygon *newply
(
  size_t nvrt,
  DPoint3d *vrts
)
{
  struct AECclipVertices *prv_v, *v;

  struct AECclipPolygon *p = (struct AECclipPolygon*) malloc (sizeof (struct AECclipPolygon));
  p->nvrt = nvrt;
  p->vrts = 0;

  for (size_t i = 0; i < nvrt; ++i)
  {
    v = newvrt ();
    v->v = vrts[i];
    v->blnk = p->vrts;
    v->vlnk = 0;
    p->vrts = v;
  }

  prv_v = p->vrts;
  if ( prv_v )
  {
    for ( v = prv_v->blnk; v; prv_v = v, v = prv_v->blnk )
      v->flnk = prv_v;

    p->vrts->flnk = prv_v;
    prv_v->blnk = p->vrts;

    plyare ( &p->area, p );
  }

  return ( p );
}



/*%-----------------------------------------------------------------------------
 FUNC: frevrt
 DESC: Its frees memory holding an array of vertices.
 HIST: Original - tmi 12-Apr-1990
 MISC: static
 KEYW: CLIP VERTICES MEMORY FREE DEALLOCATE
-----------------------------------------------------------------------------%*/

static void frevrt
(
  struct AECclipVertices *v
)
{
  if ( v != (struct AECclipVertices *)0 ) free ( (void *)v );
}



/*%-----------------------------------------------------------------------------
 FUNC: frepln
 DESC: It frees the memory holding a plane.
 HIST: Original - tmi 12-Apr-1990
 MISC: static
 KEYW: CLIP PLANE MEMORY FREE DEALLOCATE
-----------------------------------------------------------------------------%*/

static void frepln
(
  struct AECclip *p
)
{
  if ( p != (struct AECclip *)0 ) free ( (void *)p );
}



/*%-----------------------------------------------------------------------------
 FUNC: freply
 DESC: It frees the memory holding a polygon.
 HIST: Original - tmi 12-Apr-1990
 MISC: static
 KEYW: CLIP POLYGON MEMORY FREE DEALLOCATE
-----------------------------------------------------------------------------%*/

static void freply
(
  struct AECclipPolygon *p
)
{
  struct AECclipVertices *v;

  if ( p != (struct AECclipPolygon *)0 )
  {
    v = p->vrts;
    if ( v )
      do
      {
        struct AECclipVertices *nxt_v = v->flnk;
        frevrt ( v );
        v = nxt_v;
       } while ( v != p->vrts) ;
  
    free ( (void *)p );
  }
}





