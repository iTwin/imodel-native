//---------------------------------------------------------------------------+
// Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//---------------------------------------------------------------------------+
#include "stdafx.h"
#include "inroadstm.h"
#include    "DTMToGPKTin.h"

typedef struct
{
    unsigned long index;
    unsigned char cListProc;
} DTMtoTINPoint;

typedef CMap<struct CIVdtmpnt*, struct CIVdtmpnt*&, DTMtoTINPoint, DTMtoTINPoint&> DTMToTINPointMap;

static int _countTriangulatedPointsCallback(void*,long,DPoint3d*,CIVdtmtin*,unsigned long);
static int _countTriangulatedPoint(struct CIVdtmpnt*);
static int _sendTriangulatedPointsCallback(void*,long,DPoint3d*,CIVdtmtin*,unsigned long);
static int _sendTriangulatedPoint(struct CIVdtmpnt*);
static int _sendCircularListCallback(void*,long,DPoint3d*,CIVdtmtin*,unsigned long);
static int _sendPointCirularList(struct CIVdtmpnt*, struct CIVdtmsrf*, CIVdtmtin*);
static int _sendFeaturesCallback(void*,struct CIVdtmsrf*,int,struct CIVdtmftr*);
static int _getSurfacePerimeterIndices(struct CIVdtmsrf*,DPoint3d**,long*);

static int _inroadsCallback
(
void *dtmP,
int (*geopakPointsCallBackFunction)(void *dtmP, double X, double Y, double Z),
int (*geopakCircularListCallBackFunction)(void *dtmP, long pointIndex, long *cirPointIndexP,long numCirPointIndex),
int (*dtmFeatureCallBackFunction)(void *dtmP, wchar_t const* dtmFeatureName, wchar_t const* dtmFeatureDescription, wchar_t const*dtmFeaturStyle, long dtmFeatureType,DPoint3d *dtmFeaturePointIndiciesP, long numDtmFeaturePointIndicies, int excludeFromTriangulation)
);

extern "C"
{

static int (*_geopakPointsCallBackFunction)(void *dtmP,double X,double Y,double Z);
static int (*_geopakCircularListCallBackFunction)(void *dtmP, long pointIndex, long *cirPointIndexP, long numCirPointIndex);
static int (*_dtmFeatureCallBackFunction)(void *dtmP, wchar_t const*dtmFeatureName, wchar_t const* dtmFeatureDescription, wchar_t const* dtmFeatureStyle, long dtmFeatureType, DPoint3d *dtmFeaturePointIndiciesP, long numDtmFeaturePointIndicies, int excludeFromTriangulation);

}

static DTMToTINPointMap *_mapPointsByAddrP = NULL;
static unsigned long _curIndex = 0;
static unsigned long _numTriangulatedPoints = 0;
static unsigned long _numFeatures = 0;
static BOOL _bExteriorFnd = FALSE;
static struct CIVdtmsrf *_srfP = NULL;
static void *_dtmP = NULL;
static long *_perimeterIndicesP = NULL;
static DPoint3d *_perimeterPntsP = NULL;
static long _numPerimeterIndicesP = 0;

int aecDTM_convertDTMToGPKTin
(
 wchar_t *dtmFilename,
 wchar_t *gpkTinFilename,
int (*bcdtmInRoads_importGeopakTinFromInroadsDtm)(double maxTriLength, long  numTinPoints, long  numTinFeatures, wchar_t  *geopakTinFileNameP, int (*setGeopakCallBackFunctionsP)())
)
{
    struct CIVdtmsrf *srfP = NULL;
    int sts = SUCCESS;

    sts = aecDTM_load ( &srfP, NULL, NULL, dtmFilename );

    if ( sts == SUCCESS  && srfP != NULL )
    {
    sts = aecDTM_exportToGPKTin (srfP, gpkTinFilename, bcdtmInRoads_importGeopakTinFromInroadsDtm);
        aecDTM_deleteSurface( NULL, srfP, FALSE );
    }

    return sts;
}

