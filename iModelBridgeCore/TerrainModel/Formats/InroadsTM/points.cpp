//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+

//---------------------------------------------------------------------------
// Include files
//---------------------------------------------------------------------------
#include "stdafx.h"

//---------------------------------------------------------------------------
// DESC: Adds points to a CFeature object.
// HIST: Original - twlangha - 01/13/99
// MISC:
//---------------------------------------------------------------------------

int CFeature::AddPoints     // <=  Non-zero status code if error occurred.
(
    CFeaturePnt *pntsP,     //  => 3D points to add to feature.
    long numPnts            //  => Number of points to add.
)
{
    int sts = SUCCESS;

    if ( !PointsAreLoaded ( ) )
        sts = LoadPoints ( );

    if ( sts == SUCCESS && ( sts = CheckPointsAllocation ( numPnts ) ) == SUCCESS )
    {
        memcpy ( &m_pntsP[m_nNumPnts], pntsP, sizeof ( CFeaturePnt ) * numPnts );
        m_nNumPnts += numPnts;
    }

    return ( sts );
}


//---------------------------------------------------------------------------
// DESC: Adds points to a CFeature object.
// HIST: Original - twlangha - 01/13/99
// MISC:
//---------------------------------------------------------------------------

int CFeature::AddPoints     // <=  Non-zero status code if error occurred.
(
    DPoint3d *pntsP,        //  => 3D points to add to feature.
    byte *flgsP,         //  => Point properties.
    long numPnts            //  => Number of points to add.
)
{
    CFeaturePnt *ftrPntsP = NULL;
    int sts = SUCCESS;

    if ( ( sts = aecFeature_dpnt3dsToFtrpnts ( &ftrPntsP, pntsP, flgsP, numPnts ) ) == SUCCESS )
        sts = AddPoints ( ftrPntsP, numPnts );

    if ( ftrPntsP )
        free ( ftrPntsP );

    return ( sts );
}


//---------------------------------------------------------------------------
// DESC: Deletes a set of points from the feature object's point list.
// HIST: Original - twlangha - 01/13/99
// MISC:
//---------------------------------------------------------------------------

int CFeature::RemovePoints  // <=  Non-zero status code if error occurred.
(
    long index,             //  => Index of first point to delete.
    long numPnts            //  => Number of points to delete.
)
{
    int sts = SUCCESS;

    if ( !PointsAreLoaded ( ) )
        sts = LoadPoints ( );

    if ( sts == SUCCESS && index >= 0 && index < m_nNumPnts )
    {
        if ( index + numPnts > m_nNumPnts )
            numPnts = m_nNumPnts - index;

        memcpy ( &m_pntsP[index], &m_pntsP[index + numPnts],
                 sizeof ( CFeaturePnt ) * ( m_nNumPnts - numPnts - index ) );

        m_nNumPnts -= numPnts;
    }
    else
        sts = AEC_E_IDXOUTOFRNG;

    return ( sts );
}

//---------------------------------------------------------------------------
// DESC: Returns the entire set of points from the feature object's
//       point list.
// HIST: Original - twlangha - 01/13/99
// MISC:
//---------------------------------------------------------------------------

int CFeature::GetPoints // <=  Non-zero status code if error occurred.
(
CFeaturePnt **ftrPntsPP, // <=  Feature's 3D coordinates and flags. CALLER MUST FREE.
long *numPntsP           // <=  Number of points returned.
)
{
    int sts = SUCCESS;

    if ( ftrPntsPP )
        *ftrPntsPP = NULL;

    *numPntsP = 0;

    if ( !PointsAreLoaded ( ) )
        sts = LoadPoints ( );

    if ( sts == SUCCESS )
    {
        if ( m_nNumPnts )
        {
            *ftrPntsPP = ( CFeaturePnt * ) calloc ( m_nNumPnts, sizeof ( CFeaturePnt ) );
            *numPntsP = m_nNumPnts;

            if ( !(*ftrPntsPP) )
                sts = AEC_E_MEMALF;
            else
                memcpy ( *ftrPntsPP, m_pntsP, sizeof ( CFeaturePnt ) * m_nNumPnts );
        }
    }
        
    return ( sts );
}

//---------------------------------------------------------------------------
// DESC: Returns the entire set of points from the feature object's
//       point list.
// HIST: Original - twlangha - 01/13/99
// MISC:
//---------------------------------------------------------------------------

