/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshQuery.cpp $
|    $RCSfile: ScalableMeshQuery.cpp,v $
|   $Revision: 1.41 $
|       $Date: 2012/11/29 17:30:37 $
|     $Author: Mathieu.St-Pierre $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <ScalableMeshPCH.h>

//#define GPU
#undef static_assert
#include <ppl.h>
#ifdef GPU
#include <amp.h>
#endif
USING_NAMESPACE_IMAGEPP;

extern bool   GET_HIGHEST_RES;

/*------------------------------------------------------------------+
| Include of the current class header                               |
+------------------------------------------------------------------*/
#include "InternalUtilityFunctions.h"
#include "ScalableMesh.h"
#include "ScalableMeshQuadTreeBCLIBFilters.h"
#include "ScalableMeshQuadTreeQueries.h"
#include "ScalableMeshQuery.h"
#include "ScalableMesh\ScalableMeshGraph.h"

/*----------------------------------------------+
| Constant definitions                          |
+----------------------------------------------*/

/*----------------------------------------------+
| Private type definitions                      |
+----------------------------------------------*/

/*==================================================================*/
/*                                                                  */
/*          INTERNAL FUNCTIONS                                      */
/*                                                                  */
/*==================================================================*/

USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT
USING_NAMESPACE_BENTLEY_SCALABLEMESH_GEOCOORDINATES

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

#define P2P_TOLERANCE 0.0000001

/*==================================================================*/
/*        QUERY PARAMETERS SECTION - BEGIN                          */
/*==================================================================*/

/*==================================================================*/
/*                   IScalableMeshQueryParameters                          */
/*==================================================================*/
IScalableMeshQueryParameters::IScalableMeshQueryParameters()
    {
    }

IScalableMeshQueryParameters::~IScalableMeshQueryParameters()
    {
    }

bool IScalableMeshQueryParameters::GetTriangulationState()
    {
    return _GetTriangulationState();
    }

void IScalableMeshQueryParameters::SetTriangulationState(bool pi_isReturnedDataTriangulated)
    {
    _SetTriangulationState(pi_isReturnedDataTriangulated);
    }

Bentley::GeoCoordinates::BaseGCSPtr IScalableMeshQueryParameters::GetSourceGCS()
    {
    return _GetSourceGCS();
    }

Bentley::GeoCoordinates::BaseGCSPtr IScalableMeshQueryParameters::GetTargetGCS()
    {
    return _GetTargetGCS();
    }

void IScalableMeshQueryParameters::SetGCS(Bentley::GeoCoordinates::BaseGCSPtr& sourceGCSPtr,
                                   Bentley::GeoCoordinates::BaseGCSPtr& targetGCSPtr)
    {
    _SetGCS(sourceGCSPtr, targetGCSPtr);
    }

long IScalableMeshQueryParameters::GetEdgeOptionTriangulationParam()
    {
    return _GetEdgeOptionTriangulationParam();
    }

double IScalableMeshQueryParameters::GetMaxSideLengthTriangulationParam()
    {
    return _GetMaxSideLengthTriangulationParam();
    }

void IScalableMeshQueryParameters::SetEdgeOptionTriangulationParam(long edgeOption)
    {
    _SetEdgeOptionTriangulationParam(edgeOption);
    }

void IScalableMeshQueryParameters::SetMaxSideLengthTriangulationParam(double maxSideLength)
    {
    _SetMaxSideLengthTriangulationParam(maxSideLength);
    }

double IScalableMeshQueryParameters::GetToleranceTriangulationParam()
    {
    return P2P_TOLERANCE;
    }


/*==================================================================*/
/*                   IScalableMeshFullResolutionQueryParams                */
/*==================================================================*/
IScalableMeshFullResolutionQueryParams::IScalableMeshFullResolutionQueryParams()
    {
    }

IScalableMeshFullResolutionQueryParams::~IScalableMeshFullResolutionQueryParams()
    {
    }

IScalableMeshFullResolutionQueryParamsPtr IScalableMeshFullResolutionQueryParams::CreateParams()
    {
    return new ScalableMeshFullResolutionQueryParams();
    }

size_t IScalableMeshFullResolutionQueryParams::GetMaximumNumberOfPoints()
    {
    return _GetMaximumNumberOfPoints();
    }

void IScalableMeshFullResolutionQueryParams::SetMaximumNumberOfPoints(size_t maximumNumberOfPoints)
    {
    _SetMaximumNumberOfPoints(maximumNumberOfPoints);
    }

bool IScalableMeshFullResolutionQueryParams::GetReturnAllPtsForLowestLevel()
    {
    return _GetReturnAllPtsForLowestLevel();
    }

void IScalableMeshFullResolutionQueryParams::SetReturnAllPtsForLowestLevel(bool returnAllPts)
    {
    _SetReturnAllPtsForLowestLevel(returnAllPts);
    }

/*==================================================================*/
/*                   IScalableMeshFullResolutionLinearQueryParams          */
/*==================================================================*/
IScalableMeshFullResolutionLinearQueryParams::IScalableMeshFullResolutionLinearQueryParams()
    {
    }

IScalableMeshFullResolutionLinearQueryParams::~IScalableMeshFullResolutionLinearQueryParams()
    {
    }

IScalableMeshFullResolutionLinearQueryParamsPtr IScalableMeshFullResolutionLinearQueryParams::CreateParams()
    {
    return new ScalableMeshFullResolutionLinearQueryParams();
    }

size_t IScalableMeshFullResolutionLinearQueryParams::GetMaximumNumberOfPointsForLinear()
    {
    return _GetMaximumNumberOfPointsForLinear();
    }

int IScalableMeshFullResolutionLinearQueryParams::SetMaximumNumberOfPointsForLinear(size_t maximumNumberOfPointsForLinear)
    {
    return _SetMaximumNumberOfPointsForLinear(maximumNumberOfPointsForLinear);
    }

void IScalableMeshFullResolutionLinearQueryParams::SetUseDecimation(bool useDecimation)
    {
    _SetUseDecimation(useDecimation);
    }

bool IScalableMeshFullResolutionLinearQueryParams::GetUseDecimation()
    {
    return _GetUseDecimation();
    }

void IScalableMeshFullResolutionLinearQueryParams::SetCutLinears(bool cutLinears)
    {
    _SetCutLinears(cutLinears);
    }

bool IScalableMeshFullResolutionLinearQueryParams::GetCutLinears()
    {
    return _GetCutLinears();
    }

void IScalableMeshFullResolutionLinearQueryParams::SetAddLinears(const bool addLinears)
    {
    _SetAddLinears(addLinears);
    }

bool IScalableMeshFullResolutionLinearQueryParams::GetAddLinears()
    {
    return _GetAddLinears();
    }

const std::vector<int>& IScalableMeshFullResolutionLinearQueryParams::GetFilteringFeatureTypes(bool& doIncludeFilteringFeatureTypes)
    {
    return _GetFilteringFeatureTypes(doIncludeFilteringFeatureTypes);
    }

//When no feature type is specified all feature types are returned.
int IScalableMeshFullResolutionLinearQueryParams::SetFilteringFeatureTypes(const std::vector<int>& filteringFeatureTypes, bool doIncludeFilteringFeatures)
    {
    return _SetFilteringFeatureTypes(filteringFeatureTypes, doIncludeFilteringFeatures);
    }

void IScalableMeshFullResolutionLinearQueryParams::SetIncludeFilteringFeatureTypes(const bool& doIncludeFilteringFeatures)
    {
    _SetIncludeFilteringFeatureTypes(doIncludeFilteringFeatures);
    }

/*==================================================================*/
/*                   ISrDTMViewDependentQueryParams                 */
/*==================================================================*/
ISrDTMViewDependentQueryParams::ISrDTMViewDependentQueryParams()
    {
    }

ISrDTMViewDependentQueryParams::~ISrDTMViewDependentQueryParams()
    {
    }

const DPoint3d* ISrDTMViewDependentQueryParams::GetViewBox() const
    {
    return _GetViewBox();
    }

void ISrDTMViewDependentQueryParams::SetViewBox(const DPoint3d viewBox[])
    {
    _SetViewBox(viewBox);
    }

/*==================================================================*/
/*                   IScalableMeshViewDependentQueryParams                 */
/*==================================================================*/
IScalableMeshViewDependentQueryParams::IScalableMeshViewDependentQueryParams()
    {
    }

IScalableMeshViewDependentQueryParams::~IScalableMeshViewDependentQueryParams()
    {
    }

double IScalableMeshViewDependentQueryParams::GetMinScreenPixelsPerPoint() const
    {
    return _GetMinScreenPixelsPerPoint();
    }

bool IScalableMeshViewDependentQueryParams::GetUseSameResolutionWhenCameraIsOff() const
    {
    return _GetUseSameResolutionWhenCameraIsOff();
    }

bool IScalableMeshViewDependentQueryParams::GetUseSplitThresholdForLevelSelection() const
    {
    return _GetUseSplitThresholdForLevelSelection();
    }

bool IScalableMeshViewDependentQueryParams::GetUseSplitThresholdForTileSelection() const
    {
    return _GetUseSplitThresholdForTileSelection();
    }

const double* IScalableMeshViewDependentQueryParams::GetRootToViewMatrix() const
    {
    return _GetRootToViewMatrix();
    }

void IScalableMeshViewDependentQueryParams::SetMinScreenPixelsPerPoint(double minScreenPixelsPerPoint)
    {
    _SetMinScreenPixelsPerPoint(minScreenPixelsPerPoint);
    }

void IScalableMeshViewDependentQueryParams::SetRootToViewMatrix(const double rootToViewMatrix[][4])
    {
    _SetRootToViewMatrix(rootToViewMatrix);
    }

void IScalableMeshViewDependentQueryParams::SetUseSameResolutionWhenCameraIsOff(bool useSameResolution)
    {
    _SetUseSameResolutionWhenCameraIsOff(useSameResolution);
    }

void IScalableMeshViewDependentQueryParams::SetUseSplitThresholdForLevelSelection(bool useSplitThreshold)
    {
    _SetUseSplitThresholdForLevelSelection(useSplitThreshold);
    }

