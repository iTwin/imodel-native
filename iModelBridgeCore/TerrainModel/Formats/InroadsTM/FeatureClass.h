//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
#pragma once

#include <vector>

using namespace std;

#include "aecuti.h"
#include "dtmstr.h"
#include "FeatureDefs.h"

//---------------------------------------------------------------------------
// DESC: CFeature declaration
// HIST: Original - twlangha - 1/8/99
// MISC: Class for creating, loading, manipulating, and saving features.
//---------------------------------------------------------------------------
class CFeature
{
public:
    // Pointer to DTM feature and surface containing feature.
    struct CIVdtmftr *m_ftrP;
    struct CIVdtmsrf *m_srfP;

    // Constructor/Destructor
    CFeature();
    ~CFeature();

    // Methods for loading and saving features.
    int     LoadFromDTMByGuid ( GUID *guidP, struct CIVdtmsrf *srfP = NULL );
    int     LoadFromDTMExteriorBoundary ( struct CIVdtmsrf *srfP = NULL );
    int     SaveToDTM ( struct CIVdtmsrf *srfP = NULL, long opt = 0, BOOL bReTin = TRUE );
    int     SaveAsNewToDTM ( struct CIVdtmsrf *srfP = NULL, long opt = 0 );

    // Methods for retrieving and seting feature object's properties.
    boolean IsTriangulationEnabled ( );

    // Point manipulation methods.
    int     AddPoints ( CFeaturePnt *pntsP, long numPnts );
    int     AddPoints ( DPoint3d *pntsP, byte *flgsP, long numPnts );
    int     AddPoints ( DPoint3d *pntsP, long numPnts );
    int     RemovePoints ( long index, long numPnts );
    int     GetPoints ( CFeaturePnt **flgPntsPP, long *numPntsP );
    int     GetPoints ( DPoint3d **pntsPP, byte **pntFlgsPP, long *numPntsP );
    int     GetPoints ( DPoint3d **pntsPP, long *numPntsP );
    int     GetPoints ( std::vector<CFeaturePnt>& points );
    int     GetPoints ( std::vector<DPoint3d>& points );
    long    GetPointCount ( );
    void    ValidatePoints();

    int     FreeAll ( );
    int     Clear ( );

    void    *m_pStyleInfo;

public: // IPlanLinearEntity implementation
    // Point allocation methods.
    int     CheckPointsAllocation( long numNewPnts );
    void    FreePoints ( );
    int     LoadPoints ( );
    boolean PointsAreLoaded ( );

    // Style allocation methods.
    void    FreeStyles ( );
    int     LoadStyles ( );
    boolean StylesAreLoaded ( );

    // PayItem allocation methods.
    void    FreePayItems ( );
    int     LoadPayItems ( );
    boolean PayItemsAreLoaded ( );

    // Point Flags Validation
    void ReversePointDisconFlags ( BOOL bFeatureToDTM );

    // Member variables to hold feature properties.
    GUID              m_guid;
    long              m_nType;
    wchar_t           m_sName[DTM_C_NAMSIZ];
    wchar_t           m_sDesc[DTM_C_NAMSIZ];
    wchar_t           m_sParent[DTM_C_NAMSIZ];
    byte              m_flag;

    // Feature points member variables.
    CFeaturePnt      *m_pntsP;
    long              m_nNumPnts;
    long              m_nPntsAlc;
    double            m_dPntDensity;
    long              m_nPntAllocSize;

    // Feature styles member variables.
    CIVdtmstynam     *m_stylesP;
    long              m_nNumStyles;
    long              m_nStylesAlc;

    // Feature pay item member variables.
    CIVdtmpaynam     *m_payItemsP;
    long              m_nNumPayItems;
    long              m_nPayItemsAlc;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.
