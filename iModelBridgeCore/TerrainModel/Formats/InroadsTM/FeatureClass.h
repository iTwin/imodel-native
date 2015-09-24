//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
#pragma once

#include <vector>

using namespace std;

#include "aecuti.h"
#include "dtmstr.h"
#include "FeatureDefs.h"

// Information needed for feature annotation
#define ANN_PNT_NAME        0x1     // actually id/index
#define ANN_PNT_NORTHING    0x2
#define ANN_PNT_EASTING     0x4
#define ANN_PNT_ELEVATION   0x8
#define ANN_PNT_DESCRIPTION 0x10
#define ANN_PNT_STYLE       0x20
#define ANN_PNT_STATION     0x40
#define ANN_PNT_OFFSET      0x80
#define ANN_PNT_FTR_NAME    0x100
#define ANN_PNT_FTRSTATION  0x200
#define ANN_PNT_ITEMS       10       // number in the above list

#define ANN_SEG_NAME         0x1     // actually id/index
#define ANN_SEG_LENGTH       0x2
#define ANN_SEG_BEARING      0x4
#define ANN_SEG_SLOPE        0x8
#define ANN_SEG_DESCRIPTION  0x10
#define ANN_SEG_STYLE        0x20
#define ANN_SEG_FTR_NAME     0x40
#define ANN_SEG_SLOPELENGTH  0x80
#define ANN_SEG_ITEMS        8       // number in the above list


