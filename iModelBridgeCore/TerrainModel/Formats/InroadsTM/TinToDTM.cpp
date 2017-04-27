//---------------------------------------------------------------------------+
// $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+

/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"
#include "inroadstm.h"
#include    "TintoDTM.h"
#include    "dtmedtfnc.h"
#include    "dtmflgfnc.h"
#include    "dtmutifnc.h"
#include    "dtmsnd.h"
#include    "vector"
#include    "string"

typedef struct
{
    DPoint3d cor;
    long index;
    BOOL bAdded;
    unsigned char flg;
} TINtoDTMPoint;

static wchar_t exteriorBoundaryName[DTM_C_NAMSIZ];
static wchar_t exteriorBoundaryStyleName[CIV_C_NAMSIZ];

typedef CMap<long, long&, TINtoDTMPoint, TINtoDTMPoint&> TINtoDTMPointMap;

std::vector<__int64> featureIds;
std::vector<std::wstring> featureNames;
std::vector<std::wstring> featureDefinitions;

int (*pConvertTinToDTMCallbackFunc)
(
    WCharCP tinP,
    int  (*tinStatsCallBackFunctionP)(long numRandomPoints,long numFeaturePoints,long numTriangles,long NumFeatures),
    int  (*tinRandomPointsCallBackFunctionP)(long pntIndex,double X,double Y,double Z),
    int  (*tinFeaturePointsCallBackFunctionP)(long pntIndex,double X,double Y,double Z),
    int  (*tinTrianglesCallBackFunctionP)(long trgIndex,long pntIndex1,long pntIndex2,long pntIndex3,long voidTriangle,long side1TrgIndex,long side2TrgIndex,long side3TrgIndex),
    int  (*tinFeaturesCallBackFunctionP)(long dtmFeatureType,__int64 dtmUsertag,__int64 dtmFeatureId,long *pointIndicesP,long numPointIndices)
);

void (__stdcall *pConvertTinToDTMCallbackFuncStdcall)
(
    void *tinP,
    int  (*tinStatsCallBackFunctionP)(long numRandomPoints,long numFeaturePoints,long numTriangles,long NumFeatures),
    int  (*tinRandomPointsCallBackFunctionP)(long pntIndex,double X,double Y,double Z),
    int  (*tinFeaturePointsCallBackFunctionP)(long pntIndex,double X,double Y,double Z),
    int  (*tinTrianglesCallBackFunctionP)(long trgIndex,long pntIndex1,long pntIndex2,long pntIndex3,long voidTriangle,long side1TrgIndex,long side2TrgIndex,long side3TrgIndex),
    int  (*tinFeaturesCallBackFunctionP)(long dtmFeatureType,__int64 dtmUsertag,__int64 dtmFeatureId,long *pointIndicesP,long numPointIndices)
);

static struct CIVdtmsrf *newSrfP = NULL;

static TINtoDTMPointMap* mapPointsByIndexP  = NULL;
static CMapPtrToPtr* mapDtmPointsByIndexP = NULL;
static CMapPtrToPtr* mapDtmTinsByIndexP = NULL;
static unsigned long ftrNameCnt = 0;
static BOOL bStatsCallbackOn = FALSE;
static BOOL bRandomPointsCallbackOn = FALSE;
static BOOL bFeaturePointsCallbackOn = FALSE;
static BOOL bTrianglesCallbackOn = FALSE;
static BOOL bFeaturesCallbackOn = FALSE;

