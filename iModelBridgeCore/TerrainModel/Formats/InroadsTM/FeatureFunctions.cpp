//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+

//---------------------------------------------------------------------------
// Include files
//---------------------------------------------------------------------------
#include "stdafx.h"

//---------------------------------------------------------------------------
// Macros and constants
//---------------------------------------------------------------------------

#define INPUT_INRANGE(a,b,c) (( ( ((a)<=(c))&&((c)<=(b)) ) || ( ((a)>=(c))&&((c)>=(b)) ) ) ? 1 : 0 )

//---------------------------------------------------------------------------
// Private identifiers
//---------------------------------------------------------------------------
FeatureListItem *FLIhashP = NULL;


//---------------------------------------------------------------------------
// DESC: Creates a new CFeature object and returns its handle.
// HIST: Original - twlangha - 01/13/99
// MISC:
//---------------------------------------------------------------------------

int aecFeature_create // <=  Non-zero status code if error occurred.
(
    FeatureHandle *ftrHndlP // <=  Handle to created object.  CALLER DESTROY.
)
{
    int sts = SUCCESS;
    *((CFeature **)ftrHndlP) = new CFeature ( );

    return ( sts );
}


//---------------------------------------------------------------------------
// DESC: Destroys a CFeature object created by aecFeature_create.
// HIST: Original - twlangha - 01/13/99
// MISC:
//---------------------------------------------------------------------------

void aecFeature_destroy
(
    FeatureHandle ftrHndl   //  => Handle to object to be destroyed.
)
{
    delete ( (CFeature *)ftrHndl );
}


//---------------------------------------------------------------------------
// DESC: If the specified surface contains an exterior boundary, it is loaded
//       into the feature object.
// HIST: Original - twlangha - 07/23/99
// MISC:
//---------------------------------------------------------------------------

int aecFeature_loadFromDTMExteriorBoundary // <= Non-zero status code if error occurred.
(
    FeatureHandle ftrHndl,        //  => Handle to feature object.
    struct CIVdtmsrf *srfP        //  => Surface to be searched (NULL indicates
                                  //     the active surface is to be searched).
)
{
    return ( ((CFeature *)ftrHndl)->LoadFromDTMExteriorBoundary ( srfP ) );
}


//---------------------------------------------------------------------------
// DESC: Searches the specified surface (active if srfP is NULL) for a
//       feature whose GUID matches that of the feature object.  If found,
//       the surface feature will be overwritten with the contents of the
//       feature object.  If not found, the contents of the feature object
//       will be written to the surface as a new feature.  The feature will
//       be assigned a newly generated GUID and a name unique to the
//       specified surface.
//       The opt argument can be used to provide additional
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
// HIST: Original - twlangha - 01/13/99
// MISC:
//---------------------------------------------------------------------------

int aecFeature_saveToDTM // <=  Non-zero status code if error occurred.
(
    FeatureHandle ftrHndl,  //  => Handle to feature object.
    struct CIVdtmsrf *srfP, //  => Target surface (active if NULL).
    long opt,               //  => Options
    BOOL bReTin             //  => Retriangulate feature ( usually TRUE )
)
{
    return ( ((CFeature *)ftrHndl)->SaveToDTM ( srfP, opt, bReTin ) );
}


//---------------------------------------------------------------------------
// DESC: Adds points to a CFeature object.
// HIST: Original - twlangha - 01/13/99
// MISC:
//---------------------------------------------------------------------------

int aecFeature_addPoints    // <=  Non-zero status code if error occurred.
(
    FeatureHandle ftrHndl,  //  => Handle to feature object.
    CFeaturePnt *ftrPntsP,  //  => feature points to add to feature (or NULL).
    DPoint3d *pntsP,        //  => 3D points to add to feature (or NULL).
    byte *flgsP,         //  => point properties (or NULL).
    long numPnts            //  => Number of points to add.
)
{
    int sts = SUCCESS;

    if ( ftrPntsP )        
        sts = ((CFeature *)ftrHndl)->AddPoints ( ftrPntsP, numPnts );
    else if ( pntsP )
        sts = ((CFeature *)ftrHndl)->AddPoints ( pntsP, flgsP, numPnts );

    return ( sts );
}


//---------------------------------------------------------------------------
// DESC: Returns the number of 3D coordinates in the feature object's point
//       list.
// HIST: Original - twlangha - 01/13/99
// MISC:
//---------------------------------------------------------------------------