int aecDTM_exportToGPKTin
(
struct CIVdtmsrf *srfP,
wchar_t *gpkTinFilename,
int (*bcdtmInRoads_importGeopakTinFromInroadsDtm)(double maxTriLength, long  numTinPoints, long  numTinFeatures, wchar_t  *geopakTinFileNameP, int (*setGeopakCallBackFunctionsP)()))
{
    long numTins = 0;
    wchar_t newGpkTinFilename[MAX_PATH+1] = L"";
    int sts = SUCCESS;

    wcscpy ( newGpkTinFilename, gpkTinFilename );

    _mapPointsByAddrP = new DTMToTINPointMap;
    _mapPointsByAddrP->InitHashTable(1000003);

    _curIndex = 0;
    _numTriangulatedPoints = 0;
    _numFeatures = 0;
    _bExteriorFnd = FALSE;
    _srfP = srfP;
    _perimeterIndicesP = NULL;
    _perimeterPntsP = NULL;
    _numPerimeterIndicesP = 0;

    aecDTM_countSurfaceData ( NULL, &numTins, srfP );

    if ( numTins == 0 || aecDTM_isSurfaceTinOutOfDateFlagSet ( srfP ) )
    {
        if ( ( sts = aecDTM_triangulate ( NULL, NULL, NULL, srfP, TRISRF_INT, NULL, NULL, NULL, NULL ) ) != SUCCESS )
        {
            if( sts == DTM_M_TOLPRB )//DO_NOT_TRANSLATE
            {
                aecDTM_fixTolerance( srfP );

                sts = aecDTM_triangulate ( NULL, NULL, NULL, srfP, TRISRF_INT, NULL, NULL, NULL, NULL );
            }
        }
    }

    if ( sts == SUCCESS )
    {
        aecDTM_sendAllTriangles( NULL, srfP, DTM_C_NOBREK|DTM_C_DELETE, _countTriangulatedPointsCallback, srfP );

        bool bCountOnly = TRUE;
        aecDTM_sendAllFeatures ( (void *)0, srfP, DTM_C_NOBREK, 0, _sendFeaturesCallback, &bCountOnly );

        if (_bExteriorFnd == FALSE)
        {
            _getSurfacePerimeterIndices ( srfP, &_perimeterPntsP, &_numPerimeterIndicesP );
            _numFeatures++;
        }

        char filename[(MAX_PATH +1) * 2] = "";
        memset ( filename, 0, sizeof ( filename ) );
        wcstombs ( filename, newGpkTinFilename, MAX_PATH + 1 );

        (*(int(*)(double, long,long,char*,
                int  (*_inroadsCallback)
                (
                void *dtmP,
                int (*geopakPointsCallBackFunction)(void *dtmP, double X, double Y, double Z),
                int (*geopakCircularListCallBackFunction)(void *dtmP, long pointIndex, long *cirPointIndexP,long numCirPointIndex),
                int (*dtmFeatureCallBackFunction)(void *dtmP, wchar_t const*dtmFeatureName, wchar_t const* dtmFeatureDescription, wchar_t const*dtmFeatureStyle, long dtmFeatureType, DPoint3d *dtmFeaturePointIndiciesP, long numDtmFeaturePointIndicies, int excludeFromTriangulation)
                ))) bcdtmInRoads_importGeopakTinFromInroadsDtm)(srfP->par.maxsid, _numTriangulatedPoints, _numFeatures, filename, _inroadsCallback);

        if (_mapPointsByAddrP)
        {
            delete _mapPointsByAddrP;
            _mapPointsByAddrP = NULL;
        }

        if ( _perimeterPntsP )
            free ( _perimeterPntsP );
    }

    return sts;
}

static int _inroadsCallback
(
void *dtmP,
int (*geopakPointsCallBackFunction)(void *dtmP, double X, double Y, double Z),
int (*geopakCircularListCallBackFunction)(void *dtmP, long pointIndex, long *cirPointIndexP,long numCirPointIndex),
int (*dtmFeatureCallBackFunction)(void *dtmP, wchar_t const*dtmFeatureName, wchar_t const* dtmFeatureDescription, wchar_t const* dtmFeatureStyle, long dtmFeatureType, DPoint3d *dtmFeaturePointIndiciesP, long numDtmFeaturePointIndicies, int excludeFromTriangulation)
)
{
    _dtmP = dtmP;
    _geopakPointsCallBackFunction = geopakPointsCallBackFunction;
    _geopakCircularListCallBackFunction = geopakCircularListCallBackFunction;
    _dtmFeatureCallBackFunction = dtmFeatureCallBackFunction;

    aecDTM_sendAllTriangles( NULL, _srfP, DTM_C_NOBREK|DTM_C_DELETE, _sendTriangulatedPointsCallback, _srfP );

    POSITION pos = _mapPointsByAddrP->GetStartPosition();
    while (pos != NULL)
    {
        struct CIVdtmpnt *dtmPntP = NULL;
        DTMtoTINPoint dtmToTinPnt;
        _mapPointsByAddrP->GetNextAssoc(pos, dtmPntP, dtmToTinPnt);
        dtmToTinPnt.cListProc = 0;
        _mapPointsByAddrP->SetAt(dtmPntP, dtmToTinPnt);
    }

    aecDTM_sendAllTriangles( NULL, _srfP, DTM_C_NOBREK|DTM_C_DELETE, _sendCircularListCallback, _srfP );

    bool bCountOnly = FALSE;
    aecDTM_sendAllFeatures ( (void *)0, _srfP, DTM_C_NOBREK, 0, _sendFeaturesCallback, &bCountOnly );

    if (_bExteriorFnd == FALSE)
        dtmFeatureCallBackFunction ( dtmP, L"", L"", L"", 23, _perimeterPntsP, _numPerimeterIndicesP, 0);

    return SUCCESS;
}