int addDtmFeature ( struct CIVdtmpnt **dtmPntsPP, wchar_t *ftrName, wchar_t *ftrDesc, int ftrType, wchar_t *styleNamesP, int numStyles, DPoint3d *pntsP, long numPoints )
{
    struct CIVdtmftr *ftrP = NULL;
    InroadsGuid guid;
    BOOL bPntsAreValid = FALSE;
    int sts = SUCCESS;

    sts = aecDTM_addFeature ( &guid, newSrfP, 0, ftrName, ftrDesc, NULL, ftrType, numPoints, NULL, pntsP, NULL, 0.0,
                              (CIVdtmstynam *)styleNamesP, numStyles, NULL, 0, (unsigned char)0, FALSE, FALSE );

    aecDTM_findFeatureByGUID ( &ftrP, newSrfP, &guid );

    if ( ftrP != NULL && ftrP->numPnts == numPoints )
    {
        bPntsAreValid = TRUE;

        for ( int i = 0; i < ftrP->numPnts && bPntsAreValid; i++ )
        {
            if ( !VEQUAL ( ftrP->p1[i].cor, pntsP[i], AEC_C_TOL2 ) )
                bPntsAreValid = FALSE;
        }
    }

    if ( bPntsAreValid )
    {
        *dtmPntsPP = ftrP->p1;
    }
    else
    {
        if ( ftrP )
        {
            for ( int i = 0; i < ftrP->numPnts; i++ )
                aecDTM_setPointDeletedFlag ( &ftrP->p1[i] );
        }

        long pntType = aecDTM_pointFileFromFeatureFile ( ftrType );
        pntType = pntType & ADDPNT_TYPMSK;
        long ind;
        aecDTM_getSurfaceFileIndex ( &ind, pntType );
        sts = aecDTM_addPointsToFile ( dtmPntsPP, newSrfP, newSrfP->pntf[ind], pntType, FALSE, numPoints, pntsP );

        if ( ftrP )
        {
            ftrP->p1 = *dtmPntsPP;
            ftrP->numPnts = numPoints;
        }
    }

    ASSERT ( sts == SUCCESS );

    return sts;
}

int tinStatsCallBackFunction (long numRandomPoints,long numFeaturePoints,long numTriangles, long numFeatures)
{
    if ( bStatsCallbackOn == FALSE )
        return SUCCESS;

    //bcdtmWrite_message(0,0,0,"Number Random  Points = %8ld",numRandomPoints) ;
    //bcdtmWrite_message(0,0,0,"Number Feature Points = %8ld",numFeaturePoints) ;
    //bcdtmWrite_message(0,0,0,"Number Of Triangles   = %8ld",numTriangles) ;
    return(SUCCESS) ;
}

int tinRandomPointsCallBackFunction (long pntIndex,double X,double Y,double Z)
{
    if ( bRandomPointsCallbackOn == FALSE )
        return SUCCESS;

    static int pntNum=0 ;
    //bcdtmWrite_message(0,0,0,"Random Point[%8ld] ** index = %8ld ** %12.3lf %12.3lf %10.4lf",pntNum,pntIndex,X,Y,Z) ;
    ++pntNum ;

    TINtoDTMPoint tinToDtmPnt;
    memset ( &tinToDtmPnt, 0, sizeof ( tinToDtmPnt ) );
    tinToDtmPnt.cor.x = X;
    tinToDtmPnt.cor.y = Y;
    tinToDtmPnt.cor.z = Z;
    tinToDtmPnt.index = pntIndex;
    mapPointsByIndexP->SetAt ( tinToDtmPnt.index, tinToDtmPnt );

    return(SUCCESS) ;
}

int tinFeaturePointsCallBackFunction (long pntIndex,double X,double Y,double Z)
{
    if ( bFeaturePointsCallbackOn == FALSE )
        return SUCCESS;

    static int pntNum=0 ;
    //bcdtmWrite_message(0,0,0,"Feature Point[%8ld] ** index = %8ld ** %12.3lf %12.3lf %10.4lf",pntNum,pntIndex,X,Y,Z) ;
    ++pntNum ;

    TINtoDTMPoint tinToDtmPnt;
    memset ( &tinToDtmPnt, 0, sizeof ( tinToDtmPnt ) );
    tinToDtmPnt.cor.x = X;
    tinToDtmPnt.cor.y = Y;
    tinToDtmPnt.cor.z = Z;
    tinToDtmPnt.index = pntIndex;
    mapPointsByIndexP->SetAt ( tinToDtmPnt.index, tinToDtmPnt );

    return(SUCCESS) ;
}

