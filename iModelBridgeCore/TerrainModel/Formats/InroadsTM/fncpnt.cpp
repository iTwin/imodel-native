//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* fncpnt.c                                        tmi    25-Apr-1990         */
/*----------------------------------------------------------------------------*/
/* Sends DTM points inside (or outside) fence to your function.               */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"



/*----------------------------------------------------------------------------*/
/* Private data structures                                                    */
/*----------------------------------------------------------------------------*/
struct CIVfncpnt_dat
{
  void *mdlDescP;
  struct AECclip *clp;
  struct AECjoin *seg;
  struct CIVdtmpnt *pnt;
  void (__cdecl *ins)(void);
  void (__cdecl *out)(void);
  void (__cdecl *usrfnc)(void*, int, long, DPoint3d*, CIVdtmpnt*);
  int typ;
  void *usrdat;
};



/*----------------------------------------------------------------------------*/
/* Private static function prototypes                                         */
/*----------------------------------------------------------------------------*/
static int aecDTM_sendAllPointsInPolygonProcess(void *,int,long,DPoint3d *,struct CIVdtmpnt *);
static int aecDTM_sendAllPointsInPolygonClip(void *,DPoint3d *,DPoint3d *);
static int aecDTM_sendAllPointsInPolygonSegment(void *,int,long,DPoint3d *);





/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_sendAllPointsInPolygon
 DESC: This function is used to gain access to the set of points which fall
       inside or outside the input polygon.  The function will pass each
       point to a function which you specify.  Random points are passed to
       your function one at a time.  For all other point types all, a
       connected linear feature is passed to your function.  Linear
       will be clipped at the fence boundary.  The format of your function
       is as follows:

       	  void usrfnc
          (
       	    void *dat;
       	    int typ;
       	    long np;
       	    DPoint3d *p;
       	    struct CIVdtmpnt *pnt;
          );

       Currently the following options may be used with the 'opt' argument:

          DTM_C_INSIDE    :    only outputs points that are inside poly.
          DTM_C_OUTSIDE   :    only outputs points outside polygon

       You can also control which point types are sent to your function.
       Do this using the 'typmsk' argument.  OR the values defined in
       civdtm.h for the types you want.  If 'typmsk' is zero, all types
       will be processed.
 HIST: Original - tmi 25-Apr-1990
 MISC:
 KEYW: DTM POINT SEND ALL PROCESS
-----------------------------------------------------------------------------%*/

int aecDTM_sendAllPointsInPolygon /* <= TRUE if error              */
(
  void *mdlDescP,                   /* => mdl app descriptor (or NULL)        */
  struct CIVdtmsrf *srfP,           /* => surface to use                      */
  int opt,                          /* => options                             */
  int typmsk,                       /* => point type (or zero for all)        */
  void (*usrfncP)(void *,int,long,  /* => your function                       */
    DPoint3d *,struct CIVdtmpnt *),
  void *usrdatP,                    /* => your user data                      */
  long nvrt,                        /* => # verts in polygon                  */
  DPoint3d *vrtP                    /* => polygon vertices                    */
)
{
  struct CIVfncpnt_dat dat;
  int sts = SUCCESS;

  dat.pnt = 0;
  dat.mdlDescP = mdlDescP;
  dat.usrdat = usrdatP;
  dat.usrfnc = usrfncP;
  dat.seg    = aecJoin_create ( (void *)0, aecDTM_sendAllPointsInPolygonSegment, (void *)&dat, 0 );
  dat.clp    = aecClip_create ( nvrt-1, vrtP, 0 );
  dat.ins    = opt & DTM_C_INSIDE  ? (void (*)())aecDTM_sendAllPointsInPolygonClip : (void (*)())0;
  dat.out    = opt & DTM_C_OUTSIDE ? (void (*)())aecDTM_sendAllPointsInPolygonClip : (void (*)())0;

  sts = aecDTM_sendAllPoints ( (void *)0, srfP, opt, typmsk, aecDTM_sendAllPointsInPolygonProcess, (void *)&dat );

  aecClip_free ( dat.clp );
  aecJoin_free ( dat.seg );
  aecJoin_cleanup ();

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_sendAllPointsInPolygonProcess
 DESC: As each point is processed, it is first sent here.
 HIST: Original - tmi 25-Apr-1990
 MISC: static
 KEYW: DTM POINTS SEND ALL POLYGON PROCESS
-----------------------------------------------------------------------------%*/

static int aecDTM_sendAllPointsInPolygonProcess
(
  void *vod,
  int typ,
  long np,
  DPoint3d *p,
  struct CIVdtmpnt *pnt
)
{
  struct CIVfncpnt_dat *dat = (struct CIVfncpnt_dat *)vod;
  DPoint3d tmp[2];

  dat->typ = typ;
  dat->pnt = pnt;
  aecJoin_setOption ( dat->seg, ( typ == DTM_C_DTMINT || typ == DTM_C_DTMEXT ) ? 1 : 0 );

  if ( np == 1 )
  {
    tmp[0] = tmp[1] = p[0];
    aecClip_string ( (void *)0, dat->clp, 2L, tmp, 0, (void *)dat, (void *)dat, dat->ins, dat->out );
  }
  else
    aecClip_string ( (void *)0, dat->clp, np, p, 0, (void *)dat, (void *)dat, dat->ins, dat->out );

  aecJoin_flush ( dat->seg );

  return ( SUCCESS );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_sendAllPointsInPolygonClip
 DESC: After a point has been checked to determine if it is inside or
       outside a fence, it comes here.
 HIST: Original - tmi 25-Apr-1990
 MISC: static
 KEYW: DTM POINTS SEND ALL POLYGON PROCESS CLIP
-----------------------------------------------------------------------------%*/

static int aecDTM_sendAllPointsInPolygonClip
(
  void *tmp,
  DPoint3d *p0,
  DPoint3d *p1
)
{
  struct CIVfncpnt_dat *dat = (struct CIVfncpnt_dat *)tmp;
  int sts = SUCCESS;

  if ( ! VEQUAL (p0[0], p1[0], AEC_C_TOL) )
     aecJoin_add ( dat->seg, p0, p1 );
  else  /*** if ( dat->typ == DTM_C_DTMREG )  (tmi 5/25/92) just in case linear feature with single point comes through       */
     (*dat->usrfnc)( dat->usrdat, dat->typ, 1L, p0, dat->pnt );

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_sendAllPointsInPolygonSegment
 DESC: When we get here, the point has been clipped and, for non-regular
       points, the segments have been rejoined.  We can now call the user's
       function.
 HIST: Original - tmi 25-Apr-1990
 MISC: static
 KEYW: DTM POINTS SEND ALL POLYGON PROCESS SEGMENT
-----------------------------------------------------------------------------%*/

static int aecDTM_sendAllPointsInPolygonSegment
(
  void *tmp,
  int,
  long np,
  DPoint3d *p
)
{
  struct CIVfncpnt_dat *dat = (struct CIVfncpnt_dat *)tmp;

  if ( dat->typ == DTM_C_DTMREG  ||  np > 1 )			 /* just in case linear feature with single point comes through       */
    (*dat->usrfnc)( dat->usrdat, dat->typ, np, p, dat->pnt );

  return ( SUCCESS );
}
