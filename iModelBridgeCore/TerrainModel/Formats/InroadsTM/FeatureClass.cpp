//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+

//---------------------------------------------------------------------------
// Include files
//---------------------------------------------------------------------------
#include "stdafx.h"

//---------------------------------------------------------------------------
// Macros and constants
//---------------------------------------------------------------------------

#define ALLOC_SIZE  10

//---------------------------------------------------------------------------
// DESC: Constructor
// HIST: Original - twlangha - 01/13/99
// MISC: 
//---------------------------------------------------------------------------

CFeature::CFeature()
{
    m_ftrP = NULL;
    m_srfP = NULL;
    memset ( &m_guid, 0, sizeof ( BeSQLite::BeGuid ) );
    m_nType = DTM_C_DTMREGFTR;
    wcscpy ( m_sName, L"" );
    wcscpy ( m_sDesc, L"" );
    wcscpy ( m_sParent, L"" );
    m_pntsP = NULL;
    m_nNumPnts = 0;
    m_dPntDensity = 0.0;
    m_nPntsAlc = 0;
    m_nPntAllocSize = 0;
    m_stylesP = NULL;
    m_nNumStyles = 0;
    m_nStylesAlc = 0;
    m_payItemsP = NULL;
    m_nNumPayItems = 0;
    m_nPayItemsAlc = 0;
    memset ( &m_flag, 0, sizeof ( unsigned char ) );
    m_locateGuidsP = NULL;
    m_nNumLocateGuids = 0;
    m_nLocateGuidsAlc = 0;
    m_bLocateClosed = FALSE;
    m_bLocateClosedXY = FALSE;
    m_locatePointsP = NULL;
    m_nNumLocatePoints = 0;
    m_nLocatePointsAlc = 0;
    m_pStyleInfo = NULL;
}


//---------------------------------------------------------------------------
// DESC: Destructor
// HIST: Original - twlangha - 01/13/99
// MISC:
//---------------------------------------------------------------------------

CFeature::~CFeature()

{
    FreeAll ( );
}


//---------------------------------------------------------------------------
// DESC: Returns the feature's triangulation setting.  If the feature's
//       triangulation setting is enabled the feature's points will be 
//       included in triangulation.  See EnableTriangulation() for more
//       detail.
// HIST: Original - twlangha - 01/13/99
// MISC:
//---------------------------------------------------------------------------

boolean CFeature::IsTriangulationEnabled ( )    //  => TRUE or FALSE
{
    boolean sts = TRUE;

    if ( m_flag & DTM_C_FTRTIN )
        sts = FALSE;

    return ( sts );
}


//---------------------------------------------------------------------------
// DESC: Frees all memory allocated to the feature object's style list.
// HIST: Original - twlangha - 01/13/99
// MISC:
//---------------------------------------------------------------------------

void CFeature::FreeStyles ( )
{
    if ( m_stylesP )
        free ( m_stylesP );

    m_stylesP = NULL;
    m_nNumStyles = 0;
    m_nStylesAlc = 0;
}


//---------------------------------------------------------------------------
// DESC: When a feature is loaded from the DTM into a feature object.  Points
//       and styles are not loaded unless needed because of memory allocation
//       overhead.  This function searches the DTM for the feature with a
//       BeSQLite::BeGuid matching the feature object.  If found, it allocates memory for
//       and loads a feature's styles from the DTM into feature object's
//       style list.
// HIST: Original - twlangha - 01/13/99
// MISC:
//---------------------------------------------------------------------------

int CFeature::LoadStyles ( )    // <=  Non-zero status code if error occurred.
{
    int sts = SUCCESS;

    if ( !StylesAreLoaded ( ) && m_ftrP )
    {
        sts = aecDTM_getFeatureInfo ( m_ftrP, m_srfP, 
                                      NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                                      &m_stylesP, &m_nNumStyles, NULL, NULL, NULL );

        m_nStylesAlc = m_nNumStyles;
    }

    return ( sts );
}


//---------------------------------------------------------------------------
// DESC: When a feature is loaded from the DTM into a feature object.  Points
//       and styles are not loaded unless needed because of memory allocation
//       overhead.  This function indicates whether or not the feature object's
//       styles have been loaded.
// HIST: Original - twlangha - 01/13/99
// MISC:
//---------------------------------------------------------------------------

boolean CFeature::StylesAreLoaded ( )   // <=  TRUE(loaded)/FALSE(not loaded)
{
    boolean bLoaded = FALSE;

    if ( m_stylesP )
        bLoaded = TRUE;

    return ( bLoaded );
}


//---------------------------------------------------------------------------
// DESC: Frees all memory allocated to the feature object's pay item list.
// HIST: Original - twl - 10/11/2003
// MISC:
//---------------------------------------------------------------------------