int aecFeature_getPointCount // <=  Number of points.
(
    FeatureHandle ftrHndl       //  => Handle to feature object.
)
{
    return ( ((CFeature *)ftrHndl)->GetPointCount ( ) );
}


//---------------------------------------------------------------------------
// DESC: Returns the entire set of points from the feature object's
//       point list.
// HIST: Original - twlangha - 01/13/99
// MISC:
//---------------------------------------------------------------------------

int aecFeature_getPoints     // <=  Non-zero status code if error occurred.
(
    FeatureHandle ftrHndl,   //  => Handle to feature object.
    CFeaturePnt **ftrPntsPP, // <=  Feature's 3D coordinates and flags. CALLER MUST FREE.
    DPoint3d **pntsPP,       // <=  Feature's 3D coordinates. CALLER MUST FREE.
    byte **pntFlgsPP,     // <=  Feature's point properties.  CALLER MUST FREE
    long *numPntsP           // <=  Number of points returned.
)
{
    int sts = SUCCESS;

    if ( ftrPntsPP )
    {
        if ( (sts = ((CFeature *)ftrHndl)->GetPoints ( ftrPntsPP, numPntsP ) ) == SUCCESS )
            sts = aecFeature_ftrpntsToDpnt3ds ( pntsPP, pntFlgsPP, *ftrPntsPP, *numPntsP );                
    }
    else
       sts = ((CFeature *)ftrHndl)->GetPoints ( pntsPP, pntFlgsPP, numPntsP );

    return ( sts );        
}


//---------------------------------------------------------------------------
// DESC: Deletes a set of points from the feature object's point list.
// HIST: Original - twlangha - 01/13/99
// MISC:
//---------------------------------------------------------------------------

int aecFeature_removePoints // <=  Non-zero status code if error occurred.
(
    FeatureHandle ftrHndl,  //  => Handle to feature object.
    long index,             //  => Index of first point to delete.
    long numPnts            //  => Number of points to delete.
)
{
    return ( ((CFeature *)ftrHndl)->RemovePoints ( index, numPnts ) );
}


//---------------------------------------------------------------------------
// DESC: Converts a set of DPoint3ds and Point flags to CFeaturePoint's.
// HIST: Original - twlangha - 01/13/99
// MISC:
//---------------------------------------------------------------------------

int aecFeature_dpnt3dsToFtrpnts
(
    CFeaturePnt **ftrPntsPP,
    DPoint3d *dpnt3dsP,
    byte *pntflgsP,
    long numPnts
)
{
    int sts = SUCCESS;
    int i;

    if ( ( *ftrPntsPP = ( CFeaturePnt * ) calloc ( numPnts, sizeof ( CFeaturePnt ) ) ) != NULL )
    {
        for ( i = 0; i < numPnts; i++ )
        {
            memcpy ( &((*ftrPntsPP)[i].cor), &dpnt3dsP[i], sizeof ( DPoint3d ) );

            if ( pntflgsP )
                (*ftrPntsPP)[i].flg = pntflgsP[i];         
        }
    }
    else
        sts = AEC_E_MEMALF;

    return ( sts ); 
}   


//---------------------------------------------------------------------------
// DESC: Converts a set of DPoint3ds and Point flags to CFeaturePoint's.
// HIST: Original - twlangha - 01/13/99
// MISC:
//---------------------------------------------------------------------------

int aecFeature_ftrpntsToDpnt3ds
(
    DPoint3d **dpnt3dsPP,
    byte **pntflgsPP,
    CFeaturePnt *ftrPntsP,
    long numPnts
)
{
    int sts = SUCCESS;
    int i;

    if ( dpnt3dsPP )
        if ( ( *dpnt3dsPP = ( DPoint3d * ) calloc ( numPnts, sizeof ( DPoint3d ) ) ) == NULL )
            sts = AEC_E_MEMALF;

    if ( sts == SUCCESS && pntflgsPP )
        if ( ( *pntflgsPP = ( byte * ) calloc ( numPnts, sizeof ( byte ) ) ) == NULL )
            sts = AEC_E_MEMALF;

    if ( sts == SUCCESS )
    {
        for ( i = 0; i < numPnts; i++ )
        {
            if ( dpnt3dsPP )
                memcpy ( &(*dpnt3dsPP)[i], &ftrPntsP[i].cor, sizeof ( DPoint3d ) );

            if ( pntflgsPP )
                memcpy ( &(*pntflgsPP)[i], &ftrPntsP[i].flg, sizeof ( byte ) );
        }
    }

    return ( sts ); 
} 
                        