int tinTrianglesCallBackFunction(long trgIndex,long pntIndex1,long pntIndex2,long pntIndex3,long voidTriangle,long side1TrgIndex,long side2TrgIndex,long side3TrgIndex)
{
    if ( bTrianglesCallbackOn == FALSE )
        return SUCCESS;


/*
**  Notes :-
**
**  1. Triangles Are Passed Back In A Clockwise Direction
**  2. Get Triangle Coordinates From Point Index
**  3. VoidTriangle = < 0 Not Void , 1 Void Triangle >
**  4. side1TrgIndex = index of triangle adjacent to triangle edge pntIndex1-pntIndex2
**  5. side2TrgIndex = index of triangle adjacent to triangle edge pntIndex2-pntIndex3
**  6. side2TrgIndex = index of triangle adjacent to triangle edge pntIndex3-pntIndex1
**
*/

    struct CIVdtmtin tin;
    struct CIVdtmtin *newTinP = NULL;

    mapDtmPointsByIndexP->Lookup((void*)(size_t)pntIndex1, (void*&)tin.p1);
    mapDtmPointsByIndexP->Lookup((void*)(size_t)pntIndex2, (void*&)tin.p2);
    mapDtmPointsByIndexP->Lookup((void*)(size_t)pntIndex3, (void*&)tin.p3);

    tin.n12 = (struct CIVdtmtin *)(size_t)side1TrgIndex;
    tin.n23 = (struct CIVdtmtin *)(size_t)side2TrgIndex;
    tin.n31 = (struct CIVdtmtin *)(size_t)side3TrgIndex;

    aecDTM_addTriangle ( &newTinP, newSrfP, tin.p1, tin.p2, tin.p3, tin.n12, tin.n23, tin.n31 );

    if ( voidTriangle )
        aecDTM_setTriangleDeletedFlag ( newTinP );

    mapDtmTinsByIndexP->SetAt((void*)(size_t)trgIndex, newTinP);


    // bcdtmWrite_message(0,0,0,"Triangle[%8ld] = %8ld %8ld %8ld ** %1ld ** %8ld %8ld %8ld",trgIndex,pntIndex1,pntIndex2,pntIndex3,voidTriangle,side1TrgIndex,side2TrgIndex,side3TrgIndex) ;
    return(SUCCESS) ;
}

