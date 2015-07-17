//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* delftr.c                                         twl    21-Jan-1999        */
/*----------------------------------------------------------------------------*/
/* It sets the delete flag for a feature.                                     */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"


/*----------------------------------------------------------------------------*/
/* Static Functions                                                           */
/*----------------------------------------------------------------------------*/
static int aecDTM_deleteAllFeaturesProc(void *,struct CIVdtmsrf *,int,struct CIVdtmftr *);



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_deleteFeatureByGUID
 DESC: Searches for the DTM feature with a given guid and deletes it.
 HIST: Original - twl 1-Jan-1999
 MISC:
 KEYW: DTM FEATURE DELETE BY GUID
-----------------------------------------------------------------------------%*/

int aecDTM_deleteFeatureByGUID /* <= TRUE if error                   */
(
    struct CIVdtmsrf *srfP,         /* => surface with feature (or NULL)          */
    GUID *guidP                     /* => guid of feature to delete               */
)
{
    CIVdtmsrf *srf;
    CIVdtmftr *ftrP;
    int sts = SUCCESS;

    if ( srfP != NULL )
        srf = srfP;
    else
        sts = aecDTM_getActiveSurface ( &srf, NULL, NULL );

    if ( ( sts = aecDTM_findFeatureByGUID ( &ftrP, srf, guidP ) ) == SUCCESS && ftrP )
    {
        sts = aecDTM_deleteFeature ( srf, NULL, NULL, ftrP );
    }

    return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_deleteAllFeaturesProc
 DESC: Called by aecDTM_deleteAllFeatures.
 HIST: Original - twl 25-Jun-1999
 MISC:
 KEYW: DTM DELETE ALL FEATURES
-----------------------------------------------------------------------------%*/

static int aecDTM_deleteAllFeaturesProc
(
    void *tmp,
    struct CIVdtmsrf *,
    int,
    struct CIVdtmftr *ftrP
)
{
    struct CIVdtmsrf *srfP = ( struct CIVdtmsrf * ) tmp;
    return ( aecDTM_deleteFeature ( srfP, NULL, NULL, ftrP ) );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_deleteFeature
 DESC: It deletes the input feature.
 HIST: Original - twl 1-Jan-1999
 MISC:
 KEYW: DTM FEATURE DELETE
-----------------------------------------------------------------------------%*/

int aecDTM_deleteFeature /* <= TRUE if error                         */
(
    struct CIVdtmsrf *srfP,         /* => surface with feature (or NULL)          */
    struct CIVdtmfil *inftrfilP,    /* => file with feature (or NULL)             */
    struct CIVdtmfil *,             /* => file with feature's points ( or NULL )  */
    struct CIVdtmftr *ftrP          /* => feature to delete                       */
)
{
    int sts = SUCCESS;

    if ( !aecDTM_isFeatureDeletedFlagSet ( ftrP ) )
    {
        struct CIVpntedt pntdel;
        struct CIVdtmfil *ftrfilP;
        int i;

        if ( inftrfilP == (struct CIVdtmfil *)0 )
            sts = aecDTM_findFeatureFile ( &ftrfilP, srfP, ftrP );
        else
            ftrfilP = inftrfilP;

        aecDTM_setFeatureDeletedFlag ( ftrP );
        ftrfilP->ndel++;

        memset ( &pntdel, 0, sizeof ( pntdel ) );
        pntdel.state = PNTEDT_INIT;
        pntdel.opt = PNTEDT_NOPATCH;
        pntdel.srf = srfP;
        if ( ( sts = aecDTM_pointDelete ( &pntdel ) ) == SUCCESS )
        {
            pntdel.state = PNTEDT_PRCD;
            for ( i = 0; i < ftrP->numPnts && sts == SUCCESS; i++ )
            {
                if ( !aecDTM_isPointDeletedFlagSet (&ftrP->p1[i] ) )
                {
                    pntdel.pnt = &ftrP->p1[i];
                    aecDTM_pointDelete ( &pntdel );
                }
            }
        }

        if ( sts == SUCCESS )
        {
            struct CIVdtmcmp **cmpPtrsPP = NULL;
            int numCmpPtrs = 0;

            aecDTM_hashDeleteFeature ( srfP, ftrP );
            aecDTM_getComponentMemberParentsPtrs ( &cmpPtrsPP, &numCmpPtrs, srfP, &ftrP->guid );

            for ( int i = 0; i < numCmpPtrs; i++ )
            {
                aecDTM_deleteComponent ( srfP, cmpPtrsPP[i] );
            }

            if ( cmpPtrsPP )
                free ( cmpPtrsPP );
        }

        aecDTM_setSurfaceTinOutOfDateFlag ( srfP );
        aecDTM_setSurfaceModifiedFlag ( srfP );
    }

    return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_deleteStyle
 DESC: It deletes the input feature style.
 HIST: Original - tmi 1-Jan-1999
 MISC:
 KEYW: DTM STYLE DELETE
-----------------------------------------------------------------------------%*/

int aecDTM_deleteStyle /* <= TRUE if error                         */
(
    struct CIVdtmsrf *srfP,         /* => surface with point (or NULL)          */
    struct CIVdtmsty *styP          /* => style to delete                       */
)
{
    int sts = SUCCESS;

    if ( !aecDTM_isStyleDeletedFlagSet ( styP ) )
    {
        aecDTM_setStyleDeletedFlag ( styP );
        srfP->styf->ndel++;
    }

    return ( sts );
}

/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_deleteAllFeatures
 DESC: Deletes all features of the types specified in the type mask.
 HIST: Original - twl 25-Jun-1999
 MISC:
 KEYW: DTM DELETE ALL FEATURES
-----------------------------------------------------------------------------%*/

int aecDTM_deleteAllFeatures
(
    struct CIVdtmsrf *srfP,              /* => surface with triangles           */
    int typmsk                           /* => feature type (zero for all)      */
)
{
    return ( aecDTM_sendAllFeatures ( NULL, srfP, 0, typmsk, 
                                      aecDTM_deleteAllFeaturesProc, (void *) srfP ) );
}