void IScalableMeshViewDependentQueryParams::SetUseSplitThresholdForTileSelection(bool useSplitThreshold)
    {
    _SetUseSplitThresholdForTileSelection(useSplitThreshold);
    }

IScalableMeshViewDependentQueryParamsPtr IScalableMeshViewDependentQueryParams::CreateParams()
    {
    return new ScalableMeshViewDependentQueryParams();
    }

/*==================================================================*/
/*                   IScalableMeshFixResolutionIndexQueryParams            */
/*==================================================================*/
IScalableMeshFixResolutionIndexQueryParams::IScalableMeshFixResolutionIndexQueryParams()
    {
    }

IScalableMeshFixResolutionIndexQueryParams::~IScalableMeshFixResolutionIndexQueryParams()
    {
    }

int IScalableMeshFixResolutionIndexQueryParams::GetResolutionIndex() const
    {
    return _GetResolutionIndex();
    }

void IScalableMeshFixResolutionIndexQueryParams::SetResolutionIndex(int resolutionIndex)
    {
    _SetResolutionIndex(resolutionIndex);
    }

IScalableMeshFixResolutionIndexQueryParamsPtr IScalableMeshFixResolutionIndexQueryParams::CreateParams()
    {
    return new ScalableMeshFixResolutionIndexQueryParams();
    }

/*==================================================================*/
/*                   IScalableMeshFixResolutionMaxPointsQueryParams        */
/*==================================================================*/
IScalableMeshFixResolutionMaxPointsQueryParams::IScalableMeshFixResolutionMaxPointsQueryParams()
    {
    }

IScalableMeshFixResolutionMaxPointsQueryParams::~IScalableMeshFixResolutionMaxPointsQueryParams()
    {
    }

__int64 IScalableMeshFixResolutionMaxPointsQueryParams::GetMaxNumberPoints()
    {
    return _GetMaxNumberPoints();
    }

void IScalableMeshFixResolutionMaxPointsQueryParams::SetMaximumNumberPoints(__int64 maxNumberPoints)
    {
    _SetMaximumNumberPoints(maxNumberPoints);
    }

IScalableMeshFixResolutionMaxPointsQueryParamsPtr IScalableMeshFixResolutionMaxPointsQueryParams::CreateParams()
    {
    return new ScalableMeshFixResolutionMaxPointsQueryParams();
    }

/*==================================================================*/
/*                   IScalableMeshQueryAllLinearsQueryParams               */
/*==================================================================*/
IScalableMeshQueryAllLinearsQueryParams::IScalableMeshQueryAllLinearsQueryParams()
    {
    }

IScalableMeshQueryAllLinearsQueryParams::~IScalableMeshQueryAllLinearsQueryParams()
    {
    }

void IScalableMeshQueryAllLinearsQueryParams::SetFeatures (const list<HFCPtr<HVEDTMLinearFeature>>& features)
    {
    _SetFeatures(features);
    }

list<IScalableMeshFeaturePtr> IScalableMeshQueryAllLinearsQueryParams::GetFeatures()
    {
    return _GetFeatures();
    }

IScalableMeshQueryAllLinearsQueryParamsPtr IScalableMeshQueryAllLinearsQueryParams::CreateParams()
    {
    return new ScalableMeshQueryAllLinearsQueryParams();
    }

/*==================================================================*/
/*        QUERY PARAMETERS SECTION - END                            */
/*==================================================================*/

/*==================================================================*/
/*        QUERY SECTION - BEGIN                                     */
/*==================================================================*/

/*----------------------------------------------------------------------------+
|IScalableMeshQuery::Query
+----------------------------------------------------------------------------*/
int IScalableMeshQuery::Query(Bentley::TerrainModel::DTMPtr&            dtmPtr,
                       const DPoint3d*                  pQueryShapePts,
                       int                              nbQueryShapePts,
                       const IScalableMeshQueryParametersPtr&  scmQueryParamsPtr) const
    {
    return _Query(dtmPtr, pQueryShapePts, nbQueryShapePts, scmQueryParamsPtr);
    }

/*----------------------------------------------------------------------------+
|IScalableMeshQuery::GetNbClip
+----------------------------------------------------------------------------*/
int IScalableMeshQuery::GetNbClip() const
    {
    return _GetNbClip();
    }

/*----------------------------------------------------------------------------+
|IScalableMeshQuery::GetClip
+----------------------------------------------------------------------------*/
/*
int IScalableMeshQuery::GetClip(DPoint3d*& clipPointsP,
                         int&       numberOfPoints,
                         bool&      isClipMask,
                         int        clipInd) const
    {
    return _GetClip(clipPointsP, numberOfPoints, isClipMask, clipInd);
    }
*/
/*----------------------------------------------------------------------------+
|IScalableMeshQuery::AddClip
+----------------------------------------------------------------------------*/
int IScalableMeshQuery::AddClip(DPoint3d* clipPointsP,
                             int   numberOfPoints,
                             bool  isClipMask)
    {
    return _AddClip(clipPointsP, numberOfPoints, isClipMask);
    }

/*----------------------------------------------------------------------------+
|IScalableMeshQuery::RemoveAllClip
+----------------------------------------------------------------------------*/
int IScalableMeshQuery::RemoveAllClip()
    {
    return _RemoveAllClip();
    }

/*----------------------------------------------------------------------------+
|IScalableMeshQuery::GetReprojectionQueryInterface
+----------------------------------------------------------------------------*/
IScalableMeshQueryPtr ScalableMeshQuery::GetReprojectionQueryInterface(IScalableMeshPtr        scmToQueryPtr,
                                                         DTMQueryType     queryType,
                                                         DTMQueryDataType queryDataType,
                                                         const GCS&         sourceGCS,
                                                         const GCS&         targetGCS,
                                                         const DRange3d&  extentInTargetGCS)
    {
    IScalableMeshQueryPtr scmQueryPtr;



    if ((sourceGCS.IsNull() == false) || (targetGCS.IsNull() == false))
        {
        scmQueryPtr = scmToQueryPtr->GetQueryInterface(queryType, queryDataType);

        if (scmQueryPtr != 0)
            {
            scmQueryPtr = new ScalableMeshReprojectionQuery(scmQueryPtr,
                                                       sourceGCS,
                                                       targetGCS,
                                                       extentInTargetGCS);
            }
        }

    return scmQueryPtr;
    }

/*----------------------------------------------------------------------------+
|ScalableMeshQuery Method Definition Section - Begin
+----------------------------------------------------------------------------*/
ScalableMeshQuery::ScalableMeshQuery()
    {
    }

ScalableMeshQuery::~ScalableMeshQuery()
    {
    }

/*----------------------------------------------------------------------------+
|ScalableMeshQuery::CreateClipShape
| Protected... Updates the internal class m_clipShapePtr to reflect the result
| of companding all clips set in the m_clips member (added using AddClips())
| The order of the content of the clips is important and is processed in the order
| they were set.
| The given range is used for two purposes. First it indicates the limit of the
| DTM and the clip shapes are clipped upon this extent. Secondly, it serves as
| the base shape in the event only mask type clips are defined.
+----------------------------------------------------------------------------*/
HFCPtr<HVEShape> ScalableMeshQuery::CreateClipShape(DRange3d& spatialIndexRange) const
    {
    return CreateShapeFromClips(spatialIndexRange, m_clips);
    }

/*----------------------------------------------------------------------------+
|ScalableMeshQuery::CreateClipShape
| Protected... Updates the internal class m_clipShapePtr to reflect the result
| of companding all clips set in the m_clips member (added using AddClips())
| The order of the content of the clips is important and is processed in the order
| they were set.
| The given areaShape is used for two purposes. First it indicates the limit of the
| DTM and the clip shapes are clipped upon this areaShape. Secondly, it serves as
| the base shape in the event only mask type clips are defined.
+----------------------------------------------------------------------------*/
HFCPtr<HVEShape> ScalableMeshQuery::CreateClipShape(HFCPtr<HVEShape> areaShape) const
    {
    return CreateShapeFromClips(areaShape, m_clips);
    }

