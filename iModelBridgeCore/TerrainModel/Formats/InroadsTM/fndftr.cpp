//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* fndftr.c					   twl	    28-Oct-1998                           */
/*----------------------------------------------------------------------------*/
/* Various utilities to find features.                                        */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_findFeatureByGUID
 DESC: Given a feature BeGuid, this function returns a pointer to the
       corresponding feature.
 HIST: Original - twl 10-Oct-1998
 MISC:
 KEYW: DTM FIND FEATURE BeGuid
-----------------------------------------------------------------------------%*/

int aecDTM_findFeatureByGUID /* <= TRUE if error                   */
(
    struct CIVdtmftr **ftrPP,             /* <= found feature                   */
    struct CIVdtmsrf *srfP,               /* => DTM surface (or NULL)           */
    BeGuid *guidP                           /* => surface guid pointer            */
)
{
    CIVdtmsrf *srf;
    int sts = SUCCESS;

    if ( srfP != NULL )
        srf = srfP;
    else
        sts = aecDTM_getActiveSurface ( &srf, NULL, NULL );

    if ( sts == SUCCESS )
        sts = aecDTM_hashFindFeatureByGUID ( ftrPP, srf, guidP );

    return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_findFeatureByName
 DESC: Given an ASCII feature name, this function returns a pointer to the
       corresponding feature.
 HIST: Original - twl 10-Oct-1998
 MISC:
 KEYW: DTM FIND FEATURE NAME
-----------------------------------------------------------------------------%*/

int aecDTM_findFeatureByName /* <= TRUE if error                   */
(
    struct CIVdtmftr **ftrPP,             /* <= found feature                   */
    struct CIVdtmsrf *srfP,               /* => DTM surface (or NULL)           */
    wchar_t *ftrNameP                     /* => surface name pointer            */
)
{
    CIVdtmsrf *srf;
    int sts = SUCCESS;

    if ( srfP != NULL )
        srf = srfP;
    else
        sts = aecDTM_getActiveSurface ( &srf, NULL, NULL );

    if ( sts == SUCCESS )
        sts = aecDTM_hashFindFeatureByName ( ftrPP, srf, ftrNameP );

    return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_findExteriorFeature
 DESC: Returns the address of a surface's exterior boundry if it exists.
 HIST: Original - twl 24-Jul-1999
 MISC:
 KEYW: DTM FIND EXTERIOR FEATURE
-----------------------------------------------------------------------------%*/

int aecDTM_findExteriorFeature /* <= TRUE if error                 */
(
    struct CIVdtmftr **ftrPP,             /* <= found feature                 */
    struct CIVdtmsrf *srfP                /* => DTM surface (or NULL)         */
)
{
    CIVdtmsrf *srf;
    struct CIVdtmftr *ftrP = NULL;
    struct CIVdtmblk *blkP;
    BOOL fnd = FALSE;
    int sts = SUCCESS;
    
    *ftrPP = NULL;

    if ( srfP != NULL )
        srf = srfP;
    else
        sts = aecDTM_getActiveSurface ( &srf, NULL, NULL );

    if ( sts == SUCCESS )
    {
        for ( blkP = srfP->extFtrf->blk; blkP && !fnd; blkP = blkP->nxt )
            for ( ftrP = blkP->rec.ftr; ftrP < blkP->rec.ftr + blkP->use && !fnd; ftrP++ )
                if ( !aecDTM_isFeatureDeletedFlagSet(ftrP) )
                {
                    *ftrPP = ftrP;
                    fnd = TRUE;
                }
    }

    if ( !fnd )
      sts = DTM_M_NOFTRF;

    return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_findFeatureByPoint
 DESC: Given a dtm point, finds the feature to which that point belongs.
 HIST: Original - twl 10-Oct-1998
 MISC:
 KEYW: DTM FIND FEATURE NAME
-----------------------------------------------------------------------------%*/

int aecDTM_findFeatureByPoint  /* <= TRUE if error                   */
(
    struct CIVdtmftr **ftrPP,             /* <= found feature                   */
    struct CIVdtmsrf *srfP,               /* => DTM surface (or NULL)           */
    struct CIVdtmpnt *pntP                /* => feature's point                 */
)
{
    CIVdtmsrf *srf;
    struct CIVdtmblk *ftrblkP = NULL;
    struct CIVdtmfil *filP = NULL;
    struct CIVdtmftr *fP;
    BOOL fnd = FALSE;
    long ftrInd;
    long pntType = -1;
    long ftrType;
    int sts = SUCCESS;

    if ( srfP != NULL )
        srf = srfP;
    else
        sts = aecDTM_getActiveSurface ( &srf, NULL, NULL );

    if ( aecDTM_findPointType ( &pntType, NULL, NULL, srfP, pntP ) == SUCCESS &&
         pntType >= DTM_C_DTMREG && pntType <= DTM_C_DTMCTR )
    {
      ftrType = aecDTM_featureFileFromPointFile ( pntType );
      aecDTM_getSurfaceFeatureFileIndex ( &ftrInd, ftrType );
    
      filP = srfP->ftrf[ftrInd];

      for ( ftrblkP = filP->blk; ftrblkP && !fnd; ftrblkP = ftrblkP->nxt )
	      for ( fP = ftrblkP->rec.ftr; fP < ftrblkP->rec.ftr + ftrblkP->use && !fnd; fP++ )
	        if ( !aecDTM_isFeatureDeletedFlagSet ( fP ) )
          {
            if ( pntP >= fP->p1 && pntP < ( fP->p1 + fP->numPnts ) )
            {
              *ftrPP = fP;
              fnd = TRUE;
            }
          }    
    }

    if ( !fnd )
      sts = DTM_M_NOFTRF;

    return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_findFeatureType
 DESC: Given a surface and a feature within that surface, this function
       returns the feature's type.  If you don't want
       either one of these, pass in a null pointer.
 HIST: Original - twl 05-Jan-1999
 MISC:
 KEYW: DTM FIND FEATURE NAME
-----------------------------------------------------------------------------%*/

int aecDTM_findFeatureType    /* <= TRUE if error                  */
(
    long *typP,                          /* <= feature's type                   */
    struct CIVdtmblk **inpblkP,          /* <= block where feature is           */
    struct CIVdtmsrf *srfP,              /* => surface to use                   */
    struct CIVdtmftr *fP                 /* => feature to use                   */
)
{
    struct CIVdtmblk *blkP;
    struct CIVdtmftr *startP, *stopP;
    int sts = DTM_M_NOFTRF, i;

    if ( typP ) *typP = 0;
    if ( inpblkP ) *inpblkP = (struct CIVdtmblk *)0;

    for ( i = 0; i < DTM_C_NMFTRF  &&  sts != SUCCESS; i++ )
        for ( blkP = srfP->ftrf[i]->blk; blkP  &&  sts != SUCCESS; blkP = blkP->nxt )
        {
            startP = blkP->rec.ftr;
            stopP  = startP + blkP->use - 1;
            if ( fP >= startP  &&  fP <= stopP )
            {
                if ( typP ) *typP = i + DTM_C_FSTFTRF;
                if ( inpblkP ) *inpblkP = blkP;
                sts = SUCCESS;
            }
        }

    return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_findFeatureFile
 DESC: Given a surface and a feature within that surface, this function finds
       the file that contains the feature.
 HIST: Original - twl 1-Jan-1999
 MISC:
 KEYW: DTM FIND FILE FEATURE
-----------------------------------------------------------------------------%*/

int aecDTM_findFeatureFile /* <= TRUE if error                       */
(
    struct CIVdtmfil **filPP,         /* <= found file                          */
    struct CIVdtmsrf *srfP,           /* => surface to look in                  */
    struct CIVdtmftr *ftrP            /* => point within file                   */
)
{
    int sts = SUCCESS;

    if ( filPP != (struct CIVdtmfil **)0 )
    {
        long typ;

        *filPP = (struct CIVdtmfil *)0;

        if ( ( sts = aecDTM_findFeatureType ( &typ, (struct CIVdtmblk **)0, srfP, ftrP ) ) == SUCCESS )
            *filPP = srfP->ftrf[typ - DTM_C_FSTFTRF];
    }

    return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_findStyleBlock
 DESC: Given a surface and a feature style within that surface, this function
       returns the feature style's block.  If you don't want
       either one of these, pass in a null pointer.
 HIST: Original - twl 05-Jan-1999
 MISC:
 KEYW: DTM FIND FEATURE NAME
-----------------------------------------------------------------------------%*/

int aecDTM_findStyleBlock    /* <= TRUE if error                  */
(
  struct CIVdtmblk **inpblkP,          /* <= block where feature is           */
  struct CIVdtmsrf *srfP,              /* => surface to use                   */
  struct CIVdtmsty *sP                 /* => style to use                     */
)
{
    struct CIVdtmblk *blkP;
    struct CIVdtmsty *startP, *stopP;
    int sts = DTM_M_NOSTYF;

    if ( inpblkP ) *inpblkP = (struct CIVdtmblk *)0;

    for ( blkP = srfP->styf->blk; blkP  &&  sts != SUCCESS; blkP = blkP->nxt )
    {
        startP = blkP->rec.sty;
        stopP  = startP + blkP->use - 1;
        if ( sP >= startP  &&  sP <= stopP )
        {
            if ( inpblkP ) *inpblkP = blkP;
            sts = SUCCESS;
        }
    }

    return ( sts );
}



