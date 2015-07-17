//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* jonuti.c                                        tmi    11-Apr-1990         */
/*----------------------------------------------------------------------------*/
/* Set of utilities to take a series of line segments and combine them	      */
/* into a single linestring.  The individual line segments may be input       */
/* in any order.							      */
/*----------------------------------------------------------------------------*/

#include "stdafx.h"

/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include <jonfnc.h>

#pragma intrinsic ( fabs )




/*----------------------------------------------------------------------------*/
/* Constants and macros                                                       */
/*----------------------------------------------------------------------------*/
#define PNTHSH(p,q,r,j)   { if ( j->opt & JON_C_SLOWMODE )                  \
                                q = doubleHashSprintf( (p).x, r );          \
                            else                                            \
                                q = doubleHash( (p).x, r ); }



/*----------------------------------------------------------------------------*/
/* Externals                                                                  */
/*----------------------------------------------------------------------------*/
static unsigned long _nBucketSize = JON_C_HSHSIZ;



/*----------------------------------------------------------------------------*/
/* Private static function prototypes                                         */
/*----------------------------------------------------------------------------*/
static int aecJoin_out                 /* <= TRUE if error                    */
(
  struct AECjoin *joinP,               /* => pointer to joining desc.         */
  int type,                            /* => 0: open, 1: closed               */
  struct AECjoinPoint *p               /* => array of joined points           */
);

static int aecJoin_checkSegment        /* <= TRUE if it can be added          */
(
  struct AECjoin *joinP,               /* => pointer to joining desc.         */
  DPoint3d *p0,                        /* => first point to add               */
  DPoint3d *p1                         /* => second point to add              */
);

static int doubleHash
(
  double x,
  unsigned long nBuckets
);

static int doubleHashSprintf
(
  double x,
  unsigned long nBuckets
);


/*%-----------------------------------------------------------------------------
 FUNC: aecJoin_create
 DESC: It is used to open up a buffer for a linestring.
 HIST: Original - tmi 11-Apr-1990
 MISC:
 KEYW: JOIN CREATE
-----------------------------------------------------------------------------%*/

