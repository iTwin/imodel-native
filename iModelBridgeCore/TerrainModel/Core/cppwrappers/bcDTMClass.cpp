/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/cppwrappers/bcDTMClass.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma warning(disable: 4018)
#pragma warning(disable: 4786)
#pragma warning(disable: 4102)
/*----------------------------------------------------------------------+
| Include standard library header files                                 |
+----------------------------------------------------------------------*/
#include <math.h>
#include <TerrainModel/TerrainModel.h>

#include <bcDTMBaseDef.h>
#include <dtmevars.h>

/*----------------------------------------------------------------------+
| Include BCivil general header files                                   |
+----------------------------------------------------------------------*/
#include <bcMacros.h>
#include <bcGmcNorm.h>
#include <bcConversion.h>

/*------------------------------------------------------------------+
| Include of the current class header                               |
+------------------------------------------------------------------*/
#include "bcMem.h"
#include "bcDTMImpl.h"
#include "bcpoint.fdf"

#include "bcdtmSideSlope.h"
#include <TerrainModel\Core\TMTransformHelper.h>
#include <TerrainModel\Drainage\drainage.h>

#include "Drainage\bcdtmDrainageFeatures.h"
#include "Drainage\bcdtmDrainagePond.h"
#include "Drainage\bcdtmDrainageTrace.h"
#include "Drainage\bcdtmDrainageCatchment.h"

BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE

#pragma region Transform callback helpers

class FeatureCallbackTransformHelper
    {
    DTMFeatureCallback m_callBackFunctP;
    void* m_userP;
    TMTransformHelperP m_helper;
    public:
        FeatureCallbackTransformHelper (DTMFeatureCallback callBackFunctP, void* userP, TMTransformHelperP helper)
            {
            m_callBackFunctP = callBackFunctP;
            m_helper = helper;
            m_userP = userP;
            }
        void* GetUserArg ()
            {
            if (m_helper && !m_helper->IsIdentity ())
                return this;
            return m_userP;
            }
        DTMFeatureCallback GetCallBackFunc ()
            {
            if (m_helper && !m_helper->IsIdentity ())
                return &TransformFunction;
            return m_callBackFunctP;
            }
    private:
        static int TransformFunction (DTMFeatureType featureType, DTMUserTag featureTag, DTMFeatureId featureId, DPoint3d *tPoint, size_t nPoint, void *userP)
            {
            FeatureCallbackTransformHelper* data = (FeatureCallbackTransformHelper*)userP;
            data->m_helper->convertPointsFromDTM (tPoint, (int)nPoint);
            return data->m_callBackFunctP (featureType, featureTag, featureId, tPoint, nPoint, data->m_userP);
            }
    };

class SinglePointFeatureCallbackTransformHelper
    {
    DTMBrowseSinglePointFeatureCallback m_callBackFunctP;
    void* m_userP;
    TMTransformHelperP m_helper;
    public:
        SinglePointFeatureCallbackTransformHelper (DTMBrowseSinglePointFeatureCallback callBackFunctP, void* userP, TMTransformHelperP helper)
            {
            m_callBackFunctP = callBackFunctP;
            m_helper = helper;
            m_userP = userP;
            }
        void* GetUserArg ()
            {
            if (m_helper && !m_helper->IsIdentity ())
                return this;
            return m_userP;
            }
        DTMBrowseSinglePointFeatureCallback GetCallBackFunc ()
            {
            if (m_helper && !m_helper->IsIdentity ())
                return &TransformFunction;
            return m_callBackFunctP;
            }
    private:
        static int TransformFunction (DTMFeatureType featureType, DPoint3d& tPoint, void *userP)
            {
            SinglePointFeatureCallbackTransformHelper* data = (SinglePointFeatureCallbackTransformHelper*)userP;
            DPoint3d pt = tPoint;
            data->m_helper->convertPointFromDTM (pt);
            return data->m_callBackFunctP (featureType, pt, data->m_userP);
            }
    };

class BrowseSlopeIndicatorCallbackTransformHelper
    {
    DTMBrowseSlopeIndicatorCallback m_callBackFunctP;
    void* m_userP;
    TMTransformHelperP m_helper;
    public:
        BrowseSlopeIndicatorCallbackTransformHelper (DTMBrowseSlopeIndicatorCallback callBackFunctP, void* userP, TMTransformHelperP helper)
            {
            m_callBackFunctP = callBackFunctP;
            m_helper = helper;
            m_userP = userP;
            }
        void* GetUserArg ()
            {
            if (m_helper && !m_helper->IsIdentity ())
                return this;
            return m_userP;
            }
        DTMBrowseSlopeIndicatorCallback GetCallBackFunc ()
            {
            if (m_helper && !m_helper->IsIdentity ())
                return &TransformFunction;
            return m_callBackFunctP;
            }
    private:
        static int TransformFunction (bool major, DPoint3d& point1, DPoint3d& point2, void *userP)
            {
            BrowseSlopeIndicatorCallbackTransformHelper* data = (BrowseSlopeIndicatorCallbackTransformHelper*)userP;
            if (data->m_helper)
                {
                DPoint3d pt1 = point1;
                DPoint3d pt2 = point2;
                data->m_helper->convertPointFromDTM (pt1);
                data->m_helper->convertPointFromDTM (pt2);
                return data->m_callBackFunctP (major, pt1, pt2, data->m_userP);
                }
            return data->m_callBackFunctP (major, point1, point2, data->m_userP);
            }
    };

class TransformPointsCallbackTransformHelper
    {
    DTMTransformPointsCallback m_callBackFunctP;
    void* m_userP;
    TMTransformHelperP m_helper;
    public:
        TransformPointsCallbackTransformHelper (DTMTransformPointsCallback callBackFunctP, void* userP, TMTransformHelperP helper)
            {
            m_callBackFunctP = callBackFunctP;
            m_helper = helper;
            m_userP = userP;
            }
        void* GetUserArg ()
            {
            if (m_helper && !m_helper->IsIdentity ())
                return this;
            return m_userP;
            }
        DTMTransformPointsCallback GetCallBackFunc ()
            {
            if (m_helper && !m_helper->IsIdentity ())
                return &TransformFunction;
            return m_callBackFunctP;
            }
    private:
        static int TransformFunction (DPoint3dP points, size_t numPts, void *userP)
            {
            TransformPointsCallbackTransformHelper* data = (TransformPointsCallbackTransformHelper*)userP;
            data->m_helper->convertPointsFromDTM (points, (int)numPts);
            DTMStatusInt status = (DTMStatusInt)data->m_callBackFunctP (points, numPts, data->m_userP);
            data->m_helper->convertPointsToDTM (points, (int)numPts);
            return status;
            }
    };

class DuplicatePointsCallbackTransformHelper
    {
    DTMDuplicatePointsCallback m_callBackFunctP;
    void* m_userP;
    TMTransformHelperP m_helper;
    public:
        DuplicatePointsCallbackTransformHelper (DTMDuplicatePointsCallback callBackFunctP, void* userP, TMTransformHelperP helper)
            {
            m_callBackFunctP = callBackFunctP;
            m_helper = helper;
            m_userP = userP;
            }
        void* GetUserArg ()
            {
            if (m_helper && !m_helper->IsIdentity ())
                return this;
            return m_userP;
            }
        DTMDuplicatePointsCallback GetCallBackFunc ()
            {
            if (m_helper && !m_helper->IsIdentity ())
                return &TransformFunction;
            return m_callBackFunctP;
            }
    private:
        static int TransformFunction (double x, double y, DTM_DUPLICATE_POINT_ERROR* dupErrorsP, long numDupErrors, void *userP)
            {
            DuplicatePointsCallbackTransformHelper* data = (DuplicatePointsCallbackTransformHelper*)userP;
            data->m_helper->convertPointFromDTM (x, y);
            return data->m_callBackFunctP (x, y, dupErrorsP, numDupErrors, data->m_userP);
            }
    };

class CrossingFeaturesCallbackTransformHelper
    {
    DTMCrossingFeaturesCallback m_callBackFunctP;
    void* m_userP;
    TMTransformHelperP m_helper;
    public:
        CrossingFeaturesCallbackTransformHelper (DTMCrossingFeaturesCallback callBackFunctP, void* userP, TMTransformHelperP helper)
            {
            m_callBackFunctP = callBackFunctP;
            m_helper = helper;
            m_userP = userP;
            }
        void* GetUserArg ()
            {
            if (m_helper && !m_helper->IsIdentity ())
                return this;
            return m_userP;
            }
        DTMCrossingFeaturesCallback GetCallBackFunc ()
            {
            if (m_helper && !m_helper->IsIdentity ())
                return &TransformFunction;
            return m_callBackFunctP;
            }
    private:
        static int TransformFunction (DTM_CROSSING_FEATURE_ERROR& crossError, void *userP)
            {
            CrossingFeaturesCallbackTransformHelper* data = (CrossingFeaturesCallbackTransformHelper*)userP;
            data->m_helper->convertPointFromDTM (crossError.intersectionX, crossError.intersectionY);
            data->m_helper->convertElevationFromDTM (crossError.elevation1);
            data->m_helper->convertDistanceFromDTM (crossError.distance1);
            data->m_helper->convertElevationFromDTM (crossError.elevation1);
            data->m_helper->convertDistanceFromDTM (crossError.distance1);
            return data->m_callBackFunctP(crossError, data->m_userP);
            }
    };

class TriangleMeshCallbackTransformHelper
    {
    DTMTriangleMeshCallback m_callBackFunctP;
    void* m_userP;
    TMTransformHelperP m_helper;
    public:
        TriangleMeshCallbackTransformHelper (DTMTriangleMeshCallback callBackFunctP, void* userP, TMTransformHelperP helper)
            {
            m_callBackFunctP = callBackFunctP;
            m_helper = helper;
            m_userP = userP;
            }
        void* GetUserArg ()
            {
            if (m_helper && !m_helper->IsIdentity ())
                return this;
            return m_userP;
            }
        DTMTriangleMeshCallback GetCallBackFunc ()
            {
            if (m_helper && !m_helper->IsIdentity ())
                return &TransformFunction;
            return m_callBackFunctP;
            }
    private:
        static int TransformFunction (DTMFeatureType featureType, int numTriangles, int numMeshPoints, DPoint3d *meshPointsP, int numMeshFaces, long *meshFacesP, void *userP)
            {
            TriangleMeshCallbackTransformHelper* data = (TriangleMeshCallbackTransformHelper*)userP;

            if (data->m_helper)
                return data->m_callBackFunctP (featureType, numTriangles, numMeshPoints, data->m_helper->copyPointsFromDTM (meshPointsP, numMeshFaces), numMeshFaces, meshFacesP, data->m_userP);
            return data->m_callBackFunctP(featureType, numTriangles, numMeshPoints, meshPointsP, numMeshFaces, meshFacesP, data->m_userP);
            }
    };

#pragma endregion

#pragma region DigitalTM Functions
/*------------------------------------------------------------------+
    | Include COGO definitions                                          |
    +------------------------------------------------------------------*/

    /*----------------------------------------------+
    | Constant definitions                          |
    +----------------------------------------------*/
#define TOL_CALCUL                           0.0000001

/*----------------------------------------------+
| Private type definitions                      |
+----------------------------------------------*/
struct PrflPoint_t
    {
    double          s;
    DTMDrapePoint  bcdtmDrapeData;
    };


typedef int (*dtmHandler_t) ();

