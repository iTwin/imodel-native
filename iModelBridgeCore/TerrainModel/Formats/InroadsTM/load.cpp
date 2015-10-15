//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+

//---------------------------------------------------------------------------
// Include files
//---------------------------------------------------------------------------
#include "stdafx.h"


//---------------------------------------------------------------------------
// DESC: Searches the specified surface for a feature with the specified guid.
//       If found, the feature is copied into the feature object.  If srfP is
//       NULL, active surface is searched.
// HIST: Original - twlangha - 01/13/99
// MISC:
//---------------------------------------------------------------------------

int CFeature::LoadFromDTMByGuid     // <=  Non-zero status code if error occurred.
( 
    BeSQLite::BeGuid *guidP,                    //  => BeSQLite::BeGuid of feature to search for.
    struct CIVdtmsrf *srfP          //  => Surface to be searched (NULL indicates
                                    //     the active surface is to be searched).
)
{
    struct CIVdtmftr *ftrP = NULL;
    int sts = SUCCESS;

    if ( srfP == NULL )
        sts = aecDTM_getActiveSurface ( &srfP, NULL, NULL );

    if ( ( sts = aecDTM_findFeatureByGUID ( &ftrP, srfP, guidP ) ) == SUCCESS )
    {
        Clear ( );
        m_ftrP = ftrP;
        m_srfP = srfP;

        sts = aecDTM_getFeatureInfo ( m_ftrP, m_srfP, 
                                      &m_guid, &m_nType, m_sName, m_sDesc, m_sParent, NULL, NULL, &m_dPntDensity,
                                      NULL, NULL, NULL, NULL, &m_flag );
    }

    return ( sts );
}


//---------------------------------------------------------------------------
// DESC: If the specified surface contains an exterior boundary, it is loaded
//       into the feature object.
// HIST: Original - twlangha - 07/23/99
// MISC:
//---------------------------------------------------------------------------

int CFeature::LoadFromDTMExteriorBoundary // <=  Non-zero status code if error occurred.
( 
    struct CIVdtmsrf *srfP      //  => Surface to be searched (NULL indicates
                                //     the active surface is to be searched).
)
{
    struct CIVdtmftr *ftrP = NULL;
    int sts = SUCCESS;

    if ( srfP == NULL )
        sts = aecDTM_getActiveSurface ( &srfP, NULL, NULL );

    if ( ( sts = aecDTM_findExteriorFeature ( &ftrP, srfP ) ) == SUCCESS )
    {
        Clear ( );
        m_ftrP = ftrP;
        m_srfP = srfP;

        sts = aecDTM_getFeatureInfo ( m_ftrP, m_srfP, 
                                      &m_guid, &m_nType, m_sName, m_sDesc, m_sParent, NULL, NULL, &m_dPntDensity,
                                      NULL, NULL, NULL, NULL, &m_flag );
    }

    return ( sts );
}


//---------------------------------------------------------------------------
// DESC: Searches the specified surface (active if srfP is NULL) for a
//       feature whose BeSQLite::BeGuid matches that of the feature object.  If found,
//       the surface feature will be overwritten with the contents of the
//       feature object.  If not found, the contents of the feature object
//       will be written to the surface as a new feature.  The feature will
//       be assigned a newly generated BeSQLite::BeGuid and a name unique to the
//       specified surface. The opt argument can be used to provide additional
//       information which determine how duplicate feature names are handled if
//       a new feature is being save or can speed up re-triangululization of
//       the feature.
//       Valid 'opt' arguments are (1st three apply only when saving a new
//       feature:
//       DTM_C_RENAME  - (default) Genereates a unique name.
//       DTM_C_APPEND  - Appends to features with the same name.
//       DTM_C_REPLACE - Replaces features with the same name.
//       DTM_C_DELONLY - If points have only been deleted.
//       DTM_C_ADDONLY - If points have only been inserted or added.
//       DTM_C_MOVONLY - If points have only been moved.
//       DTM_C_ELVONLY - If the elevations of points have only been changed.
//
// HIST: Original - twlangha - 01/13/99
// MISC:
//---------------------------------------------------------------------------