int tinFeaturesCallBackFunction( long dtmFeatureType,__int64 dtmUserTag,__int64 dtmFeatureId, long *pointIndicesP,long numPoints)
{
    if ( bFeaturesCallbackOn == FALSE )
        return SUCCESS;

/*
**  Notes :-
**
**  1. Ignore dtmFeatureId . It is a civil platform replacement for Guid and is not used by Athens Geopak
**  2. dtmUserTag  - Used by Geopak. If possible, should Be retained for round tripping purposes.
**  3. pointIndicesP - Array Of Feature Point Index's.
**  4. numPoints - Size Of pointIndicesP array
**  5. Copy pointIndicesP if you need to. Do not free the memory as it is reused by the DTM to save continual memory allocations.
**  6. A Null User Tag   is set to -9898989898
**  6. A Null Feature Id is set to -9898989898
**
*/
    const wchar_t* cpFeatureName = NULL;
    const wchar_t* cpFeatureDefinition = NULL;
    aecDTM_getCPFeatureInfoByFeatureId ( dtmFeatureId, cpFeatureName, cpFeatureDefinition );

    long  *pntP,dbg=1 ; ;
    struct CIVdtmpnt *dtmPntsP = NULL;
    DPoint3d *pntsP = (DPoint3d*)malloc(numPoints * sizeof ( DPoint3d ));
    wchar_t ftrName[DTM_C_NAMSIZ];

    memset ( ftrName, 0, sizeof ( ftrName ) );

    if ( dtmFeatureType != 507 ) // If bounding rectangle, no name needed
    {
        if ( cpFeatureName != NULL && wcscmp ( cpFeatureName, L"" ) != 0 )
        {
            swprintf ( ftrName, L"%.*s", DTM_C_NAMSIZ-1, cpFeatureName );

            // Remember this because sometimes it gets passed in before the real exterior does.
            if ( dtmFeatureType == 23 )
                wcscpy ( exteriorBoundaryName, ftrName );
        }
        else
        {
            if ( dtmFeatureType != 23 || wcscmp ( exteriorBoundaryName, L"" ) == 0 )
                swprintf ( ftrName, L"%.*s%d", DTM_C_NAMSIZ-3, newSrfP->nam, ++ftrNameCnt );
            else if ( dtmFeatureType == 23 )
                wcscpy ( ftrName, exteriorBoundaryName );
        }
    }

    for (int i = 0; i < numPoints; i++)
    {
        TINtoDTMPoint tinToDtmPointP;
        mapPointsByIndexP->Lookup(pointIndicesP[i], tinToDtmPointP);
        memcpy ( &pntsP[i], &tinToDtmPointP.cor, sizeof ( DPoint3d ) );
        tinToDtmPointP.bAdded = TRUE;
        mapPointsByIndexP->SetAt(pointIndicesP[i], tinToDtmPointP);
    }

    wchar_t *styleNameP = NULL;
    wchar_t styleName[CIV_C_NAMSIZ];       // Feature Style
    int numStyles = 0;
    memset ( styleName, 0, sizeof  ( styleName ) );

    if ( cpFeatureDefinition != NULL && wcscmp ( cpFeatureDefinition, L"" ) != 0 )
    {
        swprintf ( styleName, L"%.*s", CIV_C_NAMSIZ-1, cpFeatureDefinition );
        styleNameP = styleName;
        numStyles = 1;

        // Remember this because sometimes it gets passed in before the real exterior does.
        if ( dtmFeatureType == 23 )
            wcscpy ( exteriorBoundaryStyleName, styleName );
    }
    else
    {
        if ( dtmFeatureType == 23 && wcscmp ( exteriorBoundaryStyleName, L"" ) != 0 )
        {
            wcscpy ( styleName, exteriorBoundaryStyleName );
            styleNameP = styleName;
            numStyles = 1;
        }
    }
    //TODO
    //if ( dtmUserTag != DTM_NULL_USER_TAG )
    //{
    //    wchar_t *styleP = gpkSiteElement_getInRoadsStyleByID ( dtmUserTag );
    //    if ( styleP )
    //    {
    //        wcsncpy ( styleName, styleP, CIV_C_NAMSIZ - 1 );
    //        styleNameP = styleName;
    //        numStyles = 1;
    //    }
    //}

   switch(dtmFeatureType)
     {

      case  507 :   // Inroads Bounding Rectangle - DTMF_INROADS_RECTANGLE - Bounding Rectangle Placed Around Geopak Tin To Simulate An Inroads DTM
        aecDTM_addPointsExt ( &dtmPntsP, newSrfP, (int)(DTM_C_DTMRNG | ADDPNT_DTM), 5L, pntsP );

        //if( dbg )bcdtmWrite_message(0,0,0,"Inroads Rectangle ** %8I64d %8I64d ** %p %8ld",dtmUserTag,dtmFeatureId,pointIndicesP,numPoints) ;
       if( dbg == 2 )
          {
           for( pntP = pointIndicesP ; pntP < pointIndicesP + numPoints ; ++pntP )
             {
              //bcdtmWrite_message(0,0,0,"Rectangle Indice[%4ld] = %8ld",(long)(pntP-pointIndicesP),*pntP) ;
             }
          }
      break     ;

      case   1 :   // Collection Spots - DTMF_GROUP_SPOT - Eg. Set Of survey control points
        addDtmFeature ( &dtmPntsP, ftrName, NULL, DTM_C_DTMREGFTR, styleNameP, numStyles, pntsP, numPoints );
        //if( dbg )bcdtmWrite_message(0,0,0,"Group Spot   ** %8I64d %8I64d ** %p %8ld",dtmUserTag,dtmFeatureId,pointIndicesP,numPoints) ;
      break    ;

      case  10  :  // Break Line - DTMF_HARD_BREAK
        addDtmFeature ( &dtmPntsP, ftrName, NULL, DTM_C_DTMBRKFTR, styleNameP, numStyles, pntsP, numPoints );
        //if( dbg ) bcdtmWrite_message(0,0,0,"Break Line   ** %8I64d %8I64d ** %p %8ld",dtmUserTag,dtmFeatureId,pointIndicesP,numPoints) ;
      break     ;

      case  14  :  // Contour Line - DTMF_CONTOUR_LINE
        addDtmFeature ( &dtmPntsP, ftrName, NULL, DTM_C_DTMCTRFTR, styleNameP, numStyles, pntsP, numPoints );
        //if( dbg ) bcdtmWrite_message(0,0,0,"Contour Line ** %8I64d %8I64d ** %p %8ld",dtmUserTag,dtmFeatureId,pointIndicesP,numPoints) ;
      break     ;

      case  23  :  // Exterior Boundary - DTMF_HULL
        addDtmFeature ( &dtmPntsP, ftrName, NULL, DTM_C_DTMEXTFTR, styleNameP, numStyles, pntsP, numPoints );
        //if( dbg ) bcdtmWrite_message(0,0,0,"Tin Hull     ** %8I64d %8I64d ** %p %8ld",dtmUserTag,dtmFeatureId,pointIndicesP,numPoints) ;
        if( *pointIndicesP != *(pointIndicesP+numPoints-1) )
          {
           //bcdtmWrite_message(1,0,0,"ERROR - Tin Hull Does not Close") ;
           goto errexit ;
          }
       if( dbg == 2 )
          {
           for( pntP = pointIndicesP ; pntP < pointIndicesP + numPoints ; ++pntP )
             {
              //bcdtmWrite_message(0,0,0,"Hull Indice[%4ld] = %8ld",(long)(pntP-pointIndicesP),*pntP) ;
             }
          }
      break     ;

      case  20  :  // Interior Boundary - DTMF_VOID
        addDtmFeature ( &dtmPntsP, ftrName, NULL, DTM_C_DTMINTFTR, styleNameP, numStyles, pntsP, numPoints );
        //if( dbg ) bcdtmWrite_message(0,0,0,"Void Hull    ** %8I64d %8I64d ** %p %8ld",dtmUserTag,dtmFeatureId,pointIndicesP,numPoints) ;
        if( *pointIndicesP != *(pointIndicesP+numPoints-1))
          {
           //bcdtmWrite_message(1,0,0,"ERROR - Void Hull Does not Close") ;
           goto errexit ;
          }
       break     ;

      case  21  :  // Interior Boundary - DTMF_ISLAND
        addDtmFeature ( &dtmPntsP, ftrName, NULL, DTM_C_DTMBRKFTR, styleNameP, numStyles, pntsP, numPoints );
        //if( dbg ) bcdtmWrite_message(0,0,0,"Island Hull  ** %8I64d %8I64d ** %p %8ld",dtmUserTag,dtmFeatureId,pointIndicesP,numPoints) ;
         if( *pointIndicesP != *(pointIndicesP+numPoints-1))
          {
           //bcdtmWrite_message(1,0,0,"ERROR - Island Hull Does not Close") ;
           goto errexit ;
          }
     break     ;

      case  22  :  // Interior Boundary - DTMF_HOLE - A Void but treated differently when merging Tins.
        addDtmFeature ( &dtmPntsP, ftrName, NULL, DTM_C_DTMINTFTR, styleNameP, numStyles, pntsP, numPoints );
        //if( dbg ) bcdtmWrite_message(0,0,0,"Hole Hull    ** %8I64d %8I64d ** %p %8ld",dtmUserTag,dtmFeatureId,pointIndicesP,numPoints) ;
        if( *pointIndicesP != *(pointIndicesP+numPoints-1))
          {
           //bcdtmWrite_message(1,0,0,"ERROR - Hole Hull Does not Close") ;
           goto errexit ;
          }
      break     ;

      case  430 :  // Interior Polygon - DTMF_POLYGON - Used To regionize the Tin - application dependent. Eg Catchment Boundaries
        addDtmFeature ( &dtmPntsP, ftrName, NULL, DTM_C_DTMBRKFTR, styleNameP, numStyles, pntsP, numPoints );
        //if( dbg ) bcdtmWrite_message(0,0,0,"Polygon Hull ** %8I64d %8I64d ** %p %8ld",dtmUserTag,dtmFeatureId,pointIndicesP,numPoints) ;
      break     ;

      case  113 :  // Interior Polygon - DTMF_ZERO_SLOPE_POLYGON - Used To regionize zero slope areas in the Tin - application dependent
        addDtmFeature ( &dtmPntsP, ftrName, NULL, DTM_C_DTMBRKFTR, styleNameP, numStyles, pntsP, numPoints );
        //if( dbg ) bcdtmWrite_message(0,0,0,"Zero  Hull   ** %8I64d %8I64d ** %p %8ld",dtmUserTag,dtmFeatureId,pointIndicesP,numPoints) ;
      break     ;

      default  :  // Ignore any other feature types for the time being. Will incorporate if needed.
        //if( dbg ) bcdtmWrite_message(0,0,0,"Unknown %4ld ** %8I64d %8I64d ** %p %8ld",dtmFeatureType,dtmUserTag,dtmFeatureId,pointIndicesP,numPoints) ;
      break    ;

     } ;

    for (int i = 0; i < numPoints; i++)
    {
    mapDtmPointsByIndexP->SetAt((void *)(size_t)pointIndicesP[i], &dtmPntsP[i]);
    }

    if ( pntsP )
        free ( pntsP );

    return(SUCCESS) ;

    errexit : return(ERROR) ;
}