/*==================================================================*/
/*                                                                  */
/*          INTERNAL FUNCTIONS                                      */
/*                                                                  */
/*==================================================================*/

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.12jul02  - Created.                                             |
|                                                                       |
+----------------------------------------------------------------------*/
BENTLEYDTM_EXPORT  int  digitalTM_freeTinHandle /* >=< DTM_SUCCESS or DTM_ERROR */
    (
    BC_DTM_OBJ **dtmHandlePP          /* <= Tin file handle */
    )
    {
    DTMStatusInt status = DTM_SUCCESS;

    bcdtmObject_destroyDtmObject (dtmHandlePP);

    *dtmHandlePP = nullptr;
    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.09sep2001   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
static int digitalTM_nextOriginPoint
    (
    DPoint3d    *lineStringPtP,/* => Line string point               */
    DPoint3dCP  origStringP,  /* => Points of original line string  */
    const double* sTabP,        /* => point chainage or nullptr          */
    int         nPtOrig,       /* => Next Origin Point               */
    int         *currPtOrigP,  /* <= Current original point          */
    double      *diffSOrigP,   /* <= Difference in abcissa on       */
    /*    original string                 */
    double      *distOrigP,    /* <= Distance between points of      */
    /*    original line                   */
    double      *distOnCurrP   /* <= Distance on current segment     */
    /*    (reinit to zero)                */
    )
    {
    BC_START ();

    // Initialize values
    *distOrigP = dc_zero;
    *diffSOrigP = dc_zero;

    if (origStringP[*currPtOrigP].x == lineStringPtP->x && origStringP[*currPtOrigP].y == lineStringPtP->y)
        {

        // In the following loop we look for the next point in the original array which is not the same than
        // the current point.
        do
            {
            (*currPtOrigP)++;

            if (*currPtOrigP == nPtOrig)
                {
                // If the index of the current point reaches the end it means that there is no
                // more further points in the original array
                // So we just break here
                break;
                }

            *distOrigP = bcPoint_distance2d (
                &origStringP[(*currPtOrigP)],
                &origStringP[(*currPtOrigP)-1]);
            }
            while (*distOrigP == dc_zero);

            // If we havve found the point we set the value of the station difference
            if (*currPtOrigP < nPtOrig)
                {
                *diffSOrigP = sTabP[(*currPtOrigP)] - sTabP[(*currPtOrigP)-1];
                }

            // We intialize the value of the distance on the current segment
            *distOnCurrP = dc_zero;
        }

BC_END:;

    BC_END_RETURNSTATUS();
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.09sep2001   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
static int digitalTM_setPoint
    (
    PrflPoint_t     *pointTabP,    /* <> Point table                     */
    int             index,         /* => Index in point table            */
    double          currAbc,      /* => Current abcissa                */
    DTMDrapePoint  drapedPts[]
)
    {
    BC_START ();

    /*------------------------------------------------------+
    | start here.                                           |
    +------------------------------------------------------*/
    pointTabP[index].s = currAbc;
    pointTabP[index].bcdtmDrapeData = drapedPts[index];

BC_END:;

    BC_END_RETURNSTATUS();
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.09sep2001   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
static int digitalTM_createProfile
    (
    PrflPoint_t     **pointTabPP,
    long            *nbPt,
    DTMDrapePoint  drapedPt[],
    int             nResPt,         /* => Number of points in line string */
    DPoint3dCP      origStringP,   /* => Points of original line string  */
    const double*   sTabP,         /* => point chainage or nullptr          */
    int             nPts            /* => Number of points in line string */
    )
    {
    PrflPoint_t   *pointTabP = nullptr;
    double        currAbc = dc_zero;
    double        distOnCurrentSegment = dc_zero;
    double        distOrig = dc_zero;
    double        diffSOrig = dc_zero;
    int           iPt;
    int           currPtOrig;
    bool      isShift = FALSE;

    BC_START ();

    // Check argument
    if (nResPt == 0) BC_RETURN ();

    // Allocate the array of points
    pointTabP = (PrflPoint_t*)bcMem_calloc (nResPt, sizeof (PrflPoint_t));

    // Set current origin point to the first one                                |
    currPtOrig = 0;

    // Set first point                                                          |
    BC_TRY (
        digitalTM_setPoint (pointTabP, 0, currAbc, drapedPt));

    if (sTabP)
        {
        // If there is an abcissa table, then go to the next origin point
        DPoint3d    pt;
        pt.x = drapedPt[0].drapePt.x;
        pt.y = drapedPt[0].drapePt.y;
        pt.z = drapedPt[0].drapePt.z;
        BC_TRY (
            digitalTM_nextOriginPoint (&pt, origStringP, sTabP, nPts, &currPtOrig, &diffSOrig, &distOrig, &distOnCurrentSegment));
        }

    for (iPt = 1; iPt < nResPt; iPt++)
        {
        DPoint3d    previousPt;
        previousPt.x = drapedPt[iPt - 1].drapePt.x;
        previousPt.y = drapedPt[iPt - 1].drapePt.y;
        previousPt.z = drapedPt[iPt - 1].drapePt.z;
        DPoint3d    currentPt;
        currentPt.x = drapedPt[iPt].drapePt.x;
        currentPt.y = drapedPt[iPt].drapePt.y;
        currentPt.z = drapedPt[iPt].drapePt.z;
        if (sTabP)
            {
            if (distOrig == dc_zero)
                {
                // If this condition is true it means that the last time we called digitalTM_nextOriginPoint
                // we have not found any further point, so the current abcissa is the same than previous
                currAbc = sTabP[currPtOrig-1];
                }
            else
                {
                DPoint3d    currOriPt;
                currOriPt = origStringP[currPtOrig];

                // If there is an abcissa table, then calculate the distance along the
                // current segment
                distOnCurrentSegment += bcPoint_distance2d (&previousPt, &currentPt);

                // Calculate the abcissa as a ratio of the current segment virtual length
                double  ratio = distOnCurrentSegment / distOrig;
                if (ratio > dc_1)
                    ratio = dc_1;
                currAbc = sTabP[currPtOrig-1] + ratio * diffSOrig;
                if (currOriPt.x == currentPt.x && currOriPt.y == currentPt.y)
                    {
                    // If the point of the line string is the same than the point of the
                    // original one, we go to the next original point
                    if (iPt < nResPt-1)
                        {
                        BC_TRY (
                            digitalTM_nextOriginPoint (&currentPt, origStringP, sTabP, nPts, &currPtOrig, &diffSOrig, &distOrig, &distOnCurrentSegment));

                        // If the difference in station on the original string is more than 0 and the distance between absissa is zero
                        // it means that we are on a shift
                        isShift = (diffSOrig == dc_zero && distOrig > dc_zero);
                        }
                    }
                }
            }
        else
            {
            // Calculate the abcissa as the distance between the two points
            currAbc += bcPoint_distance2d (&previousPt, &currentPt);;
            }

        if (isShift && distOnCurrentSegment > dc_zero)
            {
            // If we are on a shift and along the current segment (not at the beginning)
            // We mark the draped pt as a void point so that it will not be append to
            // the result (it creates a vetical gap in the profile)
            drapedPt[iPt].drapeType = DTMDrapedLineCode::Void;
            }
        // Set current point
        BC_TRY (
            digitalTM_setPoint (pointTabP, iPt, currAbc, drapedPt));
        }

    *pointTabPP = pointTabP;
    *nbPt = nResPt;

BC_END:;

    BC_END_RETURNSTATUS();
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.09sep2001   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
static int digitalTM_convertPrflPointTabToDrapedPoints
    (
    bvector<RefCountedPtr<BcDTMDrapedLinePoint>>     &drapedPoints,
    PrflPoint_t             pointTab[],
    long                    nbPt
    )
    {
    // Reserve roo in the vector
    drapedPoints.reserve (nbPt);

    for (long iPt = 0; iPt < nbPt; iPt++)
        {
        DPoint3d    pt;

        pt.x = pointTab[iPt].bcdtmDrapeData.drapePt.x;
        pt.y = pointTab[iPt].bcdtmDrapeData.drapePt.y;
        pt.z = pointTab[iPt].bcdtmDrapeData.drapePt.z;

        RefCountedPtr<BcDTMDrapedLinePoint> drapedPoint = BcDTMDrapedLinePoint::Create (pt, pointTab[iPt].s, pointTab[iPt].bcdtmDrapeData.drapeType);
        for (int iFeature = 0; iFeature < pointTab[iPt].bcdtmDrapeData.drapeFeatures.size(); iFeature++)
            {
            auto& feature = pointTab[iPt].bcdtmDrapeData.drapeFeatures[iFeature];
            drapedPoint->AddFeature (
                feature.dtmFeature,
                feature.dtmFeatureType,
                feature.userTag,
                feature.userFeatureId,
                feature.priorPoint,
                feature.nextPoint
                );
            }

        drapedPoints.push_back (drapedPoint);
        }

    return DTM_SUCCESS;
    }


/*----------------------------------------------------------------------+
|                                                                       |
|   cbe.16feb2006   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
BENTLEYDTM_EXPORT  int digitalTM_getDrapedPoints
    (
    BC_DTM_OBJ              *dtmHandleP,
    bvector<RefCountedPtr<BcDTMDrapedLinePoint>>&    drapedPoints,
    DPoint3dCP              lineStringP,   /* => Points of line string to drape      */
    const double*           absTabP,       /* => abcissa table or nullptr              */
    int                     nPts            /* => Number of points in line string     */
    )
    {
    bvector<DTMDrapePoint> drapedPts;
    PrflPoint_t     *prflPointTabP = nullptr;
    long            nbPtProfile;
    int             status = 0;

    BC_START ();

    if (dtmHandleP == nullptr) BC_RETURN_ERRSTATUS (DTM_ERROR);
    status = bcdtmDrape_stringDtmObject (dtmHandleP, (DPoint3d*)lineStringP, nPts, true, drapedPts);
    if (status != DTM_SUCCESS) BC_RETURN_ERRSTATUS(DTM_ERROR);

    // Create a digital TM profile with readjustment of the distances along
    BC_TRY(digitalTM_createProfile(&prflPointTabP, &nbPtProfile, drapedPts.data(), (int)drapedPts.size(), lineStringP, absTabP, nPts));

    // Drape the points
    BC_TRY (digitalTM_convertPrflPointTabToDrapedPoints (drapedPoints, prflPointTabP, nbPtProfile));

BC_END:;

    if (prflPointTabP != nullptr) bcMem_free (prflPointTabP);

    BC_END_RETURNSTATUS();
    }
#pragma endregion

struct Dtm_Handler_t
    {
    DTMBrowseSlopeIndicatorCallback slopeIndicatorHandler;
    DTMBrowseSinglePointFeatureCallback featuresSinglePointHandler;
    void *userP;
    };

static int loadSinglePointFeatureCallBack
    (
    DTMFeatureType featureType,
    DTMUserTag featureTag,
    DTMFeatureId featureId,
    DPoint3dP tPoint,
    size_t nPoint,
    void* userP
    )
    {
    Dtm_Handler_t* dtmHandler = (Dtm_Handler_t*)userP;
    DPoint3d  point;
    point.x = tPoint[0].x;
    point.y = tPoint[0].y;
    point.z = tPoint[0].z;


    dtmHandler->featuresSinglePointHandler(featureType, point, dtmHandler->userP);
    return (DTM_SUCCESS);
    }

static int slopeIndicatorsCallBack
    (
    DTMFeatureType featureType,
    DTMUserTag   featureTag,
    DTMFeatureId featureId,
    DPoint3dP     tPoint,
    size_t    nPoint,
    void    *userP
    )
    {
    Dtm_Handler_t* dtmHandler = (Dtm_Handler_t*)userP;
    bool majorInterval = 0 ;
    DPoint3d  startPoint;
    startPoint.x = tPoint[0].x;
    startPoint.y = tPoint[0].y;
    startPoint.z = tPoint[0].z;
    DPoint3d  endPoint;
    endPoint.x = tPoint[1].x;
    endPoint.y = tPoint[1].y;
    endPoint.z = tPoint[1].z;
    dtmHandler->slopeIndicatorHandler (majorInterval, startPoint, endPoint, dtmHandler->userP);
    return (DTM_SUCCESS);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
BcDTMPtr BcDTM::Create ()
    {
    // Create a new Digital TM instance
    return new BcDTM ();
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
BcDTMPtr BcDTM::Create (int initPoint, int incPoint)
    {
    // Create a new Digital TM instance
    return new BcDTM (initPoint, incPoint);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
BcDTMPtr BcDTM::CreateFromDtmHandle
    (
    BC_DTM_OBJ& dtmHandleP
    )
    {
    // Create a new Digital TM instance
    return new BcDTM (dtmHandleP);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.25aug2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
BcDTMPtr BcDTM::CreateFromDtmHandles
    (
    BC_DTM_OBJ* headerP,
    void *featureArraysP,
    void *pointArraysP,
    void *nodeArraysP,
    void *fListArraysP,
    void *cListArraysP
    )
    {
    // Create a new Digital TM instance
    return new BcDTM (headerP, featureArraysP, pointArraysP, nodeArraysP, fListArraysP, cListArraysP);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
BcDTMPtr BcDTM::CreateFromTinFile (WCharCP fileNameP)
    {
    int         status = 0;
    BC_DTM_OBJ* dtmObjectP = nullptr;

    if (wcslen (fileNameP) == 0)
        return nullptr;

    /* Read the tin file and get the DTMFeatureState::Tin handle*/
    status = bcdtmRead_fromFileDtmObject (&dtmObjectP, fileNameP);
    if (status != DTM_SUCCESS)
        {
        if( dtmObjectP != nullptr ) bcdtmObject_destroyDtmObject(&dtmObjectP) ;
        return nullptr ;
        }

    // Create a new DTM instance
    return new BcDTM (*dtmObjectP);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
BcDTMPtr BcDTM::CreateFromGeopakDatFile (WCharCP fileNameP)
    {
    int         status = 0;
    BC_DTM_OBJ* dtmObjectP = nullptr;

    if (wcslen (fileNameP) == 0) return nullptr;

    // Read Geopak Dat File To DTM Object
    status = bcdtmRead_geopakDatFileToDtmObject(&dtmObjectP,fileNameP) ;
    if (status != DTM_SUCCESS)
        {
        if( dtmObjectP != nullptr ) bcdtmObject_destroyDtmObject(&dtmObjectP) ;
        return nullptr ;
        }

    // Create a new DTM instance
    return new BcDTM (*dtmObjectP);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
BcDTMPtr BcDTM::CreateFromXyzFile (WCharCP fileNameP)
    {
    int         status = 0;
    BC_DTM_OBJ* dtmObjectP = nullptr;


    if (wcslen (fileNameP) == 0)
        return nullptr;

    // Read XYZ File To DTM Object
    status = bcdtmRead_xyzFileToDtmObject(&dtmObjectP,fileNameP) ;
    if (status != DTM_SUCCESS)
        {
        if( dtmObjectP != nullptr ) bcdtmObject_destroyDtmObject(&dtmObjectP) ;
        return nullptr ;
        }

    // Create a new DTM instance
    return new BcDTM (*dtmObjectP);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.18may2009   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
BcDTMPtr BcDTM::CreateFromStream (IBcDtmStreamR streamP)
    {
    int         status = 0;
    BC_DTM_OBJ* dtmObjectP = nullptr;

    // Copy Memory Block To DTM Object
    status = bcdtmReadStream_atFilePositionDtmObject(&dtmObjectP, &streamP, 0);
    if (status != DTM_SUCCESS)
        {
        if( dtmObjectP != nullptr ) bcdtmObject_destroyDtmObject(&dtmObjectP) ;
        return nullptr ;
        }

    // Create a new DTM instance
    return new BcDTM (*dtmObjectP);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.18may2009   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
BcDTMPtr BcDTM::CreateFromMemoryBlock (const char *memoryBlockP, unsigned long memoryBlockSize)
    {
    int         status = 0;
    BC_DTM_OBJ* dtmObjectP = nullptr;

    // Copy Memory Block To DTM Object
    status = bcdtmObject_createFromMemoryBlockDtmObject (&dtmObjectP, (char*)memoryBlockP, memoryBlockSize) ;
    if (status != DTM_SUCCESS)
        {
        if( dtmObjectP != nullptr ) bcdtmObject_destroyDtmObject (&dtmObjectP) ;
        return nullptr ;
        }

    // Create a new DTM instance
    return new BcDTM (*dtmObjectP);
    }


/*----------------------------------------------------------------------+
|                                                                       |
|   mah.08nov2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
BcDTMPtr BcDTM::CreateFromTinFile (WCharCP fileNameP, double filePosition)
    {
   DTMStatusInt status=DTM_SUCCESS;
    BC_DTM_OBJ  *dtmObjectP = nullptr;

    status = (DTMStatusInt)bcdtmRead_openAndReadAtFilePositionDtmObject (&dtmObjectP, fileNameP, filePosition);
    if ( status != DTM_SUCCESS)
        {
        if( dtmObjectP != nullptr ) bcdtmObject_destroyDtmObject(&dtmObjectP) ;
        return nullptr ;
        }

    // Create a new Digital TM instance
    return new BcDTM (*dtmObjectP);
    }

BcDTMPtr BcDTM::DesignPondToTargetVolumeOrElevation
    (
    long* pondFlag,
    double* pondElevation,
    double* pondVolume,
    DPoint3d* points,
    long numPoints,
    long perimeterOrInvert,
    long targetVolumeOrElevation,
    double targetVolume,
    double targetElevation,
    double sideSlope,
    double freeBoard
    )
    {

    DTMStatusInt status = DTM_ERROR;
    BcDTMPtr dtmP = BcDTM::Create ();
    // ToDo Translatation
// TODO: Drainage
    //status = bcdtmDrainage_designPondToATargetVolumeOrElevationDtmObject(dtmP->GetTinHandle(), pondFlag, pondElevation, pondVolume,
    //    points,  numPoints, perimeterOrInvert, targetVolumeOrElevation,
    //    targetVolume, targetElevation, sideSlope, freeBoard,
    //    0,0.0,0.0,0,0.0,100.0, 0, nullptr, 0.0);

    if ( status != DTM_SUCCESS)
        {
        return nullptr ;
        }

    // Return this DTM
    return dtmP;
    }


/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
BcDTM::BcDTM ()
    {
    _dtmHandleP = nullptr;
    // Assign the DTMFeatureState::Tin handle
    bcdtmObject_createDtmObject (&_dtmHandleP);
    if (_dtmHandleP != nullptr)
        _dtmHandleP->refCount++;

    // Initialisations
    _readonly = false;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
BcDTM::BcDTM (int initPoint, int incPoint)
    {
    // Assign the DTMFeatureState::Tin handle
    _dtmHandleP = nullptr;
    bcdtmObject_createDtmObject (&_dtmHandleP);
    bcdtmObject_setPointMemoryAllocationParametersDtmObject(_dtmHandleP, initPoint, incPoint) ;
    if (_dtmHandleP != nullptr)
        _dtmHandleP->refCount++;

    // Initialisations
    _readonly = false;

    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
BcDTM::BcDTM (BC_DTM_OBJ& dtmHandleP, TMTransformHelperP transformHelper, bool readonly)
    {
    // Assign the DTMFeatureState::Tin handle
    _dtmHandleP = &dtmHandleP;
    _dtmTransformHelper = transformHelper;

    _dtmHandleP->refCount++;

    // Initialisations
    _readonly = readonly;

    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.25aug2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
BcDTM::BcDTM(BC_DTM_OBJ* headerP, void* featureArraysP, void* pointArraysP, void* nodeArraysP, void* fListArraysP, void* cListArraysP)
    {
    int dbg=DTM_TRACE_VALUE(0) ;

    // Initialise
    _readonly = false;

    _dtmHandleP = nullptr ;

    if( dbg ) bcdtmWrite_message(0,0,0,"Creating DTM Object") ;
    if( bcdtmObject_createDtmObject(&_dtmHandleP) != DTM_SUCCESS )
        {
        _dtmHandleP = nullptr ;
        return ;
        }
    if ( ((BC_DTM_OBJ *)headerP)->dtmObjVersion == BC_DTM_OBJ_VERSION )
        memcpy(_dtmHandleP, headerP, DTMIOHeaderSize) ;
    else
        {
        bcdtmWrite_message(1,0,0,"Invalid DTM header version.") ;
        free (_dtmHandleP);
        _dtmHandleP = nullptr;
        return;
        }
    _dtmHandleP->DTMAllocationClass = nullptr ;
    _dtmHandleP->refCount = 0 ;
    _dtmHandleP->fTablePP = (BC_DTM_FEATURE **)featureArraysP;
    _dtmHandleP->pointsPP = (DPoint3d **)pointArraysP;
    _dtmHandleP->nodesPP = (DTM_TIN_NODE **)nodeArraysP;
    _dtmHandleP->cListPP = (DTM_CIR_LIST **)cListArraysP;
    _dtmHandleP->fListPP = (DTM_FEATURE_LIST **)fListArraysP;

    _dtmHandleP->refCount++;
    //

    if( dbg )
        {
        if     ( _dtmHandleP->dtmObjType == BC_DTM_OBJ_TYPE ) bcdtmWrite_message(0,0,0,"_dtmHandleP = %p dtmObjType = BC_DTM_OBJ_TYPE ** refCount = %2ld",_dtmHandleP,_dtmHandleP->refCount) ;
        else if( _dtmHandleP->dtmObjType == BC_DTM_ELM_TYPE ) bcdtmWrite_message(0,0,0,"_dtmHandleP = %p dtmObjType = BC_DTM_ELM_TYPE ** refCount = %2ld",_dtmHandleP,_dtmHandleP->refCount) ;
        }

    // Initialisations

    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
BcDTM::~BcDTM ()
    {
    // Free the attached DTMFeatureState::Tin handle
    _dtmTransformHelper = nullptr;
    if (_dtmHandleP != nullptr)
        {
        if( (_dtmHandleP)->refCount > 0 ) (_dtmHandleP)->refCount--;
        if ((_dtmHandleP)->refCount == 0 && ! bcdtmGeopak_checkForCurrentGeopakDtmObject(_dtmHandleP))
            {
            digitalTM_freeTinHandle (&_dtmHandleP);
            }
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::DrapeLinearPoints
    (
    BcDTMDrapedLinePtr&     drapedLinePP,
    DPoint3dCP              pointsP,
    const double*           distTabP,
    int                     nbPt
    )
    {
    BcDTMDrapedLinePtr      drapedLineP;
    bvector<RefCountedPtr<BcDTMDrapedLinePoint>>     drapedPoints;

    BC_START ();

    // Check argument
    if (pointsP == nullptr)
        return DTM_ERROR;
    if (drapedLinePP != nullptr)
        return DTM_ERROR;

    // Drape the points
    if (_dtmTransformHelper.IsValid ())
        {
        BC_TRY (digitalTM_getDrapedPoints (GetTinHandle (), drapedPoints, _dtmTransformHelper->copyPointsToDTM (pointsP, nbPt), _dtmTransformHelper->copyDistanceTableToDTM (distTabP, nbPt), nbPt));
        _dtmTransformHelper->convertDrapedPointsFromDTM (drapedPoints);
        }
    else
        {
        BC_TRY (digitalTM_getDrapedPoints (GetTinHandle (), drapedPoints, pointsP, distTabP, nbPt));
        }
    // Create the resultant point array
    // Here there is a direct call to the function which make a BcDTMDrapedLine, because
    // The creator or draped lines are not public
    drapedLineP = BcDTMDrapedLine::Create (drapedPoints);
    if (drapedLineP.IsNull())
        BC_RETURN_ERRSTATUS (DTM_ERROR);
    drapedLinePP = drapedLineP;

BC_END:

    // Cleaup arrays
    BC_END_RETURNSTATUS();
    }
/*----------------------------------------------------------------------+
|                                                                       |
|   cbe.04Apr2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::GetFeatureById (BcDTMFeaturePtr& featurePP, DTMFeatureId identP)
    {
    DTMFeatureType memFeatureType = DTMFeatureType::None;
    DPoint3d    *pointsP = nullptr;
    DPoint3d    *memPointsP = nullptr;
    long        memNbFeaturePts = 0;
   DTMStatusInt status=DTM_SUCCESS;
    DTMFeatureId    dtmFeatureId;
    DTMUserTag memUserTag;
    BC_DTM_USER_FEATURE dtmFeature ;
    long        featureFound = 0;
    long        firstCall = TRUE;

    dtmFeature.dtmFeaturePtsP = nullptr;
    BC_START ();

    featurePP = nullptr;
    do
        {

        dtmFeatureId = identP ;
        status = (DTMStatusInt)bcdtmScanLoad_nextDtmFeatureOccurrenceForDtmFeatureIdFromDtmObject (
            GetTinHandle(), dtmFeatureId, firstCall, FALSE, DTMFenceType::Block, DTMFenceOption::Inside,  nullptr,
            0, &featureFound, &dtmFeature );

        if (status != DTM_SUCCESS) BC_RETURN_ERRSTATUS(DTM_ERROR);

        if (featureFound)
            {
            Copy (&pointsP, dtmFeature.dtmFeaturePtsP,dtmFeature.numDtmFeaturePts);
            if (_dtmTransformHelper.IsValid ())
                _dtmTransformHelper->convertPointsFromDTM (pointsP, dtmFeature.numDtmFeaturePts);

            if (dtmFeature.dtmFeatureType == DTMFeatureType::GroupSpots)
                {
                if (firstCall || (featurePP == nullptr))
                    {
                    featurePP = BcDTMSpot::Create (dtmFeature.dtmUserTag, identP, dtmFeature.dtmFeatureType, pointsP, dtmFeature.numDtmFeaturePts);
                    }
                else
                    {
                    BcDTMSpotPtr pFeature = featurePP->AsSpot ();
                    if(pFeature != nullptr)
                        {
                        pFeature->AppendPoints (pointsP, dtmFeature.numDtmFeaturePts);
                        }
                    }
                }
            else
                {
                // Make a new linear feature
                if (firstCall)
                    {
                    memPointsP      = pointsP;
                    memFeatureType  = dtmFeature.dtmFeatureType ;
                    memNbFeaturePts = dtmFeature.numDtmFeaturePts;
                    memUserTag      = dtmFeature.dtmUserTag;
                    pointsP = nullptr;
                    }
                else
                    {
                    if (memPointsP != nullptr)
                        {
                        featurePP = BcDTMComplexLinearFeature::Create (memFeatureType, memUserTag, identP, memPointsP, memNbFeaturePts);
                        if (memPointsP == nullptr)
                            {
                            bcMem_free (memPointsP);
                            memPointsP = nullptr;
                            }
                        if (featurePP == nullptr) BC_RETURN_ERRSTATUS(DTM_ERROR);
                        }
                    BcDTMComplexLinearFeaturePtr lFeature = featurePP->AsComplexLinear();

                    lFeature->AppendElement (DtmString(pointsP, dtmFeature.numDtmFeaturePts));
                    }
                }
            if (pointsP)
                {
                bcMem_free (pointsP);
                pointsP = nullptr;
                }
            }
        firstCall = FALSE;
        } while (featureFound);

        if (memPointsP != nullptr)
            {
            featurePP = BcDTMLinearFeature::Create (memFeatureType, memUserTag, identP, memPointsP, memNbFeaturePts);
            if (memPointsP == nullptr)
                {
                bcMem_free (memPointsP);
                memPointsP = nullptr;
                }
            if (featurePP == nullptr) BC_RETURN_ERRSTATUS(DTM_ERROR);
            }

BC_END:;

        // Free the DPoint3d array
        if (pointsP != nullptr)
            bcMem_free (pointsP);

        BC_END_RETURNSTATUS();
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   cbe.23jmar2005   -  Created.                                        |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::GetFeatureByUserTag (BcDTMFeaturePtr& featurePP, DTMUserTag userTag)
    {
    WString     featureName;
    WString     featureDescription;
    DPoint3d    *pointsP = nullptr;
    int         status = 0;
    DTMFeatureId   guId = DTM_NULL_FEATURE_ID;
    long        featureFound = 0;
    BC_DTM_USER_FEATURE dtmFeature;

    BC_START ();

    // Check arguments
    featurePP = nullptr ;

    memset (&dtmFeature, 0, sizeof (BC_DTM_USER_FEATURE));

    status = bcdtmScanLoad_nextDtmFeatureOccurrenceForUserTagFromDtmObject (
        GetTinHandle(), userTag, TRUE, FALSE, DTMFenceType::Block, DTMFenceOption::Inside,  nullptr,
        0, &featureFound, &dtmFeature);
    if (status != DTM_SUCCESS) BC_RETURN_ERRSTATUS(DTM_ERROR);

    if (!featureFound) BC_RETURN_ERRSTATUS(DTM_SUCCESS);
    //    Copy (guId, dtmFeature.dtmFeatureId);

    // Copy DPoint3d[] in DPoint3d[]
    Copy (&pointsP, dtmFeature.dtmFeaturePtsP,dtmFeature.numDtmFeaturePts);
    if (_dtmTransformHelper.IsValid ())
        _dtmTransformHelper->convertPointsFromDTM (pointsP, dtmFeature.numDtmFeaturePts);

    if (dtmFeature.dtmFeatureType == DTMFeatureType::GroupSpots)
        {
        featurePP = BcDTMSpot::Create (userTag, guId, dtmFeature.dtmFeatureType, pointsP, dtmFeature.numDtmFeaturePts);
        }
    else
        {
        // Make a new linear feature
        featurePP = BcDTMLinearFeature::Create (dtmFeature.dtmFeatureType, userTag, guId, pointsP, dtmFeature.numDtmFeaturePts);

        if (featurePP == nullptr) BC_RETURN_ERRSTATUS(DTM_ERROR);
        }

BC_END:;

    // Free the DPoint3d array
    if (pointsP != nullptr) bcMem_free (pointsP);

    BC_END_RETURNSTATUS();
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::GetFeatureEnumerator
    (
    BcDTMFeatureEnumeratorPtr& enumPP
    )
    {
    // Make an enumerator with this DTMFeatureState::Tin handle
    enumPP = BcDTMFeatureEnumerator::Create (this);
    return DTM_SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::ClipByPointString
    (
    BcDTMPtr&        clippedPP,
    DPoint3dCP       points,
    int              nbPt,
    DTMClipOption   clippingMethod
    )
    {
    BC_DTM_OBJ  *resultTinP = nullptr;

    SetMemoryAccess (DTMAccessMode::NodesOnly);

    // Check arguments
    if (points    == nullptr)   return DTM_ERROR;
    if (nbPt < 3         )   return DTM_ERROR;

    // Clip the DTM
    DTMStatusInt status;

    if (_dtmTransformHelper.IsValid())
        status = (DTMStatusInt)bcdtmClip_cloneAndClipToPolygonDtmObject (GetTinHandle (), (BC_DTM_OBJ **)&resultTinP, _dtmTransformHelper->copyPointsToDTM (points, nbPt), nbPt, clippingMethod);
    else
        status = (DTMStatusInt)bcdtmClip_cloneAndClipToPolygonDtmObject (GetTinHandle (), (BC_DTM_OBJ **)&resultTinP, (DPoint3d*)points, nbPt, clippingMethod);
    if (status != DTM_SUCCESS) return status;

    // Return result
    if (resultTinP != nullptr)
        clippedPP = new BcDTM (*resultTinP, _dtmTransformHelper.get());
    else
        clippedPP = nullptr;

    return DTM_SUCCESS;
    }


/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::_GetRange (DRange3dR range)
    {
    DPoint3d    diffRange;

    if (GetTinHandle ()->numPoints == 0)
        {
        range = DRange3d::NullRange ();
        return DTM_SUCCESS;
        }
    // Call Rob's function
    DTMStatusInt status = (DTMStatusInt)bcdtmUtility_getBoundingCubeDtmObject (GetTinHandle (),
        &range.low.x, &range.low.y, &range.low.z,
        &range.high.x, &range.high.y, &range.high.z,
        &diffRange.x, &diffRange.y, &diffRange.z);

    if (status != DTM_SUCCESS)
        return status;

    if (_dtmTransformHelper.IsValid())
        _dtmTransformHelper->convertRangeFromDTM (range);
    return DTM_SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.07jun2010   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::DrapePoint
    (
    int&        drapeTypeP,
    DPoint3d&   pointP
    )
    {
    DTMStatusInt ret=DTM_SUCCESS ;
    long drapeFlag=0 ;

    /* Drape the point on the tin object */
    if (_dtmTransformHelper.IsValid ())
        {
        DPoint3d p = pointP;
        _dtmTransformHelper->convertPointToDTM (p);
        if (bcdtmDrape_pointDtmObject (GetTinHandle (), p.x, p.y, &p.z, &drapeFlag) != DTM_SUCCESS) goto errexit;
        pointP.z = _dtmTransformHelper->convertElevationFromDTM (p.z);
        }
    else
        if (bcdtmDrape_pointDtmObject (GetTinHandle (), pointP.x, pointP.y, &pointP.z, &drapeFlag) != DTM_SUCCESS) goto errexit;

    drapeTypeP = (int)drapeFlag;

    //  Clean Up

cleanup :

    //  Return

    return(ret) ;

    // Error Exit

errexit :

    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;

    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/


DTMStatusInt BcDTM::_DrapeLinear(Bentley::TerrainModel::DTMDrapedLinePtr& ret, DPoint3dCP pts, int numPoints)
    {
    // Drape the resulting point array
    BcDTMDrapedLinePtr drapedLineP = nullptr;

    // DrapePointArray does transformation.
    DTMStatusInt status = DrapeLinearPoints (drapedLineP, pts, nullptr, numPoints);

    if (status == DTM_SUCCESS)
        ret = drapedLineP->GetIDTMDrapedLine();
    return status;
    }

bool BcDTM::_ProjectPoint(DPoint3dR pointOnDTM, DMatrix4dCR w2vMap, DPoint3dCR testPoint)
    {
    return _GetProjectedPointOnDTM(pointOnDTM, w2vMap, testPoint);
    }

bool  BcDTM::_IntersectRay(DPoint3dR pointOnDTM, DVec3dCR direction, DPoint3dCR testPoint)
    {
    DPoint3d endPoint;
    endPoint.SumOf(testPoint, direction);
    return IntersectVector(pointOnDTM, testPoint, endPoint);
    }

bool BcDTM::_DrapeAlongVector(DPoint3d* endPt, double *slope, double *aspect, DPoint3d triangle[3], int *drapedType, DPoint3dCR point, double directionOfVector, double slopeOfVector)
    {
    long startFlag, endFlag;
    return (DTM_SUCCESS == _ShotVector(slope, aspect, triangle, drapedType, &startFlag, &endFlag, endPt, &const_cast<DPoint3dR>(point), directionOfVector, slopeOfVector) && endFlag == 0);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::_DrapePoint
    (
    double      *elevationP,
    double      *slopeP,
    double      *aspectP,
    DPoint3d    triangle[3],
    int&         drapedTypeP,
    const DPoint3d&    pointP
    )
    {
    long        drapeFlag = 0;
    double      z = 0;
    double      slope = 0;
    double      aspect = 0;
    DPoint3d    trianglePoints[3] = {{0,0,0},{0,0,0},{0,0,0}};
    double      slopeDegrees = 0;
    double      height = 0;
    int         status = 0;

    BC_START ();

    // Initialize results
    if (elevationP) *elevationP = dc_zero;
    if (slopeP) *slopeP = dc_zero;
    if (aspectP) *aspectP = dc_zero;
    if (triangle) memset (triangle, 0, 3*sizeof(DPoint3d));

    /* Drape the point on the tin object */

    if (_dtmTransformHelper.IsValid ())
        {
        DPoint3d p = pointP;
        _dtmTransformHelper->convertPointToDTM (p);
        status = bcdtmDrape_pointReturnAttributesDtmObject (
            GetTinHandle (),
            p.x,
            p.y,
            &z,
            &drapeFlag,
            (DPoint3d *)&trianglePoints[0],
            &slopeDegrees,
            &slope,
            &aspect,
            &height);
        if (drapeFlag == 1 || drapeFlag == 3)
            {
            z = _dtmTransformHelper->convertElevationFromDTM (z);
            _dtmTransformHelper->convertPointsFromDTM (trianglePoints, 3);
            slope = _dtmTransformHelper->convertSlopeFromDTM (slope);
            aspect = _dtmTransformHelper->convertAspectFromDTM (aspect);
            }
        }
    else
        {
        status = bcdtmDrape_pointReturnAttributesDtmObject (
            GetTinHandle (),
            pointP.x,
            pointP.y,
            &z,
            &drapeFlag,
            (DPoint3d *)&trianglePoints[0],
            &slopeDegrees,
            &slope,
            &aspect,
            &height);
        }
    if (status)  BC_RETURN_ERRSTATUS(status);

    if (drapeFlag == 1 || drapeFlag == 3)
        {
        if (elevationP) *elevationP = z;
        if (slopeP)     *slopeP = slope;
        if (aspectP)    *aspectP = aspect;
        if (triangle) memcpy(triangle, &trianglePoints[0], 3 * sizeof(trianglePoints[0]));
        }
    drapedTypeP = drapeFlag;

BC_END:;

    BC_END_RETURNSTATUS();
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::Save
    (
    WCharCP fileNameP
    )
    {
    // ToDo Translatation???
    DTMStatusInt status = (DTMStatusInt)bcdtmWrite_toFileDtmObject (GetTinHandle (), fileNameP);
    if (status != DTM_SUCCESS) return DTM_ERROR;
    return DTM_SUCCESS;
    }
/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.03sep2008   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::SaveAsGeopakDat
    (
    WCharCP fileNameP
    )
    {
    // ToDo Translatation
    DTMStatusInt status = (DTMStatusInt)bcdtmWrite_geopakDatFileFromDtmObject (GetTinHandle (), fileNameP);
    if (status != DTM_SUCCESS) return DTM_ERROR;
    return DTM_SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   vru.16apr2010   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::SaveAsGeopakAsciiDat
    (
    WCharCP fileNameP,
    int numDecPts
    )
    {
    // ToDo Translatation
    DTMStatusInt status = (DTMStatusInt)bcdtmWrite_asciiGeopakDatFileFromDtmObject (GetTinHandle (), fileNameP, numDecPts);
    if (status != DTM_SUCCESS) return DTM_ERROR;
    return DTM_SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   djh.03mar2010   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::SaveToStream
    (
    IBcDtmStreamR streamP
    )
    {
    // ToDo Translatation
    DTMStatusInt status = (DTMStatusInt)bcdtmWriteStream_atFilePositionDtmObject (GetTinHandle (), &streamP, 0);
    if (status != DTM_SUCCESS) return DTM_ERROR;
    return DTM_SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.28jul2008   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::SaveAsGeopakTinFile
    (
    WCharCP fileNameP
    )
    {
    // ToDo Translatation
    return((DTMStatusInt)bcdtmExport_geopakTriangulationFromDtmObject (GetTinHandle (), fileNameP));
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   MathieuSt 4 april 2016                                              |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::_ExportToGeopakTinFile 
    (
    WCharCP fileNameP
    )
    {
    return SaveAsGeopakTinFile(fileNameP);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.29jul2008   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::PopulateFromGeopakTinFile
    (
    WCharCP fileNameP
    )
    {
    // ToDo Translatation???
    SetMemoryAccess(DTMAccessMode::Write);
    return( (DTMStatusInt)bcdtmImport_geopakTriangulationToExistingDtmObject(GetTinHandle(), fileNameP)) ;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::CalculateSlopeArea
    (
    double&         areaP,
    DPoint3dCP      points,
    int             nbPt
    )
    {
    DTMStatusInt status = DTM_SUCCESS;
    long    numSlopePolygons=0 ;
    double  flatArea = dc_zero;
    DTM_POINT_ARRAY **slopePolygonsPP=nullptr ;

    SetMemoryAccess(DTMAccessMode::Temporary);
    // Call the core DTM slope area function
    if (_dtmTransformHelper.IsValid())
        status = (DTMStatusInt)bcdtmSlope_calculateSlopeAreaDtmObject (GetTinHandle (), (DPoint3d *)_dtmTransformHelper->copyPointsToDTM (points, nbPt), nbPt, nullptr, &flatArea, &areaP, &slopePolygonsPP, &numSlopePolygons);
    else
        status = (DTMStatusInt)bcdtmSlope_calculateSlopeAreaDtmObject (GetTinHandle (), (DPoint3d *)points, nbPt, nullptr, &flatArea, &areaP, &slopePolygonsPP, &numSlopePolygons);

    if (_dtmTransformHelper.IsValid())
        areaP = _dtmTransformHelper->convertAreaFromDTM (areaP);
    // Clean Up
    if( slopePolygonsPP != nullptr )bcdtmMem_freePointerArrayToPointArrayMemory(&slopePolygonsPP,numSlopePolygons) ;

    // Return
    return status ;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::CalculateSlopeArea
    (
    double&          areaP
    )
    {
    DTMStatusInt status = DTM_SUCCESS;
    long    numSlopePolygons=0 ;
    double  flatArea = dc_zero;
    DTM_POINT_ARRAY **slopePolygonsPP=nullptr ;

    SetMemoryAccess(DTMAccessMode::Temporary);
    // Call the core DTM slope area function
    status = (DTMStatusInt)bcdtmSlope_calculateSlopeAreaDtmObject (GetTinHandle (), nullptr, 0, nullptr, &flatArea, &areaP, &slopePolygonsPP, &numSlopePolygons);

    if (_dtmTransformHelper.IsValid())
        areaP = _dtmTransformHelper->convertAreaFromDTM (areaP);

    // Clean Up
    if( slopePolygonsPP != nullptr )bcdtmMem_freePointerArrayToPointArrayMemory(&slopePolygonsPP,numSlopePolygons) ;

    // Return
    return status ;
    }
/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.04Oct2010   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::_CalculateSlopeArea
    (
    double&         flatAreaP ,
    double&         slopeAreaP,
    DPoint3dCP      polygonPts,
    int             numPolygonPts

    )
    {
    DTMStatusInt status=DTM_SUCCESS ;
    long    numSlopePolygons=0 ;
    DTM_POINT_ARRAY **slopePolygonsPP=nullptr ;

    SetMemoryAccess(DTMAccessMode::Temporary);
    // Call the core DTM slope area function
    if (_dtmTransformHelper.IsValid())
        status = (DTMStatusInt)bcdtmSlope_calculateSlopeAreaDtmObject (GetTinHandle (), (DPoint3d *)_dtmTransformHelper->copyPointsToDTM (polygonPts, numPolygonPts), (long)numPolygonPts, nullptr, &flatAreaP, &slopeAreaP, &slopePolygonsPP, &numSlopePolygons);
    else
        status = (DTMStatusInt)bcdtmSlope_calculateSlopeAreaDtmObject (GetTinHandle (), (DPoint3d *)polygonPts, (long)numPolygonPts, nullptr, &flatAreaP, &slopeAreaP, &slopePolygonsPP, &numSlopePolygons);

    if (_dtmTransformHelper.IsValid())
        {
        flatAreaP = _dtmTransformHelper->convertAreaFromDTM (flatAreaP);
        slopeAreaP = _dtmTransformHelper->convertAreaFromDTM (slopeAreaP);
        }
    // Clean Up
    if( slopePolygonsPP != nullptr )bcdtmMem_freePointerArrayToPointArrayMemory(&slopePolygonsPP,numSlopePolygons) ;

    // Return
    return status ;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::Merge
    (
    BcDTMR toMergeP
    )
    {
    BcDTMPtr toMerge = (TMTransformHelper::convertDTMToDTM (*this, toMergeP));

    if (SetMemoryAccess(DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message(1,0,0, "DTM is readonly");
        return DTM_ERROR;
        }
    toMerge->SetMemoryAccess (DTMAccessMode::Temporary);
    DTMStatusInt status = (DTMStatusInt)bcdtmMerge_dtmObjects (GetTinHandle (), toMerge->GetTinHandle (), 0);
    if (status != DTM_SUCCESS) return DTM_ERROR;
    return DTM_SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   cbe.10jan2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::MergeAdjacent
    (
    BcDTMR toMergeP
    )
    {
    BcDTMPtr toMerge = (TMTransformHelper::convertDTMToDTM (*this, toMergeP));

    if (SetMemoryAccess(DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message(1,0,0, "DTM is readonly");
        return DTM_ERROR;
        }
    toMerge->SetMemoryAccess(DTMAccessMode::Temporary);
    DTMStatusInt status = (DTMStatusInt)bcdtmMerge_dtmObjects (GetTinHandle (), (BC_DTM_OBJ*)toMerge->GetTinHandle (), 1);
    if (status != DTM_SUCCESS) return DTM_ERROR;
    return DTM_SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   cbe.10jan2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
BcDTMPtr BcDTM::Clone()
    {
    BC_DTM_OBJ *dtmHandleP = nullptr;

    // ToDo what coordinates do we want this to be.
    // Merge the TINs in this DTMFeatureState::Tin
    bcdtmObject_cloneDtmObject(GetTinHandle(), (BC_DTM_OBJ **) &dtmHandleP);

    // Create a new Digital TM instance
    return new BcDTM (*dtmHandleP, _dtmTransformHelper.get());
    }
/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.02may2008   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::Append (BcDTMR dtmP)
    {
    BcDTMPtr dtm = (TMTransformHelper::convertDTMToDTM (*this, dtmP));

    if (SetMemoryAccess(DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message(1,0,0, "DTM is readonly");
        return DTM_ERROR;
        }
    // Append The DTM Object
    return((DTMStatusInt)bcdtmObject_appendDtmObject (GetTinHandle (), dtm->GetTinHandle ()));

    }

/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.17Oct2008   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::AppendWithUserTag (BcDTMR dtmP, DTMUserTag userTag)
    {
    BcDTMPtr dtm = (TMTransformHelper::convertDTMToDTM (*this, dtmP));

    if (SetMemoryAccess(DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message(1,0,0, "DTM is readonly");
        return DTM_ERROR;
        }
    // Append The DTM Object
    return((DTMStatusInt)bcdtmObject_appendWithUsertagDtmObject (GetTinHandle (), dtm->GetTinHandle (), userTag));

    }

/*----------------------------------------------------------------------+
|                                                                       |
|   djh.10apr2008   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
BcDTMPtr BcDTM::Delta (BcDTMR toDeltaP, DPoint3dCP points, int numPoints)
    {
    BcDTMPtr toDelta = (TMTransformHelper::convertDTMToDTM (*this, toDeltaP));

    BC_DTM_OBJ *deltaP = nullptr;

    SetMemoryAccess(DTMAccessMode::Temporary);
    toDelta->SetMemoryAccess(DTMAccessMode::Temporary);
    BeAssert (GetTinHandle() != nullptr);

    BC_DTM_OBJ* toDeltaHandleP = toDelta->GetTinHandle();
    // make the delta tin
    DTMStatusInt status = (DTMStatusInt)bcdtmDelta_cloneAndCreateDeltaTinToSurfaceDtmObject (GetTinHandle (), toDeltaHandleP, &deltaP, (DPoint3d*)points, numPoints);
    if (status == DTM_ERROR)
        {
        return nullptr;
        }

    // Create a new Digital TM instance
    return new BcDTM (*deltaP, _dtmTransformHelper.get());
    }
/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.04Oct2011   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
BcDTMPtr BcDTM::DeltaElevation (double elevation, DPoint3dCP points, int numPoints)
    {
    BC_DTM_OBJ *deltaP = nullptr;

    DTMStatusInt status;
    // Call the Core Method To Create The Delta Tin To An Elevation
    if (_dtmTransformHelper.IsValid())
        status = (DTMStatusInt)bcdtmDelta_cloneAndCreateDeltaTinToElevationDtmObject (GetTinHandle (), &deltaP, _dtmTransformHelper->convertElevationToDTM (elevation), _dtmTransformHelper->copyPointsToDTM (points, numPoints), numPoints);
    else
        status = (DTMStatusInt)bcdtmDelta_cloneAndCreateDeltaTinToElevationDtmObject (GetTinHandle (), &deltaP, elevation, (DPoint3d*)points, numPoints);
    if (status == DTM_ERROR)
        {
        return nullptr;
        }

    // Create a new Digital TM instance
    return new BcDTM (*deltaP, _dtmTransformHelper.get());
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   cbe.10jan2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::OffsetDeltaElevation (double offset)
    {
    BeAssert (GetTinHandle() != nullptr);
    if (SetMemoryAccess(DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message(1,0,0, "DTM is readonly");
        return DTM_ERROR;
        }
    DTMStatusInt status;
    if (_dtmTransformHelper.IsValid ())
        offset = _dtmTransformHelper->convertDistanceToDTM (offset);
    status = (DTMStatusInt)bcdtmData_moveZDtmObject (GetTinHandle (), 2 /*increment*/, offset);
    return status ;
    }
    
    
/*----------------------------------------------------------------------+
|                                                                       |
|   spu.06nov2003   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::_ShotVector(double* endSlopeP, double* endAspectP, DPoint3d endTriangle[3], int* endDrapedTypeP, long* startFlag, long* endFlag, DPoint3dP endPtP, DPoint3dP startPtP,
    double direction, double slope)
    {
    DPoint3d    endPt = {0, 0, 0};
    *startFlag = 0;
    *endFlag = 0;

    // Call DTMFeatureState::Tin function
    DTMStatusInt status;

    if (_dtmTransformHelper.IsValid ())
        {
        DPoint3d startPt = *startPtP;
        _dtmTransformHelper->convertPointToDTM (startPt);
        direction = _dtmTransformHelper->convertAspectToDTM (direction);
        slope = _dtmTransformHelper->convertSlopeToDTM (slope);
        status = (DTMStatusInt)bcdtmSideSlope_intersectSurfaceDtmObject (GetTinHandle (), startPt.x, startPt.y, startPt.z,
                                                         direction, slope, 1, dc_zero, &endPt.x, &endPt.y, &endPt.z, startFlag, endFlag);
        if (status == DTM_SUCCESS)
            _dtmTransformHelper->convertPointFromDTM (endPt);
        }
    else
        {
        status = (DTMStatusInt)bcdtmSideSlope_intersectSurfaceDtmObject (GetTinHandle (), startPtP->x, startPtP->y, startPtP->z,
                                                         direction, slope, 1, dc_zero, &endPt.x, &endPt.y, &endPt.z, startFlag, endFlag);
        }
    if (status != DTM_SUCCESS) return DTM_ERROR;

    if (endPtP != nullptr)
        {
        // If the end point is required return it
        *endPtP = endPt;
        }

    if (endSlopeP != nullptr || endAspectP != nullptr || endTriangle != nullptr || endDrapedTypeP != nullptr)
        {
        // If other info about the end point are required return them
        DTMStatusInt status = DrapePoint (nullptr, endSlopeP, endAspectP, endTriangle, *endDrapedTypeP, endPt);
        if (status != DTM_SUCCESS)
            return DTM_ERROR;
        }

    return DTM_SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.06nov2003   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::ShotVector (double* endSlopeP, double* endAspectP, DPoint3d endTriangle[3], int* endDrapedTypeP, DPoint3dP endPtP, DPoint3dP startPtP,
    double direction, double slope)
    {
    DPoint3d    endPt = {0, 0, 0};
    long        startFlag = 0, endFlag = 0;

    // Call DTMFeatureState::Tin function
    DTMStatusInt status;

    if (_dtmTransformHelper.IsValid ())
        {
        DPoint3d startPt = *startPtP;
        _dtmTransformHelper->convertPointToDTM (startPt);
        direction = _dtmTransformHelper->convertAspectToDTM (direction);
        slope = _dtmTransformHelper->convertSlopeToDTM (slope);
        status = (DTMStatusInt)bcdtmSideSlope_intersectSurfaceDtmObject (GetTinHandle (), startPt.x, startPt.y, startPt.z,
                                                         direction, slope, 1, dc_zero, &endPt.x, &endPt.y, &endPt.z, &startFlag, &endFlag);
        if (status == DTM_SUCCESS)
            _dtmTransformHelper->convertPointFromDTM (endPt);
        }
    else
        {
        status = (DTMStatusInt)bcdtmSideSlope_intersectSurfaceDtmObject (GetTinHandle (), startPtP->x, startPtP->y, startPtP->z,
                                                         direction, slope, 1, dc_zero, &endPt.x, &endPt.y, &endPt.z, &startFlag, &endFlag);
        }
    if (status != DTM_SUCCESS) return DTM_ERROR;

    if (endPtP != nullptr)
        {
        // If the end point is required return it
        *endPtP = endPt;
        }

    if (endSlopeP != nullptr || endAspectP != nullptr || endTriangle != nullptr || endDrapedTypeP != nullptr)
        {
        // If other info about the end point are required return them
        int tmp;
        DTMStatusInt status = _DrapePoint (nullptr, endSlopeP, endAspectP, endTriangle, endDrapedTypeP ? *endDrapedTypeP : tmp, endPt);
        if (status != DTM_SUCCESS)
            return DTM_ERROR;
        }

    return DTM_SUCCESS;
    }


/*----------------------------------------------------------------------+
|                                                                       |
|   mah.09nov2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::ComputePlanarPrismoidalVolume (
    BcDTMVolumeAreaResult& result, double elevation, DPoint3dCP points,
    int nbPt, VOLRANGETAB* rangeTableP, int numRanges)
    {
    DTMStatusInt status;

    if (_dtmTransformHelper.IsValid())
        {
        TMTransformHelper::VOLRANGETABCopy volRange = _dtmTransformHelper->copyVOLRANGETABToDTM (rangeTableP, numRanges);
        status = (DTMStatusInt)bcdtmTinVolume_surfaceToElevationDtmObject (GetTinHandle (), volRange, numRanges, (DPoint3d*)points, nbPt, _dtmTransformHelper->convertElevationToDTM (elevation), nullptr, nullptr, result.cutVolume, result.fillVolume, result.balanceVolume, result.cutArea, result.fillArea);
        if(rangeTableP && status == DTM_SUCCESS)
            _dtmTransformHelper->copyVOLRANGETABFromDTM (volRange, rangeTableP, numRanges);
        }
    else
        status = (DTMStatusInt)bcdtmTinVolume_surfaceToElevationDtmObject (GetTinHandle (), rangeTableP, numRanges, (DPoint3d*)points, nbPt, elevation, nullptr, nullptr, result.cutVolume, result.fillVolume, result.balanceVolume, result.cutArea, result.fillArea);
    if (status != DTM_SUCCESS) return DTM_ERROR;

    if (_dtmTransformHelper.IsValid())
        {
        result.cutVolume = _dtmTransformHelper->convertVolumeFromDTM (result.cutVolume);
        result.fillVolume = _dtmTransformHelper->convertVolumeFromDTM (result.fillVolume);
        result.balanceVolume = _dtmTransformHelper->convertVolumeFromDTM (result.balanceVolume);
        result.cutArea = _dtmTransformHelper->convertAreaFromDTM (result.cutArea);
        result.fillArea = _dtmTransformHelper->convertAreaFromDTM (result.fillArea);
        }

    result.totalArea = result.cutArea + result.fillArea;
    return  DTM_SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.06nov2003   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::CalculateCutFillVolume (BcDTMVolumeAreaResult& result, BcDTMR otherDtmP, DPoint3dCP points,
                                             int nbPt, VOLRANGETAB* rangeTableP, int numRanges)
    {
    DTMStatusInt status;

    BcDTMPtr otherDtm = (TMTransformHelper::convertDTMToDTM (*this, otherDtmP));
    if (_dtmTransformHelper.IsValid ())
        {
        TMTransformHelper::VOLRANGETABCopy volRange = _dtmTransformHelper->copyVOLRANGETABToDTM (rangeTableP, numRanges);
        status = (DTMStatusInt)bcdtmTinVolume_surfaceToSurfaceDtmObjects (GetTinHandle (), (BC_DTM_OBJ*)otherDtm->GetTinHandle (), volRange, numRanges, (DPoint3d*)_dtmTransformHelper->copyPointsToDTM (points, nbPt), nbPt, nullptr, nullptr, result.cutVolume, result.fillVolume, result.balanceVolume, result.cutArea, result.fillArea);

        if (_dtmTransformHelper.IsValid ())
            {
            result.cutVolume = _dtmTransformHelper->convertVolumeFromDTM (result.cutVolume);
            result.fillVolume = _dtmTransformHelper->convertVolumeFromDTM (result.fillVolume);
            result.balanceVolume = _dtmTransformHelper->convertVolumeFromDTM (result.balanceVolume);
            result.cutArea = _dtmTransformHelper->convertAreaFromDTM (result.cutArea);
            result.fillArea = _dtmTransformHelper->convertAreaFromDTM (result.fillArea);
            if (rangeTableP) _dtmTransformHelper->copyVOLRANGETABFromDTM (volRange, rangeTableP, numRanges);
            }
        }
    else
        status = (DTMStatusInt)bcdtmTinVolume_surfaceToSurfaceDtmObjects (GetTinHandle (), otherDtm->GetTinHandle (), rangeTableP, numRanges, (DPoint3d*)points, nbPt, nullptr, nullptr, result.cutVolume, result.fillVolume, result.balanceVolume, result.cutArea, result.fillArea);

    if (status != DTM_SUCCESS) return status;

    result.totalArea = result.cutArea + result.fillArea;

    return  DTM_SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.12mar2009   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
BENTLEYDTM_Public int  bcDTM_volumeCallBackFunction (DTMFeatureType dtmFeatureType,DTMUserTag dtmUserTag, DTMFeatureId dtmFeatureId,DPoint3dP polyPtsP,size_t numPolyPts,void *userP)
    /*
    ** This Function Receives The Volume Polygons From The Volume Calculation Functions
    */
    {
    int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
    DPoint3dCP p3dP ;
    DtmString polygon  ;
    DtmVectorString *DtmVolumePolygonsP = (DtmVectorString*)userP ;
    /*
    ** Test For Polygon Feature
    */
    if( dtmFeatureType == DTMFeatureType::Polygon && numPolyPts > 0 )
        {
        if( dbg ) bcdtmWrite_message(0,0,0,"Polygon Returned ** polyPtsP = %p numPolyPts = %8ld",polyPtsP,numPolyPts) ;
        if( dbg == 2 )
            {
            for( p3dP = polyPtsP ; p3dP < polyPtsP + numPolyPts ; ++p3dP )
                {
                bcdtmWrite_message(0,0,0,"Point[%4ld] ** x = %12.5lf y = %12.5lf z = %10.4lf",(long)(p3dP-polyPtsP),p3dP->x,p3dP->y,p3dP->z) ;
                }
            }
        polygon.resize (numPolyPts) ;
        memcpy(polygon.data(),polyPtsP,numPolyPts*sizeof(DPoint3d)) ;
        DtmVolumePolygonsP->push_back(polygon) ;
        }
    /*
    ** Return
    */
cleanup :
    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Volume Call Back Function Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Volume Call Back Function Error") ;
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.12mar2009   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::CalculatePrismoidalVolumeToElevation (BcDTMVolumeAreaResult& result, DtmVectorString* volumePolygonsP,
    double elevation, DPoint3dCP points, int nbPt, VOLRANGETAB* rangeTableP, int numRanges)
    {
    DTMStatusInt status=DTM_SUCCESS ;

    // Clear Existing Volume Polygons

    volumePolygonsP->clear() ;

    // Call Core Dtm Function
    if (_dtmTransformHelper.IsValid ())
        {
        TMTransformHelper::VOLRANGETABCopy volRange = _dtmTransformHelper->copyVOLRANGETABToDTM (rangeTableP, numRanges);
        status = (DTMStatusInt)bcdtmTinVolume_surfaceToElevationDtmObject (GetTinHandle (), volRange, numRanges, (DPoint3d*)_dtmTransformHelper->copyPointsToDTM (points, nbPt), nbPt, _dtmTransformHelper->convertElevationToDTM (elevation), bcDTM_volumeCallBackFunction, volumePolygonsP, result.cutVolume, result.fillVolume, result.balanceVolume, result.cutArea, result.fillArea);

        if (status != DTM_SUCCESS)
            {
            result.cutVolume = _dtmTransformHelper->convertVolumeFromDTM (result.cutVolume);
            result.fillVolume = _dtmTransformHelper->convertVolumeFromDTM (result.fillVolume);
            result.balanceVolume = _dtmTransformHelper->convertVolumeFromDTM (result.balanceVolume);
            result.cutArea = _dtmTransformHelper->convertAreaFromDTM (result.cutArea);
            result.fillArea = _dtmTransformHelper->convertAreaFromDTM (result.fillArea);
            if (rangeTableP) _dtmTransformHelper->copyVOLRANGETABFromDTM (volRange, rangeTableP, numRanges);
            if (volumePolygonsP) _dtmTransformHelper->convertDtmStringVectorFromDTM (*volumePolygonsP);
            }
        }
    else
        {
        status = (DTMStatusInt)bcdtmTinVolume_surfaceToElevationDtmObject (GetTinHandle (), rangeTableP, numRanges, (DPoint3d*)points, nbPt, elevation, bcDTM_volumeCallBackFunction, volumePolygonsP, result.cutVolume, result.fillVolume, result.balanceVolume, result.cutArea, result.fillArea);
        }
    result.totalArea = result.cutArea + result.fillArea;

    return  status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|    rsc.12mar2009   -  Created.                                        |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::CalculatePrismoidalVolumeToSurface (BcDTMVolumeAreaResult& result, DtmVectorString* volumePolygonsP,
                                                BcDTMR otherDtmP, DPoint3dCP points, int nbPt, VOLRANGETAB* rangeTableP, int numRanges)
    {
    DTMStatusInt status = DTM_SUCCESS;

    // Clear Existing Volume Polygons

    volumePolygonsP->clear ();

    BcDTMPtr otherDtm = (TMTransformHelper::convertDTMToDTM (*this, otherDtmP));
    // Call Core Dtm Function
    if (_dtmTransformHelper.IsValid ())
        {
        TMTransformHelper::VOLRANGETABCopy volRange = _dtmTransformHelper->copyVOLRANGETABToDTM (rangeTableP, numRanges);
        status = (DTMStatusInt)bcdtmTinVolume_surfaceToSurfaceDtmObjects (GetTinHandle (), (BC_DTM_OBJ*)otherDtm->GetTinHandle (), volRange, numRanges, (DPoint3d*)_dtmTransformHelper->copyPointsToDTM (points, nbPt), nbPt, bcDTM_volumeCallBackFunction, volumePolygonsP, result.cutVolume, result.fillVolume, result.balanceVolume, result.cutArea, result.fillArea);

        if (status != DTM_SUCCESS)
            {
            result.cutVolume = _dtmTransformHelper->convertVolumeFromDTM (result.cutVolume);
            result.fillVolume = _dtmTransformHelper->convertVolumeFromDTM (result.fillVolume);
            result.balanceVolume = _dtmTransformHelper->convertVolumeFromDTM (result.balanceVolume);
            result.cutArea = _dtmTransformHelper->convertAreaFromDTM (result.cutArea);
            result.fillArea = _dtmTransformHelper->convertAreaFromDTM (result.fillArea);
            if (rangeTableP) _dtmTransformHelper->copyVOLRANGETABFromDTM (volRange, rangeTableP, numRanges);
            if (volumePolygonsP) _dtmTransformHelper->convertDtmStringVectorFromDTM (*volumePolygonsP);
            }
        }
    else
        status = (DTMStatusInt)bcdtmTinVolume_surfaceToSurfaceDtmObjects (GetTinHandle (), otherDtm->GetTinHandle (), rangeTableP, numRanges, (DPoint3d*)points, nbPt, bcDTM_volumeCallBackFunction, volumePolygonsP, result.cutVolume, result.fillVolume, result.balanceVolume, result.cutArea, result.fillArea);
    result.totalArea = result.cutArea + result.fillArea;

    // Return
    return  status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|    rsc.13mar2009   -  Created.                                        |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::CalculatePrismoidalVolumeBalanceToSurface (double &fromAreaP, double &toAreaP, double &balanceP, DtmVectorString* volumePolygonsP,
                                                       BcDTMR otherDtmP, DPoint3dCP points, int nbPt)
    {
   DTMStatusInt status=DTM_SUCCESS;

    // Clear Existing Volume Polygons
    volumePolygonsP->clear ();

    BcDTMPtr otherDtm = (TMTransformHelper::convertDTMToDTM (*this, otherDtmP));
    // Call Core Dtm Function
    if (_dtmTransformHelper.IsValid ())
        {
        status = (DTMStatusInt)bcdtmTinVolume_surfaceToSurfaceBalanceDtmObjects (GetTinHandle (), otherDtm->GetTinHandle (), (DPoint3d*)_dtmTransformHelper->copyPointsToDTM (points, nbPt), nbPt, bcDTM_volumeCallBackFunction, volumePolygonsP, &fromAreaP, &toAreaP, &balanceP);
        if (status == DTM_SUCCESS)
            {
            fromAreaP = _dtmTransformHelper->convertAreaFromDTM (fromAreaP);
            toAreaP = _dtmTransformHelper->convertAreaFromDTM (toAreaP);
            balanceP = _dtmTransformHelper->convertVolumeFromDTM (balanceP);
            if (volumePolygonsP) _dtmTransformHelper->convertDtmStringVectorFromDTM (*volumePolygonsP);
            }
        }
    else
        status = (DTMStatusInt)bcdtmTinVolume_surfaceToSurfaceBalanceDtmObjects (GetTinHandle (), otherDtm->GetTinHandle (), (DPoint3d*)points, nbPt, bcDTM_volumeCallBackFunction, volumePolygonsP, &fromAreaP, &toAreaP, &balanceP);

    return status;
    }
/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.13mar2009   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::CalculateGridVolumeToElevation (BcDTMVolumeAreaResult& result, long& numCellsUsedP, double& cellAreaP,
                                            DtmVectorString* volumePolygonsP, long numLatticePoints, double elevation, DPoint3dCP points, int nbPt,
                                            VOLRANGETAB* rangeTableP, int numRanges)
    {
   DTMStatusInt status=DTM_SUCCESS;

    // Clear Existing Volume Polygons

    volumePolygonsP->clear ();

    // Call Core Dtm Function
    if (_dtmTransformHelper.IsValid ())
        {
        TMTransformHelper::VOLRANGETABCopy volRange = _dtmTransformHelper->copyVOLRANGETABToDTM (rangeTableP, numRanges);
        status = (DTMStatusInt)bcdtmLatticeVolume_surfaceToElevationDtmObject (GetTinHandle (), numLatticePoints, volRange, numRanges, (DPoint3d*)_dtmTransformHelper->copyPointsToDTM (points, nbPt), nbPt, _dtmTransformHelper->convertElevationToDTM (elevation), bcDTM_volumeCallBackFunction, volumePolygonsP, result.cutVolume, result.fillVolume, result.balanceVolume, result.cutArea, result.fillArea, numCellsUsedP, cellAreaP);
        if (status != DTM_SUCCESS)
            {
            result.cutVolume = _dtmTransformHelper->convertVolumeFromDTM (result.cutVolume);
            result.fillVolume = _dtmTransformHelper->convertVolumeFromDTM (result.fillVolume);
            result.balanceVolume = _dtmTransformHelper->convertVolumeFromDTM (result.balanceVolume);
            result.cutArea = _dtmTransformHelper->convertAreaFromDTM (result.cutArea);
            result.fillArea = _dtmTransformHelper->convertAreaFromDTM (result.fillArea);
            if (cellAreaP) _dtmTransformHelper->convertAreasFromDTM (&cellAreaP, numCellsUsedP);
            if (volumePolygonsP) _dtmTransformHelper->convertDtmStringVectorFromDTM (*volumePolygonsP);
            if (rangeTableP) _dtmTransformHelper->copyVOLRANGETABFromDTM (volRange, rangeTableP, numRanges);
            }
        }
    else
        status = (DTMStatusInt)bcdtmLatticeVolume_surfaceToElevationDtmObject (GetTinHandle (), numLatticePoints, rangeTableP, numRanges, (DPoint3d*)points, nbPt, elevation, bcDTM_volumeCallBackFunction, volumePolygonsP, result.cutVolume, result.fillVolume, result.balanceVolume, result.cutArea, result.fillArea, numCellsUsedP, cellAreaP);

    result.totalArea = result.cutArea + result.fillArea;
    return  status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.13mar2009   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::CalculateGridVolumeToSurface (BcDTMVolumeAreaResult& result, long& numCellsUsedP, double& cellAreaP,
                                          DtmVectorString* volumePolygonsP, BcDTMR otherDtmP, long numLatticePoints, DPoint3dCP points, int nbPt,
                                          VOLRANGETAB* rangeTableP, int numRanges)
    {
   DTMStatusInt status=DTM_SUCCESS;

    // Clear Existing Volume Polygons

    volumePolygonsP->clear ();

    // Call Core Dtm Function
    BcDTMPtr otherDtm = (TMTransformHelper::convertDTMToDTM (*this, otherDtmP));

    if (_dtmTransformHelper.IsValid ())
        {
        TMTransformHelper::VOLRANGETABCopy volRange = _dtmTransformHelper->copyVOLRANGETABToDTM (rangeTableP, numRanges);
        status = (DTMStatusInt)bcdtmLatticeVolume_surfaceToSurfaceDtmObjects (GetTinHandle (), otherDtm->GetTinHandle (), numLatticePoints, volRange, numRanges, (DPoint3d*)_dtmTransformHelper->copyPointsToDTM (points, nbPt), nbPt, bcDTM_volumeCallBackFunction, volumePolygonsP, result.cutVolume, result.fillVolume, result.balanceVolume, result.cutArea, result.fillArea, numCellsUsedP, cellAreaP);
        if (status != DTM_SUCCESS)
            {
            result.cutVolume = _dtmTransformHelper->convertVolumeFromDTM (result.cutVolume);
            result.fillVolume = _dtmTransformHelper->convertVolumeFromDTM (result.fillVolume);
            result.balanceVolume = _dtmTransformHelper->convertVolumeFromDTM (result.balanceVolume);
            result.cutArea = _dtmTransformHelper->convertAreaFromDTM (result.cutArea);
            result.fillArea = _dtmTransformHelper->convertAreaFromDTM (result.fillArea);
            _dtmTransformHelper->convertAreasFromDTM (&cellAreaP, numCellsUsedP);
            if (volumePolygonsP) _dtmTransformHelper->convertDtmStringVectorFromDTM (*volumePolygonsP);
            if (rangeTableP) _dtmTransformHelper->copyVOLRANGETABFromDTM (volRange, rangeTableP, numRanges);
            }
        }
    else
        status = (DTMStatusInt)bcdtmLatticeVolume_surfaceToSurfaceDtmObjects (GetTinHandle (), otherDtm->GetTinHandle (), numLatticePoints, rangeTableP, numRanges, (DPoint3d*)points, nbPt, bcDTM_volumeCallBackFunction, volumePolygonsP, result.cutVolume, result.fillVolume, result.balanceVolume, result.cutArea, result.fillArea, numCellsUsedP, cellAreaP);

    result.totalArea = result.cutArea + result.fillArea;

    return  status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   djh.05mar2011   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::CalculateFeatureStatistics (DTMFeatureStatisticsInfo& info) const
    {
    DTMState dummyState;
    long dummy = 0;
    long numPoints;
    long numTinLines;
    long numTriangles;
    long numDtmFeatures;
    long numBreaks;
    long numContourLines;
    long numVoids;
    long numIslands;
    long numHoles;
    long numGroupSpots;
    bool hasHull;
    BeAssert (GetTinHandle() != nullptr);

    DTMStatusInt status = (DTMStatusInt)bcdtmUtility_getStatisticsDtmObject (GetTinHandle (), dummyState, numPoints, numTinLines, numTriangles, dummy,
        dummy, numDtmFeatures, numBreaks, numContourLines, numVoids, numIslands, numHoles, numGroupSpots, hasHull, dummy);

    info.numPoints = numPoints;
    info.numTinLines = numTinLines;
    info.numTriangles = numTriangles;
    info.numDtmFeatures = numDtmFeatures;
    info.numBreaks = numBreaks;
    info.numContourLines = numContourLines;
    info.numVoids = numVoids;
    info.numIslands = numIslands;
    info.numHoles = numHoles;
    info.numGroupSpots = numGroupSpots;
    info.hasHull = hasHull;

    BeAssert (status == DTM_SUCCESS);
    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   djh.05mar2011   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
int BcDTM::GetTrianglesCount () const
    {
    long    nbTriangles, nbTriangleEdges = 0;

    BeAssert (GetTinHandle() != nullptr);

    DTMStatusInt status = (DTMStatusInt)bcdtmList_countNonVoidTrianglesAndLinesDtmObject (GetTinHandle (), &nbTriangles, &nbTriangleEdges);
    BeAssert (status == DTM_SUCCESS);

    return (int)nbTriangles;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   cbe.11jan2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::BrowseSlopeIndicator (BcDTMR dtmP, double majorInterval, double minorInterval, void* userP, DTMBrowseSlopeIndicatorCallback callBackFunctP)
    {
    BeAssert (GetTinHandle() != nullptr);
    BrowseSlopeIndicatorCallbackTransformHelper helper (callBackFunctP, userP, _dtmTransformHelper.get ());
    Dtm_Handler_t dtmHandlerArgs;
    dtmHandlerArgs.slopeIndicatorHandler = callBackFunctP;
    dtmHandlerArgs.userP = userP;

    DTMStatusInt status;
    BcDTMPtr dtm = (TMTransformHelper::convertDTMToDTM (*this, dtmP));
    status = (DTMStatusInt)bcdtmInterruptLoad_slopeLinesBetweenDtmObjects (GetTinHandle (), dtm->GetTinHandle (), slopeIndicatorsCallBack, majorInterval, minorInterval, &dtmHandlerArgs);

    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.3Mar2011  -  Created.                                           |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::CalculateCatchments (bool refineOption, double falseLowDepth, const DTMFenceParams& fence, void* userP, DTMFeatureCallback callBackFunctP)
    {
    // BeAssert If Tin handle Is nullptr
    BeAssert (GetTinHandle () != nullptr);

    SetMemoryAccess (DTMAccessMode::Temporary);

    // Check For And Set Fence
    FeatureCallbackTransformHelper helper (callBackFunctP, userP, _dtmTransformHelper.get ());
    userP = helper.GetUserArg ();
    callBackFunctP = helper.GetCallBackFunc ();

    if (_dtmTransformHelper.IsValid ())
        falseLowDepth = _dtmTransformHelper->convertDistanceToDTM (falseLowDepth);

    return BcDTMDrainage::ReturnCatchments (this, nullptr, falseLowDepth, refineOption, callBackFunctP, fence, userP);
    }


/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.15Mar2010   -  Created                                          |
|                                                                       |
+----------------------------------------------------------------------*/
static int AddDtmFeatureToBuffer (DTMFeatureType featureType, DTMUserTag featureTag, DTMFeatureId featureId, DPoint3d* featurePtsP, size_t numFeaturePts, void* userP)
    {
    DTMFeatureBuffer* buffer = (DTMFeatureBuffer*) userP;
    return buffer->AddDtmFeatureToBuffer (featureType, featureTag, featureId, featurePtsP, (int)numFeaturePts);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.15Mar2010   -  Created                                          |
|                                                                       |
+----------------------------------------------------------------------*/
void DTMFeatureBuffer::EmptyDtmFeatureBuffer()
    {
    dtmFeatures.clear();
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.15Mar2010   -  Created                                          |
|                                                                       |
+----------------------------------------------------------------------*/
int DTMFeatureBuffer::AddDtmFeatureToBuffer (DTMFeatureType featureType, DTMUserTag featureTag, DTMFeatureId featureId, DPoint3dP featurePtsP, int numFeaturePts)
    {
   DTMStatusInt status=DTM_SUCCESS;

    int index = (int)dtmFeatures.size ();
    dtmFeatures.push_back (DtmFeature ());
    DtmFeature* featP = &dtmFeatures[index];

    //   Add Feature To Vector Buffer
    featP->featureType = featureType;
    featP->featureTag = featureTag;
    featP->featureId = featureId;
    featP->featurePts.resize (numFeaturePts);

    memcpy (&featP->featurePts[0], featurePtsP, sizeof(DPoint3d) * numFeaturePts);

    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.15Mar2010   -  Created                                          |
|                                                                       |
+----------------------------------------------------------------------*/
int DTMFeatureBuffer::GetDtmDynamicFeaturesArray (DTMDynamicFeatureArray& dynamicArray, TMTransformHelper* transformHelper)
    {
   DTMStatusInt status=DTM_SUCCESS;

    // Initialise
    int index = (int)dtmFeatures.size ();

    // Scan Features And Accumulate Dynamic Features
    if (index > 0)
        {
        dynamicArray.resize (index);

        // Copy Buffer Items
        for (int n = 0; n < index; ++n)
            {
            dynamicArray[n].featureType = dtmFeatures[n].featureType;
            dynamicArray[n].featurePts = dtmFeatures[n].featurePts;
            if (transformHelper)
                transformHelper->convertPointsFromDTM (&dtmFeatures[n].featurePts[0], (int)dtmFeatures[n].featurePts.size ());
            }

        }

    //   Clean Up
    return status;
    }
/*----------------------------------------------------------------------+
|                                                                       |
|   cbe.11jan2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|   cbe.11jan2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::BrowseDrainageFeatures (DTMFeatureType featureType, double* minLowPointP, const DTMFenceParams& fence, void *userP, DTMFeatureCallback callBackFunctP)
    {
    DTMStatusInt status = DTM_SUCCESS;
    bool useFence = false;
    int numFeatures = 0;
    DTMFenceOption fenceOption = fence.fenceOption;
    FeatureCallbackTransformHelper helper (callBackFunctP, userP, _dtmTransformHelper.get());
    userP = helper.GetUserArg ();
    callBackFunctP = helper.GetCallBackFunc ();
    // BeAssert If Tin handle Is nullptr
    BeAssert (GetTinHandle () != nullptr);

    SetMemoryAccess (DTMAccessMode::Temporary);

    // Check For And Set Fence
    if (fence.points)
        useFence = true;

    // Call the core DTM function
    switch (featureType)
        {
            case DTMFeatureType::LowPoint:
                if (*minLowPointP == 0)
                    {
                    status = (DTMStatusInt)bcdtmDrainage_returnLowPointsDtmObject (GetTinHandle (), callBackFunctP, useFence, fence.fenceType, fenceOption,
                                                                     TMTransformHelper::copyPointsToDTM (_dtmTransformHelper.get (), fence.points, fence.numPoints), fence.numPoints, userP, numFeatures);
                    }
                else
                    {
                    status = (DTMStatusInt)bcdtmDrainage_returnNoneFalseLowPointsDtmObject (GetTinHandle (), *minLowPointP, callBackFunctP, useFence, fence.fenceType, fenceOption,
                                                                              TMTransformHelper::copyPointsToDTM (_dtmTransformHelper.get (), fence.points, fence.numPoints), fence.numPoints, userP, numFeatures);
                    }
                break;

            case  DTMFeatureType::HighPoint:
                status = (DTMStatusInt)bcdtmDrainage_returnHighPointsDtmObject (GetTinHandle (), callBackFunctP, useFence, fence.fenceType, fenceOption,
                                                                  TMTransformHelper::copyPointsToDTM (_dtmTransformHelper.get (), fence.points, fence.numPoints), fence.numPoints, userP, numFeatures);
                break;

            case DTMFeatureType::SumpLine:
                status = (DTMStatusInt)bcdtmDrainage_returnSumpLinesDtmObject (GetTinHandle (), callBackFunctP, useFence, fence.fenceType, fenceOption,
                                                                 TMTransformHelper::copyPointsToDTM (_dtmTransformHelper.get (), fence.points, fence.numPoints), fence.numPoints, userP, numFeatures);
                break;

            case DTMFeatureType::RidgeLine:
                status = (DTMStatusInt)bcdtmDrainage_returnRidgeLinesDtmObject (GetTinHandle (), callBackFunctP, useFence, fence.fenceType, fenceOption,
                                                                  TMTransformHelper::copyPointsToDTM (_dtmTransformHelper.get (), fence.points, fence.numPoints), fence.numPoints, userP, numFeatures);
                break;

            case DTMFeatureType::Catchment:
                // ToDo Drainage status = (DTMStatusInt)bcdtmDrainage_determineCatchmentsDtmObject (GetTinHandle (), callBackFunctP, *minLowPointP, useFence, fence.fenceType, fenceOption,  TMTransformHelper::copyPointsToDTM (_dtmTransformHelper.get (), fence.points, fence.numPoints), fence.numPoints, userP, numFeatures);
                break;
        };
    return status;
    }
/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.26June2009  -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::BrowseTriangleMesh (long maxTriangles, const DTMFenceParams& fence, void* userP, DTMTriangleMeshCallback callBackFunctP)
    {
    DTMStatusInt status = DTM_SUCCESS;

    BeAssert (GetTinHandle () != nullptr);
    TriangleMeshCallbackTransformHelper helper (callBackFunctP, userP, _dtmTransformHelper.get ());
    callBackFunctP = helper.GetCallBackFunc ();
    userP = helper.GetUserArg ();

    // Call the core DTM function
    if (fence.fenceType != DTMFenceType::None)
        status = (DTMStatusInt)bcdtmInterruptLoad_triangleMeshFromDtmObject (GetTinHandle (), maxTriangles, callBackFunctP, true, fence.fenceType, fence.fenceOption, (DPoint3d *)fence.points, fence.numPoints, userP);
    else
        status = (DTMStatusInt)bcdtmInterruptLoad_triangleMeshFromDtmObject (GetTinHandle (), maxTriangles, callBackFunctP, false, DTMFenceType::None, DTMFenceOption::None, nullptr, 0, userP);

    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   cbe.11jan2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::BrowseFeatures (DTMFeatureType featureType, long maxSpots, void* userP, DTMFeatureCallback callBackFunctP)
    {
    return BrowseFeatures (featureType, DTMFenceParams (), maxSpots, userP, callBackFunctP);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   cbe.11jan2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::BrowseFeatures (DTMFeatureType featureType, const DTMFenceParams& fence, long maxSpots, void* userP, DTMFeatureCallback callBackFunctP)
    {
    DTMStatusInt status = DTM_ERROR;

    FeatureCallbackTransformHelper helper (callBackFunctP, userP, _dtmTransformHelper.get ());
    userP = helper.GetUserArg ();
    callBackFunctP = helper.GetCallBackFunc ();

    int numFeatures = 0;
    bool isFence = true;

    BeAssert (GetTinHandle () != nullptr);

    switch (featureType)
        {
            case DTMFeatureType::SumpLine:
                status = (DTMStatusInt)bcdtmDrainage_returnSumpLinesDtmObject (GetTinHandle (), callBackFunctP, isFence, fence.fenceType, fence.fenceOption,
                                                                 TMTransformHelper::copyPointsToDTM (_dtmTransformHelper.get (), fence.points, fence.numPoints), fence.numPoints, userP, numFeatures);
                break;

            case DTMFeatureType::RidgeLine:
                // ToDo : Drainage
                status = (DTMStatusInt)bcdtmDrainage_returnRidgeLinesDtmObject (GetTinHandle (), callBackFunctP, isFence, fence.fenceType, fence.fenceOption,
                                                                  TMTransformHelper::copyPointsToDTM (_dtmTransformHelper.get (), fence.points, fence.numPoints), fence.numPoints, userP, numFeatures);
                break;

            case DTMFeatureType::Catchment:
                // Not Yet Implemented So Return Error
                break;

            default:
                status = (DTMStatusInt)bcdtmInterruptLoad_dtmFeatureTypeFromDtmObject (GetTinHandle (), featureType, maxSpots, callBackFunctP, isFence, fence.fenceType, fence.fenceOption,
                                                                         TMTransformHelper::copyPointsToDTM (_dtmTransformHelper.get (), fence.points, fence.numPoints), fence.numPoints, userP);
                break;
        }
    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   cbe.11jan2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::BrowseSinglePointFeatures (DTMFeatureType featureType, double* minDepthP, const DTMFenceParams& fence, long* nPointP, void* userP, DTMBrowseSinglePointFeatureCallback callBackFunctP)
    {
    DTMStatusInt status=DTM_SUCCESS;
    bool useFence = false;
    int nPoints;
    SinglePointFeatureCallbackTransformHelper helper (callBackFunctP, userP, _dtmTransformHelper.get ());
    Dtm_Handler_t dtmHandlerArgs;
    dtmHandlerArgs.featuresSinglePointHandler = helper.GetCallBackFunc();
    dtmHandlerArgs.userP = helper.GetUserArg();
    double transformedMinDepth;

    // BeAssert If Tin handle Is nullptr
    BeAssert (GetTinHandle () != nullptr);

    // Check For And Set Fence
    if (fence.points)
        useFence = TRUE;

    if (_dtmTransformHelper.IsValid ())
        {
        if (minDepthP)
            {
            transformedMinDepth = _dtmTransformHelper->convertDistanceToDTM (*minDepthP);
            minDepthP = &transformedMinDepth;
            }
        }
    // Call the core DTM function
    switch (featureType)
        {
            case DTMFeatureType::LowPoint:
                if (*minDepthP <= 0.0)
                    {
                    status = (DTMStatusInt)bcdtmDrainage_returnLowPointsDtmObject (GetTinHandle (), loadSinglePointFeatureCallBack, useFence, fence.fenceType, fence.fenceOption,
                                                                     TMTransformHelper::copyPointsToDTM (_dtmTransformHelper.get (), fence.points, fence.numPoints), fence.numPoints, &dtmHandlerArgs, nPoints);
                    }
                else
                    {
                    status = (DTMStatusInt)bcdtmDrainage_returnNoneFalseLowPointsDtmObject (GetTinHandle (), *minDepthP, loadSinglePointFeatureCallBack, useFence, fence.fenceType, fence.fenceOption,
                                                                              TMTransformHelper::copyPointsToDTM (_dtmTransformHelper.get (), fence.points, fence.numPoints), fence.numPoints, &dtmHandlerArgs, nPoints);
                    }
                break;
            case DTMFeatureType::HighPoint:
                status = (DTMStatusInt)bcdtmDrainage_returnHighPointsDtmObject (GetTinHandle (), loadSinglePointFeatureCallBack, useFence, fence.fenceType, fence.fenceOption,
                                                                  TMTransformHelper::copyPointsToDTM (_dtmTransformHelper.get (), fence.points, fence.numPoints), fence.numPoints, &dtmHandlerArgs, nPoints);
                break;
        };
    *nPointP = nPoints;

    // Return
    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   cbe.11jan2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::BrowseContours (DTMContourParamsCR contourParams, const DTMFenceParams& fence, void* userP, DTMFeatureCallback callBackFunctP)
    {
    BeAssert (GetTinHandle () != nullptr);

    DTMStatusInt status = DTM_SUCCESS;
    double smoothLength = 0.0;

    if (_dtmTransformHelper.IsValid ())
        {
        FeatureCallbackTransformHelper helper (callBackFunctP, userP, _dtmTransformHelper.get ());
        userP = helper.GetUserArg ();
        callBackFunctP = helper.GetCallBackFunc ();
        DTMContourParams transformedParams (contourParams);
        transformedParams.interval = _dtmTransformHelper->convertDistanceToDTM (contourParams.interval);
        transformedParams.conReg = _dtmTransformHelper->convertDistanceToDTM (contourParams.conReg);
        transformedParams.conMin = _dtmTransformHelper->convertElevationToDTM (contourParams.conMin);
        transformedParams.conMax = _dtmTransformHelper->convertElevationToDTM (contourParams.conMax);
        transformedParams.smoothFactor = _dtmTransformHelper->convertDistanceToDTM (contourParams.smoothFactor);
        TMTransformHelper::DoubleCopy contourValues = _dtmTransformHelper->copyElevationTableToDTM (contourParams.conValuesP, contourParams.numConValues);
        transformedParams.conValuesP = contourValues;


        status = (DTMStatusInt)bcdtmLoad_contoursFromDtmObject ((BC_DTM_OBJ *)GetTinHandle (), contourParams, _dtmTransformHelper->convertDTMFenceParamsToDTM (fence), callBackFunctP, userP);
        }
    else
        status = (DTMStatusInt)bcdtmLoad_contoursFromDtmObject ((BC_DTM_OBJ *)GetTinHandle (), contourParams, fence, callBackFunctP, userP);
    return status;
    }
/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.06Jul2010   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::ContourAtPoint (double x, double y, double contourInterval, DTMContourSmoothing smoothOption, double smoothFactor, int smoothDensity,
                            const DTMFenceParams& fence, void* userP, DTMFeatureCallback callBackFunctP)
    {
    BeAssert (GetTinHandle () != nullptr);

    DTMStatusInt status = DTM_SUCCESS;

    if (_dtmTransformHelper.IsValid ())
        {
        FeatureCallbackTransformHelper helper (callBackFunctP, userP, _dtmTransformHelper.get ());
        userP = helper.GetUserArg ();
        callBackFunctP = helper.GetCallBackFunc ();
        status = (DTMStatusInt)bcdtmLoad_contourForPointDtmObject ((BC_DTM_OBJ *)GetTinHandle (), x, y, contourInterval,
                                                     smoothOption, _dtmTransformHelper->convertDistanceToDTM (smoothFactor), smoothDensity, callBackFunctP, fence.points != nullptr, fence.fenceOption, fence.fenceType,
                                                     _dtmTransformHelper->copyPointsToDTM (fence.points, fence.numPoints), fence.numPoints, userP);
        }
    else
        {
        status = (DTMStatusInt)bcdtmLoad_contourForPointDtmObject ((BC_DTM_OBJ *)GetTinHandle (), x, y, contourInterval,
                                                     smoothOption, smoothFactor, smoothDensity, callBackFunctP,
                                                     fence.points != nullptr, fence.fenceOption, fence.fenceType, (DPoint3d*)fence.points, fence.numPoints, userP);
        }
    return status;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Daryl.Holmwood  02/11
+---------------+---------------+---------------+---------------+---------------+------*/
int GetContourPointArray (DTMFeatureType featureType, DTMUserTag featureTag, DTMFeatureId featureId, DPoint3d *tPoint, size_t nPoint, void* userP)
    {
    Bentley::TerrainModel::DTMPointArray* ret = static_cast<Bentley::TerrainModel::DTMPointArray*>(userP);
    // ToDo DHMEM check refcount.
    ret->resize (nPoint);
    memcpy (ret->data (), tPoint, nPoint * sizeof (DPoint3d));
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Daryl.Holmwood  12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt BcDTM::_ContourAtPoint (Bentley::TerrainModel::DTMPointArray& ret, DPoint3dCR pt, double contourInterval, DTMContourSmoothing smoothOption, double smoothFactor, int smoothDensity, DTMFenceParamsCR fence)
    {
    return ContourAtPoint (pt.x, pt.y, contourInterval, smoothOption, smoothFactor, smoothDensity, fence, &ret, &GetContourPointArray);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Daryl.Holmwood  12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt BcDTM::_ContourAtPoint (Bentley::TerrainModel::DTMPointArray& ret, DPoint3dCR pt, double contourInterval, DTMContourSmoothing smoothOption, double smoothFactor, int smoothDensity)
    {
    return ContourAtPoint (pt.x, pt.y, contourInterval, smoothOption, smoothFactor, smoothDensity, Bentley::TerrainModel::DTMFenceParams (), &ret, &GetContourPointArray);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   cbe.11jan2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::GetAscentTrace (double minDepth, double pX, double pY, void* userP, DTMFeatureCallback callBackFunctP)
    {
    DTMStatusInt status=DTM_SUCCESS ;
    BeAssert (GetTinHandle() != nullptr);
    SetMemoryAccess(DTMAccessMode::Temporary);
    if (_dtmTransformHelper.IsValid ())
        {
        FeatureCallbackTransformHelper helper (callBackFunctP, userP, _dtmTransformHelper.get ());
        userP = helper.GetUserArg ();
        callBackFunctP = helper.GetCallBackFunc ();
        _dtmTransformHelper->convertPointToDTM (pX, pY);
        status = (DTMStatusInt)bcdtmDrainage_traceMaximumAscentDtmObject (GetTinHandle (), nullptr, callBackFunctP, 0.0, pX, pY, userP);
        }
    else
        status = (DTMStatusInt)bcdtmDrainage_traceMaximumAscentDtmObject (GetTinHandle (), nullptr, callBackFunctP, 0.0, pX, pY, userP);

    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   cbe.11jan2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::GetDescentTrace (double minDepth, double pX, double pY, void* userP, DTMFeatureCallback callBackFunctP)
    {
    DTMStatusInt status=DTM_SUCCESS ;
    BeAssert (GetTinHandle() != nullptr);
    SetMemoryAccess(DTMAccessMode::Temporary);
    if (_dtmTransformHelper.IsValid ())
        {
        FeatureCallbackTransformHelper helper (callBackFunctP, userP, _dtmTransformHelper.get ());
        userP = helper.GetUserArg ();
        callBackFunctP = helper.GetCallBackFunc ();
        _dtmTransformHelper->convertPointToDTM (pX, pY);
        status = (DTMStatusInt)bcdtmDrainage_traceMaximumDescentDtmObject (GetTinHandle (), nullptr, callBackFunctP, _dtmTransformHelper->convertDistanceToDTM (minDepth), pX, pY, userP);
        }
    else
        status = (DTMStatusInt)bcdtmDrainage_traceMaximumDescentDtmObject (GetTinHandle (), nullptr, callBackFunctP, minDepth, pX, pY, userP);
    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   cbe.11jan2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::AnalyzeElevation (DRange1d* tInterval, int nInterval, bool polygonized, const DTMFenceParams& fence, void* userP, DTMFeatureCallback callBackFunctP)
    {
    DTMStatusInt status = DTM_SUCCESS;

    BeAssert (GetTinHandle () != nullptr);

    FeatureCallbackTransformHelper helper (callBackFunctP, userP, _dtmTransformHelper.get ());
    userP = helper.GetUserArg ();
    callBackFunctP = helper.GetCallBackFunc ();


    // Call the core DTM theme function
    if (_dtmTransformHelper.IsNull ())
        status = (DTMStatusInt)bcdtmTheme_loadThemeFromDtmObject ((BC_DTM_OBJ *)GetTinHandle (), polygonized, 1, tInterval, nInterval,
          callBackFunctP, fence.points != nullptr ? TRUE : FALSE, (DPoint3dP)fence.points, fence.numPoints, userP);
    else
        status = (DTMStatusInt)bcdtmTheme_loadThemeFromDtmObject ((BC_DTM_OBJ *)GetTinHandle (), polygonized, 1, _dtmTransformHelper->copyBcDTMElevationRangeToDTM (tInterval, nInterval), nInterval,
        callBackFunctP, fence.points != nullptr ? TRUE : FALSE, _dtmTransformHelper->copyPointsToDTM (fence.points, fence.numPoints), fence.numPoints, userP);
    // Return
    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   cbe.11jan2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::AnalyzeSlope (DRange1d* tInterval, int nInterval, bool polygonized, const DTMFenceParams& fence, void* userP, DTMFeatureCallback callBackFunctP)
    {
   DTMStatusInt status=DTM_SUCCESS;
   DRange1d   *themeTable = tInterval;

    BeAssert (GetTinHandle () != nullptr);

    if (_dtmTransformHelper.IsValid ())
        {
        themeTable = (DRange1d *)bcMem_malloc (nInterval * sizeof (DRange1d));
        if (themeTable == nullptr)
            return DTM_ERROR;
        for (int iRange = 0; iRange < nInterval; iRange++)
            {
            themeTable[iRange].low = _dtmTransformHelper->convertSlopeToDTM (tInterval[iRange].low);
            themeTable[iRange].high = _dtmTransformHelper->convertSlopeToDTM (tInterval[iRange].high);
            }

        }

    FeatureCallbackTransformHelper helper (callBackFunctP, userP, _dtmTransformHelper.get ());
    userP = helper.GetUserArg ();
    callBackFunctP = helper.GetCallBackFunc ();

    // Call the core DTM theme function
    status = (DTMStatusInt)bcdtmTheme_loadThemeFromDtmObject ((BC_DTM_OBJ *)GetTinHandle (), polygonized, 2, themeTable, nInterval,
                                                callBackFunctP, fence.points != nullptr ? TRUE : FALSE, TMTransformHelper::copyPointsToDTM (_dtmTransformHelper.get (), fence.points, fence.numPoints), fence.numPoints, userP);
    if (_dtmTransformHelper.IsValid () && themeTable != nullptr)
        bcMem_free ((void*)themeTable);

    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   cbe.11jan2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::AnalyzeAspect (DRange1d* tInterval, int nInterval, bool polygonized, const DTMFenceParams& fence, void* userP, DTMFeatureCallback callBackFunctP)
    {
    DTMStatusInt  status=DTM_SUCCESS ;
    DRange1d   *themeTable = tInterval;

    BeAssert (GetTinHandle() != nullptr);

    themeTable = (DRange1d *)bcMem_malloc (nInterval * sizeof (DRange1d));
    if (themeTable == nullptr)
        return DTM_ERROR;

    // Convert from conevntionnal radian to azimuthal degree
    for (int iRange = 0; iRange < nInterval; iRange++)
        {
        double alpha = tInterval[iRange].low;
        if (_dtmTransformHelper.IsValid ())
            alpha = _dtmTransformHelper->convertAspectToDTM (alpha);
        alpha = bcConversion_directionToAzimuth (alpha);
        // Convert to degrees
        alpha = bcConversion_radianToDegree(alpha);
        themeTable[iRange].low = alpha;

        alpha = tInterval[iRange].high;
        if (_dtmTransformHelper.IsValid ())
            alpha = _dtmTransformHelper->convertAspectToDTM (alpha);
        alpha = bcConversion_directionToAzimuth (alpha);
        // Convert to degrees
        alpha = bcConversion_radianToDegree(alpha);
        themeTable[iRange].high = alpha;
        }

    FeatureCallbackTransformHelper helper (callBackFunctP, userP, _dtmTransformHelper.get ());
    userP = helper.GetUserArg ();
    callBackFunctP = helper.GetCallBackFunc ();

    // Call the core DTM theme function
    status = (DTMStatusInt)bcdtmTheme_loadThemeFromDtmObject ((BC_DTM_OBJ *)GetTinHandle (), polygonized, 4, themeTable, nInterval,
                                                callBackFunctP, fence.points != nullptr ? TRUE : FALSE, TMTransformHelper::copyPointsToDTM (_dtmTransformHelper.get (), fence.points, fence.numPoints), fence.numPoints, userP);

    if( themeTable != nullptr ) bcMem_free ((void*)themeTable);

    // Return
    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   cbe.08feb2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::TracePath (double pointX, double pointY, double slope, double dist, void* userP, DTMFeatureCallback callBackFunctP)
    {
   DTMStatusInt status=DTM_SUCCESS;
    double        pointZ = 0;
    long          tp1 = 0, tp2 = 0, tp3 = 0, numbOfStartgAng = 0;
    double        startingAngle[4], triangleSlope = 0;

    // Check For nullptr DTM Pointer
    BeAssert (GetTinHandle () != nullptr);

    FeatureCallbackTransformHelper helper (callBackFunctP, userP, _dtmTransformHelper.get ());
    userP = helper.GetUserArg ();
    callBackFunctP = helper.GetCallBackFunc ();

    if (_dtmTransformHelper.IsValid ())
        {
        _dtmTransformHelper->convertPointToDTM (pointX, pointY);
        slope = _dtmTransformHelper->convertSlopeToDTM (slope);
        dist = _dtmTransformHelper->convertDistanceToDTM (dist);
        }
    status = (DTMStatusInt)bcdtmGrade_getGradeSlopeStartDirectionsDtmObject (GetTinHandle (), pointX, pointY, slope, &pointZ, &tp1, &tp2, &tp3, &triangleSlope, startingAngle, &numbOfStartgAng);

    if (status == DTM_SUCCESS)
        {
        for (int iSol = 0; iSol < numbOfStartgAng; iSol++)
            {
            status = (DTMStatusInt)bcdtmGrade_traceGradeDtmObject (GetTinHandle (), pointX, pointY, pointZ, callBackFunctP, tp1, tp2, tp3, slope, startingAngle[iSol], dist, userP);
            if (status != DTM_SUCCESS)
                break;
            }
        }

    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   cbe.08feb2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::TraceLine (double pointX, double pointY, double slope, double dist, void* userP, DTMFeatureCallback callBackFunctP)
    {
    long        numbOfStartgAng = 0;
    double      *anglesP = nullptr;
    BeAssert (GetTinHandle() != nullptr);

    FeatureCallbackTransformHelper helper (callBackFunctP, userP, _dtmTransformHelper.get ());
    userP = helper.GetUserArg ();
    callBackFunctP = helper.GetCallBackFunc ();

    DTMStatusInt status = (DTMStatusInt)bcdtmGrade_getMacaoGradeSlopeStartDirectionsDtmObject(GetTinHandle(), pointX, pointY,
        slope, dist, &anglesP, &numbOfStartgAng);

    for (int iSol = 0; iSol < numbOfStartgAng; iSol++)
        {
        }


    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.26aug2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
void BcDTM::GetHandles (void** headerPP, int* nHeaderP, void*** featureArraysPP, int*  nFeatureArraysP, int*  featureArraysSize, int*  lastFeatureArraysSize, void*** pointArraysPP, int*  nPointArraysP, int*  pointArraysSize, int*  lastPointArraysSize, void*** nodeArraysPP, int*  nNodeArraysP, int*  nodeArraysSize, int*  lastNodeArraysSize, void*** cListArraysPP, int*  nCListArraysP, int*  cListArraysSize, int*  lastCListArraysSize, void*** fListArraysPP, int*  nFListArraysP, int*  fListArraysSize, int*  lastFListArraysSize)
    {
    BC_DTM_OBJ* _dtmHandleP = GetTinHandle ();
    *headerPP = GetTinHandle ();
    *nHeaderP = BCDTMSize;

    *featureArraysPP = (void**)_dtmHandleP->fTablePP;
    *nFeatureArraysP = _dtmHandleP->numFeaturePartitions;
    if (_dtmHandleP->numFeaturePartitions)
        {
        *featureArraysSize = _dtmHandleP->featurePartitionSize * sizeof(BC_DTM_FEATURE);
        *lastFeatureArraysSize = (_dtmHandleP->memFeatures * sizeof(BC_DTM_FEATURE))
            - ((*nFeatureArraysP - 1) * (*featureArraysSize));
        }
    else
        {
        *featureArraysSize = 0;
        *lastFeatureArraysSize = 0;
        }

    *pointArraysPP = (void**)_dtmHandleP->pointsPP;
    *nPointArraysP = _dtmHandleP->numPointPartitions;
    if (_dtmHandleP->numPointPartitions)
        {
        *pointArraysSize = _dtmHandleP->pointPartitionSize * sizeof (DPoint3d);
        *lastPointArraysSize = (_dtmHandleP->memPoints * sizeof(DPoint3d))
            - ((*nPointArraysP - 1) * (*pointArraysSize));
        }
    else
        {
        *pointArraysSize = 0;
        *lastPointArraysSize = 0;
        }

    *nodeArraysPP = (void**)_dtmHandleP->nodesPP;
    *nNodeArraysP = _dtmHandleP->numNodePartitions;
    if (_dtmHandleP->numNodePartitions)
        {
        *nodeArraysSize = _dtmHandleP->nodePartitionSize * sizeof (DTM_TIN_NODE);
        *lastNodeArraysSize = (_dtmHandleP->memNodes * sizeof(DTM_TIN_NODE))
            - ((*nNodeArraysP - 1) * (*nodeArraysSize));
        }
    else
        {
        *nodeArraysSize = 0;
        *lastNodeArraysSize = 0;
        }

    *cListArraysPP = (void**)_dtmHandleP->cListPP;
    *nCListArraysP = _dtmHandleP->numClistPartitions;
    if (_dtmHandleP->numClistPartitions)
        {
        *cListArraysSize = _dtmHandleP->clistPartitionSize * sizeof (DTM_CIR_LIST);
        *lastCListArraysSize = (_dtmHandleP->memClist * sizeof(DTM_CIR_LIST))
            - ((*nCListArraysP - 1) * (*cListArraysSize));
        }
    else
        {
        *cListArraysSize = 0;
        *lastCListArraysSize = 0;
        }

    *fListArraysPP = (void**)_dtmHandleP->fListPP;
    *nFListArraysP = _dtmHandleP->numFlistPartitions;
    if (_dtmHandleP->numFlistPartitions)
        {
        *fListArraysSize = _dtmHandleP->flistPartitionSize * sizeof (DTM_FEATURE_LIST);
        *lastFListArraysSize = (_dtmHandleP->memFlist * sizeof(DTM_FEATURE_LIST))
            - ((*nFListArraysP - 1) * (*fListArraysSize));
        }
    else
        {
        *fListArraysSize = 0;
        *lastFListArraysSize = 0;
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.04sep2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
BcDTMMeshPtr BcDTM::GetMesh (bool firstCall, long maxMeshSize, DPoint3dCP fencePts, int numFencePts)
    {
    bvector<DPoint3d> meshPts;
    bvector<long> meshFaces;

    DTMStatusInt status = (DTMStatusInt)bcdtmLoad_tinMeshFromDtmObject (GetTinHandle (), firstCall, maxMeshSize, fencePts != nullptr, DTMFenceType::Block, DTMFenceOption::Overlap, fencePts, numFencePts, meshPts, meshFaces);

    if (status == DTM_ERROR || meshFaces.empty())
        return nullptr;

    if (_dtmTransformHelper.IsValid ())
        _dtmTransformHelper->convertPointsFromDTM (meshPts.data(), (int)meshPts.size());
    return BcDTMMesh::Create (meshPts, meshFaces);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.24aou2006   -  Created.                                         |
|   rsc.2008Apr03   -  Updated                                          |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::Transform (TransformCR trsfMatP)
    {
    // This method gets new header and point array after a DTM transformation.
    BC_DTM_OBJ* dtmHandleP = GetTinHandle();
    double dtmTransform[3][4] ;

    // ToDo Translatation???
    if (SetMemoryAccess(DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message(1,0,0, "DTM is readonly");
        return DTM_ERROR;
        }

    memcpy (dtmTransform, &trsfMatP, 12 * sizeof(double)) ;

    // Call Core DTM Function

    return((DTMStatusInt)bcdtmMath_transformDtmObject (dtmHandleP, dtmTransform));

    };

/*----------------------------------------------------------------------+
|                                                                       |
|   djh.2010Feb22   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::TransformUsingCallback (DTMTransformPointsCallback callback, void* userP)
    {
    // This method gets new header and point array after a DTM transformation.
    BC_DTM_OBJ *dtmHandleP = GetTinHandle();

    if (SetMemoryAccess(DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message(1,0,0, "DTM is readonly");
        return DTM_ERROR;
        }

    TransformPointsCallbackTransformHelper helper (callback, userP, _dtmTransformHelper.get ());
    callback = helper.GetCallBackFunc ();
    userP = helper.GetUserArg ();

    // Call Core DTM Function
    return (DTMStatusInt)bcdtmMath_transformViaCallbackDtmObject (dtmHandleP, callback, userP);
    };

/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.2009Nov25   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::ConvertUnits (double xyFactor, double zFactor)
    {
    // Call Core DTM Function
    if (SetMemoryAccess(DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message(1,0,0, "DTM is readonly");
        return DTM_ERROR;
        }
// ToDo Translatation probably only if there is rotation.
    return((DTMStatusInt)bcdtmMath_convertUnitsDtmObject (GetTinHandle (), xyFactor, zFactor));
    };

/*----------------------------------------------------------------------+
|                                                                       |
|   cbe.07apr2006   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::Triangulate()
    {
    if (SetMemoryAccess(DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message(1,0,0, "DTM is readonly");
        return DTM_ERROR;
        }
    // Set the autoCleanOptions
    SetCleanUpOptions (DTMCleanupFlags::All);

    DTMStatusInt status = (DTMStatusInt)bcdtmObject_triangulateDtmObject (GetTinHandle ());
    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   djh.18feb2008   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::CheckTriangulation()
    {
    return((DTMStatusInt)bcdtmCheck_tinComponentDtmObject (GetTinHandle ()));
    }
/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.05mar2010   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::BrowseFeaturesWithTinErrors (void* userP, DTMFeatureCallback callback)
    {
    FeatureCallbackTransformHelper helper (callback, userP, _dtmTransformHelper.get ());
    userP = helper.GetUserArg ();
    callback = helper.GetCallBackFunc ();

    return (DTMStatusInt)bcdtmInterruptLoad_dtmFeaturesWithTinErrorsDtmObject (GetTinHandle (), (DTMFeatureCallback)callback, userP);
    }
/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.05mar2010   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::BrowseFeaturesWithUserTag(DTMUserTag userTag, void* userP, DTMFeatureCallback callback)
    {
    long maxSpots=50000 ;
    long useFence=FALSE ;
    DTMFenceType fenceType=DTMFenceType::Block ;
    DTMFenceOption fenceOption=DTMFenceOption::Overlap ;
    DPoint3d  *fencePts = nullptr ;
    long numFencePts=0 ;

    FeatureCallbackTransformHelper helper (callback, userP, _dtmTransformHelper.get ());
    userP = helper.GetUserArg ();
    callback = helper.GetCallBackFunc ();
    return (DTMStatusInt)bcdtmInterruptLoad_dtmFeaturesForUsertagDtmObject (GetTinHandle (), (DTMUserTag)userTag, maxSpots, (DTMFeatureCallback)callback, useFence, fenceType, fenceOption, fencePts, numFencePts, userP);
    }
/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.05mar2010   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::BrowseFeaturesWithFeatureId(DTMFeatureId featureId, void* userP, DTMFeatureCallback callback)
    {
    long maxSpots=50000 ;
    long useFence=FALSE ;
    DTMFenceType fenceType=DTMFenceType::Block ;
    DTMFenceOption fenceOption=DTMFenceOption::Overlap ;
    DPoint3d  *fencePts =nullptr ;
    long numFencePts =0 ;
    FeatureCallbackTransformHelper helper (callback, userP, _dtmTransformHelper.get ());
    userP = helper.GetUserArg ();
    callback = helper.GetCallBackFunc ();
    return (DTMStatusInt)bcdtmInterruptLoad_dtmFeaturesForFeatureIdDtmObject (GetTinHandle (), (DTMFeatureId)featureId, maxSpots, (DTMFeatureCallback)callback, useFence, fenceType, fenceOption, fencePts, numFencePts, userP);
    }
/*----------------------------------------------------------------------+
|                                                                       |
|   djh.13mar2008   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::BrowseDuplicatePoints(void* userP, DTMDuplicatePointsCallback callback)
    {
    DuplicatePointsCallbackTransformHelper helper (callback, userP, _dtmTransformHelper.get ());
    callback = helper.GetCallBackFunc ();
    userP = helper.GetUserArg ();

    SetMemoryAccess(DTMAccessMode::Temporary);
    return (DTMStatusInt)bcdtmReport_duplicatePointErrorsDtmObject (GetTinHandle (), callback, userP);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   djh.13mar2008   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::BrowseCrossingFeatures (const DTMFeatureType featureList[], int numFeatureTypes, void* userP, DTMCrossingFeaturesCallback callback)
    {
    CrossingFeaturesCallbackTransformHelper helper (callback, userP, _dtmTransformHelper.get ());
    callback = helper.GetCallBackFunc ();
    userP = helper.GetUserArg ();

    SetMemoryAccess(DTMAccessMode::Temporary);
    return (DTMStatusInt)bcdtmReport_crossingFeaturesDtmObject(GetTinHandle(), featureList, numFeatureTypes, callback, userP);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   djh.18feb2008   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::SetTriangulationParameters (double pointTol, double lineTol, long edgeOption, double maxSide)
    {
    if (SetMemoryAccess(DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message(1,0,0, "DTM is readonly");
        return DTM_ERROR;
        }

    if (_dtmTransformHelper.IsValid ())
        {
        pointTol = _dtmTransformHelper->convertDistanceToDTM (pointTol);
        lineTol = _dtmTransformHelper->convertDistanceToDTM (lineTol);
        maxSide = _dtmTransformHelper->convertDistanceToDTM (maxSide);
        }
    return (DTMStatusInt)bcdtmObject_setTriangulationParametersDtmObject (GetTinHandle (), pointTol, lineTol, edgeOption, maxSide);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   djh.18feb2008   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::GetTriangulationParameters (double& pointTol, double& lineTol, long& edgeOption, double& maxSide)
    {
    DTMStatusInt status = (DTMStatusInt)bcdtmObject_getTriangulationParametersDtmObject (GetTinHandle (), &pointTol, &lineTol, &edgeOption, &maxSide);

    if (status == DTM_SUCCESS && _dtmTransformHelper.IsValid ())
        {
        pointTol = _dtmTransformHelper->convertDistanceFromDTM (pointTol);
        lineTol = _dtmTransformHelper->convertDistanceFromDTM (lineTol);
        maxSide = _dtmTransformHelper->convertDistanceFromDTM (maxSide);
        }
    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   djh.25feb2008   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMState BcDTM::GetDTMState()
    {
    DTMState dtmState;
// ToDo Vancouver may need two functions,   bcdtmUtility_getDtmStateDtmObject (GetTinHandle(), &dtmState);

    dtmState = GetTinHandle()->dtmState;
    return dtmState;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   djh.18mar2008   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::TinFilterPoints (long filterOption, int reinsertOption, double zTolerance, long* numPointsBefore, long* numPointsAfter, double* filterReduction)
    {
    if (SetMemoryAccess(DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message(1,0,0, "DTM is readonly");
        return DTM_ERROR;
        }
    if (_dtmTransformHelper.IsValid ())
        zTolerance = _dtmTransformHelper->convertDistanceToDTM (zTolerance);
    return (DTMStatusInt)bcdtmFilter_tinFilterRandomSpotsDtmObject (GetTinHandle (), filterOption, reinsertOption, zTolerance, numPointsBefore, numPointsAfter, filterReduction);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   djh.19mar2008   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::TileFilterPoints (long minTilePts, long maxTileDivide, double tileLength, double zTolerance, long* numPointsBefore, long* numPointsAfter,
                                   double* filterReduction)
    {
    if (SetMemoryAccess(DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message(1,0,0, "DTM is readonly");
        return DTM_ERROR;
        }

    if (_dtmTransformHelper.IsValid ())
        zTolerance = _dtmTransformHelper->convertDistanceToDTM (zTolerance);
    return (DTMStatusInt)bcdtmFilter_tileFilterRandomSpotsDtmObject (GetTinHandle (), minTilePts, maxTileDivide, tileLength, zTolerance, numPointsBefore, numPointsAfter, filterReduction);
    }
/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.28mar2008   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::TinFilterSinglePointPointFeatures (long filterOption, int reinsertOption, double zTolerance, long* numPointsBefore, long* numPointsAfter,
                                            double* filterReduction)
    {
    if (SetMemoryAccess(DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message(1,0,0, "DTM is readonly");
        return DTM_ERROR;
        }
    if (_dtmTransformHelper.IsValid ())
        zTolerance = _dtmTransformHelper->convertDistanceToDTM (zTolerance);
    return (DTMStatusInt)bcdtmFilter_tinFilterSinglePointGroupSpotsDtmObject (GetTinHandle (), filterOption, reinsertOption, zTolerance, numPointsBefore, numPointsAfter, filterReduction);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.28mar2008   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::TileFilterSinglePointPointFeatures (long minTilePts, long maxTileDivide, double tileLength, double zTolerance, long* numPointsBefore,
                                             long* numPointsAfter, double* filterReduction)
    {
    if (SetMemoryAccess(DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message(1,0,0, "DTM is readonly");
        return DTM_ERROR;
        }
    if (_dtmTransformHelper.IsValid ())
        zTolerance = _dtmTransformHelper->convertDistanceToDTM (zTolerance);
    return (DTMStatusInt)bcdtmFilter_tileFilterSinglePointGroupSpotsDtmObject (GetTinHandle (), minTilePts, maxTileDivide, tileLength, zTolerance, numPointsBefore, numPointsAfter, filterReduction);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   djh.12apr2008   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::EditorSelectDtmTinFeature(DTMFeatureType dtmFeatureType, DPoint3d pt, bool& featureFoundP, bvector<DPoint3d>& featurePts)
    {
    if (_dtmTransformHelper.IsValid())
        _dtmTransformHelper->convertPointToDTM (pt);
    DTMStatusInt status = (DTMStatusInt)bcdtmEdit_selectDtmEditFeatureDtmObject (GetTinHandle (), dtmFeatureType, pt.x, pt.y, featureFoundP, featurePts);
    if (status == DTM_SUCCESS && _dtmTransformHelper.IsValid() && featurePts.empty())
        _dtmTransformHelper->convertPointsFromDTM (featurePts);
    return status;
    }
/*----------------------------------------------------------------------+
|                                                                       |
|   djh.12apr2008   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::EditorDeleteDtmTinFeature (long* result)
    {
    if (SetMemoryAccess(DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message(1,0,0, "DTM is readonly");
        return DTM_ERROR;
        }
    return (DTMStatusInt)bcdtmEdit_modifyDtmEditFeatureDtmObject (GetTinHandle (), 1, result);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   djh.09may2008   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::GetMemoryUsed (size_t& memoryUsed)
    {
    unsigned long a1,a2,a4,a5,a6,a7,total = 0;
    try
        {
        bcdtmObject_reportMemoryUsageDtmObject(GetTinHandle(), &a1, &a2, nullptr, &a4, &a5 ,&a6, &a7, &total);
        }
    catch(...)
        {
        }
    memoryUsed = total;
    return DTM_SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   djh.8jun2008    -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::PurgeDTM (unsigned int flags)
    {
    DTMStatusInt ret = DTM_SUCCESS;

    if (SetMemoryAccess(DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message(1,0,0, "DTM is readonly");
        return DTM_ERROR;
        }
    if ((flags & 4) == 4)
        bcdtmData_deleteAllRollBackFeaturesDtmObject (GetTinHandle ());
    if (ret != SUCCESS) return ret;

    if ((flags & 2) == 2)
        bcdtmData_deleteAllTinErrorFeaturesDtmObject(GetTinHandle());

    if(ret != SUCCESS) return ret;

    if((flags & 1) == 1)
        ret = (DTMStatusInt)bcdtmData_compactFeatureTableDtmObject (GetTinHandle ());

    return ret;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   djh.8jun2008    -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::GetPoint (long index, DPoint3d& pt)
    {
    DTMStatusInt status = (DTMStatusInt)bcdtmObject_getPointByIndexDtmObject (GetTinHandle (), index, (DPoint3d*)&pt);
    if (status && _dtmTransformHelper.IsValid())
        _dtmTransformHelper->convertPointFromDTM (pt);
    return status;
    }
/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.18may2009    -  Created.                                        |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::CopyToMemoryBlock (char** memoryBlockPP,unsigned long *memoryBlockSizeP)
    {
    // ToDo Translatation
    return (DTMStatusInt)bcdtmObject_copyToMemoryBlockDtmObject (GetTinHandle (), memoryBlockPP, memoryBlockSizeP);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.05Aug2009    -  Created.                                        |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::Clip (DPoint3dCP clipPolygonPtsP, int numClipPolygonPts, DTMClipOption clippingMethod)
    {
    if (SetMemoryAccess(DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message(1,0,0, "DTM is readonly");
        return DTM_ERROR;
        }

    // Check arguments
    if (clipPolygonPtsP == nullptr )   return DTM_ERROR;
    if (numClipPolygonPts < 3   )   return DTM_ERROR;

    // Clip the DTM
    return (DTMStatusInt)bcdtmClip_toPolygonDtmObject (GetTinHandle (), TMTransformHelper::copyPointsToDTM (_dtmTransformHelper.get (), clipPolygonPtsP, numClipPolygonPts), numClipPolygonPts, clippingMethod);
    }
/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.05Aug2009    -  Created.                                        |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::ConvertToDataState(void)
    {
    // Convert To Data Sate
    if (SetMemoryAccess(DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message(1,0,0, "DTM is readonly");
        return DTM_ERROR;
        }
    DTMStatusInt status = (DTMStatusInt)bcdtmObject_changeStateDtmObject (GetTinHandle (), DTMState::Data);
    if( status ) return DTM_ERROR ;
    else         return DTM_SUCCESS;
    }
/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.05Aug2009    -  Created.                                        |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::SetPointMemoryAllocationParameters (int iniPointAllocation, int incPointAllocation)
    {
    if (SetMemoryAccess(DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message(1,0,0, "DTM is readonly");
        return DTM_ERROR;
        }
    if( iniPointAllocation < 0 ) iniPointAllocation = DTM_INI_MEM_PTS ;
    if( incPointAllocation < 0 ) incPointAllocation = DTM_INC_MEM_PTS ;
    // Convert To Data Sate
    DTMStatusInt status = (DTMStatusInt)bcdtmObject_setPointMemoryAllocationParametersDtmObject (GetTinHandle (), iniPointAllocation, incPointAllocation);
    if( status ) return DTM_ERROR ;
    else         return DTM_SUCCESS;
    }
/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.19Jan2010    -  Created.                                        |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::InterpolateDtmFeatureType (DTMFeatureType dtmFeatureType, double snapTolerance, BcDTMR pointsDtmP, BcDTMR intDtmP, long* numDtmFeaturesP, long* numDtmFeaturesInterpolatedP )
    {
    // Interpolate Dtm Feature Type
    BcDTMPtr pointsDtm = TMTransformHelper::convertDTMToDTM (*this, pointsDtmP);
    BcDTMPtr intDtm = TMTransformHelper::convertDTMToDTM (*this, intDtmP);
    return (DTMStatusInt)bcdtmInterpolate_dtmFeatureTypeDtmObject (GetTinHandle (), dtmFeatureType, snapTolerance, pointsDtm->GetTinHandle (), intDtm->GetTinHandle (), numDtmFeaturesP, numDtmFeaturesInterpolatedP);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.24Jan2011    -  Created.                                        |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::BrowsePonds (void* userP, DTMFeatureCallback callBack)
    {
    // ToDo : Drainage
    bool loadFlag=true ;
    bool buildTable=false ;

    //  Call Core DTM Function

    SetMemoryAccess(DTMAccessMode::Temporary);
    FeatureCallbackTransformHelper helper (callBack, userP, _dtmTransformHelper.get ());
    userP = helper.GetUserArg ();
    callBack = helper.GetCallBackFunc ();

    return (DTMStatusInt)bcdtmDrainage_determinePondsDtmObject(GetTinHandle(),nullptr, (DTMFeatureCallback)callBack, loadFlag ,buildTable, userP);
    }
/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.12Mar2010    -  Created.                                        |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::CalculatePond (double x , double y , bool& pondCalculatedP, double& pondElevationP, double& pondDepthP, double& pondAreaP,
                           double& pondVolumeP, DTMDynamicFeatureArray& ponds)
    {
    return CalculatePond (x, y, 0, pondCalculatedP, pondElevationP, pondDepthP, pondAreaP, pondVolumeP, ponds);
    }
/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.12Mar2010    -  Created.                                        |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::CalculatePond (double x, double y, double falseLowDepth, bool& pondCalculatedP, double& pondElevationP, double& pondDepthP,
                           double& pondAreaP, double& pondVolumeP, DTMDynamicFeatureArray& ponds)
    {
    //  Call Core DTM Function
    SetMemoryAccess(DTMAccessMode::Temporary);
    if (_dtmTransformHelper.IsValid ())
        {
        _dtmTransformHelper->convertPointToDTM (x, y);
        falseLowDepth = _dtmTransformHelper->convertDistanceToDTM (falseLowDepth);
        }
    if (BcDTMDrainage::CalculatePondForPoint (this, x, y, falseLowDepth, pondCalculatedP, pondElevationP, pondDepthP, pondAreaP, pondVolumeP, ponds) != DTM_SUCCESS)
        return DTM_ERROR;

    if (pondCalculatedP && _dtmTransformHelper.IsValid ())
        {
        _dtmTransformHelper->convertDTMDynamicFeaturesFromDTM (ponds);
        pondElevationP = _dtmTransformHelper->convertDistanceFromDTM (pondElevationP);
        pondDepthP = _dtmTransformHelper->convertDistanceFromDTM (pondDepthP);
        pondAreaP = _dtmTransformHelper->convertAreaFromDTM (pondAreaP);
        pondVolumeP = _dtmTransformHelper->convertVolumeFromDTM (pondVolumeP);
        }
    return DTM_SUCCESS ;
    }
/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.16Mar2010    -  Created.                                        |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::TraceCatchmentForPoint (double x, double y, double maxPondDepth, bool& catchmentDetermined, DPoint3d& sumpPoint, bvector<DPoint3d>& catchmentPts)
    {
    SetMemoryAccess (DTMAccessMode::Temporary);
    if (_dtmTransformHelper.IsValid ())
        {
        _dtmTransformHelper->convertPointToDTM (x, y);
        maxPondDepth = _dtmTransformHelper->convertDistanceToDTM (maxPondDepth);
        }

    if (BcDTMDrainage::TraceCatchmentForPoint (this, DPoint3d::FromXYZ (x, y, 0), maxPondDepth, catchmentDetermined, sumpPoint, catchmentPts) != DTM_SUCCESS)
        return DTM_ERROR;

    if (catchmentDetermined)
        {
        if (_dtmTransformHelper.IsValid ())
            {
            _dtmTransformHelper->convertPointToDTM (sumpPoint);
            _dtmTransformHelper->convertPointsFromDTM (catchmentPts);
            }
        }

    return DTM_SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.12Mar2010    -  Created.                                        |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::RemoveNoneFeatureHullLines()
    {
    if (SetMemoryAccess(DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message(1,0,0, "DTM is readonly");
        return DTM_ERROR;
        }
    return (DTMStatusInt)bcdtmList_removeNoneFeatureHullLinesDtmObject (GetTinHandle ());
    }
/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.17Mar2010    -  Created                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::PointVisibility (bool& pointVisibleP, double eyeX, double eyeY, double eyeZ, double pntX, double pntY, double pntZ)
    {
    DTMStatusInt status = DTM_SUCCESS;
    long isVisible = 0;

    if (_dtmTransformHelper.IsValid ())
        {
        _dtmTransformHelper->convertPointToDTM (eyeX, eyeY, eyeZ);
        _dtmTransformHelper->convertPointToDTM (pntX, pntY, pntZ);
        }
    //   Call Core DTM Function
    status = (DTMStatusInt)bcdtmVisibility_determinePointVisibilityDtmObject (GetTinHandle (), eyeX, eyeY, eyeZ, pntX, pntY, pntZ, &isVisible);

    pointVisibleP = isVisible != 0;
    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.17Mar2010    -  Created                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::LineVisibility (bool& lineVisibleP, double eyeX, double eyeY, double eyeZ, double X1, double Y1, double Z1,
                            double X2, double Y2, double Z2, DTMDynamicFeatureArray& visibilityFeatures)
    {
    DTMStatusInt status = DTM_SUCCESS;
    DTMFeatureBuffer buffer;

    if (_dtmTransformHelper.IsValid ())
        {
        _dtmTransformHelper->convertPointToDTM (eyeX, eyeY, eyeZ);
        _dtmTransformHelper->convertPointToDTM (X1, Y1, Z1);
        _dtmTransformHelper->convertPointToDTM (X2, Y2, Z2);
        }

    //   Call Core DTM Function
    long lineVisible = false;
    status = (DTMStatusInt)bcdtmVisibility_determineLineVisibiltyDtmObject (GetTinHandle (), eyeX, eyeY, eyeZ, X1, Y1, Z1, X2, Y2, Z2, &lineVisible, AddDtmFeatureToBuffer, &buffer);
    lineVisibleP = lineVisible != false;

    if (status == DTM_SUCCESS)
        {
        //   Get Visibility Features From Buffer
        status = (DTMStatusInt)buffer.GetDtmDynamicFeaturesArray (visibilityFeatures, _dtmTransformHelper.get ());
        }

    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.18mar2010   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::BrowseTinPointsVisibility (double eyeX, double eyeY, double eyeZ, void* userP, DTMFeatureCallback callBackFunctP)
    {
    FeatureCallbackTransformHelper helper (callBackFunctP, userP, _dtmTransformHelper.get ());
    userP = helper.GetUserArg ();
    callBackFunctP = helper.GetCallBackFunc ();

    BeAssert (GetTinHandle () != nullptr);

    if (_dtmTransformHelper.IsValid ())
        _dtmTransformHelper->convertPointToDTM (eyeX, eyeY, eyeZ);
    // Call the core DTM function
    return (DTMStatusInt)bcdtmVisibility_determineVisibilityTinPointsDtmObject (GetTinHandle (), eyeX, eyeY, eyeZ, callBackFunctP, userP);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.18mar2010   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::BrowseTinLinesVisibility (double eyeX, double eyeY, double eyeZ, void*userP, DTMFeatureCallback callBackFunctP)
    {
    BeAssert (GetTinHandle () != nullptr);

    FeatureCallbackTransformHelper helper (callBackFunctP, userP, _dtmTransformHelper.get ());
    userP = helper.GetUserArg ();
    callBackFunctP = helper.GetCallBackFunc ();
    // Call the core DTM function
    if (_dtmTransformHelper.IsValid ())
        _dtmTransformHelper->convertPointToDTM (eyeX, eyeY, eyeZ);
    return (DTMStatusInt)bcdtmVisibility_determineVisibilityTinLinesDtmObject (GetTinHandle (), eyeX, eyeY, eyeZ, callBackFunctP, userP);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.18mar2010   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::BrowseRadialViewSheds(double eyeX, double eyeY, double eyeZ, long viewShedOption, long numberRadials, double radialIncrement, void *userP, DTMFeatureCallback callBackFunctP)
    {
    FeatureCallbackTransformHelper helper (callBackFunctP, userP, _dtmTransformHelper.get ());
    userP = helper.GetUserArg ();
    callBackFunctP = helper.GetCallBackFunc ();
    BeAssert (GetTinHandle () != nullptr);

    // Call the core DTM function
    if (_dtmTransformHelper.IsValid ())
        _dtmTransformHelper->convertPointToDTM (eyeX, eyeY, eyeZ);

    return (DTMStatusInt)bcdtmVisibility_determineRadialViewShedsDtmObject (GetTinHandle (), eyeX, eyeY, eyeZ, viewShedOption, numberRadials, radialIncrement, callBackFunctP, userP);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.22mar2010   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::BrowseRegionViewSheds (double eyeX, double eyeY, double eyeZ, void *userP, DTMFeatureCallback callBackFunctP)
    {
    FeatureCallbackTransformHelper helper (callBackFunctP, userP, _dtmTransformHelper.get ());
    userP = helper.GetUserArg ();
    callBackFunctP = helper.GetCallBackFunc ();

    BeAssert (GetTinHandle () != nullptr);

    // Call the core DTM function
    if (_dtmTransformHelper.IsValid ())
        _dtmTransformHelper->convertPointToDTM (eyeX, eyeY, eyeZ);

    return (DTMStatusInt)bcdtmVisibility_determineRegionViewShedsDtmObject (GetTinHandle (), eyeX, eyeY, eyeZ, callBackFunctP, userP);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.26May2010   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
// ToDo Should this function be in here?
DTMStatusInt BcDTM::ClipToTinHull (DTMClipOption clipOption, DPoint3dCP featurePtsP, int numFeaturePts, bvector<DtmString>& clipSections)
    {
    DTMStatusInt status = DTM_SUCCESS;
    long     n, numClipArrays = 0, clipResult = 0;
    DTM_POINT_ARRAY **clipArraysPP = nullptr;
    DtmString clipString;

    if (SetMemoryAccess (DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message (1, 0, 0, "DTM is readonly");
        return DTM_ERROR;
        }

    BeAssert (GetTinHandle () != nullptr);

    // Call the core DTM function

    if (_dtmTransformHelper.IsValid ())
        status = (DTMStatusInt)bcdtmClip_featurePointArrayToTinHullDtmObject (GetTinHandle (), (DTMFenceOption)clipOption, (DPoint3d *)_dtmTransformHelper->copyPointsToDTM (featurePtsP, numFeaturePts), numFeaturePts, &clipResult, &clipArraysPP, &numClipArrays);
    else
        status = (DTMStatusInt)bcdtmClip_featurePointArrayToTinHullDtmObject (GetTinHandle (), (DTMFenceOption)clipOption, (DPoint3d *)featurePtsP, numFeaturePts, &clipResult, &clipArraysPP, &numClipArrays);
    if (status == DTM_SUCCESS)
        {
        if (clipResult == 1) // Feature Was Not Clipped - Copy Entire Feature
            {
            clipString.resize (numFeaturePts);

            memcpy (clipString.data(), featurePtsP, numFeaturePts * sizeof(DPoint3d));
            clipSections.push_back (clipString);
            }
        else if (clipResult == 2) // Feature Was Clipped - Copy Clipped Sections
            {
            for (n = 0; n < numClipArrays; ++n)
                {
                clipString.resize (clipArraysPP[n]->numPoints);
                memcpy (clipString.data(), clipArraysPP[n]->pointsP, clipArraysPP[n]->numPoints * sizeof(DPoint3d));
                clipSections.push_back (clipString);
                }
            }
        }

    if (status == DTM_SUCCESS && _dtmTransformHelper.IsValid ())
        _dtmTransformHelper->convertDtmStringVectorFromDTM (clipSections);
    if (clipArraysPP != nullptr) bcdtmMem_freePointerArrayToPointArrayMemory (&clipArraysPP, numClipArrays);

    return status;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Rob.Cormack  01/11
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt BcDTM::ReplaceFeaturePoints(DTMFeatureId dtmFeatureId, DPoint3d *pointsP , int numPoints)
    {
    if (SetMemoryAccess(DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message(1,0,0, "DTM is readonly");
        return DTM_ERROR;
        }
    if (_dtmTransformHelper.IsValid())
        return (DTMStatusInt)bcdtmData_replaceDtmFeaturePointsDtmObject (GetTinHandle (), (DTMFeatureId)dtmFeatureId, (DPoint3d *)_dtmTransformHelper->copyPointsToDTM (pointsP, (long)numPoints), (long)numPoints);
    else
        return (DTMStatusInt)bcdtmData_replaceDtmFeaturePointsDtmObject (GetTinHandle (), (DTMFeatureId)dtmFeatureId, (DPoint3d *)pointsP, (long)numPoints);
    }
/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.29Aug2011   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::BulkDeleteFeaturesByUserTag (DTMUserTag* userTagP, int numUserTag)
    {
    if (SetMemoryAccess (DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message (1, 0, 0, "DTM is readonly");
        return DTM_ERROR;
        }
    DTMStatusInt status = (DTMStatusInt)bcdtmData_bulkUserTagDeleteDtmFeaturesDtmObject (GetTinHandle (), (DTMUserTag *)userTagP, (long)numUserTag);
    return status;
    }
/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.29Aug2011   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/

DTMStatusInt BcDTM::BulkDeleteFeaturesByFeatureId (DTMFeatureId* featureIdP, int numFeatureId)
    {
    if (SetMemoryAccess (DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message (1, 0, 0, "DTM is readonly");
        return DTM_ERROR;
        }
    DTMStatusInt status = (DTMStatusInt)bcdtmData_bulkFeatureIdDeleteDtmFeaturesDtmObject (GetTinHandle (), (DTMFeatureId *)featureIdP, (long)numFeatureId);
    return status;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Rob.Cormack  01/11
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt BcDTM::SetCleanUpOptions (DTMCleanupFlags options)
    {
    return (DTMStatusInt)bcdtmObject_setApiCleanUpDtmObject (GetTinHandle (), options);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Rob.Cormack  01/11
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt BcDTM::GetCleanUpOptions (DTMCleanupFlags& options)
    {
    return (DTMStatusInt)bcdtmObject_getApiCleanUpDtmObject (GetTinHandle (), &options);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt BcDTM::SetMemoryAccess (DTMAccessMode accessMode)
    {
    // Todo check for readonly instance.
    if (DTMAccessMode::Commit == accessMode && _readonly)
        return DTMStatusInt::DTM_ERROR;
    return (DTMStatusInt)bcdtmMemory_setMemoryAccess (GetTinHandle (), accessMode);
    }

///*---------------------------------------------------------------------------------------
//* @bsimethod                                                    Daryl.Holmwood  07/11
//+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt BcDTM::GetLastModifiedTime (Int64& lastModifiedTime)
    {
    lastModifiedTime = GetTinHandle()->lastModifiedTime;

    if (lastModifiedTime == 0)
        {
        const Int64 date1Jan1970 = 116444736000000000;
        lastModifiedTime = date1Jan1970 + ((Int64)GetTinHandle()->modifiedTime) * 10000000;
        }
    return DTM_SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
Int64 BcDTM::_GetPointCount ()
    {
    DTMFeatureStatisticsInfo info;
    CalculateFeatureStatistics (info);
    return info.numPoints;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
Bentley::TerrainModel::IDTMDraping* BcDTM::_GetDTMDraping ()
    {
    return this;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
Bentley::TerrainModel::IDTMContouring* BcDTM::_GetDTMContouring ()
    {
    return this;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
Bentley::TerrainModel::IDTMDrainage* BcDTM::_GetDTMDrainage()
    {
    return this;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Elenie.Godzaridis 1/16
+---------------+---------------+---------------+---------------+---------------+------*/
BENTLEY_NAMESPACE_NAME::TerrainModel::IDTMVolume* BcDTM::_GetDTMVolume()
    {
    return nullptr;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Daryl.Holmwood  02/11
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt BcDTM::_GetTransformDTM (Bentley::TerrainModel::DTMPtr& transformedDTM, TransformCR transformation)
    {
    if (transformation.isIdentity())
        transformedDTM = this;
    else
        {
        Bentley::Transform trfs;
        Bentley::Transform curTrfs;

        if (_dtmTransformHelper.IsValid() && _dtmTransformHelper->GetTransformationFromDTM (curTrfs))
            trfs.productOf (&curTrfs, &transformation);
        else
            trfs = transformation;

        TMTransformHelper* transformHelper = nullptr;
        if (!trfs.isIdentity())
            transformHelper = TMTransformHelper::Create (trfs);

        BcDTMPtr bcDtm = new BcDTM (*GetTinHandle(), transformHelper);
        transformedDTM = bcDtm;
        }
    return DTM_SUCCESS;
    }

DTMStatusInt BcDTM::GetReadOnlyDTM (Bentley::TerrainModel::BcDTMPtr& readonlyDTM)
    {
    readonlyDTM = new BcDTM (*GetTinHandle (), _dtmTransformHelper.get(), true);
    return DTM_SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Daryl.Holmwood  02/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool BcDTM::_GetTransformation (TransformR transformation)
    {
    if (_dtmTransformHelper.IsValid ())
        return _dtmTransformHelper->GetTransformationFromDTM (transformation);
    transformation.initIdentity();
    return true;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
BcDTMP BcDTM::_GetBcDTM()
    {
    return this;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt BcDTM::_GetBoundary(Bentley::TerrainModel::DTMPointArray& ret)
    {
    DPoint3d *verticesP = nullptr;
    long nPts;
    BeAssert (GetTinHandle () != nullptr);

    DTMStatusInt status = (DTMStatusInt)bcdtmList_extractHullDtmObject (GetTinHandle (), &verticesP, &nPts);
    if (status == DTM_SUCCESS)
        {
        if (_dtmTransformHelper.IsValid ())
            _dtmTransformHelper->convertPointsFromDTM (verticesP, nPts);
        ret.resize (nPts);
        memcpy (ret.data (), verticesP, nPts * sizeof (DPoint3d));

        bcMem_free (verticesP);
        }
    return status;
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Elenie.Godzaridis  03/16
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt BcDTM::_CalculateSlopeArea(double& flatArea, double& slopeArea, DPoint3dCP pts, int numPoints, DTMAreaValuesCallback progressiveCallback, DTMCancelProcessCallback isCancelledCallback)
    {
    DTMStatusInt retval = CalculateSlopeArea(flatArea, slopeArea, pts, numPoints);
    progressiveCallback(retval,flatArea, slopeArea);
    return retval;
    }

/*---------------------------------------------------------------------------------------
* @bsiclass                                                     Daryl.Holmwood  12/10
+---------------+---------------+---------------+---------------+---------------+------*/
struct DTMDrainageFeature : public Bentley::TerrainModel::IDTMDrainageFeature
    {
    private:
        bvector<Bentley::TerrainModel::DTMPointArray> m_array;
        bvector<bool> m_pondArray;
        mutable int m_count;

    public:
        DTMDrainageFeature ()
            {
            m_count = 0;
            }
        virtual UInt32 AddRef() const
            {
            m_count++;
            return m_count;
            }

        virtual UInt32 Release() const
            {
            m_count--;
            if (m_count == 0)
                {
                delete this;
                return 0;
                }
            return m_count;
            }
        virtual int _GetNumParts()
            {
            return (int)m_array.size();
            }
        virtual bool _GetIsPond(int index)
            {
            if (m_array.size () <= index)
                return false;
            return m_pondArray [index];
            }
        virtual DTMStatusInt _GetPoints(Bentley::TerrainModel::DTMPointArray& ret, int index)
            {
            if (m_array.size () > index)
                {
                ret = m_array [index];
                return DTM_SUCCESS;
                }
            return DTM_ERROR;
            }

        void AddFeature (const Bentley::TerrainModel::DTMPointArray& points, bool isPond)
            {
            m_pondArray.push_back (isPond);
            // ToDo DHMEM check refcount.
            m_array.push_back (points);
            }
    };

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Daryl.Holmwood  02/11
+---------------+---------------+---------------+---------------+---------------+------*/
int DTMDrainageFeatureCallback (
    DTMFeatureType featureType,
    DTMUserTag   featureTag,
    DTMFeatureId featureId,
    DPoint3d       *tPoint,
    size_t         nPoint,
    void           *userP
    )
    {
    DTMDrainageFeature* feat = static_cast<DTMDrainageFeature*>(userP);
    DTMPointArray pts;

    pts.resize (nPoint);
    memcpy (pts.data (), tPoint, nPoint * sizeof (DPoint3d));
    feat->AddFeature (pts, featureType == DTMFeatureType::LowPointPond);
    return SUCCESS;
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Daryl.Holmwood  12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt BcDTM::_GetDescentTrace (Bentley::TerrainModel::DTMDrainageFeaturePtr& ret, DPoint3dCR pt, double minDepth)
    {
    DTMDrainageFeature* result =  new DTMDrainageFeature();

    if (GetDescentTrace (minDepth, pt.x, pt.y, result, &DTMDrainageFeatureCallback) == SUCCESS)
        {
        ret = result;
        return DTM_SUCCESS;
        }
    delete result;
    return DTM_ERROR;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Daryl.Holmwood  12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt BcDTM::_GetAscentTrace (Bentley::TerrainModel::DTMDrainageFeaturePtr& ret, DPoint3dCR pt, double minDepth)
    {
    DTMDrainageFeature* result =  new DTMDrainageFeature();

    if (GetAscentTrace (minDepth, pt.x, pt.y, result, &DTMDrainageFeatureCallback) == DTM_SUCCESS)
        {
        ret = result;
        return DTM_SUCCESS;
        }
    delete result;
    return DTM_ERROR;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Daryl.Holmwood  12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt BcDTM::_TraceCatchmentForPoint (Bentley::TerrainModel::DTMDrainageFeaturePtr& ret, DPoint3dCR pt, double maxPondDepth)
    {
    bvector<DPoint3d> catchmentPts;
    DPoint3d sumpPoint;
    bool catchmentDetermined;
    DTMDynamicFeatureArray catchmentFeatures;

    if (TraceCatchmentForPoint (pt.x, pt.y, maxPondDepth, catchmentDetermined, sumpPoint, catchmentPts) == DTM_SUCCESS)
        {
        if (catchmentDetermined)
            {
            DTMDrainageFeature* result = new DTMDrainageFeature ();
            ret = result;
            result->AddFeature (catchmentPts, false);
            }
        return DTM_SUCCESS;
        }
    return DTM_ERROR;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.18Oct2012   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::CreatePointStockPile
    (
    DPoint3d    headCoordinates,    // ==> Conveyor Head Coordinates
    double      stockPileSlope,     // ==> Slope Of Stock Pile Expressed As Ratio Of Rise / Run
    bool        mergeOption,        // ==> Merge Option If True Create DTM Of Stockpile And Ground DTM
    double&     stockPileVolume,    // <== Volume Of Stock Pile
    BcDTMPtr     *stockPileTmPP,    // <== Stock Pile TM
    BcDTMPtr     *mergedTmPP        // <== Merged Ground And StockPile TM
    )
    {
    BeAssert (GetTinHandle() != nullptr);
// ToDo Translatation

    DTMStatusInt status = DTM_SUCCESS;
    double volume=0.0 ;
    BC_DTM_OBJ  *stockPileDtmP=nullptr,*mergedDtmP=nullptr ;
    DPoint3d p3dHeadCoordinates ;

    // Check arguments

    if (stockPileTmPP == nullptr)   return DTM_ERROR;
    if (mergedTmPP    == nullptr)   return DTM_ERROR;

    //  Set Variables

    stockPileVolume = 0.0 ;
    p3dHeadCoordinates.x = headCoordinates.x ;
    p3dHeadCoordinates.y = headCoordinates.y ;
    p3dHeadCoordinates.z = headCoordinates.z ;

    //  Call Core To Create Stock Pile

    status = (DTMStatusInt)bcdtmStockPile_createPointStockPileDtmObject ((BC_DTM_OBJ *)GetTinHandle (), p3dHeadCoordinates, stockPileSlope, mergeOption, &stockPileDtmP, &mergedDtmP, &volume);

    if( status == DTM_SUCCESS )
        {
        stockPileVolume = volume ;
        *stockPileTmPP = BcDTM::CreateFromDtmHandle (*stockPileDtmP);
        if( mergeOption )
            {
            *mergedTmPP = BcDTM::CreateFromDtmHandle (*mergedDtmP);
            }
        }
    else
        {
        if( stockPileDtmP != nullptr )
            bcdtmObject_destroyDtmObject(&stockPileDtmP) ;

        if( mergedDtmP != nullptr )
            bcdtmObject_destroyDtmObject(&mergedDtmP) ;
        }

    // Return

    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.19Oct2012   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTM::CreateAlignmentStockPile
    (
    DPoint3dCP      headCoordinates,   // ==> Conveyor Head Alignment Coordinates
    long            numHeadCoordinates, // ==> Number Of Head Coordinates
    double          stockPileSlope,     // ==> Slope Of Stock Pile Expressed As Ratio Of Rise / Run
    bool            mergeOption,        // ==> Merge Option If True Create DTM Of Stockpile And Ground DTM
    double&         stockPileVolume,    // <== Volume Of Stock Pile
    BcDTMPtr*       stockPileTmPP,    // <== Stock Pile TM
    BcDTMPtr*       mergedTmPP        // <== Merged Ground And StockPile TM
    )
    {
    BeAssert (GetTinHandle() != nullptr);
// ToDo Translatation

    DTMStatusInt status = DTM_SUCCESS;
    double volume=0.0 ;
    BC_DTM_OBJ  *stockPileDtmP=nullptr,*mergedDtmP=nullptr ;

    // Check arguments

    if (stockPileTmPP == nullptr)   return DTM_ERROR;
    if (mergedTmPP    == nullptr)   return DTM_ERROR;

    //  Set Variables

    stockPileVolume = 0.0 ;

    //  Call Core To Create Stock Pile

    status = (DTMStatusInt)bcdtmStockPile_createStringStockPileDtmObject ((BC_DTM_OBJ *)GetTinHandle (), (DPoint3d *)headCoordinates, numHeadCoordinates, stockPileSlope, mergeOption, &stockPileDtmP, &mergedDtmP, &volume);

    if( status == DTM_SUCCESS )
        {
        stockPileVolume = volume ;
        *stockPileTmPP = BcDTM::CreateFromDtmHandle (*stockPileDtmP);
        if( mergeOption )
            {
            *mergedTmPP = BcDTM::CreateFromDtmHandle (*mergedDtmP);
            }
        }
    else
        {
        if( stockPileDtmP != nullptr )
            bcdtmObject_destroyDtmObject(&stockPileDtmP) ;

        if( mergedDtmP != nullptr )
            bcdtmObject_destroyDtmObject(&mergedDtmP) ;
        }

    // Return

    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   djh.27May2014   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
bool BcDTM::IntersectVector (DPoint3dR intersectionPoint, DPoint3dCR startPoint, DPoint3dCR endPoint)
    {
    DPoint3d startPt = startPoint;
    DPoint3d endPt = endPoint;

    // if we have a
    if (_dtmTransformHelper.IsValid ())
        {
        if (_dtmTransformHelper.IsValid ())
            {
            _dtmTransformHelper->convertPointToDTM (startPt);
            _dtmTransformHelper->convertPointToDTM (endPt);
            BcDTMPtr tempDtm = BcDTM::CreateFromDtmHandle (*GetTinHandle ());
            if (!tempDtm->IntersectVector (intersectionPoint, startPt, endPt))
                return false;
            _dtmTransformHelper->convertPointFromDTM (intersectionPoint);
            return true;
            }
        }

    if(fabs (startPt.x - endPt.x) > 1e-5 || fabs (startPt.y - endPt.y) > 1e-5)
        {
        // Intersect line with the DTM Range
        DRange3d range;
        DPoint3d sP;
        DPoint3d eP;
        DPoint3d point;

        GetRange (range);
        endPt.x -= startPt.x;
        endPt.y -= startPt.y;
        endPt.z -= startPt.z;
        if (!range.intersectRay (nullptr, nullptr, &sP, &eP, &startPt, &endPt))
            return false;

        // Non Top View
        DPoint3d trianglePts[4];
        long drapedType;
        BC_DTM_OBJ* bcDTM = GetTinHandle ();
        bool voidFlag;

        if (bcdtmDrape_intersectTriangleDtmObject (bcDTM, &sP, &eP, &drapedType, &point, trianglePts, voidFlag) != DTM_SUCCESS || drapedType == 0 || voidFlag != false)
            return false;

        startPt = point;
        }

    // TopView
    DPoint3d trianglePts[4];
    int drapedType;
    double elevation;

    if (DTM_SUCCESS != _DrapePoint (&elevation, nullptr, nullptr, trianglePts, drapedType, startPt))
        {
        return false;
        }
    startPt.z = elevation;

    return true;
    }


DTMStatusInt BcDTM::Clean ()
    {
    if (SetMemoryAccess (DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message (1, 0, 0, "DTM is Read Only");
        return DTM_ERROR;
        }

    BC_DTM_OBJ* bcDTM = GetTinHandle ();

    DTMStatusInt status = DTM_SUCCESS;
    if (bcDTM->dtmState == DTMState::Tin)
        status = (DTMStatusInt)bcdtmList_cleanDtmObject(bcDTM);
    else if (bcDTM->dtmState == DTMState::Data)
        status = (DTMStatusInt)bcdtmData_compactUntriangulatedFeatureTableDtmObject(bcDTM);
    return status;
    }

//=======================================================================================
//
//+===============+===============+===============+===============+===============+======
DTMStatusInt BcDTM::AddFeatureWithMultipleSegments
(
DTMFeatureType dtmFeatureType,   // =>
const DtmVectorString& features, // =>
DTMUserTag   userTag,          // =>
DTMFeatureId *featureIdP       // <=
)
    {
    if (features.size () == 0)
        return DTM_ERROR;
    if (SetMemoryAccess (DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message (1, 0, 0, "DTM is readonly");
        return DTM_ERROR;
        }
    DTMStatusInt sts = DTM_SUCCESS;

    TMTransformHelperP helper = GetTransformHelper ();
    // Store the feature in the DTM
    for (size_t i = 0; i < features.size (); i++)
        {
        if (helper)
            {
            if ((sts = (DTMStatusInt)bcdtmObject_storeDtmFeatureInDtmObject(GetTinHandle(), dtmFeatureType, userTag, i == 0 ? 3 : 2, featureIdP, helper->copyPointsToDTM(features[i].data(), (int)features[i].size()), (int)features[i].size())) != DTM_SUCCESS)
                return sts;
            }
        else
            {
            if ((sts = (DTMStatusInt)bcdtmObject_storeDtmFeatureInDtmObject(GetTinHandle(), dtmFeatureType, userTag, i == 0 ? 3 : 2, featureIdP, features[i].data(), (int)features[i].size())) != DTM_SUCCESS)
                return sts;
            }
        }

    return sts;
    }

//=======================================================================================
//
//+===============+===============+===============+===============+===============+======
DTMStatusInt BcDTM::ReplaceFeatureWithMultipleSegments
(
const DtmVectorString& features, // =>
DTMFeatureId featureId       // <=
)
    {
    if (features.size () == 0)
        return DTM_ERROR;

    if (SetMemoryAccess (DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message (1, 0, 0, "DTM is readonly");
        return DTM_ERROR;
        }
    TMTransformHelperP helper = GetTransformHelper ();

    // Try and replace the
    if (helper)
        {
        DtmVectorString transformedfeatures = features;
        helper->convertDtmStringVectorToDTM (transformedfeatures);
        return (DTMStatusInt)bcdtmData_replaceDtmFeaturePointsMultipleDtmObject(GetTinHandle(), (DTMFeatureId)featureId, transformedfeatures);
        }
    return (DTMStatusInt)bcdtmData_replaceDtmFeaturePointsMultipleDtmObject(GetTinHandle(), (DTMFeatureId)featureId, features);
    }

bool BcDTM::GetProjectedPointOnDTM (DPoint3dR pointOnDTM, DMatrix4dCR w2vMap, DPoint3dCR testPoint)
    {
    TMTransformHelperP helper = GetTransformHelper ();
    DPoint3d startPt = testPoint;
    DPoint3d endPt;
    DPoint3d pt;
    DMatrix4d invW2vMap;
    DPoint4d pt4;
    DPoint4d endPt4;

    invW2vMap.qrInverseOf (&w2vMap);
    w2vMap.multiplyAndRenormalize (&pt, &testPoint, 1);
    pt.z -= 100;
    invW2vMap.multiplyAndRenormalize (&endPt, &pt, 1);
    pt4.init (&testPoint, 0);
    w2vMap.multiply (&pt4, &pt4, 1);
    pt4.z -= 100;
    invW2vMap.multiply (&endPt4, &pt4, 1);
    endPt.init (endPt4.x, endPt4.y, endPt4.z);

    if (helper)
        {
        helper->convertPointToDTM (startPt);
        helper->convertPointToDTM (endPt);
        }

    if (fabs (startPt.x - endPt.x) > 1e-5 || fabs (startPt.y - endPt.y) > 1e-5)
        {
        // Intersect line with the DTM Range
        DRange3d range;
        DPoint3d sP;
        DPoint3d eP;
        DPoint3d point;

        GetRange (range);
        endPt.x -= startPt.x;
        endPt.y -= startPt.y;
        endPt.z -= startPt.z;
        if (!range.intersectRay (NULL, NULL, &sP, &eP, &startPt, &endPt))
            return false;

        // Non Top View
        DPoint3d trianglePts[4];
        long drapedType;
        BC_DTM_OBJ* bcDTM = GetTinHandle ();
        bool voidFlag;

        if (bcdtmDrape_intersectTriangleDtmObject (bcDTM, ((DPoint3d*)&sP), ((DPoint3d*)&eP), &drapedType, (DPoint3d*)&point, (DPoint3d*)&trianglePts, voidFlag) != DTM_SUCCESS || drapedType == 0 || voidFlag != 0)
            {
            return false;
            }

        startPt = point;
        }

    // TopView
    DPoint3d trianglePts[4];
    int drapedType;
    double elevation;

    if (DTM_SUCCESS != _DrapePoint (&elevation, NULL, NULL, trianglePts, drapedType, startPt))
        {
        return false;
        }
    startPt.z = elevation;

    pointOnDTM = startPt;
    if (helper) helper->convertPointFromDTM (pointOnDTM);
    return true;
    }
    
bool BcDTM::_GetProjectedPointOnDTM (DPoint3dR pointOnDTM, DMatrix4dCR w2vMap, DPoint3dCR testPoint)
    {
    TMTransformHelperP helper = GetTransformHelper ();
    DPoint3d startPt = testPoint;
    DPoint3d endPt;
    DPoint3d pt;
    DMatrix4d invW2vMap;
    DPoint4d pt4;
    DPoint4d endPt4;

    invW2vMap.QrInverseOf (w2vMap);
    w2vMap.MultiplyAndRenormalize (&pt, &testPoint, 1);
    pt.z -= 100;
    invW2vMap.MultiplyAndRenormalize (&endPt, &pt, 1);
    pt4.Init (testPoint, 0);
    w2vMap.Multiply (&pt4, &pt4, 1);
    pt4.z -= 100;
    invW2vMap.Multiply (&endPt4, &pt4, 1);
    endPt.Init (endPt4.x, endPt4.y, endPt4.z);

    if (helper)
        {
        helper->convertPointToDTM (startPt);
        helper->convertPointToDTM (endPt);
        }

    if (fabs (startPt.x - endPt.x) > 1e-5 || fabs (startPt.y - endPt.y) > 1e-5)
        {
        // Intersect line with the DTM Range
        DRange3d range;
        DPoint3d sP;
        DPoint3d eP;
        DPoint3d point;

        GetRange (range);
        endPt.x -= startPt.x;
        endPt.y -= startPt.y;
        endPt.z -= startPt.z;
        double p1, p2; // Not Used
        if (!range.IntersectRay (p1, p2, sP, eP, startPt, endPt))
            return false;

        // Non Top View
        DPoint3d trianglePts[4];
        long drapedType;
        BC_DTM_OBJ* bcDTM = GetTinHandle ();
        bool voidFlag;

        if (bcdtmDrape_intersectTriangleDtmObject (bcDTM, ((DPoint3d*)&sP), ((DPoint3d*)&eP), &drapedType, (DPoint3d*)&point, (DPoint3d*)&trianglePts, voidFlag) != DTM_SUCCESS || drapedType == 0 || voidFlag != 0)
            {
            return false;
            }

        startPt = point;
        }

    // TopView
    DPoint3d trianglePts[4];
    int drapedType;
    double elevation;

    if (DTM_SUCCESS != DrapePoint (&elevation, NULL, NULL, trianglePts, drapedType, startPt))
        {
        return false;
        }
    startPt.z = elevation;

    pointOnDTM = startPt;
    if (helper) helper->convertPointFromDTM (pointOnDTM);
    return true;
    }

DTMStatusInt BcDTM::FilterPoints (long numPointsToRemove, double percentageToRemove, long& pointsBefore, long& pointsAfter)
    {
    if (SetMemoryAccess (DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message (1, 0, 0, "DTM is readonly");
        return DTM_ERROR;
        }

    return (DTMStatusInt)bcdtmFilter_tileDecimateRandomSpotsDtmObject(GetTinHandle(), numPointsToRemove, percentageToRemove, pointsBefore, pointsAfter);
    }

DTMStatusInt BcDTM::DrapeLinearPoints(bvector<DTMDrapePoint>& drapedPts, DPoint3dCP pointsP, int nPts, bool getFeatures)
    {
    DTMStatusInt status = DTM_SUCCESS;
    
    drapedPts.clear();
    if (_dtmTransformHelper.IsValid())
        {
        status = (DTMStatusInt)bcdtmDrape_stringDtmObject(GetTinHandle(), _dtmTransformHelper->copyPointsToDTM(pointsP, nPts), nPts, getFeatures, drapedPts);
        if (SUCCESS == status)
            {
            for (auto& pt : drapedPts)
                {
                _dtmTransformHelper->convertPointToDTM(pt.drapePt);
                }
            }
        }
    else
        status = (DTMStatusInt)bcdtmDrape_stringDtmObject(GetTinHandle(), pointsP, nPts, getFeatures, drapedPts);
    return status;
    }


/////////////////////////////////////////////
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt BcDTM::DrapePoint(double* elevation, double* slope, double* aspect, DPoint3d triangle[3], int& drapedType, const DPoint3d& point)
    {
    return _DrapePoint(elevation, slope, aspect, triangle, drapedType, point);
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Rob.Cormack    09/10
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt BcDTM::CalculateSlopeArea(double& flatAreaP, double& slopeAreaP, DPoint3dCP points, int numPoints)
    {
    return _CalculateSlopeArea(flatAreaP, slopeAreaP, points, numPoints);
    }

END_BENTLEY_TERRAINMODEL_NAMESPACE