typedef struct featureAnnotationSettings
{
    BOOL m_displayPnt;
    BOOL m_displaySeg;
    BOOL m_displaySlope;
    BOOL m_displayCrests;
    BOOL m_displaySags;

    int m_pntLocation;
    int m_pntInfo;
    int m_pntPos[ANN_PNT_ITEMS][2];
    int m_pntNorthingPrecision;
    int m_pntEastingPrecision;
    int m_pntElevationPrecision;
    int m_pntStationPrecision;
    int m_pntFtrStationPrecision;
    int m_pntOffsetPrecision;
    int m_pntStationFormat;
    int m_pntFtrStationFormat;
    double m_pntPointInterval;
    double m_pntStartStation;
    double m_pntStopStation;
    int m_pntAlongFeature;
    BOOL m_pntIncludeVertices;
    BOOL m_pntDropEquation;
    void *m_pntHalg;

    int m_segLocation;
    int m_segInfo;
    int m_segPos[ANN_SEG_ITEMS][2];
    int m_segLengthPrecision;
    int m_segBearingPrecision;
    int m_segSlopePrecision;
    int m_segBearingFormat;
    int m_segSlopeFormat;
    int m_segSlopeLength;
    int m_segSlopeLengthPrecision;

    CString m_slopeCellName;
    double m_slopeInterval;
    double m_slopeOffset;
    int m_slopeSegment;

    CString m_strCrestsCellName;
    CString m_strSagsCellName;

    double m_crestsOffset;
    double m_sagsOffset;

    CString m_strIndexPrefixPTS;
    CString m_strNorthingPrefixPTS;
    CString m_strEastingPrefixPTS;
    CString m_strElevationPrefixPTS;
    CString m_strFeatureNamePrefixPTS;
    CString m_strDescriptionPrefixPTS;
    CString m_strStylePrefixPTS;
    CString m_strStationPrefixPTS;
    CString m_strFtrStationPrefixPTS;
    CString m_strOffsetPrefixPTS;
    CString m_strIndexSuffixPTS;
    CString m_strNorthingSuffixPTS;
    CString m_strEastingSuffixPTS;
    CString m_strElevationSuffixPTS;
    CString m_strFeatureNameSuffixPTS;
    CString m_strDescriptionSuffixPTS;
    CString m_strStyleSuffixPTS;
    CString m_strStationSuffixPTS;
    CString m_strFtrStationSuffixPTS;
    CString m_strOffsetSuffixPTS;

    CString m_strIndexPrefixSEG;
    CString m_strLengthPrefixSEG;
    CString m_strBearingPrefixSEG;
    CString m_strSlopeLengthPrefixSEG;
    CString m_strSlopePrefixSEG;
    CString m_strFeatureNamePrefixSEG;
    CString m_strDescriptionPrefixSEG;
    CString m_strStylePrefixSEG;
    CString m_strIndexSuffixSEG;
    CString m_strLengthSuffixSEG;
    CString m_strBearingSuffixSEG;
    CString m_strSlopeLengthSuffixSEG;
    CString m_strSlopeSuffixSEG;
    CString m_strFeatureNameSuffixSEG;
    CString m_strDescriptionSuffixSEG;
    CString m_strStyleSuffixSEG;
}	FeatureAnnotationSettings;


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
    int     LoadFromDTMByGuid ( BeGuid *guidP, struct CIVdtmsrf *srfP = NULL );
    int     LoadFromDTMExteriorBoundary ( struct CIVdtmsrf *srfP = NULL );
    int     SaveToDTM ( struct CIVdtmsrf *srfP = NULL, long opt = 0, BOOL bReTin = TRUE );
    int     SaveToDTM ( long opt, BOOL bReTin = TRUE );
    int     SaveAsNewToDTM ( struct CIVdtmsrf *srfP = NULL, long opt = 0 );
    int     SaveAsNewToDTM ( long opt );

    // Methods for locating features and feature vertexes by XYZ coordinate.
    void    LocateByDPInit ( BOOL bClosed = FALSE, BOOL bClosedXY = FALSE );
    int     LocateByDP ( DPoint3d *dpP, struct CIVdtmsrf *srfP );
    int     LocateByDP ( DPoint3d *dpP, struct CIVdtmsrf **srfsPP, int numSrfs );
    void    LocatePointByDPInit ( );
    int     LocatePointByDP ( long *indexP, CFeaturePnt *pntP, DPoint3d *dpP, BOOL bProjectToPlane = TRUE );
    int     LocatePointByDP ( long *indexP, DPoint3d *pntP, unsigned char *flg, DPoint3d *dpP );
    int     LocatePointByDP ( long *indexP, DPoint3d *pntP, DPoint3d *dpP );

    // Methods for retrieving and seting feature object's properties.
    int     GetType ( );
    void    GetType ( long *typeP );
    boolean IsTriangulationEnabled ( );

    // Point manipulation methods.
    int     AddPoints ( CFeaturePnt *pntsP, long numPnts );
    int     AddPoints ( DPoint3d *pntsP, unsigned char *flgsP, long numPnts );
    int     AddPoints ( DPoint3d *pntsP, long numPnts );
    int     InsertPoints ( long index, CFeaturePnt *pntsP, long numPnts );
    int     InsertPoints ( long index, DPoint3d *pntsP, unsigned char *flgsP, long numPnts );
    int     InsertPoints ( long index, DPoint3d *pntsP, long numPnts );
    int     MovePoints ( long index, long numPnts, double deltaX, double deltaY, double deltaZ );
    int     RemovePoints ( long index, long numPnts );
    int     FindPoint( DPoint3d *pntP, long* index );
    int     GetPointAndIndex ( DPoint3d *pntP, long *index, DPoint3d *prjPntP );
    int     GetPoint ( long index, CFeaturePnt *ftrPntP );
    int     GetPoint ( long index, DPoint3d *pntP, unsigned char *flgP );
    int     GetPoint ( long index, DPoint3d *pntP );
    int     GetPoints ( CFeaturePnt **flgPntsPP, long *numPntsP );
    int     GetPoints ( DPoint3d **pntsPP, unsigned char **pntFlgsPP, long *numPntsP );
    int     GetPoints ( DPoint3d **pntsPP, long *numPntsP );
    int     GetPoints ( std::vector<CFeaturePnt>& points );
    int     GetPoints ( std::vector<DPoint3d>& points );
    int     GetLength ( double *dLenP );
    int     Get2dLength ( double *dLenP );
    int     SetPoint ( long index, CFeaturePnt *pntP );
    int     SetPoint ( long index, DPoint3d *pntP, unsigned char *pntFlgP = NULL );
    int     SetDiscontinuity ( long index, BOOL bDiscnt );
    BOOL    IsDiscontinuity ( long index );
    BOOL    IsClosed ( BOOL bTestForClosedSections = FALSE, BOOL bTestXYonly = FALSE );
    BOOL    IsClosedXY ( BOOL bTestForClosedSections = FALSE );
    long    GetPointCount ( );
    void    GetPointDensity ( double *densityP );
    void    SetPointDensity ( double density );
    void    SetPointAllocSize ( long pntAllocSize );
    void    ValidatePoints();
    long    GetSectionCount ( );
    int     GetSectionStart ( long sectionIndex, long *pntIndexP );
    long    GetSectionPointCount ( long index );
    int     GetSectionPoint ( long index, long pntIndex, CFeaturePnt *ftrPntP );
    int     GetSectionPoint ( long index, long pntIndex, DPoint3d *pntP, unsigned char *flgP );
    int     GetSectionPoint ( long index, long pntIndex, DPoint3d *pntP );
    int     GetSectionPoints ( long index, CFeaturePnt **flgPntsPP, long *numPntsP );
    int     GetSectionPoints ( long index, DPoint3d **pntsPP, unsigned char **pntFlgsPP, long *numPntsP );
    int     GetSectionPoints ( long index, DPoint3d **pntsPP, long *numPntsP );
    int     GetSectionPoints ( long index, std::vector<DPoint3d>& points );
    int     GetSectionLength ( double *dLenP, long index );
    int     Get2dSectionLength ( double *dLenP, long index );
    BOOL    IsSectionClosedXY ( long index );
    BOOL    IsSectionClosed ( long index, BOOL bTestXYonly = FALSE );
    int     DeletePointsAlongFeature( double startDis, double  stopDis );
    int     DistanceAtPoint ( double *distance, DPoint3d *position, DPoint3d *intPnt1, double tol, BOOL bIgnoreDiscon = FALSE, BOOL bHorzDist = FALSE );
    BOOL    FindClosestIntersection ( DPoint3d *intersection, DPoint3d *origin, DPoint3d *end, BOOL bIgnoreDiscon = FALSE );

    void    FreeLocateGuids ( );
    void    FreeLocatePoints ( );
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
    BeGuid              m_guid;
    long              m_nType;
    wchar_t           m_sName[DTM_C_NAMSIZ];
    wchar_t           m_sDesc[DTM_C_NAMSIZ];
    wchar_t           m_sParent[DTM_C_NAMSIZ];
    unsigned char              m_flag;

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

    // Located guids and points lists.
    BeGuid             *m_locateGuidsP;
    long              m_nNumLocateGuids;
    long              m_nLocateGuidsAlc;
    BOOL              m_bLocateClosed;
    BOOL              m_bLocateClosedXY;

    long             *m_locatePointsP;
    long              m_nNumLocatePoints;
    long              m_nLocatePointsAlc;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.