int CFeature::GetPoints // <=  Non-zero status code if error occurred.
(
DPoint3d **pntsPP,       // <=  Feature's 3D coordinates. CALLER MUST FREE.
byte **pntFlgsPP,     // <=  Feature's point properties.  CALLER MUST FREE
long *numPntsP           // <=  Number of points returned.
)
{
    CFeaturePnt *ftrPntsP = NULL;
    int sts = SUCCESS;

    if (pntsPP )
        *pntsPP = NULL;

    if ( pntFlgsPP )
        *pntFlgsPP = NULL;


    if ( ( sts = GetPoints ( &ftrPntsP, numPntsP ) ) == SUCCESS )
    {
        sts = aecFeature_ftrpntsToDpnt3ds ( pntsPP, pntFlgsPP, ftrPntsP, *numPntsP );                
    }

    if ( ftrPntsP )
        free ( ftrPntsP );
        
    return ( sts );
}


//---------------------------------------------------------------------------
// DESC: Returns the number of 3D coordinates in the feature object's point
//       list.
// HIST: Original - twlangha - 01/13/99
// MISC:
//---------------------------------------------------------------------------

long CFeature::GetPointCount ( )    // <=  Number of points.
{    
    if ( !PointsAreLoaded ( ) )
        LoadPoints ( );

    return ( m_nNumPnts );
}


//---------------------------------------------------------------------------
// DESC: Compares amount of free space in the feature object's point list 
//       with the number of new points to be added.  Allocates new space
//       if required.
// HIST: Original - twlangha - 01/13/99
// MISC:
//---------------------------------------------------------------------------

int CFeature::CheckPointsAllocation // <=  Non-zero status code if
                                    //     error occurred.
(
    long numNewPnts                 //  => Number of new points to add.
)
{
    int sts = SUCCESS;

    if ( !PointsAreLoaded ( ) )
        sts = LoadPoints ( );

    if ( sts == SUCCESS && m_nNumPnts + numNewPnts >= m_nPntsAlc )
    {
        long nEmptyPntsAllocSize = m_nPntAllocSize;

        // If the default allocation size is zero, calculate one 10%
        // larger than the number of points to be contained.
        if ( nEmptyPntsAllocSize <= 0 )
            nEmptyPntsAllocSize = (long)((m_nPntsAlc + numNewPnts) / 10);

        // Make sure we don't come up with a negative value;
        if ( nEmptyPntsAllocSize < 0 )
            nEmptyPntsAllocSize = 0;

        // Limit the increase in allocation to 30 thousand points.
        if ( nEmptyPntsAllocSize > 30000 )
            nEmptyPntsAllocSize = 30000;

        // Make sure at least one point gets allocated to avoid corruption problems.
        if ( m_nPntsAlc + numNewPnts + nEmptyPntsAllocSize <= 0 )
            nEmptyPntsAllocSize = 1;

        m_pntsP = ( CFeaturePnt * ) realloc ( m_pntsP, ( m_nPntsAlc + numNewPnts + nEmptyPntsAllocSize ) * sizeof ( CFeaturePnt ) );
        m_nPntsAlc += numNewPnts + nEmptyPntsAllocSize;
    }

    if ( !m_pntsP )
        sts = AEC_E_MEMALF;
    
    return ( sts );    
}


//---------------------------------------------------------------------------
// DESC: Frees all memory allocated to the feature object's point list.
// HIST: Original - twlangha - 01/13/99
// MISC:
//---------------------------------------------------------------------------

void CFeature::FreePoints ( )
{
    if ( m_pntsP )
        free ( m_pntsP );

    m_pntsP = NULL;
    m_nNumPnts = 0;
    m_nPntsAlc = 0;
}


//---------------------------------------------------------------------------
// DESC: Loads the points of the feature contained by the feature object
//       from the feature's parent DTM.
// HIST: Original - twlangha - 01/13/99
// MISC:
//---------------------------------------------------------------------------