int ScalableMeshQuery::AddLinears(const DTMPtr&                      dtmPtr,
                           list<HFCPtr<HVEDTMLinearFeature>>& linearList,
                           size_t                             maxNumberOfPoints,
                           bool                               useDecimation)
    {

    assert((dtmPtr != 0) && (dtmPtr->GetBcDTM() != 0) && (dtmPtr->GetBcDTM()->GetTinHandle()));

    int status = SUCCESS;

    if (linearList.size() > 0)
        {
        BC_DTM_OBJ* dtmObjP(dtmPtr->GetBcDTM()->GetTinHandle());

        //Compute the number of points
        list<HFCPtr<HVEDTMLinearFeature> >::iterator linearIter    = linearList.begin();
        list<HFCPtr<HVEDTMLinearFeature> >::iterator linearIterEnd = linearList.end();

        size_t nbOfLinearPoints = 0;

        while (linearIter != linearIterEnd)
            {
            nbOfLinearPoints += (*linearIter)->GetSize();
            linearIter++;
            }

        int decimationStep;

        if (nbOfLinearPoints > maxNumberOfPoints)
            {
            if (!useDecimation)
                return S_NBPTSEXCEEDMAX;

            //Should result in a number of points roughly in the range of (maxNumberOfPoints * 0.75) - (maxNumberOfPoints * 1.5) points
            decimationStep = round((double)nbOfLinearPoints / maxNumberOfPoints);
            }
        else
            {
            decimationStep = 1;
            }

        linearIter    = linearList.begin();
        linearIterEnd = linearList.end();

        uint32_t       tileNumber = 0;
        HAutoPtr<DPoint3d> linePts;
        size_t        linePtsMaxSize = 0;

        int globalLinearPointInd = 0;

        while (linearIter != linearIterEnd)
            {
            if (linePtsMaxSize < (*linearIter)->GetSize())
                {
                linePtsMaxSize = (*linearIter)->GetSize();
                linePts = new DPoint3d[linePtsMaxSize];
                }

            long nbLinePts = 0;

            for (size_t indexPoints = 0 ; indexPoints < (*linearIter)->GetSize(); indexPoints++)
                {
                if (globalLinearPointInd % decimationStep == 0)
                    {
                    linePts[nbLinePts].x = (*linearIter)->GetPoint(indexPoints).GetX();
                    linePts[nbLinePts].y = (*linearIter)->GetPoint(indexPoints).GetY();
                    linePts[nbLinePts].z = (*linearIter)->GetPoint(indexPoints).GetZ();
                    nbLinePts++;
                    }

                globalLinearPointInd++;
                }

            if (nbLinePts > 0)
                {
                if (((*linearIter)->GetSize() == 1) || (nbLinePts > 1))
                    {
                    //Ensure that those features are closed if filtered, otherwise they won't work as expected.
                    if (nbLinePts < (long)(*linearIter)->GetSize())
                        {
                        switch ((*linearIter)->GetFeatureType())
                            {
                            case DTMFeatureType::Void :
                            case DTMFeatureType::BreakVoid :
                            case DTMFeatureType::DrapeVoid :
                            case DTMFeatureType::Hole :
                            case DTMFeatureType::Island :
                            case DTMFeatureType::Hull :
                            case DTMFeatureType::DrapeHull :
                            case DTMFeatureType::Polygon :
                            case DTMFeatureType::Region :
                                linePts[nbLinePts] = linePts[0];
                                nbLinePts++;
                            }
                        }

                    status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, (DTMFeatureType)(*linearIter)->GetFeatureType(), tileNumber, 1, &dtmObjP->nullFeatureId, linePts.get(), nbLinePts);
                    }
                else //Change the linear feature to a random spot.
                    {
                    status = bcdtmObject_storeDtmFeatureInDtmObject (dtmObjP, DTMFeatureType::RandomSpots, tileNumber, 1, &dtmObjP->nullFeatureId, linePts.get (), nbLinePts);
                    }
                }

            if (status != SUCCESS)
                {
                break;
                }

            linearIter++;
            tileNumber++;
            }
        }

    return status;
    }

                // If the segment is not NULL and if the segment intersect the query polyline or if is the segment is inside the query polyline, add the extremity points

int ScalableMeshQuery::_GetNbClip() const
    {
    return m_clips == 0 ? 0 : (int)m_clips->GetNbClips();
    }

int ScalableMeshQuery::_AddClip(DPoint3d* clipPointsP,
                         int       numberOfPoints,
                         bool      isClipMask)
    {
    assert(clipPointsP != 0 && numberOfPoints > 0);

    if (m_clips == 0)
        {
        m_clips = IScalableMeshClipContainer::Create();
        }

    IScalableMeshClipInfoPtr clipInfoPtr(IScalableMeshClipInfo::Create(clipPointsP, numberOfPoints, isClipMask));

    m_clips->AddClip(clipInfoPtr);

    return 0;
    }

int ScalableMeshQuery::_RemoveAllClip()
    {
    m_clips = 0;
    return SUCCESS;
    }

// Provide default implementation of the Query method
int ScalableMeshQuery::_Query(Bentley::TerrainModel::DTMPtr&   dtmPtr,
                       const DPoint3d*                  pClipShapePts,
                       int                              nbClipShapePts,
                       const IScalableMeshQueryParametersPtr&  scmQueryParamsPtr) const
{
    return 0;
}

/*----------------------------------------------------------------------------+
|ScalableMeshQuery Method Definition Section - End
+----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------+
|ScalableMeshReprojectionQuery Method Definition Section - Begin
+----------------------------------------------------------------------------*/
ScalableMeshReprojectionQuery::ScalableMeshReprojectionQuery(IScalableMeshQueryPtr         originalQueryPtr,
                                               const GCS& sourceGCS,
                                               const GCS& targetGCS,
                                               const DRange3d&        extentInTargetGCS)

: m_sourceGCS(sourceGCS),
  m_targetGCS(targetGCS),
  m_targetToSourceReproj(Reprojection::GetNull()),
  m_sourceToTargetReproj(Reprojection::GetNull()),
  m_extentInTargetGCS(extentInTargetGCS)
    {
    m_originalQueryPtr = originalQueryPtr;

    // We do not tolerate an original query will already set clipping or mask
    assert(m_originalQueryPtr->GetNbClip() == 0);


    static const ReprojectionFactory REPROJECTION_FACTORY;

    ReprojectionFactory::Status reprojCreateStatus;
    m_targetToSourceReproj = REPROJECTION_FACTORY.Create(m_targetGCS, m_sourceGCS, 0, reprojCreateStatus);
    assert(Reprojection::S_SUCCESS == reprojCreateStatus);

    m_sourceToTargetReproj = REPROJECTION_FACTORY.Create(m_sourceGCS, m_targetGCS, 0, reprojCreateStatus);
    assert(Reprojection::S_SUCCESS == reprojCreateStatus);
    }

ScalableMeshReprojectionQuery::~ScalableMeshReprojectionQuery()
    {
    }

/*----------------------------------------------------------------------------+
// @bsimethod                                           Mathieu.St-Pierre 08/11
+----------------------------------------------------------------------------*/
int IBcDTMReprojectionFunction(DPoint3d* pts, size_t numPoints, void* userP)
    {
    int status = BSISUCCESS;

    GeoCoords::Reprojection* reprojectionFunctionP = (GeoCoords::Reprojection*)userP;

    if (Reprojection::S_SUCCESS != reprojectionFunctionP->Reproject(pts, (int)numPoints, pts))
        {
        status = BSIERROR;
        }

    return status;
    }

/*----------------------------------------------------------------------------+
|ScalableMeshReprojectionQuery::_Query
| Performs a query (the wrapped query) then reprojects result from the source DTM
| geographic coordinate system to the effective DTM element geographic coordinate
| system.
+----------------------------------------------------------------------------*/
int ScalableMeshReprojectionQuery::_Query(Bentley::TerrainModel::DTMPtr&            dtmPtr,
                                   const DPoint3d*                  pQueryShapePts,
                                   int                              nbQueryShapePts,
                                   const IScalableMeshQueryParametersPtr&  scmQueryParamsPtr) const
    {
    int status;
    HArrayAutoPtr<DPoint3d> clipShapeInSourceGCS;
    size_t clipShapeInSourceGCSPtCount = 0;

    const bool fullExtentQuery = (0 == pQueryShapePts);

    if (!fullExtentQuery)
        {
//#define NO_INTERSECT_WITH_RESULTING_DOMAIN_SHAPE
#ifdef NO_INTERSECT_WITH_RESULTING_DOMAIN_SHAPE

        clipShapeInSourceGCSPtCount = nbQueryShapePts;
        clipShapeInSourceGCS = new DPoint3d[clipShapeInSourceGCSPtCount];

        Reprojection::Status reprojStatus = m_targetToSourceReproj.Reproject(pQueryShapePts, nbQueryShapePts, &clipShapeInSourceGCS[0]);
        assert(reprojStatus == Reprojection::S_SUCCESS);

#else //NO_INTERSECT_WITH_RESULTING_DOMAIN_SHAPE
        // Create a shape from clip points
        HFCPtr<HVE2DShape> pReprojectedShape = ReprojectShapeDomainLimited((BaseGCSPtr&)m_sourceGCS.GetGeoRef().GetBasePtr(),
                                                                           (BaseGCSPtr&)m_targetGCS.GetGeoRef().GetBasePtr(), pQueryShapePts, nbQueryShapePts);

        assert(NULL != pReprojectedShape); // TODO: User should probably be notified of that... See what can be done.

        // We extract the list of points from the simple shape
        HGF2DLocationCollection listOfPoints;
        if (NULL != pReprojectedShape)
            pReprojectedShape->Drop(&listOfPoints, 0.0);

        clipShapeInSourceGCSPtCount = listOfPoints.size();
        clipShapeInSourceGCS = new DPoint3d[clipShapeInSourceGCSPtCount];

        struct HGF2DLocationToDPoint3d : unary_function<HGF2DLocation, DPoint3d>
            {
            DPoint3d operator () (const HGF2DLocation& rhs) const
                {
                const DPoint3d myPoint = {rhs.GetX(), rhs.GetY(), 0.0};
                return myPoint;
                }
            };
        std::transform(listOfPoints.begin(), listOfPoints.end(), &clipShapeInSourceGCS[0], HGF2DLocationToDPoint3d());
#endif //NO_INTERSECT_WITH_RESULTING_DOMAIN_SHAPE
        }

    const bool triangulationState = scmQueryParamsPtr->GetTriangulationState();

    //MS It would be better to have some cloning function here and not modify mrDTMQueryParamsPtr.
    scmQueryParamsPtr->SetTriangulationState(false);

    scmQueryParamsPtr->SetGCS((BaseGCSPtr&)m_sourceGCS.GetGeoRef().GetBasePtr(),
        (BaseGCSPtr&)m_targetGCS.GetGeoRef().GetBasePtr());

    bool isQueryDone = false;

   if ((dtmPtr != 0) && (dtmPtr->GetBcDTM()->GetTinHandle()->numPoints > 0))
        {
        DTMPtr tempDTMPtr;

        status = m_originalQueryPtr->Query(tempDTMPtr, &clipShapeInSourceGCS[0], (int)clipShapeInSourceGCSPtCount, scmQueryParamsPtr);

        if (tempDTMPtr != 0)
            {
            tempDTMPtr->GetBcDTM()->SetMemoryAccess (DTMAccessMode::Write);

            // TODO: Remove cast. Due to bad interface.
            tempDTMPtr->GetBcDTM()->TransformUsingCallback (&IBcDTMReprojectionFunction, const_cast<Reprojection*>(&m_sourceToTargetReproj));

            assert(status == 0);

            //According to Rob ->append should be used when the two DTMs are not triangulated
            //and ->merge when the two DTMs are triangulated. For now it is assumed that the two DTMs are
            //not triangulated.
            assert((dtmPtr->GetBcDTM()->GetDTMState() != DTMState::Tin) &&
                (dtmPtr->GetBcDTM()->GetDTMState() != DTMState::TinError) &&
                (tempDTMPtr->GetBcDTM()->GetDTMState() != DTMState::Tin) &&
                (tempDTMPtr->GetBcDTM()->GetDTMState() != DTMState::TinError));

            dtmPtr->GetBcDTM()->Append(*tempDTMPtr->GetBcDTM());

            isQueryDone = true;
            }
        }
    else
        {
        status = m_originalQueryPtr->Query(dtmPtr, &clipShapeInSourceGCS[0], (int)clipShapeInSourceGCSPtCount, scmQueryParamsPtr);

        if (dtmPtr != 0)
            {
            dtmPtr->GetBcDTM()->SetMemoryAccess (DTMAccessMode::Write);

            // TODO: Remove cast. Due to bad interface.
            dtmPtr->GetBcDTM()->TransformUsingCallback (&IBcDTMReprojectionFunction, const_cast<Reprojection*>(&m_sourceToTargetReproj));

            assert(status == 0);

            isQueryDone = true;
            }
        }

    if ((dtmPtr != 0) && (isQueryDone == true) && (triangulationState == true))
        {
        // Since triangulation was requested, the process needs to be completed. This means that current query result
        // must be clipped according to current query object clips then triangulated.

        if ((m_clips != 0) && (m_clips->GetNbClips() > 0))
            {
            status = TriangulateDTM(dtmPtr, scmQueryParamsPtr);

            // Set clip shape to DTM
            SetClipsToDTM(dtmPtr, m_extentInTargetGCS, m_clips);
            }

        // Triangulate
        status = TriangulateDTM(dtmPtr, scmQueryParamsPtr);
        }

    scmQueryParamsPtr->SetTriangulationState(triangulationState);

    return status;
    }

