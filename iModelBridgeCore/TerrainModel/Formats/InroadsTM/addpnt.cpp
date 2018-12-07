//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* addpnt.c                                     tmi    24-Oct-1990            */
/*----------------------------------------------------------------------------*/
/* Various utilities to add points to surfaces and files.                     */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"



/*----------------------------------------------------------------------------*/
/* Private static function prototypes                                         */
/*----------------------------------------------------------------------------*/



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_addPointsExt
 DESC: Add points to surface and (optionally) return pointer to points.  Any
       duplicate points in the input list are removed from the list.
 HIST: Original - tmi 17-Jan-1994
 MISC:
 KEYW: DTM POINTS ADD
-----------------------------------------------------------------------------%*/

int aecDTM_addPointsExt      /* <= TRUE if error                   */
(
  struct CIVdtmpnt **pntPP,             /* <= pntr to 1st pnt (or NULL)       */
  struct CIVdtmsrf *srfP,               /* => surface to add pnts to          */
  long typ,                             /* => type of point to add            */
  long ncor,                            /* => # of points being added         */
  DPoint3d *corP                        /* => list of point coordinates       */
)
{
  int sts = SUCCESS, closeString;
  long pntType = typ & ADDPNT_TYPMSK;

  if ( ( sts = aecDTM_addPointsCheck ( srfP, pntType, &ncor, corP, &closeString, TRUE ) ) == SUCCESS )
    if ( !( typ & ADDPNT_DTM )  &&  aecParams_getFeatureState( NULL ) )                                           /* Auto-Plot with TAG data */
    {
    }

  if ( sts == SUCCESS )
  {
    long ind;

    aecDTM_getSurfaceFileIndex ( &ind, pntType );
    sts = aecDTM_addPointsToFile ( pntPP, srfP, srfP->pntf[ind], pntType, closeString, ncor, corP );
  }

  return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_addPointsToFile
 DESC: Add points directly to a specific point file.
 HIST: Original - tmi 24-Oct-1990
 MISC:
 KEYW: DTM POINTS ADD
-----------------------------------------------------------------------------%*/

int aecDTM_addPointsToFile    /* <= TRUE if error                  */
(
  struct CIVdtmpnt **pntPP,              /* <= ptr to 1st pnt (or NULL)       */
  struct CIVdtmsrf *srfP,                /* => surface to add points to       */
  struct CIVdtmfil *filP,                /* => file ptr to add pnts to        */
  long typ,                              /* => type of pnt being added        */
  int closeString,                       /* => TRUE: ensure closed ply.       */
  long ncor,                             /* => # of points being added        */
  DPoint3d *corP                         /* => list of point coords.          */
)
{
    return ( aecDTM_addPointsToFileEx ( pntPP, srfP, filP, typ,
                                        closeString, ncor, corP, 0 ) );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_addPointsToFileEx
 DESC: Add points directly to a specific point file.  Allows the size of the 
       block allocated to be specified.
 HIST: Original - tmi 24-Oct-1990
 MISC:
 KEYW: DTM POINTS ADD EX
-----------------------------------------------------------------------------%*/

int aecDTM_addPointsToFileEx  /* <= TRUE if error                  */
(
  struct CIVdtmpnt **pntPP,              /* <= ptr to 1st pnt (or NULL)       */
  struct CIVdtmsrf *srfP,                /* => surface to add points to       */
  struct CIVdtmfil *filP,                /* => file ptr to add pnts to        */
  long typ,                              /* => type of pnt being added        */
  int closeString,                       /* => TRUE: ensure closed ply.       */
  long ncor,                             /* => # of points being added        */
  DPoint3d *corP,                        /* => list of point coords.          */
  long nPntsAlc                          /* => # pnts to allocate             */
)
{
  struct CIVdtmpnt *tmpP;
  struct CIVdtmblk *blkP;
  int sts = SUCCESS, frc;
  long nrec, i;

  frc  = ( typ != DTM_C_DTMREG  ||  ncor >  1L ) ? 1 : 0;
  nrec = ( typ == DTM_C_DTMREG  &&  ncor == 1L ) ? (long)DTM_C_BLKSIZ : ncor + closeString;

  if ( !nPntsAlc || nPntsAlc < nrec )
    nPntsAlc = nrec;

  if ( ( sts = aecDTM_allocateBlock ( &blkP, filP, nPntsAlc, frc ) ) == SUCCESS )
  {
    aecDTM_setSurfaceModifiedFlag ( srfP );

    tmpP = blkP->rec.pnt + blkP->use;
    if ( pntPP != (struct CIVdtmpnt **)0 ) *pntPP = tmpP;
    for ( i = 0; i < ncor; i++, tmpP++ )
    {
      DTMDPOINTTOPOINT ( srf, corP[i], tmpP );
      if ( i > 0  &&  ( typ == DTM_C_DTMBRK || typ == DTM_C_DTMCTR || typ == DTM_C_DTMINF || typ == DTM_C_DTMINT || typ == DTM_C_DTMEXT ) )
        tmpP->flg |= DTM_C_PNTPUD;
    }

    if ( closeString )
    {
      DTMDPOINTTOPOINT ( srf, corP[0], tmpP );
      tmpP->flg |= DTM_C_PNTPUD;
    }

    filP->nrec += ncor + closeString;
    blkP->use  += ncor + closeString;
  }

  if ( sts == SUCCESS && ncor + closeString > 0 )
    aecDTM_setSurfaceTinOutOfDateFlag ( srfP );

  return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_addPointsCheck
 DESC: Checks integrity (sp?) of point data being added to surface.
       duplicate points are removed from the list.
 HIST: Original - tmi 17-Jan-1994
 MISC: static
 KEYW: DTM POINTS ADD
-----------------------------------------------------------------------------%*/

int aecDTM_addPointsCheck       /*  <= TRUE if error                   */
(
  struct CIVdtmsrf *srfP,              /*  => srf pnts are added to           */
  long typ,                            /*  => type of points we're add.       */
  long *ncorP,                         /* <=> # coords being added            */
  DPoint3d *corP,                      /*  => list of coordinates             */
  int *closeStringP,                   /* <=  TRUE: close polygons            */
  int deallocExteriors                 /*  => TRUE or FALSE                   */
)
{
  int sts = SUCCESS;
  *closeStringP = 0;

  if ( *ncorP > 1L ) aecPolygon_removeDuplicatePoints ( ncorP, corP, AEC_C_TOL );

  switch ( typ )
  {
    case ( DTM_C_DTMREG ) :
      break;

    case ( DTM_C_DTMBRK ) :
    case ( DTM_C_DTMCTR ) :
    case ( DTM_C_DTMINF ) :
      if ( *ncorP < 2L ) sts = DTM_M_BADLIN;
      break;

    case ( DTM_C_DTMINT ) :
      if ( *ncorP < 3L )
        sts = DTM_M_BADBND;
      else if ( ! VEQUALXY ( corP[0], corP[*ncorP-1], AEC_C_TOL ) )
        *closeStringP = 1;
      break;

    case ( DTM_C_DTMEXT ) :
      if ( *ncorP < 3L )
        sts = DTM_M_BADBND;
      else
      {
        if ( deallocExteriors && srfP->extf->nrec - srfP->extf->ndel > 0 )
          sts = aecDTM_deallocateFile ( srfP->extf );

        if ( sts == SUCCESS )
          if ( ! VEQUALXY ( corP[0], corP[*ncorP-1], AEC_C_TOL ) )
            *closeStringP = 1;
      }
      break;
  }

  return ( sts );
}