int CFeature::LoadPoints ( )    // <=  Non-zero status code if error occurred.
{
    long nType;
    int sts = SUCCESS;

    if ( !PointsAreLoaded ( ) && m_ftrP )
    {
        sts = aecDTM_getFeatureInfo ( m_ftrP, m_srfP, 
                                      NULL, &nType, NULL, NULL, NULL, &m_pntsP, &m_nNumPnts, NULL,
                                      NULL, NULL, NULL, NULL, NULL );

        m_nPntsAlc = m_nNumPnts;

        if ( sts == SUCCESS )
        {
            if ( nType == DTM_C_DTMREGFTR )
            {
                long oldType = m_nType;
                m_nType = nType;
                ValidatePoints ( );
                m_nType = oldType;
            }
            else
                ReversePointDisconFlags ( FALSE );
        }
    }

    return ( sts );
}


//---------------------------------------------------------------------------
// DESC: Checks to see if the points of the feature contained by the feature
//       object have been loaded into the feature object from the feature's
//       parent DTM.
// HIST: Original - twlangha - 01/13/99
// MISC:
//---------------------------------------------------------------------------

boolean CFeature::PointsAreLoaded ( )   // <=  TRUE or FALSE
{
    boolean bLoaded = FALSE;

    if ( m_pntsP )
        bLoaded = TRUE;

    return ( bLoaded );
}


//---------------------------------------------------------------------------
// DESC:
// HIST: Original - twlangha - 01/13/99
// MISC:
//---------------------------------------------------------------------------

void CFeature::ValidatePoints ( )
{

    if ( m_nType == DTM_C_DTMREGFTR )
    {
        for ( int i = 0; i < m_nNumPnts; i++ )
            m_pntsP[i].flg &= ~FTR_C_DISCNT;
    }
    else
    {
        // Validate discontinuity flags
        if ( m_nNumPnts > 0 )
        {
            for ( int i = GetPointCount() - 1; i >= 0; i-- )
            {
                if ( 
                     ( ( i == GetPointCount() - 1 || ( m_pntsP[i].flg & FTR_C_DISCNT ) ) && 
                       (m_pntsP[i-1].flg & FTR_C_DISCNT ) )                                       
                                                                                            ||
                     ( i == 0 && ( m_pntsP[i].flg & FTR_C_DISCNT ) )
                   )            
                {
                    RemovePoints ( i, 1 );
                }
            }

            if ( GetPointCount ( ) > 0 )
                m_pntsP[GetPointCount() - 1].flg &= ~FTR_C_DISCNT;
        }
    }
}


//---------------------------------------------------------------------------
// DESC:
// HIST: Original - twlangha - 01/13/99
// MISC:
//---------------------------------------------------------------------------

void CFeature::ReversePointDisconFlags ( BOOL bFeatureToDTM )
{
    if ( m_nType == DTM_C_DTMREGFTR )
    {
        ValidatePoints();
    }
    else
    {
        if ( bFeatureToDTM )
        {
            ValidatePoints();

            // Starting with next to last point, search backwards. 
            // Find each discontinuity and set the subsequent PUD
            // flag to x8.
            for ( int i = m_nNumPnts - 2; i >= 0; i-- )
            {
                if ( m_pntsP[i].flg & FTR_C_DISCNT )
                    m_pntsP[i + 1].flg &= ~DTM_C_PNTPUD;
                else
                    m_pntsP[i + 1].flg |= DTM_C_PNTPUD;
            }

            // Set the first, second and last PUD flags to 0, x8, x8.
            if ( m_nNumPnts > 0 )
                m_pntsP[0].flg &= ~DTM_C_PNTPUD;

            if ( m_nNumPnts > 1 )
            {
                m_pntsP[1].flg |= DTM_C_PNTPUD;
                m_pntsP[m_nNumPnts - 1].flg |= DTM_C_PNTPUD;
            }
        }
        else
        {
            // Starting with the second point, find each PUD flag set to 
            // 0 and set the previous points flag to be a discontinuity.
            for ( int i = 1; i < m_nNumPnts; i++ )
            {
                if ( m_pntsP[i].flg & DTM_C_PNTPUD )
                    m_pntsP[i - 1].flg &= ~FTR_C_DISCNT;
                else
                    m_pntsP[i - 1].flg |= FTR_C_DISCNT;
            }

            // The last point should never be a discontinuity.
            if ( m_nNumPnts > 0 )
                m_pntsP[m_nNumPnts - 1].flg &= ~FTR_C_DISCNT;

            ValidatePoints();
        }
    }     
}