/*----------------------------------------------------------------------------+
|ScalableMeshReprojectionQuery::_AddClip
| Performs required reprojection to the data source geographic coordinate system.
| and adds the result clip for the wrapped query
+----------------------------------------------------------------------------*/
int ScalableMeshReprojectionQuery::_AddClip(DPoint3d* clipPointsP,
                         int   numberOfPoints,
                         bool  isClipMask)
    {
    if(HRFGeoCoordinateProvider::GetServices() == NULL)
        return ERROR;

    // We first validate the clip shape being provided
    HFCPtr<HGF2DCoordSys>   coordSysPtr(new HGF2DCoordSys());       HArrayAutoPtr<double> tempBuffer(new double[numberOfPoints * 2]);
    int bufferInd = 0;
    for (int pointInd = 0; pointInd < numberOfPoints; pointInd++)
        {
        tempBuffer[bufferInd * 2] = clipPointsP[pointInd].x;
        tempBuffer[bufferInd * 2 + 1] = clipPointsP[pointInd].y;
        bufferInd++;
        }

    HVE2DPolySegment shapeContour(numberOfPoints * 2, tempBuffer, coordSysPtr);
    if (shapeContour.AutoCrosses() || shapeContour.IsAutoContiguous())
        return ERROR;

    // Add it in our own structure ... note that this structure is not used during the query but can be looked at
    // using the API.
    ScalableMeshQuery::_AddClip(clipPointsP, numberOfPoints, isClipMask);


    // At this point the clip is set for the reprojection query but not the original query ... we must first reproject the
    // clip shape then add it to the original query
    // In order to do this we create a polygon of segments using the points provided ...

    HVE2DPolygonOfSegments polygon(numberOfPoints * 2, tempBuffer, coordSysPtr);

    // Notice that at this point a dummy coordinate system was assigned to the shape ...

    // We do not want the clips to effectively be outside the DTM extent so we clip upon the extent given at construction
    HVE2DRectangle limitingShape(m_extentInTargetGCS.low.x, m_extentInTargetGCS.low.y,
                                 m_extentInTargetGCS.high.x, m_extentInTargetGCS.high.y, coordSysPtr);

    HFCPtr<HVE2DShape> limitedShape = polygon.IntersectShape(limitingShape);

    HFCPtr<HVEShape>        shapePtr;
    shapePtr = new HVEShape(*limitedShape);

    // We now create Image++ compatible Geographic Coordinate System objects ...
    assert(m_sourceGCS.HasGeoRef());
    assert(m_targetGCS.HasGeoRef());

    IRasterBaseGcsPtr pSource = HRFGeoCoordinateProvider::GetServices()->_CreateRasterBaseGcsFromBaseGcs(m_sourceGCS.GetGeoRef().GetBasePtr().get());
    IRasterBaseGcsPtr pTarget = HRFGeoCoordinateProvider::GetServices()->_CreateRasterBaseGcsFromBaseGcs(m_targetGCS.GetGeoRef().GetBasePtr().get());

    // We then create the Image++ compatible geographic transformation between these
    // Geographic coordinate systems...
    HFCPtr<HCPGCoordModel> pTransfo = new HCPGCoordModel(*pTarget,*pSource);

    // We create two dummies coordinate systems linked using this geographic transformation
    HFCPtr<HGF2DCoordSys> pSourceCS = new HGF2DCoordSys();
    HFCPtr<HGF2DCoordSys> pTargetCS = new HGF2DCoordSys(*pTransfo, pSourceCS);

    // And now we perform the reprojection proper. The purpose of using this Image++ architecture compliant
    // transformation allows automatic densification, needle removal and processing of berserking
    // transformations. What it does not provide however is error processing in case te transformation
    // requested is meaningless.
    shapePtr->SetCoordSys(pTargetCS);
    shapePtr->ChangeCoordSys(pSourceCS);

    // The result should be a simple shape
    const HVE2DShape* pInternalShape = shapePtr->GetShapePtr();
    if (pInternalShape != NULL)
        {
        assert(pInternalShape->IsSimple());

        const HVE2DSimpleShape* pMySimpleShape = dynamic_cast<const HVE2DSimpleShape*>(&*pInternalShape);

        if (pMySimpleShape != NULL)
            {
            HGF2DLocationCollection listOfPoints;

            // We extract the list of points from the simple shape
            pMySimpleShape->Drop(&listOfPoints, 0.0);
            HArrayAutoPtr<DPoint3d> reprojClipPoints(new DPoint3d[listOfPoints.size()]);
            for (size_t indexPoints = 0 ; indexPoints < listOfPoints.size() ; indexPoints++)
                {
                double X = listOfPoints[indexPoints].GetX();
                double Y = listOfPoints[indexPoints].GetY();

                DPoint3d myPoint;
                myPoint.x = X;
                myPoint.y = Y;

                reprojClipPoints[indexPoints] = myPoint;
                }

            if (listOfPoints.size() > 0)
               m_originalQueryPtr->AddClip (reprojClipPoints, (int)listOfPoints.size(), isClipMask);
            else
                {
                // If we get there then we have a simple shape that has no points. The only likely explaination is
                // that the original shape is folded onto itself to produce a voidshape
                // or that the result of reprojection resulted in a void shape due to extreme scaling factor.
                assert (dynamic_cast<const HVE2DVoidShape*>(&*pInternalShape) != NULL);
                }
            }
        else
            {
            // If we get there then we do not have a simple shape. The only likely explaination is
            // , however unlikely, that the shape folded as a result of berserking lines
            // and the result is a complex shape ... in this case we will ignore the result and it will not be added
            // as a clip shape.
            assert (dynamic_cast<const HVE2DComplexShape*>(&*pInternalShape) != NULL);
            }

        }

    return 0;
    }


/*----------------------------------------------------------------------------+
|ScalableMeshReprojectionQuery::ReprojectDTMObject
| Performs the reprojection of the DTM object. Since it quite possible that
| some of the points fall outside the domain of validtity of either
| coordinate system, we will simply abandon any points which result in a reprojection
| error. The list of triangulation points usually contains the very first level of the
| quadtree, regardless if the points are within the domain of validity. The other
| non-first-level points are usually validated for such validity. The purpose in
| doing so is that first level points that may be technically outside the domain of
| validity may result in a valid reprojection nevertheless and be useful in
| extending the display of the DTM a bit further. DTM cliping on coordinate system
| boundary can be awkward to the lay user and minimizing this effect will surely
| benifit all parties.

+----------------------------------------------------------------------------*/
#if 0
int ScalableMeshReprojectionQuery::ReprojectDTMObject(BC_DTM_OBJ*                   dtmObjP,
                                               const Reprojection&          reprojector)
    {
#if (1)
            int       status = BSISUCCESS;

    DPoint3d* p1P;
    DPoint3d  reprojectedPt;

    for (int ptInd = 0; ptInd < dtmObjP->numPoints; ptInd++)
        {
        p1P = (DPoint3d*)pointAddrP(dtmObjP, ptInd);

        status = reprojector.Reproject(*p1P, reprojectedPt);

        if (Reprojection::S_SUCCESS != reprojector.Reproject(*p1P, reprojectedPt))
            {
            status = BSIERROR;
            break;
            }

        *p1P = reprojectedPt;
        }

    return status;
#else

    int       status = BSISUCCESS;

    DPoint3d* p1P;
    DPoint3d  reprojectedPt;
    int ptIndOut = -1;

    if (dtmObjP->numPoints > 0)
        {
        for (int ptInd = 0; ptInd < dtmObjP->numPoints; ptInd++)
            {
            p1P = (DPoint3d*)pointAddrP(dtmObjP, ptInd);

            if (Reprojection::S_SUCCESS == reprojector.Reproject(*p1P, reprojectedPt))
                {
                ptIndOut++;
                p1P = (DPoint3d*)pointAddrP(dtmObjP, ptIndOut);
                *p1P = reprojectedPt;
                }
            }

        dtmObjP->numPoints = ptIndOut + 1;
        }

    return status;
#endif
    }
#endif

/*----------------------------------------------------------------------------+
|ScalableMeshReprojectionQuery Method Definition Section - End
+----------------------------------------------------------------------------*/

/*==================================================================*/
/*        QUERY SECTION - END                                       */
/*==================================================================*/