int CFeature::SaveToDTM     // <=  Non-zero status code if error occurred.
( 
    struct CIVdtmsrf *srfP,  //  => Target surface (active if NULL).
    long opt,                //  => Options (optional).
    BOOL bReTin              //  => Retriangulate feature ( optional )
)
{
    wchar_t newName[DTM_C_NAMSIZ];
    int sts = SUCCESS;

    if ( !( m_flag & DTM_C_FTRDEL ) )
    {
        if ( srfP != NULL )
        {
            if ( m_srfP != srfP )
            {
                if ( !PointsAreLoaded ( ) )
                    sts = LoadPoints ( );

                if ( sts == SUCCESS && !StylesAreLoaded ( ) )
                    sts = LoadStyles ( );

                if ( sts == SUCCESS && !PayItemsAreLoaded ( ) )
                    sts = LoadPayItems ( );
            }

            m_srfP = srfP;
        }
        else if ( m_srfP == NULL )
            sts = aecDTM_getActiveSurface ( &m_srfP, NULL, NULL );

        if ( ( sts = aecDTM_findFeatureByGUID ( &m_ftrP, m_srfP, &m_guid ) ) == SUCCESS )
        {
            if ( wcscmp ( m_ftrP->nam, m_sName ) )
            {
                aecDTM_generateUniqueFeatureName ( newName, m_sName, m_srfP );
                wcscpy ( m_sName, newName );
            }

            ReversePointDisconFlags ( TRUE );
            sts = aecDTM_setFeatureInfo ( m_ftrP, m_srfP, opt, NULL, &m_nType, m_sName, m_sDesc, m_sParent,
                                          m_pntsP, m_nNumPnts, &m_dPntDensity, m_stylesP, m_nNumStyles, m_payItemsP, m_nNumPayItems, &m_flag, bReTin );

            if ( sts == SUCCESS )
                ReversePointDisconFlags ( FALSE );
        }
        else if ( sts == DTM_M_NOFTRF )
        {
            sts = SaveAsNewToDTM ( m_srfP, opt );
        }

        if ( sts == SUCCESS )
            sts = LoadFromDTMByGuid ( &m_guid, m_srfP );
    }
    return ( sts );        
}


//---------------------------------------------------------------------------
// DESC: Writes the contents of the feature object to the specified
//       surface (active if srfP is NULL) as a new feature.  The feature
//       will be assigned a new BeSQLite::BeGuid and a name unique to the specified
//       surface.  The opt argument can be used to provide additional
//       information which determine how duplicate feature names are handled if
//       a new feature is being save or can speed up re-triangululization of
//       the feature.
//       Valid 'opt' arguments are (1st three apply only when saving a new
//       feature:
//       DTM_C_RENAME  - (default) Genereates a unique name.
//       DTM_C_APPEND  - Appends to features with the same name.
//       DTM_C_REPLACE - Replaces features with the same name.
// HIST: Original - twlangha - 01/13/99
// MISC:
//---------------------------------------------------------------------------

int CFeature::SaveAsNewToDTM    // <=  Non-zero status code if error occurred.
(
    struct CIVdtmsrf *srfP,     //  => Target surface (active if NULL).
    long opt                    //  => Options ( optional )
)
{
    int sts = SUCCESS;


    if ( !( m_flag & DTM_C_FTRDEL ) )
    {
        if ( !PointsAreLoaded ( ) )
            sts = LoadPoints ( );

        if ( sts == SUCCESS && !StylesAreLoaded ( ) )
            sts = LoadStyles ( );

        if ( sts == SUCCESS && !PayItemsAreLoaded ( ) )
            sts = LoadPayItems ( );

        if ( sts == SUCCESS )
        {
            if ( srfP != NULL )
                m_srfP = srfP;
            else if ( m_srfP == NULL )
                sts = aecDTM_getActiveSurface ( &m_srfP, NULL, NULL );

            if ( sts == SUCCESS )
            {
                ReversePointDisconFlags ( TRUE );

                sts = aecDTM_addFeature ( &m_guid, m_srfP, opt, m_sName, m_sDesc, m_sParent, m_nType, m_nNumPnts, m_pntsP, NULL, NULL, m_dPntDensity,
                                        m_stylesP, m_nNumStyles, m_payItemsP, m_nNumPayItems, m_flag );
                if ( sts == SUCCESS )
                    ReversePointDisconFlags ( FALSE );
            }
        }

        if ( sts == SUCCESS )
        {
            sts = LoadFromDTMByGuid ( &m_guid, m_srfP );
        }
    }

    return ( sts );
}