static int aecDTM_sendAllTrianglesCallback
(
    void *datP,
    long,
    DPoint3d *,
    CIVdtmtin *tinP,
    unsigned long
)
{
    CMapPtrToPtr *mapDtmTinsByIndex = (CMapPtrToPtr*)datP;

    if ( tinP->n12 == (CIVdtmtin*)-1 )
        tinP->n12 = NULL;
    else
        mapDtmTinsByIndex->Lookup(tinP->n12, (void*&)tinP->n12);

    if ( tinP->n23 == (CIVdtmtin*)-1 )
        tinP->n23 = NULL;
    else
        mapDtmTinsByIndex->Lookup(tinP->n23, (void*&)tinP->n23);

    if ( tinP->n31 == (CIVdtmtin*)-1 )
        tinP->n31 = NULL;
    else
        mapDtmTinsByIndex->Lookup(tinP->n31, (void*&)tinP->n31);

    return( SUCCESS );
}

void aecDTM_setConvertTinToDTMFunction
(
    int (*pFunc)
    (
        WCharCP tinP,
        int  (*tinStatsCallBackFunctionP)(long numRandomPoints,long numFeaturePoints,long numTriangles,long NumFeatures),
        int  (*tinRandomPointsCallBackFunctionP)(long pntIndex,double X,double Y,double Z),
        int  (*tinFeaturePointsCallBackFunctionP)(long pntIndex,double X,double Y,double Z),
        int  (*tinTrianglesCallBackFunctionP)(long trgIndex,long pntIndex1,long pntIndex2,long pntIndex3,long voidTriangle,long side1TrgIndex,long side2TrgIndex,long side3TrgIndex),
        int  (*tinFeaturesCallBackFunctionP)(long dtmFeatureType,__int64 dtmUsertag,__int64 dtmFeatureId,long *pointIndicesP,long numPointIndices)
    )
)
{
    pConvertTinToDTMCallbackFunc = pFunc;
    pConvertTinToDTMCallbackFuncStdcall = NULL;
}

