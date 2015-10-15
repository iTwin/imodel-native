//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* addftr.c                                     twl    16-Nov-1998            */
/*----------------------------------------------------------------------------*/
/* Various utilities to add features to surfaces and files.                   */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"


/*----------------------------------------------------------------------------*/
/* Private static function prototypes                                         */
/*----------------------------------------------------------------------------*/



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_addFeature
 DESC: Add a feature to a surface.
         Valid 'opt' arguments are (1st three apply only when saving a new
         feature:
         DTM_C_RENAME  - (default) Genereates a unique name.
         DTM_C_APPEND  - Appends to features with the same name.
         DTM_C_REPLACE - Replaces features with the same name.

 HIST: Original - twl 16-Nov-1998
 MISC:
 KEYW: DTM POINTS ADD
-----------------------------------------------------------------------------%*/

int aecDTM_addFeature        /* <= TRUE if error                   */
(
  BeSQLite::BeGuid *guidP,                          /*<=  guid of created feature         */
  struct CIVdtmsrf *srfP,               /* => surface to add feature to       */
  long opt,                             /* => DTM_C_APPEND, etc.              */
  wchar_t *name,                        /* => name of feature                 */
  wchar_t *description,                 /* => feature description (or NULL)   */
  wchar_t *parentName,                  /* => name of parent feature (or NULL)*/
  long featureType,                     /* => type of feature to add (or NULL)*/
  long numPnts,                         /* => # of points being added         */
  CIVdtmpnt *ftrPntsP,                  /* => list of feature points          */
  DPoint3d *pntsP,                      /* => list of point coordinates       */
  unsigned char *pntFlgsP,                    /* => list of point properties        */
  double pntDensity,                    /* => pointDensity                    */
  CIVdtmstynam *stylesP,                /* => feature's styles (or NULL)      */
  long numStyles,                       /* => # of styles                     */
  CIVdtmpaynam *payItemsP,              /* => pay items (or NULL)             */
  long numPayItems,                     /* => # of pay items                  */
  unsigned char flag,                          /* => feature's flags                 */
  BOOL bRemoveDuplicates, // = TRUE     /* => should almost always be TRUE    */
  BOOL bCloseString // = TRUE           /* => should almost always be TRUE    */
)
{
  struct CIVdtmfil *ftrFilP, *styFilP, *payFilP;;
  struct CIVdtmftr *newFtrP = NULL;
  struct CIVdtmftr *extFtrP;
  struct CIVdtmblk *ftrBlkP, *styBlkP, *payBlkP;
  struct CIVdtmpnt *p1P;
  CIVdtmstynam defaultStyle[1];
  BOOL bFree_ftrPntsP = FALSE;
  long ftrInd;
  long pntInd;
  long allocSize;
  long ftrType = featureType;
  long pntType;
  int  sts = SUCCESS, closeString;
  int  i;

  if ( featureType == DTM_C_DTMREGFTR )
    pntDensity = 0.0;

  pntType = aecDTM_pointFileFromFeatureFile ( featureType );

  if ( !ftrPntsP )
  {
    if ( ( ftrPntsP = ( CIVdtmpnt * ) calloc (  numPnts, sizeof ( CIVdtmpnt ) ) ) != NULL )
    {
      for ( i = 0; i < numPnts; i++ )
      {
        memcpy ( &ftrPntsP[i].cor, &pntsP[i], sizeof ( DPoint3d ) );
       
        if ( pntFlgsP )
            ftrPntsP[i].flg = pntFlgsP[i];
        else if ( i > 0  && ( pntType == DTM_C_DTMBRK || pntType == DTM_C_DTMCTR || pntType == DTM_C_DTMINF || pntType == DTM_C_DTMINT || pntType == DTM_C_DTMEXT ) )
            ftrPntsP[i].flg |= DTM_C_PNTPUD;
      }

      bFree_ftrPntsP = TRUE;
    }
    else
      sts = DTM_M_MEMALF;
  }

  if ( opt == DTM_C_APPEND && aecDTM_findFeatureByName ( &extFtrP, srfP, name ) == SUCCESS )
  {
    struct CIVdtmpnt *extPnts = NULL;
    long extTyp;
    long nExtPnts;

    if ( ( sts = aecDTM_getFeatureInfo ( extFtrP, srfP, 
                                  NULL, &extTyp, NULL, NULL, NULL, &extPnts, &nExtPnts, NULL,
                                  NULL, NULL, NULL, NULL, NULL ) ) == SUCCESS )
    {


      if ( ( extPnts = ( struct CIVdtmpnt * ) realloc ( extPnts, sizeof ( struct CIVdtmpnt ) * ( nExtPnts + numPnts ) ) ) != NULL )
      {
        for ( i = 0; i < numPnts; i++ )
        {
          if ( extTyp != DTM_C_DTMREGFTR && i > 0 )
            ftrPntsP[i].flg |= DTM_C_PNTPUD;
          else
            ftrPntsP[i].flg &= ~DTM_C_PNTPUD;

          memcpy ( &extPnts[nExtPnts], &ftrPntsP[i], sizeof ( struct CIVdtmpnt ) );
          nExtPnts++;
        }
        
        sts = aecDTM_setFeatureInfo (  extFtrP, srfP, DTM_C_ADDONLY, NULL, NULL, NULL, 
                                       NULL, NULL, extPnts, nExtPnts, NULL, NULL, 0, NULL, 0, NULL, TRUE );

        newFtrP = extFtrP;
      }
      else
        sts = DTM_M_MEMALF;
    }

    if ( extPnts )
      free ( extPnts );
  }
  else
  {
    if ( sts == SUCCESS && 
         ( sts = aecDTM_addFeatureCheck ( srfP, ftrType, &numPnts, ftrPntsP, &closeString, TRUE, bRemoveDuplicates ) ) == SUCCESS )
    {
      if ( bCloseString == FALSE )
          closeString = FALSE;

      if ( opt == DTM_C_REPLACE && aecDTM_findFeatureByName ( &extFtrP, srfP, name ) == SUCCESS )
        aecDTM_deleteFeature ( srfP, NULL, NULL, extFtrP );

      if ( pntDensity > AEC_C_TOL3 )
      {
          CIVdtmpnt *newPntsP = NULL;
          long numNewPnts = 0;

          sts = aecDTM_densifyFeaturePoints ( &numNewPnts, &newPntsP, numPnts, ftrPntsP, pntDensity );

          if ( bFree_ftrPntsP && ftrPntsP )
            free ( ftrPntsP );

          ftrPntsP = newPntsP;
          numPnts = numNewPnts;
          bFree_ftrPntsP = TRUE;
      }

      if ( sts == SUCCESS )
      {
        aecDTM_getSurfaceFeatureFileIndex ( &ftrInd, featureType );
        aecDTM_getSurfaceFileIndex ( &pntInd, pntType );

        if ( ( sts = aecDTM_addFeaturePointsToFile ( &p1P, srfP, srfP->pntf[pntInd], pntType, closeString, numPnts, ftrPntsP, 0 ) ) == SUCCESS )
        {
          ftrFilP = srfP->ftrf[ftrInd];
          ftrBlkP = ftrFilP->blk;

          if ( !ftrBlkP || ftrBlkP->use == ftrBlkP->alc )
            sts = aecDTM_allocateBlock ( &ftrBlkP, ftrFilP, DTM_C_BLKSIZ, 1 );

          if ( sts == SUCCESS )
          {
	        newFtrP = &ftrBlkP->rec.ftr[ftrBlkP->use++];
            aecGuid_generate ( &newFtrP->guid );

            if ( name && !wcscmp ( name, L"" ) )
                wcscpy ( name, aecOutput_getMessageString ( DTM_M_NOFTRNAME ) );

            aecDTM_generateUniqueFeatureName ( newFtrP->nam, name, srfP );

            if ( description )
                wcscpy ( newFtrP->des, description );

            if ( parentName )
            wcscpy ( newFtrP->par, parentName );

            newFtrP->p1 = p1P;
            newFtrP->numPnts = numPnts + closeString;
            newFtrP->pntDensity = pntDensity;

            styFilP = srfP->styf;
            styBlkP = styFilP->blk;

            if ( !stylesP || numStyles == 0 )
            {
              if ( ftrType == DTM_C_DTMREGFTR )
                wcscpy ( defaultStyle[0], aecOutput_getMessageString ( DTM_M_RNDFTRSTY ) );
              else
                wcscpy ( defaultStyle[0], DEFAULT_PREFERENCE_NAME );

              stylesP = defaultStyle;
              numStyles = 1;
            }

            if ( !styFilP->blk || numStyles > styFilP->blk->alc - styFilP->blk->use )
            {
              allocSize = ( numStyles > DTM_C_BLKSIZ ) ? numStyles : DTM_C_BLKSIZ;
              sts = aecDTM_allocateBlock ( &styBlkP, styFilP, allocSize, 1 );
            }

            newFtrP->s1 = &styBlkP->rec.sty[styBlkP->use];

            for ( i = 0; i < numStyles; i++ )
              wcscpy ( styBlkP->rec.sty[styBlkP->use++].nam, stylesP[i] );

            styFilP->nrec += numStyles;
            newFtrP->numStyles = numStyles;

            payFilP = srfP->payf;
            payBlkP = payFilP->blk;

            if ( !payFilP->blk || numPayItems > payFilP->blk->alc - payFilP->blk->use )
            {
              allocSize = ( numPayItems > DTM_C_BLKSIZ ) ? numPayItems : DTM_C_BLKSIZ;
              sts = aecDTM_allocateBlock ( &payBlkP, payFilP, allocSize, 1 );
            }

            newFtrP->pay = &payBlkP->rec.pay[payBlkP->use];

            for ( i = 0; i < numPayItems; i++ )
              _tcscpy ( payBlkP->rec.pay[payBlkP->use++].nam, payItemsP[i] );

            payFilP->nrec += numPayItems;
            newFtrP->numPayItems = numPayItems;

            ftrFilP->nrec++;

            if ( sts == SUCCESS )
            {
              aecDTM_setFeatureFlag ( newFtrP, srfP, &flag );
            }

            if ( sts == SUCCESS )
              aecDTM_hashInsertFeature ( srfP, newFtrP );
          }
        }
      }
    }
  }

  if ( bFree_ftrPntsP && ftrPntsP )
    free ( ftrPntsP );

  if ( sts == SUCCESS && guidP )
  {
      if (newFtrP)
        *guidP = newFtrP->guid;
  }

  aecDTM_setSurfaceTinOutOfDateFlag ( srfP );

  return ( sts );
}


/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_addFeatureCheck
 DESC: Checks integrity (sp?) of feature data being added to surface.
       duplicate points are removed from the list.
 HIST: Original - twl 16-Nov-1998
 MISC: static
 KEYW: DTM FEATURE ADD
-----------------------------------------------------------------------------%*/

int aecDTM_addFeatureCheck  /*  <= TRUE if error                   */
(
  struct CIVdtmsrf *srfP,              /*  => srf pnts are added to           */
  long ftrTyp,                         /*  => type of feature we're adding.   */
  long *numPntsP,                      /* <=> # coords being added            */
  CIVdtmpnt *pntsP,                    /*  => list of coordinates             */
  int *closeStringP,                   /* <=  TRUE: close polygons            */
  int deallocExteriors,                /*  => TRUE or FALSE                   */
  BOOL bRemoveDuplicates // = TRUE     /*  => should almost always be TRUE    */
)
{
  struct CIVdtmftr *oldExtP = NULL;
  long pntType;
  int sts = SUCCESS;

  *closeStringP = 0;

  switch ( ftrTyp )
  {
  case DTM_C_DTMREGFTR :
  case DTM_C_DTMBRKFTR :
  case DTM_C_DTMINTFTR :
  case DTM_C_DTMCTRFTR :
    break;

  case DTM_C_DTMEXTFTR :
    if ( deallocExteriors && srfP->extFtrf->nrec - srfP->extFtrf->ndel > 0 )
    {
      if ( aecDTM_findExteriorFeature ( &oldExtP, srfP ) == SUCCESS )
        aecDTM_hashDeleteFeature ( srfP, oldExtP );

      sts = aecDTM_deallocateFile ( srfP->extFtrf );
    }
    break;

  default :
    sts = DTM_M_BADFTRTYP;
  }


  if ( sts == SUCCESS )
  {
    
    pntType = aecDTM_pointFileFromFeatureFile ( ftrTyp );

    if ( *numPntsP > 1L && bRemoveDuplicates) aecDTM_removeDuplicateDTMPoints ( numPntsP, pntsP, AEC_C_TOL );

    switch ( pntType )
    {
      case ( DTM_C_DTMREG ) :
        break;

      case ( DTM_C_DTMBRK ) :
      case ( DTM_C_DTMCTR ) :
      case ( DTM_C_DTMINF ) :
        if ( *numPntsP < 2L ) sts = DTM_M_BADLIN;
        break;

      case ( DTM_C_DTMINT ) :
        if ( *numPntsP < 3L )
          sts = DTM_M_BADBND;
        else if ( ! VEQUALXY ( pntsP[0].cor, pntsP[*numPntsP-1].cor, AEC_C_TOL ) )
          *closeStringP = 1;
        break;

      case ( DTM_C_DTMEXT ) :
        if ( *numPntsP < 3L )
          sts = DTM_M_BADBND;
        else
        {
          if ( deallocExteriors && srfP->extf->nrec - srfP->extf->ndel > 0 )
          {
            if ( aecDTM_findExteriorFeature ( &oldExtP, srfP ) == SUCCESS )
              aecDTM_hashDeleteFeature ( srfP, oldExtP );

            sts = aecDTM_deallocateFile ( srfP->extf );
          }

          if ( sts == SUCCESS )
            if ( ! VEQUALXY ( pntsP[0].cor, pntsP[*numPntsP-1].cor, AEC_C_TOL ) )
              *closeStringP = 1;
        }
        break;
    }
  }

  return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_addFeaturePointsToFile
 DESC: Add points directly to a specific point file.  Allows the size of the 
       block allocated to be specified.
 HIST: Original - tmi 24-Oct-1990
 MISC:
 KEYW: DTM POINTS ADD EX
-----------------------------------------------------------------------------%*/

int aecDTM_addFeaturePointsToFile /* <= TRUE if error              */
(
  struct CIVdtmpnt **pntPP,              /* <= ptr to 1st pnt (or NULL)       */
  struct CIVdtmsrf *srfP,                /* => surface to add points to       */
  struct CIVdtmfil *filP,                /* => file ptr to add pnts to        */
  long typ,                              /* => type of pnt being added        */
  int closeString,                       /* => TRUE: ensure closed ply.       */
  long ncor,                             /* => # of points being added        */
  CIVdtmpnt *corP,                       /* => list of point coords.          */
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
      memcpy ( tmpP, &corP[i], sizeof ( CIVdtmpnt ) );

    if ( closeString )
    {
      memcpy ( tmpP, &corP[0], sizeof ( CIVdtmpnt ) );
      tmpP->flg |= DTM_C_PNTPUD;
    }

    filP->nrec += ncor + closeString;
    blkP->use  += ncor + closeString;
  }

  return ( sts );
}