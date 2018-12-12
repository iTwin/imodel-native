//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* fndlin.c                                         tmi    19-Oct-1990        */
/*----------------------------------------------------------------------------*/
/* Finds a point in the surface given the point location.                     */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_findLinearByLocation
 DESC: Finds a linear feature given it a location near one of its vertices.
 HIST: Original - dgc July 1994
 MISC:
 KEYW: DTM LINEAR FIND LOCATION
-----------------------------------------------------------------------------%*/

int aecDTM_findLinearByLocation /* <= TRUE if error                */
(
  struct CIVdtmpnt **pntPP,           /* <= returned pointer to pnt (or NULL) */
  long *typeP,                        /* <= type found (or NULL)              */
  struct CIVdtmsrf *srfP,             /* => surface to look in                */
  long typeMask,                      /* => type to look for (or zero)        */
  DPoint3d *locP                      /* => location to look at               */
)
{
  struct CIVdtmpnt *pntP = NULL;
  struct CIVdtmblk *blkP = NULL;
  int sts = SUCCESS;

  *pntPP = NULL;

  if( srfP == NULL || locP == NULL )
    sts = DTM_M_INVDAT;

  if( sts == SUCCESS )
    sts = aecDTM_findPointByLocation( &pntP, NULL, srfP, typeMask, locP );

  if( sts == SUCCESS )
    sts = aecDTM_findPointType( typeP, NULL, &blkP, srfP, pntP );

  if( sts == SUCCESS )
  {
    struct CIVdtmpnt *begP = blkP->rec.pnt;
    struct CIVdtmpnt *endP = begP + blkP->use - 1;
    struct CIVdtmpnt *curP = NULL;
    int found = FALSE;

    for( curP = pntP; curP >= begP && curP <= endP && !found; --curP )
      if( aecDTM_isPointFirstLinearPoint( curP ) )
      {
        if( pntPP ) *pntPP = curP;
        found = TRUE;
      }

    if( !found )
      sts = DTM_M_NOPNTF;
  }
      
  return ( sts );
}