/*==================================================================*/
/*        3D MESH RELATED CODE - START                              */
/*==================================================================*/
IScalableMeshMeshPtr IScalableMeshMesh::Create(size_t nbPoints, DPoint3d* points, size_t nbFaceIndexes, int32_t* faceIndexes, size_t normalCount, DVec3d* pNormal, int32_t* pNormalIndex)
    {
    return new ScalableMeshMesh(nbPoints, points, nbFaceIndexes, faceIndexes, normalCount, pNormal, pNormalIndex);
    }

ScalableMeshMeshPtr ScalableMeshMesh::Create (DVec3d viewNormal)
    {
    return new ScalableMeshMesh(viewNormal);
    }

ScalableMeshMeshPtr ScalableMeshMesh::Create ()
    {
    return new ScalableMeshMesh (DVec3d::From(0,0,1));
    }

const PolyfaceQuery* IScalableMeshMesh::GetPolyfaceQuery () const
    {
    return _GetPolyfaceQuery();
    }

DPoint3d* IScalableMeshMesh::EditPoints()
    {
    return _EditPoints();
    }

size_t IScalableMeshMesh::GetNbPoints() const
    {
    return _GetNbPoints();
    }

int IScalableMeshMesh::ProjectPolyLineOnMesh(DPoint3d& endPt, bvector<bvector<DPoint3d>>& projectedPoints, const DPoint3d* points, int nPts, int* segment, const int* triangle, DPoint3d startPt, MTGNodeId& lastEdge) const
    {
    return _ProjectPolyLineOnMesh(endPt, projectedPoints, points, nPts, segment, triangle, startPt, lastEdge);
    }

int IScalableMeshMesh::ProjectPolyLineOnMesh(DPoint3d& endPt, bvector<bvector<DPoint3d>>& projectedPoints, const DPoint3d* points, int nPts, int* segment, const MTGNodeId triangleEdge, DPoint3d startPt) const
    {
    return _ProjectPolyLineOnMesh(endPt, projectedPoints, points, nPts, segment, triangleEdge, startPt);
    }

bool IScalableMeshMesh::FindTriangleForProjectedPoint(int* outTriangle, DPoint3d& point, bool use2d) const
    {
    return _FindTriangleForProjectedPoint(outTriangle, point,use2d);
    }

bool IScalableMeshMesh::FindTriangleForProjectedPoint(MTGNodeId& outTriangle, DPoint3d& point, bool use2d) const
    {
    return _FindTriangleForProjectedPoint(outTriangle, point, use2d);
    }

bool IScalableMeshMesh::FindTriangleAlongRay(int* outTriangle, DRay3d& ray, MTGNodeId edge) const
    {
    return _FindTriangleAlongRay(outTriangle, ray, edge);
    }

bool IScalableMeshMesh::FindTriangleAlongRay(MTGNodeId& outTriangle, DRay3d& ray) const
    {
    return _FindTriangleAlongRay(outTriangle, ray);
    }

bool IScalableMeshMesh::CutWithPlane(bvector<DSegment3d>& segmentList, DPlane3d& cuttingPlane) const
    {
    return _CutWithPlane(segmentList, cuttingPlane);
    }


void CheckFaceIndexes(size_t& nbFaceIndexes, int32_t* faceIndexes, size_t nbPoints)
    {
    // compress faceIndx and remove invalid faces.

    for (size_t i = 0; i < nbFaceIndexes; i += 3)
        {
        size_t p1 = faceIndexes[i] - 1;
        size_t p2 = faceIndexes[i + 1] - 1;
        size_t p3 = faceIndexes[i + 2] - 1;

        if (p1 < 0 || p1 >= nbPoints || p2 < 0 || p2 >= nbPoints || p3 < 0 || p3 >= nbPoints)
            {
            faceIndexes[i] = faceIndexes[nbFaceIndexes - 3];
            faceIndexes[i + 1] = faceIndexes[nbFaceIndexes - 2];
            faceIndexes[i + 2] = faceIndexes[nbFaceIndexes - 1];
            nbFaceIndexes -= 3;
            i -= 3;
            }
        }

    }

double checkNormal (DVec3dCR viewNormal, DVec3dCR normal) restrict (amp,cpu)
    {
    return (viewNormal.x*normal.x + viewNormal.y*normal.y + viewNormal.z*normal.z);
    }