static int _countTriangulatedPointsCallback
(
void *datP,
long,
DPoint3d *,
CIVdtmtin *tinP,
unsigned long
)
{
    _countTriangulatedPoint ( tinP->p1 );
    _countTriangulatedPoint ( tinP->p2 );
    _countTriangulatedPoint ( tinP->p3 );

    return( SUCCESS );
}

static int _countTriangulatedPoint ( struct CIVdtmpnt *pntP )
{
    DTMtoTINPoint dtmToTinPnt;
    memset ( &dtmToTinPnt, 0, sizeof ( dtmToTinPnt ) );

    if ( pntP != NULL && _mapPointsByAddrP->Lookup ( pntP, dtmToTinPnt ) == FALSE )
    {
        memset ( &dtmToTinPnt, 0, sizeof ( dtmToTinPnt ) );

        dtmToTinPnt.index = _curIndex++;
        _mapPointsByAddrP->SetAt ( pntP, dtmToTinPnt );
        _numTriangulatedPoints++;
    }

    return SUCCESS;
}

static int _sendTriangulatedPointsCallback
(
void *datP,
long,
DPoint3d *,
CIVdtmtin *tinP,
unsigned long
)
{
    _sendTriangulatedPoint ( tinP->p1 );
    _sendTriangulatedPoint ( tinP->p2 );
    _sendTriangulatedPoint ( tinP->p3 );

    return( SUCCESS );
}

static int _sendTriangulatedPoint ( struct CIVdtmpnt *pntP )
{
    DTMtoTINPoint dtmToTinPnt;
    memset ( &dtmToTinPnt, 0, sizeof ( dtmToTinPnt ) );

    if ( pntP != NULL && _mapPointsByAddrP->Lookup ( pntP, dtmToTinPnt ) == TRUE && dtmToTinPnt.cListProc == 0 )
    {
        dtmToTinPnt.cListProc = 1;
        _mapPointsByAddrP->SetAt( pntP, dtmToTinPnt );
        _geopakPointsCallBackFunction ( _dtmP, pntP->cor.x, pntP->cor.y, pntP->cor.z );
    }

    return SUCCESS;
}

static int _sendCircularListCallback
(
void *datP,
long,
DPoint3d *,
CIVdtmtin *tinP,
unsigned long
)
{
    struct CIVdtmsrf *srfP = (struct CIVdtmsrf*)datP;

    _sendPointCirularList ( tinP->p1, srfP, tinP );
    _sendPointCirularList ( tinP->p2, srfP, tinP );
    _sendPointCirularList ( tinP->p3, srfP, tinP );

    return( SUCCESS );
}

static int _sendPointCirularList
(
struct CIVdtmpnt *pntP,
struct CIVdtmsrf *srfP,
CIVdtmtin *tinP
)
{
    int sts = SUCCESS;
    DTMtoTINPoint dtmToTinPnt;

    if ( pntP != NULL && _mapPointsByAddrP->Lookup ( pntP, dtmToTinPnt ) == TRUE && dtmToTinPnt.cListProc == 0 )
    {
        dtmToTinPnt.cListProc = 1;
        _mapPointsByAddrP->SetAt ( pntP, dtmToTinPnt );

        long *indicesP = NULL;
        long numIndices = 0;
        long *pntsP = NULL;
        long numPnts = 0;
        long *tinsP = NULL;
        long numTins = 0;
        aecDTM_getPointNeighbors ( &numPnts, &pntsP, &numTins, &tinsP, NULL, NULL, srfP, pntP, tinP, 0 );

        if (numPnts > 0)
        {
            indicesP = (long*) malloc ( sizeof ( long ) * numPnts );

            for (int i = 0; i < numPnts; i++)
            {
                DTMtoTINPoint dtmToTinPnt_;
                struct CIVdtmpnt *dtmPntP = (struct CIVdtmpnt *)(size_t)pntsP[i];
                if ( _mapPointsByAddrP->Lookup ( dtmPntP, dtmToTinPnt_ ) == TRUE )
                {
                    indicesP[numIndices] = dtmToTinPnt_.index;
                    numIndices++;
                }
            }

            if ( numIndices > 1 )
            {
                _geopakCircularListCallBackFunction ( _dtmP, dtmToTinPnt.index, indicesP, numIndices-1 );
            }
        }

        if ( indicesP )
            free ( indicesP );

        if ( pntsP )
            free ( pntsP );

        if ( tinsP )
            free ( tinsP );
    }

    return sts;
}