struct AECjoin *aecJoin_create /* <= segment joiner descriptor    */
(
  void *mdlDescP,                      /* mdl desc. (or NULL)                */
  int (*userFunctionP)(void *,int,long,/* => func to call                    */
    DPoint3d *),
  void *userDataP,                     /* => pointer to user data            */
  int opt                              /* => options                         */
)
{
  struct AECjoin *buf;

  if ( ( buf = (struct AECjoin *) calloc ( (unsigned int)1, sizeof(struct AECjoin) ) ) != 0L )
  {
    if ( ( buf->hsh = (struct AECjoinPoint **) calloc ( (unsigned int)_nBucketSize, sizeof(struct AECjoinPoint *) ) ) == 0L )
    {
      free ( buf );
      buf = (struct AECjoin *)0;
    }
    else
    {
      buf->nBuckets = _nBucketSize;
      buf->mdlDescP = mdlDescP;
      buf->dat	= userDataP;
      buf->usrfnc = userFunctionP;
      buf->opt  = opt;
    }
  }

  return ( buf );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecJoin_flush
 DESC: When the user is all done adding line segments, this function should
       then be called.  It clears the buffers of any remaining open line-
       strings.
 HIST: Original - tmi 11-Apr-1990
 MISC:
 KEYW: JOIN FLUSH
-----------------------------------------------------------------------------%*/

int aecJoin_flush           /* <= TRUE if error                    */
(
  struct AECjoin *joinP                /* => pointer to joining desc.         */
)
{
  int sts = SUCCESS;
  unsigned long i;

  for ( i = 0; i < joinP->nBuckets  &&  sts == SUCCESS; ++i )
  {
    struct AECjoinPoint *hp = joinP->hsh[i];

    while ( hp )
    {
      struct AECjoinPoint *p, **pp;
      unsigned int hsh;
      unsigned long prvp, nxtp;

      joinP->hsh[i] = hp->hshlnk;

      for ( prvp = 0, p = hp; p->lnk != prvp; nxtp = prvp ^ p->lnk, prvp = (unsigned long)p, p = (struct AECjoinPoint *) nxtp );

      PNTHSH(p->p,hsh,joinP->nBuckets,joinP);
      for ( pp = &joinP->hsh[hsh]; *pp != p; pp= &(*pp)->hshlnk );

      *pp = p->hshlnk;
      sts = aecJoin_out ( joinP, JON_C_OPNSTR, p );
      hp = joinP->hsh[i];
    }
  }

  return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecJoin_free
 DESC: Call this function after calling aecJoin_flush. This one frees up
       memory associated with a single buffer.
 HIST: Original - tmi 11-Apr-1990
 MISC:
 KEYW: JOIN FREE
-----------------------------------------------------------------------------%*/

int aecJoin_free            /* <= TRUE if error                    */
(
  struct AECjoin *joinP                /* => pointer to joining desc.         */
)
{
  int sts = SUCCESS;

  if ( joinP != (struct AECjoin *)0 )
    if ( ( sts = aecJoin_flush ( joinP ) ) == SUCCESS )
    {
      if ( joinP->nvrt )
      {
        if ( joinP->vrtsP != (DPoint3d *)0 ) free ( joinP->vrtsP );
        joinP->nvrt = 0;
        joinP->vrtsP = (DPoint3d *)0;
      }

      while ( joinP->frepntP )
      {
        struct AECjoinPoint *p = joinP->frepntP;
        joinP->frepntP = joinP->frepntP->hshlnk;
        free ( p );
      }

      free ( joinP->hsh );
      free ( joinP );
    }

  return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecJoin_cleanup
 DESC: Call this function after aecJoin_close has been called for all buffers.
       This function frees up left over memory.
 HIST: Original - tmi 11-Apr-1990
 MISC:
 KEYW: JOIN EXIT
-----------------------------------------------------------------------------%*/

int aecJoin_cleanup         /* <= TRUE if error                    */
(
  void
)
{
  return ( SUCCESS );
}


/*%-----------------------------------------------------------------------------
 FUNC: aecJoin_out
 DESC: Calls the user-defined output function when a closed linestring is
       encountered or when the buffers are being flushed.
 HIST: Original - tmi 11-Apr-1990
 MISC: static
 KEYW: JOIN OUT
-----------------------------------------------------------------------------%*/

static int aecJoin_out                 /* <= TRUE if error                    */
(
  struct AECjoin *joinP,               /* => pointer to joining desc.         */
  int type,                            /* => 0: open, 1: closed               */
  struct AECjoinPoint *p               /* => array of joined points           */
)
{
  unsigned long prvp = 0, n = 0;
  int sts = SUCCESS;

  while (p)
  {
    unsigned long nxtp = prvp ^ p->lnk;

    if ( n >= (unsigned long)joinP->nvrt )
    {
      joinP->nvrt = ( joinP->nvrt + 1 ) * 2;
      if ( ! joinP->vrtsP )
	joinP->vrtsP = (DPoint3d *) malloc ( joinP->nvrt * sizeof (DPoint3d) );
      else
	joinP->vrtsP = (DPoint3d *) realloc ( (void *)joinP->vrtsP, joinP->nvrt * sizeof (DPoint3d) );
      if ( ! joinP->vrtsP )
	return ( ERROR );
    }

    joinP->vrtsP[n++] = p->p;
    p->hshlnk = joinP->frepntP;
    joinP->frepntP = p;

    prvp = (unsigned long) p;
    p = (struct AECjoinPoint *) nxtp;
  }

  if ( n > 2  &&  ( type == JON_C_CLSSTR  ||  joinP->opt & JON_C_CLSSTR ) )
  {
    if ( n >= (unsigned long)joinP->nvrt )
      if ( ( joinP->vrtsP = (DPoint3d *) realloc ( (void *)joinP->vrtsP, ++joinP->nvrt * sizeof (DPoint3d) ) ) == 0L )
	return ( sts = ERROR );
    joinP->vrtsP[n++] = joinP->vrtsP[0];
  }

  if ( joinP->usrfnc )
    sts = (*joinP->usrfnc) ( joinP->dat, type, n, joinP->vrtsP );

  return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecJoin_add
 DESC: Use this function to add line segments to a buffer.
 HIST: Original - tmi 11-Apr-1990
 MISC:
 KEYW: JOIN ADD
-----------------------------------------------------------------------------%*/

int aecJoin_add             /* <= TRUE if error                    */
(
  struct AECjoin *joinP,               /* => pointer to joining desc.         */
  DPoint3d *p0,                        /* => first point to add               */
  DPoint3d *p1                         /* => second point to add              */
)
{
  struct AECjoinPoint **ppa, **ppb;
  unsigned int hsh0, hsh1;
  int sts = SUCCESS;

  if ( !aecJoin_checkSegment ( joinP, p0, p1 ) )
    return ( sts );

  PNTHSH (p0[0],hsh0,joinP->nBuckets,joinP);
  PNTHSH (p1[0],hsh1,joinP->nBuckets,joinP);

  for ( ppa = &joinP->hsh[hsh0]; *ppa; ppa= &(*ppa)->hshlnk )
    if ( VEQUAL ((*ppa)->p, p0[0], AEC_C_TOL) )
      break;

  if ( *ppa )				       /* 1st point matches	   */
  {
    struct AECjoinPoint *pa = *ppa;

    *ppa = pa->hshlnk;

    for ( ppb = &joinP->hsh[hsh1]; *ppb; ppb = &(*ppb)->hshlnk )
      if ( VEQUAL ((*ppb)->p, p1[0], AEC_C_TOL) )
	break;

    if ( *ppb ) 			       /* 1st & 2nd point match    */
    {
      struct AECjoinPoint *pb = *ppb;
      struct AECjoinPoint *p;
      unsigned long prvp, nxtp;

      *ppb = pb->hshlnk;

#ifdef BUG
      if ( pa->lnk == (unsigned long)pb  &&  pb->lnk == (unsigned long)pa )                         /*** added 1-15-93 - tmi ***/
        if ( ( VEQUALXY ( pa->p, p0[0], AEC_C_TOL )  &&  VEQUALXY ( pb->p, p1[0], AEC_C_TOL ) ) ||  /*** keeps line on top of another line ***/
             ( VEQUALXY ( pa->p, p1[0], AEC_C_TOL )  &&  VEQUALXY ( pb->p, p0[0], AEC_C_TOL ) ) )   /*** from being flushed out ***/
          {                                                                                         /***      ...               ***/
            *ppa = pa;                                                                              /***      ...               ***/
            *ppb = pb;                                                                              /***      ...               ***/
            return ( sts );                                                                         /***      ...               ***/
          }                                                                                         /***      ...               ***/
#endif

      for ( prvp = 0, p = pa; p->lnk != prvp; nxtp = prvp ^ p->lnk, prvp = (unsigned long)p, p = (struct AECjoinPoint *) nxtp );

        if ( p == pb )
          sts = aecJoin_out ( joinP, JON_C_CLSSTR, pa );
        else
        {
	  pb->lnk ^= (unsigned long) pa;
	  pa->lnk ^= (unsigned long) pb;
        }
    }
    else				       /* 1st point only matches    */
    {
      struct AECjoinPoint *fp;

      if ( joinP->frepntP )
      {
	fp = joinP->frepntP;
	joinP->frepntP = joinP->frepntP->hshlnk;
      }
      else if ( ( fp = (struct AECjoinPoint *) malloc ( sizeof(struct AECjoinPoint) ) ) == 0L )
	return ( ERROR );

      fp->lnk  = (unsigned long) pa;
      pa->lnk ^= (unsigned long) fp;

      fp->p = p1[0];

      fp->hshlnk = joinP->hsh[hsh1];
      joinP->hsh[hsh1] = fp;
    }
  }
  else
  {
    for ( ppb = &joinP->hsh[hsh1]; *ppb; ppb = &(*ppb)->hshlnk )
      if ( VEQUAL ((*ppb)->p, p1[0], AEC_C_TOL) )
	break;

    if ( *ppb ) 			       /* 2nd point only matches    */
    {
      struct AECjoinPoint *fp, *p;

      if ( joinP->frepntP )
      {
	fp = joinP->frepntP;
	joinP->frepntP = joinP->frepntP->hshlnk;
      }
      else if ( ( fp = (struct AECjoinPoint *) malloc ( sizeof(struct AECjoinPoint) ) ) == 0L )
	return ( ERROR );

      p = *ppb;
      *ppb = p->hshlnk;

      p->lnk ^= (unsigned long) fp;
      fp->lnk = (unsigned long) p;

      fp->p = p0[0];
      fp->hshlnk = joinP->hsh[hsh0];
      joinP->hsh[hsh0] = fp;
    }
    else				       /* neither point matches    */
    {
      struct AECjoinPoint *pa, *pb;

      if ( joinP->frepntP )
      {
	pa = joinP->frepntP;
	joinP->frepntP = joinP->frepntP->hshlnk;
      }
      else if ( ( pa = (struct AECjoinPoint *) malloc ( sizeof(struct AECjoinPoint) ) ) == 0L )
	return ( ERROR );

      if ( joinP->frepntP )
      {
	pb = joinP->frepntP;
	joinP->frepntP = joinP->frepntP->hshlnk;
      }
      else if ( ( pb = (struct AECjoinPoint *) malloc ( sizeof(struct AECjoinPoint) ) ) == 0L )
	return ( ERROR );

      pa->lnk = (unsigned long) pb;
      pb->lnk = (unsigned long) pa;

      pa->p = p0[0];
      pa->hshlnk = joinP->hsh[hsh0];
      joinP->hsh[hsh0] = pa;

      pb->p = p1[0];
      pb->hshlnk = joinP->hsh[hsh1];
      joinP->hsh[hsh1] = pb;
    }
  }

  return ( sts	);
}



/*%-----------------------------------------------------------------------------
 FUNC: aecJoin_setOption
 DESC: Sets the option part of the join structure.  Normally not needed.
 HIST: Original - tmi 31-Dec-1992
 MISC:
 KEYW: JOIN OPTION SET
-----------------------------------------------------------------------------%*/

void aecJoin_setOption
(
  struct AECjoin *joinP,               /* => join descriptor                  */
  int opt                              /* => options to set                   */
)
{
  joinP->opt = opt;
}



/*%-----------------------------------------------------------------------------
 FUNC: aecJoin_checkSegment
 DESC: Checks the segment to see if it is ok to add.  It returns TRUE if
       it's ok to add segment.
 HIST: Original - tmi 14-Jan-1993
 MISC: static
 KEYW: JOIN SEGMENT CHECK
-----------------------------------------------------------------------------%*/

static int aecJoin_checkSegment        /* <= TRUE if it can be added          */
(
  struct AECjoin *joinP,               /* => pointer to joining desc.         */
  DPoint3d *p0,                        /* => first point to add               */
  DPoint3d *p1                         /* => second point to add              */
)
{
  int sts = 1;

  if ( VEQUAL ( p0[0], p1[0], AEC_C_TOL ) )
    sts = 0;
  else if ( joinP->opt & JON_C_NODUPLICATE )
  {
    struct AECjoinPoint *p, *nxtp, **hshlnk;
    unsigned long i;
    unsigned long prvlnk, nxtlnk;

    for ( i = 0; i < joinP->nBuckets; i++ )
      for ( hshlnk = &joinP->hsh[i]; *hshlnk; hshlnk = &(*hshlnk)->hshlnk )
      {
        prvlnk = 0;
        p = *hshlnk;

        while ( p )
        {
          nxtlnk = prvlnk ^ p->lnk;
          nxtp = (struct AECjoinPoint *) nxtlnk;

          if ( nxtp )
            if ( ( VEQUAL ( p0[0], p->p, AEC_C_TOL )  &&  VEQUAL ( p1[0], nxtp->p, AEC_C_TOL ) ) ||
                 ( VEQUAL ( p1[0], p->p, AEC_C_TOL )  &&  VEQUAL ( p0[0], nxtp->p, AEC_C_TOL ) ) )
              return ( 0 );

          prvlnk = (unsigned long) p;
          p = nxtp;
        }
      }
    sts = 1;
  }

  return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: doubleHash
 DESC: Hash function.
 HIST: Original - dgc  Oct - 1994
 MISC: static
 KEYW: JOIN SEGMENT CHECK
-----------------------------------------------------------------------------%*/

static int doubleHash
(
  double x,
  unsigned long nBuckets
)
{
  typedef union tagUDoubleIEEE
  {
    double dValue;
    struct tagSDoubleIEEE
    {
      unsigned long lowerMantissa;
      unsigned long upperMantissa : 20;
      unsigned long exponent : 11;
      unsigned long signBit : 1;
    } s;
  } DIEEE;

  DIEEE dVal;
  unsigned long lVal, lowerMantissa, upperMantissa;


  /* must transfer mantissa's out due to overflow problems */

  dVal.dValue = x;
  lowerMantissa = dVal.s.lowerMantissa;
  upperMantissa = dVal.s.upperMantissa;


  /* now compute hash value */

  lVal = ( lowerMantissa & 0xFFF00000 ) + upperMantissa;

  return ( lVal % nBuckets );
}


static int doubleHashSprintf
(
  double x,
  unsigned long nBuckets
)
{
  wchar_t text[64];
  long sum = 0;

  swprintf( text, L"%.4lf", x );
  size_t len = wcslen( text );

  for( size_t i = 0; i < len; i++ )
  {
    long factor = 1;

    switch( i % 2 )
    {
      case 0:
      factor = 10;
      break;

      case 1:
      factor = 1;
      break;
    }

    if( iswdigit( text[i] ) )
      sum += ( text[i] - L'0' ) * factor;
  }

  return( sum % nBuckets );
}