void ScalableMeshMesh::CalcNormals () const
    {
    int triangleCount = (int)(m_nbFaceIndexes / 3);
    bvector<DVec3d> sTriangleNormals;
    sTriangleNormals.resize (triangleCount);
    DVec3d viewNormal = m_viewNormal;
#ifdef GPU
    concurrency::array_view <DVec3d, 1> triangleNormals (concurrency::extent<1> (triangleCount), sTriangleNormals.data());
    concurrency::array_view <DPoint3d, 1> points ((int)m_nbPoints, m_points);
    concurrency::array_view <int, 1> faceIndx((int)m_nbFaceIndexes, m_faceIndexes);
    concurrency::parallel_for_each (concurrency::extent<1> (triangleCount), [=](concurrency::index<1> idx) restrict (amp,cpu)
#else
    auto triangleNormals = sTriangleNormals.data ();
    auto points = m_points;
    auto faceIndx = m_faceIndexes;
    concurrency::combinable<size_t> _numberSwapped;
    concurrency::parallel_for (0, triangleCount, 1, [&](int aidx) restrict (cpu)
#endif
        {
#ifdef GPU
        int aidx = idx[0];
#endif
        int faceInx = aidx * 3;
        int p1 = faceIndx[faceInx] - 1;
        int p2 = faceIndx[faceInx + 1] - 1;
        int p3 = faceIndx[faceInx + 2] - 1;

        DVec3d normal;
        DPoint3d origin = points[p1];
        DPoint3d target1 = points[p2];
        DPoint3d target2 = points[p3];

        double x1 = target1.x - origin.x;
        double y1 = target1.y - origin.y;
        double z1 = target1.z - origin.z;

        double x2 = target2.x - origin.x;
        double y2 = target2.y - origin.y;
        double z2 = target2.z - origin.z;
        normal.x = y1 * z2 - z1 * y2;
        normal.y = z1 * x2 - x1 * z2;
        normal.z = x1 * y2 - y1 * x2;

        if (checkNormal (viewNormal, normal) >= 0)
            {
            _numberSwapped.local ()++;
            normal.x = -normal.x;
            normal.y = -normal.y;
            normal.z = -normal.z;
            faceIndx[faceInx] = p3 + 1;
            faceIndx[faceInx + 2] = p1 + 1;
            }
        triangleNormals[aidx] = normal;
        });
    size_t numberswapped = 0;

    _numberSwapped.combine_each ([&](size_t v)
        {
        numberswapped += v;
        });

#ifdef GPU
    triangleNormals.synchronize ();
    faceIndx.synchronize ();
#endif

    m_pNormalAuto = new DVec3d[m_nbPoints];

    for (size_t i = 0; i < m_nbPoints; i++)
        {
        DVec3d* norm = &m_pNormalAuto[i];
        norm->x = 0;
        norm->y = 0;
        norm->z = 0;
        }

    for (size_t i = 0, face = 0; i < m_nbFaceIndexes; face++, i += 3)
        {
        DVec3d* faceNorm = &triangleNormals[face];
        for (int j = 0; j < 3; j++)
            {
            long ptNum = m_faceIndexes[i + j] - 1;
            //if (ptNum >= 0 && ptNum < m_nbPoints)
                {
                DVec3d* norm = &m_pNormalAuto[ptNum];
                norm->x += faceNorm->x;
                norm->y += faceNorm->y;
                norm->z += faceNorm->z;
                }
            }
        }
    }

   
static bool s_dontCalculNormal = true;

const PolyfaceQuery* ScalableMeshMesh::_GetPolyfaceQuery() const
    {
    if (m_polyfaceQueryCarrier == 0)
        {
      /*  for (int i = 0; i < m_nbFaceIndexes; i++)
            {
            assert(m_faceIndexes[i] <= m_nbPoints);
            }*/

        if (nullptr != m_pNormalAuto)
            {
            delete [] m_pNormalAuto;
            m_pNormalAuto = nullptr;
            }
        if (nullptr == m_pNormal && !s_dontCalculNormal)
            {
            CalcNormals ();
            m_polyfaceQueryCarrier = new PolyfaceQueryCarrier (3, false/*twoSided*/, m_nbFaceIndexes, m_nbPoints, m_points, m_faceIndexes, m_nbPoints, m_pNormalAuto, m_faceIndexes);
            }
        else
            m_polyfaceQueryCarrier = new PolyfaceQueryCarrier(3, false/*twoSided*/, m_nbFaceIndexes, m_nbPoints, m_points, m_faceIndexes, m_normalCount, m_pNormal, m_pNormalIndex);
        }

    return m_polyfaceQueryCarrier;
    }

size_t ScalableMeshMesh::_GetNbPoints() const
    {
    return m_nbPoints;
    }

DPoint3d* ScalableMeshMesh::_EditPoints()
    {
    return m_points;
    }

inline bool isPointOnLineSegment(DPoint3d& p, DPoint3d& p1, DPoint3d& p2)
    {
    DVec3d p01 = DVec3d::FromStartEnd(p1, p);
    DVec3d p12 = DVec3d::FromStartEnd(p1, p2);
    DVec3d p02 = DVec3d::FromStartEnd(p, p2);
    DVec3d cross;
    cross.CrossProduct(p01, p12);
    return (cross.MagnitudeSquared() == 0 && p01.Magnitude() < p12.Magnitude() && p02.Magnitude() < p12.Magnitude());
    }

bool ScalableMeshMesh::_FindTriangleForProjectedPoint(int* outTriangle, DPoint3d& point, bool use2d) const
    {
    if (m_nbPoints < 3 || m_nbFaceIndexes < 3) return false;
    double maxParam = -DBL_MAX;
    for (size_t i = 0; i < m_nbFaceIndexes; i += 3)
        {
        DPoint3d pts[3];
        pts[0] = m_points[m_faceIndexes[i]-1];
        pts[1] = m_points[m_faceIndexes[i+1]-1];
        pts[2] = m_points[m_faceIndexes[i + 2]-1];
        DPoint3d bary,projectedPt;
        double param;
        if (use2d)
            {
            double minX = pts[0].x, minY = pts[0].y, maxX = pts[0].x, maxY = pts[0].y;
            minX = pts[1].x > pts[2].x ? (minX > pts[2].x ? pts[2].x : (minX > pts[1].x ? pts[1].x : minX)) : (minX > pts[1].x ? pts[1].x : (minX > pts[2].x ? pts[2].x : minX));
            minY = pts[1].y > pts[2].y ? (minY > pts[2].y ? pts[2].y : (minY > pts[1].y ? pts[1].y : minY)) : (minY > pts[1].y ? pts[1].y : (minY > pts[2].y ? pts[2].y : minY));
            maxX = pts[1].x < pts[2].x ? (maxX < pts[2].x ? pts[2].x : (maxX < pts[1].x ? pts[1].x : maxX)) : (maxX < pts[1].x ? pts[1].x : (maxX < pts[2].x ? pts[2].x : maxX));
            maxY = pts[1].y < pts[2].y ? (maxY < pts[2].y ? pts[2].y : (maxY < pts[1].y ? pts[1].y : maxY)) : (maxY < pts[1].y ? pts[1].y : (maxY < pts[2].y ? pts[2].y : maxY));
            if (minX > point.x || minY > point.y || maxX < point.x || maxY < point.y) continue;
            }
        DRay3d ray = DRay3d::FromOriginAndVector(point, DVec3d::From(0, 0, -1));
        if (bsiDRay3d_intersectTriangle(&ray, &projectedPt, &bary, &param, pts) && bary.x >= -1.0e-6f
            && bary.x <= 1.0&& bary.y >= -1.0e-6f && bary.y <= 1.0 && bary.z >= -1.0e-6f && bary.z <= 1.0 && param > maxParam)
            {
                outTriangle[0] = m_faceIndexes[i];
                outTriangle[1] = m_faceIndexes[i + 1];
                outTriangle[2] = m_faceIndexes[i + 2];
                maxParam = param;
            if (use2d) return true;
            }
        //if (maxParam > -DBL_MAX) return true;
        }
    if (maxParam > -DBL_MAX) return true;
    return false;
    }

ScalableMeshMesh::ScalableMeshMesh(size_t nbPoints, DPoint3d* points, size_t nbFaceIndexes, int32_t* faceIndexes, size_t normalCount, DVec3d* pNormal, int32_t* pNormalIndex)
    {
    m_nbPoints = nbPoints;
    m_points   = new DPoint3d[nbPoints];
    memcpy(m_points, points, sizeof(DPoint3d) * nbPoints);

  //  CheckFaceIndexes (nbFaceIndexes, faceIndexes, nbPoints);

    m_nbFaceIndexes = nbFaceIndexes;
    m_faceIndexes = new int32_t[nbFaceIndexes];
    memcpy(m_faceIndexes, faceIndexes, sizeof(int32_t) * nbFaceIndexes);
    m_pNormalAuto = nullptr;
    if (normalCount > 0)
        {
        m_normalCount = normalCount;
        m_pNormal = new DVec3d[normalCount];
        memcpy(m_pNormal, pNormal, sizeof(DVec3d) * normalCount);
        m_pNormalIndex = new int32_t[nbFaceIndexes];
        memcpy(m_pNormalIndex, pNormalIndex, sizeof(int32_t) * nbFaceIndexes);
        }
    else
        {
        m_normalCount = nbPoints;
        m_pNormal = new DVec3d[nbPoints];
        memcpy (m_pNormal, points, sizeof (DVec3d) * m_normalCount);
        m_pNormalIndex = new int32_t[nbFaceIndexes];
        memcpy(m_pNormalIndex, faceIndexes, sizeof(int32_t) * nbFaceIndexes);
        }

    m_polyfaceQueryCarrier = 0;
    }

ScalableMeshMesh::ScalableMeshMesh (DVec3d viewNormal)
    {
    m_viewNormal = viewNormal;
    m_nbPoints = 0;
    m_points = 0;
    m_nbFaceIndexes = 0;
    m_faceIndexes = 0;
    m_normalCount = 0;
    m_pNormal = 0;
    m_pNormalIndex = 0;
    m_polyfaceQueryCarrier = 0;
    m_pNormalAuto = nullptr;
    }

ScalableMeshMesh::~ScalableMeshMesh()
    {
    delete [] m_points;
    delete [] m_faceIndexes;
    delete [] m_pNormal;
    delete [] m_pNormalIndex;
    if (m_pNormalAuto == nullptr)
        delete[] m_pNormalAuto;
    if (m_polyfaceQueryCarrier != 0)
        {
        delete m_polyfaceQueryCarrier;
        }
    }

int ScalableMeshMesh::AppendMesh(size_t nbPoints, DPoint3d* points, size_t nbFaceIndexes, int32_t* faceIndexes, size_t normalCount, DVec3d* pNormal, int32_t* pNormalIndex)
    {
    //NEEDS_WORK_SM - Not Supported yet
    assert(normalCount == 0 && pNormal == 0 && pNormalIndex == 0);

    int status = ERROR;

    if (nbPoints > 0 && nbFaceIndexes > 0)
        {
        //CheckFaceIndexes (nbFaceIndexes, faceIndexes, nbPoints);

        DPoint3d* newPoints = new DPoint3d[nbPoints + m_nbPoints];
        memcpy(&newPoints[m_nbPoints], points, sizeof(DPoint3d) * nbPoints);

        if (m_nbPoints > 0)
            {
            memcpy(newPoints, m_points, sizeof(DPoint3d) * m_nbPoints);
            delete [] m_points;
            }

        Int32* newfaceIndexes = new int32_t[m_nbFaceIndexes + nbFaceIndexes];

        memcpy(&newfaceIndexes[m_nbFaceIndexes], faceIndexes, sizeof(int32_t) * nbFaceIndexes);

        if (m_nbPoints > 0)
            {
            for (size_t faceIndIter = m_nbFaceIndexes; faceIndIter < m_nbFaceIndexes + nbFaceIndexes; faceIndIter++)
                {
                newfaceIndexes[faceIndIter] += (int32_t)m_nbPoints;
                }
            }

        if (m_nbFaceIndexes > 0)
            {
            memcpy(newfaceIndexes, m_faceIndexes, sizeof(int32_t) * m_nbFaceIndexes);
            delete [] m_faceIndexes;
            }


        if (normalCount > 0)
            {
            DVec3d* newNormals = new DVec3d[normalCount + m_normalCount];
            for (size_t i = 0; i < normalCount; i++)
                pNormal[i] = DVec3d::From (1, 0, 0);
            memcpy (&newNormals[m_normalCount], pNormal, sizeof (DVec3d) * normalCount);

            if (m_normalCount > 0)
                {
                memcpy (newNormals, m_pNormal, sizeof (DVec3d) * m_normalCount);
                delete[] m_pNormal;
                }

            int32_t* newNormalIndex = new int32_t[nbFaceIndexes + m_nbFaceIndexes];

            if (m_normalCount > 0)
                {
                for (size_t newFaceIndIter = m_nbFaceIndexes, faceIndIter = 0; newFaceIndIter < m_nbFaceIndexes + nbFaceIndexes; newFaceIndIter++, faceIndIter++)
                    {
                    newNormalIndex[newFaceIndIter] = pNormalIndex[faceIndIter] + (int32_t)m_normalCount;
                    }
                }
            else
                memcpy(&newNormalIndex[m_nbFaceIndexes], pNormalIndex, sizeof(int32_t) * nbFaceIndexes);


            if (m_nbFaceIndexes > 0)
                {
                memcpy(newNormalIndex, m_pNormalIndex, sizeof(int32_t) * m_nbFaceIndexes);
                delete[] m_pNormalIndex;
                }
            m_normalCount = normalCount + m_normalCount;
            m_pNormal = newNormals;
            m_pNormalIndex = newNormalIndex;
            }
        else
            {
            bvector<DVec3d> calculatedNormals;
            calculatedNormals.resize (nbPoints);

            }
        m_points = newPoints;
        m_nbPoints = nbPoints + m_nbPoints;
        m_faceIndexes = newfaceIndexes;
        m_nbFaceIndexes = nbFaceIndexes + m_nbFaceIndexes;

        if (m_polyfaceQueryCarrier != 0)
            {
            delete m_polyfaceQueryCarrier;
            m_polyfaceQueryCarrier = 0;
            }

        status = SUCCESS;
        }

    return status;
    }

int ScalableMeshMesh::_ProjectPolyLineOnMesh(DPoint3d& endPt, bvector<bvector<DPoint3d>>& projectedPoints, const DPoint3d* points, int nPts, int* segment, const int* triangle, DPoint3d startPt, MTGNodeId& lastEdge) const
    {
    return ERROR;//not supported
    }

int ScalableMeshMesh::_ProjectPolyLineOnMesh(DPoint3d& endPt, bvector<bvector<DPoint3d>>& projectedPoints, const DPoint3d* points, int nPts, int* segment, const MTGNodeId triangleEdge, DPoint3d startPt) const
    {
    return ERROR;//not supported
    }

bool ScalableMeshMesh::_FindTriangleAlongRay(int* outTriangle, DRay3d& ray, MTGNodeId edge) const
    {
    return false;
    }

bool ScalableMeshMesh::_FindTriangleAlongRay(MTGNodeId& outTriangle, DRay3d& ray) const
    {
    return false;
    }

bool ScalableMeshMesh::_FindTriangleForProjectedPoint(MTGNodeId& outTriangle, DPoint3d& point, bool use2d) const
    {
    return false;
    }

bool ScalableMeshMesh::_CutWithPlane(bvector<DSegment3d>& segmentList, DPlane3d& cuttingPlane) const
    {
    if (m_nbPoints < 3 || m_nbFaceIndexes < 3) return false;
    for (size_t i = 0; i < m_nbFaceIndexes; i += 3)
        {
        DPoint3d pts[3];
        pts[0] = m_points[m_faceIndexes[i] - 1];
        pts[1] = m_points[m_faceIndexes[i + 1] - 1];
        pts[2] = m_points[m_faceIndexes[i + 2] - 1];

        double sign = 0;
        bool planeCutsTriangle = false;
        for (size_t j = 0; j < 3 && !planeCutsTriangle; j++)
            {
            double sideOfPoint = cuttingPlane.Evaluate(pts[j]);
            if (sign == 0) sign = sideOfPoint;
            else if ((sign > 0 && sideOfPoint < 0) || (sign < 0 && sideOfPoint > 0))
                planeCutsTriangle = true;
            }

        if (planeCutsTriangle)
            {
            DPoint3d intersectPts[2];
            size_t nOfIntersects = 0;
            //intersect segments to find cut
            for (size_t j = 0; j < 3 && nOfIntersects < 2; j++)
                {
                DSegment3d edgeSegment = DSegment3d::From(pts[j], pts[(j + 1) % 3]);
                double param;
                if (bsiDSegment3d_intersectDPlane3d(&edgeSegment, &intersectPts[nOfIntersects], &param, &cuttingPlane) && param > -1.0e-5 && param <= 1.0)
                    {
                    ++nOfIntersects;
                    }
                }
            DSegment3d intersectSegment = DSegment3d::From(intersectPts[0], intersectPts[1]);
            segmentList.push_back(intersectSegment);
            }
        
        }
    return true;
    }

int ScalableMeshMeshWithGraph::_ProjectPolyLineOnMesh(DPoint3d& endPt, bvector<bvector<DPoint3d>>& projectedPoints, const DPoint3d* points, int nPts, int* segment, const MTGNodeId triangleEdge, DPoint3d startPt) const
    {
    if (triangleEdge == -1) return ERROR;
    MTGNodeId edge = triangleEdge;
    if (!FollowPolylineOnGraph(m_graphData, endPt, projectedPoints, m_points, edge, segment, points, nPts, startPt, m_nbPoints, m_is3d)) return ERROR;
    return SUCCESS;
    }

int ScalableMeshMeshWithGraph::_ProjectPolyLineOnMesh(DPoint3d& endPt, bvector<bvector<DPoint3d>>& projectedPoints, const DPoint3d* points, int nPts, int* segment, const int* triangle, DPoint3d startPt, MTGNodeId& lastEdge) const
    {
    MTGNodeId triangleStartEdge = FindFaceInGraph(m_graphData, triangle[0], triangle[1], triangle[2]);
    if (triangleStartEdge == -1) return ERROR;
    if (!FollowPolylineOnGraph(m_graphData, endPt, projectedPoints, m_points, triangleStartEdge, segment, points, nPts, startPt, m_nbPoints, m_is3d)) return ERROR;
    lastEdge = triangleStartEdge;
    return SUCCESS;
    }

bool ScalableMeshMeshWithGraph::_FindTriangleAlongRay(int* outTriangle, DRay3d& ray, MTGNodeId edge) const
    {
    MTGNodeId triangleStartEdge = edge;
    DPoint3d pt;
    bvector<bvector<DPoint3d>> vec;
    if (!FindNextTriangleOnRay(triangleStartEdge, pt, NULL, m_graphData, ray, NULL, m_points, vec, NULL, m_nbPoints, m_is3d)) return false;
    GetFaceDefinition(m_graphData, outTriangle, triangleStartEdge);
    return true;
    }

bool ScalableMeshMeshWithGraph::_FindTriangleAlongRay(MTGNodeId& outTriangle, DRay3d& ray) const
    {
    MTGNodeId triangleStartEdge = -1;
    DPoint3d pt;
    bvector<bvector<DPoint3d>> vec;
    if (!FindNextTriangleOnRay(triangleStartEdge, pt, NULL, m_graphData, ray, NULL, m_points, vec, NULL, m_nbPoints)) return false;
    outTriangle = triangleStartEdge;
    return true;
    }

bool ScalableMeshMeshWithGraph::_FindTriangleForProjectedPoint(MTGNodeId& outTriangle, DPoint3d& point, bool use2d) const
    {
    double maxParam = -DBL_MAX;
    MTGMask visitedMask = m_graphData->GrabMask();
    MTGARRAY_SET_LOOP(edgeId, m_graphData)
        {
        if (m_graphData->GetMaskAt(edgeId, MTG_EXTERIOR_MASK)) continue;
        if (m_graphData->GetMaskAt(edgeId, visitedMask)) continue;
        m_graphData->SetMaskAt(edgeId, visitedMask);
        DPoint3d pts[3];
        int labels[3] = { -1, -1, -1 };
        m_graphData->TryGetLabel(edgeId, 0, labels[0]);
        m_graphData->TryGetLabel(m_graphData->FSucc(edgeId), 0, labels[1]);
        m_graphData->TryGetLabel(m_graphData->FSucc(m_graphData->FSucc(edgeId)), 0, labels[2]);
        m_graphData->SetMaskAt(m_graphData->FSucc(edgeId), visitedMask);
        m_graphData->SetMaskAt(m_graphData->FSucc(m_graphData->FSucc(edgeId)), visitedMask);
        if (labels[0] < 1 || labels[1] < 1 || labels[2] < 1) continue;
        pts[0] = m_points[labels[0] - 1];
        pts[1] = m_points[labels[1] - 1];
        pts[2] = m_points[labels[2] - 1];
        DPoint3d bary, projectedPt;
        double param;
        if (use2d)
            {
            double minX = pts[0].x, minY = pts[0].y, maxX = pts[0].x, maxY = pts[0].y;
            minX = pts[1].x > pts[2].x ? (minX > pts[2].x ? pts[2].x : (minX > pts[1].x ? pts[1].x : minX)) : (minX > pts[1].x ? pts[1].x : (minX > pts[2].x ? pts[2].x : minX));
            minY = pts[1].y > pts[2].y ? (minY > pts[2].y ? pts[2].y : (minY > pts[1].y ? pts[1].y : minY)) : (minY > pts[1].y ? pts[1].y : (minY > pts[2].y ? pts[2].y : minY));
            maxX = pts[1].x < pts[2].x ? (maxX < pts[2].x ? pts[2].x : (maxX < pts[1].x ? pts[1].x : maxX)) : (maxX < pts[1].x ? pts[1].x : (maxX < pts[2].x ? pts[2].x : maxX));
            maxY = pts[1].y < pts[2].y ? (maxY < pts[2].y ? pts[2].y : (maxY < pts[1].y ? pts[1].y : maxY)) : (maxY < pts[1].y ? pts[1].y : (maxY < pts[2].y ? pts[2].y : maxY));
            if (minX > point.x || minY > point.y || maxX < point.x || maxX < point.y) continue;
            }
        DRay3d ray = DRay3d::FromOriginAndVector(point, DVec3d::From(0, 0, -1));
        if (bsiDRay3d_intersectTriangle(&ray, &projectedPt, &bary, &param, pts) && bary.x >= -1.0e-6f
            && bary.x <= 1.0&& bary.y >= -1.0e-6f && bary.y <= 1.0 && bary.z >= -1.0e-6f && bary.z <= 1.0 && param > maxParam)
            {
            outTriangle = edgeId;
            maxParam = param;
            if (use2d)
                {
                m_graphData->ClearMask(visitedMask);
                m_graphData->DropMask(visitedMask);
                return true;
                }
            }
        //if (maxParam > -DBL_MAX) return true;
        }
    MTGARRAY_END_SET_LOOP(edgeId, m_graphData)
        m_graphData->ClearMask(visitedMask);
    m_graphData->DropMask(visitedMask);
    if (maxParam > -DBL_MAX) return true;
    return false;
    }

ScalableMeshMeshWithGraph::ScalableMeshMeshWithGraph(size_t nbPoints, DPoint3d* points, size_t nbFaceIndexes, int32_t* faceIndexes, size_t normalCount, DVec3d* pNormal, int32_t* pNormalIndex, MTGGraph* pGraph, bool is3d)
    : ScalableMeshMesh(nbPoints, points, nbFaceIndexes, faceIndexes, normalCount, pNormal, pNormalIndex)
    {
    m_graphData = pGraph;
    m_is3d = is3d;
    }

ScalableMeshMeshWithGraph::ScalableMeshMeshWithGraph(MTGGraph* pGraph, bool is3d)
 : ScalableMeshMesh(DVec3d::From(0, 0, 1))
 {
    m_graphData = pGraph;
    m_is3d = is3d;
 }

ScalableMeshMeshWithGraph::~ScalableMeshMeshWithGraph()
    {
    }

ScalableMeshMeshWithGraphPtr ScalableMeshMeshWithGraph::Create(size_t nbPoints, DPoint3d* points, size_t nbFaceIndexes, int32_t* faceIndexes, size_t normalCount, DVec3d* pNormal, int32_t* pNormalIndex, MTGGraph* pGraph, bool is3d)
    {
    return new ScalableMeshMeshWithGraph(nbPoints, points, nbFaceIndexes, faceIndexes, normalCount, pNormal, pNormalIndex, pGraph, is3d);
    }

ScalableMeshMeshWithGraphPtr ScalableMeshMeshWithGraph::Create(MTGGraph* pGraph, bool is3d)
 {
    return new ScalableMeshMeshWithGraph(pGraph, is3d);
 }


IScalableMeshMeshQueryParams::IScalableMeshMeshQueryParams()
    {}

IScalableMeshMeshQueryParams::~IScalableMeshMeshQueryParams()
    {}

Bentley::GeoCoordinates::BaseGCSPtr IScalableMeshMeshQueryParams::GetSourceGCS()
    {
    return _GetSourceGCS();
    }

Bentley::GeoCoordinates::BaseGCSPtr IScalableMeshMeshQueryParams::GetTargetGCS()
    {
    return _GetTargetGCS();
    }

void IScalableMeshMeshQueryParams::SetGCS(Bentley::GeoCoordinates::BaseGCSPtr& sourceGCSPtr,
                                   Bentley::GeoCoordinates::BaseGCSPtr& targetGCSPtr)
    {
    _SetGCS(sourceGCSPtr, targetGCSPtr);
    }

IScalableMeshViewDependentMeshQueryParams::IScalableMeshViewDependentMeshQueryParams()
    {
    }

IScalableMeshViewDependentMeshQueryParams::~IScalableMeshViewDependentMeshQueryParams()
    {
    }

const DPoint3d* IScalableMeshViewDependentMeshQueryParams::GetViewBox() const
    {
    return _GetViewBox();
    }

void IScalableMeshViewDependentMeshQueryParams::SetViewBox(const DPoint3d viewBox[])
    {
    _SetViewBox(viewBox);
    }

double IScalableMeshViewDependentMeshQueryParams::GetMinScreenPixelsPerPoint() const
    {
    return _GetMinScreenPixelsPerPoint();
    }

bool IScalableMeshViewDependentMeshQueryParams::GetUseSameResolutionWhenCameraIsOff() const
    {
    return _GetUseSameResolutionWhenCameraIsOff();
    }

bool IScalableMeshViewDependentMeshQueryParams::GetUseSplitThresholdForLevelSelection() const
    {
    return _GetUseSplitThresholdForLevelSelection();
    }

bool IScalableMeshViewDependentMeshQueryParams::GetUseSplitThresholdForTileSelection() const
    {
    return _GetUseSplitThresholdForTileSelection();
    }

const double* IScalableMeshViewDependentMeshQueryParams::GetRootToViewMatrix() const
    {
    return _GetRootToViewMatrix();
    }

const ClipVectorPtr IScalableMeshViewDependentMeshQueryParams::GetViewClipVector() const
    {
    return _GetViewClipVector();
    }

void IScalableMeshViewDependentMeshQueryParams::SetMinScreenPixelsPerPoint(double minScreenPixelsPerPoint)
    {
    _SetMinScreenPixelsPerPoint(minScreenPixelsPerPoint);
    }

void IScalableMeshViewDependentMeshQueryParams::SetRootToViewMatrix(const double rootToViewMatrix[][4])
    {
    _SetRootToViewMatrix(rootToViewMatrix);
    }

void IScalableMeshViewDependentMeshQueryParams::SetUseSameResolutionWhenCameraIsOff(bool useSameResolution)
    {
    _SetUseSameResolutionWhenCameraIsOff(useSameResolution);
    }

void IScalableMeshViewDependentMeshQueryParams::SetUseSplitThresholdForLevelSelection(bool useSplitThreshold)
    {
    _SetUseSplitThresholdForLevelSelection(useSplitThreshold);
    }

void IScalableMeshViewDependentMeshQueryParams::SetUseSplitThresholdForTileSelection(bool useSplitThreshold)
    {
    _SetUseSplitThresholdForTileSelection(useSplitThreshold);
    }

void IScalableMeshViewDependentMeshQueryParams::SetViewClipVector(ClipVectorPtr& viewClipVector)
    {
    _SetViewClipVector(viewClipVector);
    }

//NEEDS_WORK_SM : Maybe should be activate only on a specialized define
bool IScalableMeshViewDependentMeshQueryParams::GetGatherQueriedNodeBoundaries() const
    {
    return _GetGatherQueriedNodeBoundaries();
    }

void IScalableMeshViewDependentMeshQueryParams::SetGatherQueriedNodeBoundaries(bool gatherNodeBoundaries)
    {
    _SetGatherQueriedNodeBoundaries(gatherNodeBoundaries);
    }

bool IScalableMeshViewDependentMeshQueryParams::GetQueriedNodeBoundaries(bvector<DSegment3d>& nodeBoundaries) const
    {
    return _GetQueriedNodeBoundaries(nodeBoundaries);
    }

void IScalableMeshViewDependentMeshQueryParams::SetQueriedNodeBoundaries(bvector<DSegment3d>& nodeBoundaries)
    {
    _SetQueriedNodeBoundaries(nodeBoundaries);
    }

//NEEDS_WORK_SM : Maybe should be activate only on a specialized define END

IScalableMeshViewDependentMeshQueryParamsPtr IScalableMeshViewDependentMeshQueryParams::CreateParams()
    {
    return new ScalableMeshViewDependentMeshQueryParams();
    }

IScalableMeshMeshQueryParamsPtr IScalableMeshMeshQueryParams::CreateParams()
    {
    return new ScalableMeshMeshQueryParams();
    }

//IScalableMeshMeshQuery
int IScalableMeshMeshQuery::Query(IScalableMeshMeshPtr&                               meshPtr,
                           const DPoint3d*                              pQueryExtentPts,
                           int                                          nbQueryExtentPts,
                           const IScalableMeshMeshQueryParamsPtr& scmQueryParamsPtr) const
    {
    return _Query(meshPtr, pQueryExtentPts, nbQueryExtentPts, scmQueryParamsPtr);
    }

int IScalableMeshMeshQuery::Query(bvector<IScalableMeshNodePtr>&                      meshNodes, 
                           const DPoint3d*                              pQueryExtentPts,
                           int                                          nbQueryExtentPts,
                           const IScalableMeshMeshQueryParamsPtr& scmQueryParamsPtr) const
    {
    return _Query(meshNodes, pQueryExtentPts, nbQueryExtentPts, scmQueryParamsPtr);
    }


static bool s_passNormal = false;

int draw(DTMFeatureType dtmFeatureType,int numTriangles, int numMeshPts,DPoint3d *meshPtsP,DPoint3d *meshVectorsP,int numMeshFaces, long *meshFacesP,void *userP)
    {
    IScalableMeshMeshPtr* meshPtr = (IScalableMeshMeshPtr*)userP;
    /*
    uint32_t numPerFace = 3;
    bool   twoSided = false;
    size_t indexCount = numMeshFaces;
    size_t pointCount = numMeshPts;
    DPoint3dCP pPoint = meshPtsP;
    Int32 const* pPointIndex = (Int32*)meshFacesP;
    size_t normalCount = numMeshPts;
    DVec3dCP  pNormal = (DVec3dCP)meshVectorsP;
    Int32 const* pNormalIndex = (Int32*)meshFacesP;
    */
    if (s_passNormal)
        {
        *meshPtr = IScalableMeshMesh::Create(numMeshPts, meshPtsP, numMeshFaces, (int32_t*)meshFacesP, numMeshPts, (DVec3d*)meshVectorsP, (int32_t*)meshFacesP);
        }
    else
        {
        *meshPtr = IScalableMeshMesh::Create(numMeshPts, meshPtsP, numMeshFaces, (int32_t*)meshFacesP, 0, 0, 0);
        }

    return SUCCESS;
    }

const double ScalableMeshNodeRayQueryParams::INFINITE_DEPTH = -1.0;
const double ScalableMeshNodePlaneQueryParams::INFINITE_DEPTH = -1.0;

IScalableMeshNodeQueryParamsPtr IScalableMeshNodeQueryParams::CreateParams()
    {
    return IScalableMeshNodeQueryParamsPtr(new ScalableMeshNodeRayQueryParams());
    }

IScalableMeshNodePlaneQueryParamsPtr IScalableMeshNodePlaneQueryParams::CreateParams()
    {
    return IScalableMeshNodePlaneQueryParamsPtr(new ScalableMeshNodePlaneQueryParams());
    }

bool IScalableMeshNode::ArePoints3d() const
    {
    return _ArePoints3d();
    }

bool IScalableMeshNode::ArePointsFullResolution() const
    {
    return _ArePointsFullResolution();
    }

IScalableMeshMeshPtr IScalableMeshNode::GetMesh(bool loadGraph) const
    {
    return _GetMesh(loadGraph);
    }

//Gets neighbors by relative position. For example, neighbor (-1, 0, 0) shares the node's left face. (1,1,0) shares the node's top-right diagonal. 
bvector<IScalableMeshNodePtr>  IScalableMeshNode::GetNeighborAt( char relativePosX, char relativePosY, char relativePosZ) const
    {
    return _GetNeighborAt(relativePosX, relativePosY, relativePosZ);
    }

DRange3d  IScalableMeshNode::GetNodeExtent() const
    {
    return _GetNodeExtent();
    }

DRange3d  IScalableMeshNode::GetContentExtent() const
    {
    return _GetContentExtent();
    }

__int64 IScalableMeshNode::GetNodeId() const
    {
    return _GetNodeId();
    }

size_t IScalableMeshNode::GetLevel() const
    {
    return _GetLevel();
    }

size_t  IScalableMeshNode::GetPointCount() const
    {
    return _GetPointCount();
    }

bool IScalableMeshNode::IsHeaderLoaded() const 
    {
    return _IsHeaderLoaded();
    }

bool IScalableMeshNode::IsMeshLoaded() const 
    {
    return _IsMeshLoaded();
    }

void IScalableMeshNode::LoadHeader() const 
    {
    return _LoadHeader();
    }

int IScalableMeshNodeRayQuery::Query(IScalableMeshNodePtr&                               nodePtr,
                              const DPoint3d*                           pTestPt,
                              const DPoint3d*                              pClipShapePts,
                              int                                          nbClipShapePts,
                              const IScalableMeshNodeQueryParamsPtr& scmQueryParamsPtr) const
    {
    return _Query(nodePtr, pTestPt, pClipShapePts, nbClipShapePts, scmQueryParamsPtr);
    }



StatusInt  IScalableMeshNodeEdit::AddMesh(DPoint3d* vertices, size_t nVertices, int32_t* indices, size_t nIndices)
    {
    return _AddMesh(vertices, nVertices, indices, nIndices);
    }

StatusInt  IScalableMeshNodeEdit::SetNodeExtent(DRange3d& extent)
    {
    return _SetNodeExtent(extent);
    }

StatusInt  IScalableMeshNodeEdit::SetContentExtent(DRange3d& extent)
    {
    return _SetContentExtent(extent);
    }


/*==================================================================*/
/*        3D MESH RELATED CODE - END                                */
/*==================================================================*/




END_BENTLEY_SCALABLEMESH_NAMESPACE