static int _sendFeaturesCallback
(
void *dat,
struct CIVdtmsrf *srf,
int ftrTyp,
struct CIVdtmftr *ftrP
)
{
    bool bCountOnly = (*(bool*)dat);
    int sts = SUCCESS;

    if (ftrP->numPnts > 0)
    {

        long excludeFromTriangulation = (ftrP->flg & DTM_C_FTRTIN);

        DPoint3d *pointIndicesP = (DPoint3d *)malloc ( sizeof ( DPoint3d ) * ftrP->numPnts );
        long numIndices = 0;
        long tinFtrType = 0;

        switch ( ftrTyp )
        {
        case DTM_C_DTMREGFTR:
            tinFtrType = 1;
            break;

        case DTM_C_DTMBRKFTR:
            tinFtrType = 10;
            break;

        case DTM_C_DTMINTFTR:
            tinFtrType = 20;
            break;

        case DTM_C_DTMEXTFTR:
            tinFtrType = 23;
            _bExteriorFnd = TRUE;
            break;

        case DTM_C_DTMCTRFTR:
            tinFtrType = 14;
            break;
        }

        for (int i = 0; i < ftrP->numPnts; i++)
        {
            if ( i > 0 && (ftrP->p1[i].flg & DTM_C_PNTPUD) == FALSE && ftrTyp != DTM_C_DTMREGFTR)
            {
                if (numIndices > 0)
                {
                    if (bCountOnly == TRUE)
                        _numFeatures++;
                    else
                    {
                        wchar_t style[CIV_C_NAMSIZ+1];
                        memset(style, 0, sizeof(style));
                        if ( ftrP->numStyles > 0 )
                            memcpy(style, ftrP->s1->nam, sizeof(ftrP->s1->nam));

                        _dtmFeatureCallBackFunction ( _dtmP, ftrP->nam, ftrP->des, style, tinFtrType, pointIndicesP, numIndices, excludeFromTriangulation);
                    }
                }

                numIndices = 0;
            }

            if ( (ftrP->p1[i].flg & DTM_C_PNTDEL) == FALSE )
            {
                pointIndicesP[numIndices] = ftrP->p1[i].cor;
                numIndices++;
            }
        }

        if (numIndices > 0)
        {
            if (bCountOnly == TRUE)
                _numFeatures++;
            else
            {
                wchar_t style[CIV_C_NAMSIZ+1];
                memset(style, 0, sizeof(style));
                if ( ftrP->numStyles > 0 )
                    memcpy(style, ftrP->s1->nam, sizeof(ftrP->s1->nam));

                _dtmFeatureCallBackFunction ( _dtmP, ftrP->nam, ftrP->des, style, tinFtrType, pointIndicesP, numIndices, excludeFromTriangulation);
            }
        }

        if ( pointIndicesP )
            free ( pointIndicesP );

    }

    return sts;
}


static int _getSurfacePerimeterIndices ( struct CIVdtmsrf *srfP, DPoint3d **pointIndicesPP, long *numIndicesP )
{
    DPoint3d *vrtsP = NULL;
    long numVrts = 0;

    if (aecDTM_getSurfacePerimeter ( &numVrts, &vrtsP, srfP ) == SUCCESS)
    {
        *pointIndicesPP = (DPoint3d*)malloc ( numVrts * sizeof ( DPoint3d ) );
        *numIndicesP = 0;

        for (int i = 0; i < numVrts; i++)
        {
            PUSH_MSVC_IGNORE(6386);
            (*pointIndicesPP)[i] = vrtsP[i];
            POP_MSVC_IGNORE;
            (*numIndicesP)++;
        }
    }

    if (vrtsP)
        free ( vrtsP );

    return SUCCESS;
}