void CFeature::FreePayItems ( )
{
    if ( m_payItemsP )
        free ( m_payItemsP );

    m_payItemsP = NULL;
    m_nNumPayItems = 0;
    m_nPayItemsAlc = 0;
}


//---------------------------------------------------------------------------
// DESC: When a feature is loaded from the DTM into a feature object.  Points
//       and pay items are not loaded unless needed because of memory allocation
//       overhead.  This function searches the DTM for the feature with a
//       BeSQLite::BeGuid matching the feature object.  If found, it allocates memory for
//       and loads a feature's pay items from the DTM into feature object's
//       pay items list.
// HIST: Original - twl - 10/11/2003
// MISC:
//---------------------------------------------------------------------------

int CFeature::LoadPayItems ( )    // <=  Non-zero status code if error occurred.
{
    int sts = SUCCESS;

    if ( !PayItemsAreLoaded ( ) && m_ftrP )
    {
        sts = aecDTM_getFeatureInfo ( m_ftrP, m_srfP, 
                                      NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                                      NULL, NULL, &m_payItemsP, &m_nNumPayItems, NULL );

        m_nPayItemsAlc = m_nNumPayItems;
    }

    return ( sts );
}


//---------------------------------------------------------------------------
// DESC: When a feature is loaded from the DTM into a feature object.  Pay Items
//       and styles are not loaded unless needed because of memory allocation
//       overhead.  This function indicates whether or not the feature object's
//       pay items have been loaded.
// HIST: Original - twlangha - 10/11/2003
// MISC:
//---------------------------------------------------------------------------

boolean CFeature::PayItemsAreLoaded ( )  // <=  TRUE(loaded)/FALSE(not loaded)
{
    boolean bLoaded = FALSE;

    if ( m_payItemsP )
        bLoaded = TRUE;

    return ( bLoaded );
}


//---------------------------------------------------------------------------
// DESC: Frees memory allocated to the feature objects located guid's list,
//       located points list, point list, and styles list.
// HIST: Original - twlangha - 01/13/99
// MISC:
//---------------------------------------------------------------------------

int CFeature::FreeAll ( )   // <=  Non-zero status code if error occurred.
{
    int sts = SUCCESS;

    Clear ( );
    
    FreeLocateGuids ( );
    FreeLocatePoints ( );

    return ( sts );
}


//---------------------------------------------------------------------------
// DESC: Frees memory allocated to feature object's point and style name lists,
//       Clears feature object's feature property member variables. Does NOT
//       Free located guid's and located points lists.
// HIST: Original - twlangha - 01/13/99
// MISC:
//---------------------------------------------------------------------------

int CFeature::Clear ( )     // <=  Non-zero status code if error occurred.
{
    int sts = SUCCESS;

    FreePoints ( );
    FreeStyles ( );
    FreePayItems ( );
    
    m_ftrP = NULL;
    m_srfP = NULL;
    memset ( &m_guid, 0, sizeof ( BeSQLite::BeGuid ) );
    m_nType = DTM_C_DTMREGFTR;
    wcscpy ( m_sName, L"" );
    wcscpy ( m_sDesc, L"" );
    wcscpy ( m_sParent, L"" );
    m_pntsP = NULL;
    m_nNumPnts = 0;
    m_dPntDensity = 0.0;
    m_nPntsAlc = 0;
    m_nPntAllocSize = 0;
    m_stylesP = NULL;
    m_nNumStyles = 0;
    m_nStylesAlc = 0;
    m_payItemsP = NULL;
    m_nNumPayItems = 0;
    m_nPayItemsAlc = 0;
    memset ( &m_flag, 0, sizeof ( unsigned char ) );
    m_pStyleInfo = NULL;
    return ( sts );
}


//---------------------------------------------------------------------------
// DESC: Frees memory allocated to the feature object's located guid's list.
// HIST: Original - twlangha - 01/13/99
// MISC:
//---------------------------------------------------------------------------

void CFeature::FreeLocateGuids ( )
{
    if ( m_locateGuidsP )
        free ( m_locateGuidsP );

    m_locateGuidsP = NULL;
    m_nNumLocateGuids = 0;
    m_nLocateGuidsAlc = 0;
}


//---------------------------------------------------------------------------
// DESC: Frees memory allocated to the feature object's located points list.
// HIST: Original - twlangha - 01/13/99
// MISC:
//---------------------------------------------------------------------------

void CFeature::FreeLocatePoints ( )
{
    if ( m_locatePointsP )
        free ( m_locatePointsP );

    m_locatePointsP = NULL;
    m_nNumLocatePoints = 0;
    m_nLocatePointsAlc = 0;

}