void aecDTM_setConvertTinToDTMFunctionStdcall
(
    void (__stdcall *pFunc)
    (
        void *tinP,
        int  (*tinStatsCallBackFunctionP)(long numRandomPoints,long numFeaturePoints,long numTriangles,long NumFeatures),
        int  (*tinRandomPointsCallBackFunctionP)(long pntIndex,double X,double Y,double Z),
        int  (*tinFeaturePointsCallBackFunctionP)(long pntIndex,double X,double Y,double Z),
        int  (*tinTrianglesCallBackFunctionP)(long trgIndex,long pntIndex1,long pntIndex2,long pntIndex3,long voidTriangle,long side1TrgIndex,long side2TrgIndex,long side3TrgIndex),
        int  (*tinFeaturesCallBackFunctionP)(long dtmFeatureType,__int64 dtmUsertag,__int64 dtmFeatureId,long *pointIndicesP,long numPointIndices)
    )
)
{
    pConvertTinToDTMCallbackFunc = NULL;
    pConvertTinToDTMCallbackFuncStdcall = pFunc;
}

int aecDTM_convertTinToDTM
(
    CIVdtmsrf **srfPP,
    void *tinP,
    WCharCP name,
    WCharCP description,
    int updateExplorer,
    int updateSurface
)
{
    int sts = SUCCESS;

    if ( tinP == NULL )
        return ERROR;

    // Attempt to find the surface named and update that if it exists, otherwise create a new surface
    CIVdtmprj *prjP = NULL;
    CIVdtmsrf *dtmSrfP = NULL;
    wchar_t *prjNameP = NULL;
    wchar_t *prjDescP = NULL;

    if (updateSurface)
        {
        aecDTM_getActiveProject( &prjP, prjNameP, prjDescP);
        if (prjP)
            {
            aecDTM_findSurfaceByName( &dtmSrfP, prjP,const_cast<WCharP>(name));
            if (dtmSrfP)
                {
                aecDTM_deleteAllFeatures(dtmSrfP, 0);
                wcscpy(dtmSrfP->des, description);
                newSrfP = dtmSrfP;
                }
            }
        }

    if (!dtmSrfP)
        aecDTM_createSurfaceEx(&newSrfP, NULL, const_cast<WCharP>(name), const_cast<WCharP>(description), NULL, NULL, 0., 0., updateExplorer);

    if ( newSrfP )
    {
        memset ( exteriorBoundaryName, 0, sizeof ( exteriorBoundaryName ) );
        memset ( exteriorBoundaryStyleName, 0, sizeof ( exteriorBoundaryStyleName ) );

        mapPointsByIndexP = new TINtoDTMPointMap;
        mapPointsByIndexP->InitHashTable(1000003);

        mapDtmPointsByIndexP = new CMapPtrToPtr;
        mapDtmPointsByIndexP->InitHashTable(1000003);

        ftrNameCnt = 0;

        bStatsCallbackOn = FALSE;
        bRandomPointsCallbackOn = TRUE;
        bFeaturePointsCallbackOn = TRUE;
        bTrianglesCallbackOn = FALSE;
        bFeaturesCallbackOn = TRUE;

        if ( pConvertTinToDTMCallbackFunc )
        {
            (*pConvertTinToDTMCallbackFunc)( (WCharCP) tinP,
                                             tinStatsCallBackFunction,
                                             tinRandomPointsCallBackFunction,
                                             tinFeaturePointsCallBackFunction,
                                             tinTrianglesCallBackFunction,
                                             tinFeaturesCallBackFunction );
        }
        else
        {
            (*pConvertTinToDTMCallbackFuncStdcall)( tinP,
                                                    NULL,
                                                    tinRandomPointsCallBackFunction,
                                                    tinFeaturePointsCallBackFunction,
                                                    NULL,
                                                    tinFeaturesCallBackFunction );
        }

        int pntCount = 0;
        POSITION pos = mapPointsByIndexP->GetStartPosition();
        while (pos != NULL)
        {
            long index;
            TINtoDTMPoint tinToDtmPnt;
            mapPointsByIndexP->GetNextAssoc(pos, index, tinToDtmPnt);

            if ( !tinToDtmPnt.bAdded )
                pntCount++;
        }

        std::vector<DPoint3d> pnts;

        if ( pntCount > 0 )
            pnts.reserve ( pntCount );

        pntCount = 0;
        pos = mapPointsByIndexP->GetStartPosition();
        while (pos != NULL)
        {
            long index;
            TINtoDTMPoint tinToDtmPnt;
            mapPointsByIndexP->GetNextAssoc(pos, index, tinToDtmPnt);

            if (!tinToDtmPnt.bAdded)
                {
                pntCount++;
                pnts.push_back (tinToDtmPnt.cor);
                }
        }

        ASSERT ( pntCount == pnts.size() );

        if ( pnts.size() > 0 )
        {
            CString strFtrName;
            strFtrName.LoadString ( DTM_S_RNDPNTNAM );
            if ( strFtrName.IsEmpty() || wcscmp(strFtrName, L"") == 0 )
                strFtrName.Format ( L"%.*s%d", DTM_C_NAMSIZ-3, newSrfP->nam, ++ftrNameCnt );

            CString strFtrDesc;
            strFtrDesc.LoadString ( DTM_S_RNDPNTDES );

            wchar_t *styleNameP = NULL;
            wchar_t styleName[CIV_C_NAMSIZ];       // Feature Style
            int numStyles = 0;
            memset ( styleName, 0, sizeof  ( styleName ) );

            struct CIVdtmpnt *dtmPntsP = NULL;
            addDtmFeature ( &dtmPntsP, (LPWSTR)(LPCWSTR)strFtrName, (LPWSTR)(LPCWSTR)strFtrDesc, DTM_C_DTMREGFTR,
                            styleNameP, numStyles, &pnts[0], (int)pnts.size() );

            pnts.clear();
            pnts.clear();

            int i = 0;
            pos = mapPointsByIndexP->GetStartPosition();
            while (pos != NULL)
            {
                long index;
                TINtoDTMPoint tinToDtmPnt;
                mapPointsByIndexP->GetNextAssoc(pos, index, tinToDtmPnt);

                if ( !tinToDtmPnt.bAdded )
                {
                    mapDtmPointsByIndexP->SetAt ( (void *)(size_t)tinToDtmPnt.index, &dtmPntsP[i] );
                    i++;
                }
            }
        }

        aecDTM_computeSurfaceRange ( newSrfP );

        memset ( exteriorBoundaryName, 0, sizeof ( exteriorBoundaryName ) );
        memset ( exteriorBoundaryStyleName, 0, sizeof ( exteriorBoundaryStyleName ) );

        delete mapPointsByIndexP;
        mapPointsByIndexP = NULL;

        mapDtmTinsByIndexP = new CMapPtrToPtr;
        mapDtmTinsByIndexP->InitHashTable(1000003);

        bStatsCallbackOn = FALSE;
        bRandomPointsCallbackOn = FALSE;
        bFeaturePointsCallbackOn = FALSE;
        bTrianglesCallbackOn = TRUE;
        bFeaturesCallbackOn = FALSE;

        if ( pConvertTinToDTMCallbackFunc )
        {
            (*pConvertTinToDTMCallbackFunc)( (WCharCP) tinP,
                                             tinStatsCallBackFunction,
                                             tinRandomPointsCallBackFunction,
                                             tinFeaturePointsCallBackFunction,
                                             tinTrianglesCallBackFunction,
                                             tinFeaturesCallBackFunction );
        }
        else
        {
            (*pConvertTinToDTMCallbackFuncStdcall)( tinP,
                                                    NULL,
                                                    NULL,
                                                    NULL,
                                                    tinTrianglesCallBackFunction,
                                                    NULL );
        }

        delete mapDtmPointsByIndexP;
        mapDtmPointsByIndexP = NULL;

        aecDTM_sendAllTriangles( NULL, newSrfP, DTM_C_NOBREK|DTM_C_DELETE, aecDTM_sendAllTrianglesCallback, mapDtmTinsByIndexP );

        aecDTM_clearSurfaceTinOutOfDateFlag ( newSrfP );

        delete mapDtmTinsByIndexP;
        mapDtmTinsByIndexP = NULL;

        if (srfPP)
            *srfPP = newSrfP;
    }

    return sts;
}

void aecDTM_clearCPFeatureNamesList()
    {
    // Clear feature names
    featureIds.clear();
    featureDefinitions.clear();
    featureNames.clear();
    }

void aecDTM_addCPFeature(__int64 featureID, LPWSTR featureName, LPWSTR featureDefinition)
    {
    // Add featurename, etc to list
    featureIds.push_back(featureID);
    featureNames.push_back(std::wstring(featureName));
    featureDefinitions.push_back(std::wstring(featureDefinition));
    }

void aecDTM_getCPFeatureInfoByFeatureId(__int64 featureID, const wchar_t* &featureName, const wchar_t* &featureDefinition)
    {
    // Get feature name and feature definition for feature id
    for (unsigned int index = 0;index<featureIds.size();index++)
        {
        if (featureID==featureIds[index])
            {
            featureName = featureNames[index].c_str();
            featureDefinition = featureDefinitions[index].c_str();
            break;
            }
        }
    }