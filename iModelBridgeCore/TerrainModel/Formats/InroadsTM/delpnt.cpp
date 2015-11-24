//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* delpnt.c                                         tmi    13-Mar-1991        */
/*----------------------------------------------------------------------------*/
/* It sets the delete flag for a point.                                       */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"


/*----------------------------------------------------------------------------*/
/* Private data structures                                                    */
/*----------------------------------------------------------------------------*/
struct CIVremdup_dat
{
  struct CIVdtmsrf *srf;
  CIVdtmftr *pntHshTblP;
};


/*----------------------------------------------------------------------------*/
/* Private static function prototypes                                         */
/*----------------------------------------------------------------------------*/
//static int aecDTM_removeDuplicatePointsProcess(void *,int,long,DPoint3d *,struct CIVdtmpnt *);
//static int aecDTM_hashPoint(const void *);
//static int aecDTM_comparePoints(const void *, const void *);


/*----------------------------------------------------------------------------*/
/* Constants and macros                                                       */
/*----------------------------------------------------------------------------*/
#define     CIVSTATUS(a,b,c,d) { double zzz = (double)(a+=b)/(double)c * 100.; aecStatus_show ( zzz, d ); }
#define     HASHSIZE 50021


/*----------------------------------------------------------------------------*/
/* Externals                                                                  */
/*----------------------------------------------------------------------------*/
static long npnt, cpnt;




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_deletePoint
 DESC: It deletes the input point.
 HIST: Original - tmi 13-Mar-1991
 MISC:
 KEYW: DTM POINT DELETE
-----------------------------------------------------------------------------%*/

int aecDTM_deletePoint /* <= TRUE if error                         */
(
  struct CIVdtmsrf *srfP,         /* => surface with point (or NULL)          */
  struct CIVdtmfil *inpfilP,      /* => file with point (or NULL)             */
  struct CIVdtmpnt *pntP          /* => point to delete                       */
)
{
  int sts = SUCCESS;

  if ( !aecDTM_isPointDeletedFlagSet ( pntP ) )
  {
    struct CIVdtmfil *filP;

    if ( inpfilP == (struct CIVdtmfil *)0 )
      sts = aecDTM_findPointFile ( &filP, srfP, pntP );
    else
      filP = inpfilP;

    aecDTM_setPointDeletedFlag ( pntP );
    filP->ndel++;
  }

  return ( sts );
}

#ifdef NOTUSED
/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_removeDuplicatePointsProcess
 DESC: Helps out the previous function.
 HIST: Original - tmi 22-May-1990
 MISC: static
 KEYW: DTM POINT DUPLICATE REMOVE DELETE PROCESS
-----------------------------------------------------------------------------%*/

static int aecDTM_removeDuplicatePointsProcess
(
  void *tmp,
  int,
  long np,
  DPoint3d *pnt,
  struct CIVdtmpnt *p
)
{
  struct CIVremdup_dat *dat = (struct CIVremdup_dat *)tmp;
  struct CIVdtmtin *t = (struct CIVdtmtin *)0;

  CIVSTATUS ( cpnt, np, npnt, DTM_M_MRKDUP );

  if ( !(p->flg & DTM_C_PNTTIN) )
    if ( aecDTM_findTriangle ( &t, 0, 0, 0, dat->srf, &pnt[0] ) == SUCCESS )
      if ( p != t->p1  &&  p != t->p2  &&  p != t->p3 )
        aecDTM_setPointDeletedFlag ( p );

  return ( SUCCESS );
}


/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_hashPoint
 DESC: Hashes a dtm point based on it's coordinate value.
 HIST: Original - twl 4-Oct-1999
 MISC: static
 KEYW: DTM POINT DUPLICATE REMOVE DELETE PROCESS NO TRIANGLES HASH
-----------------------------------------------------------------------------%*/

static int aecDTM_hashPoint
(
  const void *p
)
{
  struct CIVdtmpnt *pPnt = ( struct CIVdtmpnt * )p;
  int hashValue;
  int x = 0;
  int y = 0;

  if ( pPnt->cor.x > -50e6 && pPnt->cor.x < 50e6 &&
       pPnt->cor.y > -50e6 && pPnt->cor.y < 50e6 )
  {
      x = (int)pPnt->cor.x;
      y = (int)pPnt->cor.y;
      hashValue = x * 31 + y;
  }
  else
  {
      if ( pPnt->cor.x != 0.0 )
        x = (int)(pPnt->cor.x / 2.0);

      if ( pPnt->cor.y != 0.0 )
        y = (int)(pPnt->cor.y / 2.0);

      hashValue = x + y;
  }

  hashValue = abs ( hashValue );

  return ( hashValue % HASHSIZE );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_comparePoints
 DESC: Compares the coordinates of two DTM points.
 HIST: Original - twl 11-Dec-1998
 MISC: static
 KEYW: DTM COMPARE FEATURE HASH TABLE
-----------------------------------------------------------------------------%*/

static int aecDTM_comparePoints
(
  const void *p1,
  const void *p2
)
{
  struct CIVdtmpnt *pPnt1 = ( struct CIVdtmpnt * ) p1;
  struct CIVdtmpnt *pPnt2 = ( struct CIVdtmpnt * ) p2;

  if ( VEQUALXY ( pPnt1->cor, pPnt2->cor, AEC_C_TOL ) )
    return ( 0 );

  return ( memcmp ( &pPnt1->cor, &pPnt2->cor, sizeof ( DPoint3d ) ) );
}
#endif
