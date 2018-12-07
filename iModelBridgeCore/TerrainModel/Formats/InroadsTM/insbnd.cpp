//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* insbnd.c                                         tmi    31-Oct-1990        */
/*----------------------------------------------------------------------------*/
/* This function returns true if the input point is either inside an          */
/* interior boundary or outside of the exterior boundary for the              */
/* input surface.                                                             */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"



/*----------------------------------------------------------------------------*/
/* Private data structures                                                    */
/*----------------------------------------------------------------------------*/
struct CIVinsbnd_dat
{
  DPoint3d pnt;
  int checkingInterior;
  int tru;
};




/*----------------------------------------------------------------------------*/
/* Private static function prototypes                                         */
/*----------------------------------------------------------------------------*/
static int aecDTM_isPointInsideBoundaryProcess(void *,int,long,DPoint3d *,struct CIVdtmpnt *);





/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_isPointInsideBoundary
 DESC: This function returns true if the input point is either inside an
       interior boundary or outside of the exterior boundary for the
       input surface.
 HIST: Original - tmi 31-Oct-1990
 MISC:
 KEYW: DTM POINT BOUNDARY INSIDE
-----------------------------------------------------------------------------%*/

int aecDTM_isPointInsideBoundary /* <= TRUE if point is ins.       */
(
  struct CIVdtmsrf *srfP,              /* => surface to use                   */
  struct CIVdtmpnt *pntP               /* => point to use                     */
)
{
  struct CIVinsbnd_dat dat;

  memset ( &dat, 0, sizeof(struct CIVinsbnd_dat) );
  DTMPOINTTODPOINT ( srfP, pntP, dat.pnt );

  dat.checkingInterior = TRUE;
  aecDTM_sendAllPoints ( (void *)0, srfP, (int)DTM_C_NOBREK|DTM_C_NOCONS, (int)DTM_C_INTMSK, aecDTM_isPointInsideBoundaryProcess, &dat );

  if ( !dat.tru )
  {
    dat.checkingInterior = FALSE;
    aecDTM_sendAllPoints ( (void *)0, srfP, (int)DTM_C_NOBREK|DTM_C_NOCONS, (int)DTM_C_EXTMSK, aecDTM_isPointInsideBoundaryProcess, &dat );
  }

  return ( dat.tru );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_isPointInsideBoundaryProcess
 DESC: It checks to see if the input point was inside any interior
       boundaries or outide of the exterior boundary.  If it is, then the
       triangles connect to it are marked as deleted.
 HIST: Original - tmi 26-Oct-1990
 MISC: static
 KEYW: DTM POINT BOUNDARY INSIDE PROCESS
-----------------------------------------------------------------------------%*/

static int aecDTM_isPointInsideBoundaryProcess
(
  void *tmp,
  int,
  long np,
  DPoint3d *p,
  struct CIVdtmpnt *
)
{
  struct CIVinsbnd_dat *dat = (struct CIVinsbnd_dat *) tmp;
  int sts = SUCCESS, insideBox;

  if ( dat->checkingInterior )
  {
    DPoint3d box[5];

    aecPolygon_computeRange ( &box[0], &box[2], np, p );
    box[1].x = box[2].x;  box[1].y = box[0].y;
    box[3].x = box[0].x;  box[3].y = box[2].y;
    box[4]   = box[0];
    box[1].z = box[3].z = box[0].z;

    insideBox = aecPolygon_isPointInside ( 5L, box, &dat->pnt );
  }
  else
    insideBox = TRUE;

  if ( insideBox )
  {
    dat->tru = aecPolygon_isPointInside ( np, p, &dat->pnt );
    if ( dat->checkingInterior == FALSE ) dat->tru ^= TRUE;
  }

  if ( dat->tru ) sts = DTM_M_PRCINT;

  return ( sts );
}

