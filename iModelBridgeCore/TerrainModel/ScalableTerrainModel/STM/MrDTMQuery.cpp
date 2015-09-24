/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/STM/MrDTMQuery.cpp $
|    $RCSfile: MrDTMQuery.cpp,v $
|   $Revision: 1.41 $
|       $Date: 2012/11/29 17:30:37 $
|     $Author: Mathieu.St-Pierre $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/     
  
#include <ScalableTerrainModelPCH.h>

USING_NAMESPACE_IMAGEPP;

extern bool   GET_HIGHEST_RES;

/*------------------------------------------------------------------+
| Include of the current class header                               |
+------------------------------------------------------------------*/

#include "MrDTM.h"
#include "MrDTMQuadTreeBCLIBFilters.h"
#include "MrDTMQuadTreeQueries.h"
#include "MrDTMQuery.h"
#include "InternalUtilityFunctions.h"

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

USING_NAMESPACE_BENTLEY_MRDTM_IMPORT
USING_NAMESPACE_BENTLEY_MRDTM_GEOCOORDINATES

BEGIN_BENTLEY_MRDTM_NAMESPACE

#define P2P_TOLERANCE 0.0000001

struct ToBcPtConverter
    {
    DPoint3d operator () (const IDTMFile::Point3d64fM64f& inputPt) const
        {
        DPoint3d outPt = {inputPt.x, inputPt.y, inputPt.z};
        // DPoint3d outPt = {inputPt.x, inputPt.y, 0};
        return outPt;
        }

    DPoint3d operator () (const IDTMFile::Point3d64f& inputPt) const
        {
        DPoint3d outPt = {inputPt.x, inputPt.y, inputPt.z};
        // DPoint3d outPt = {inputPt.x, inputPt.y, 0};
        return outPt;
        }

    DPoint3d operator () (const HGF3DCoord<double>& inputPt) const
        {
        DPoint3d outPt = {inputPt.GetX(), inputPt.GetY(), inputPt.GetZ()};
        //  DPoint3d outPt = {inputPt.GetX(), inputPt.GetY(), 0};
        return outPt;
        }
    };

   
/*==================================================================*/
/*        QUERY PARAMETERS SECTION - BEGIN                          */
/*==================================================================*/

/*==================================================================*/
/*                   IMrDTMQueryParameters                          */
/*==================================================================*/
IMrDTMQueryParameters::IMrDTMQueryParameters() 
    {
    }                            
    
IMrDTMQueryParameters::~IMrDTMQueryParameters() 
    {
    }

bool IMrDTMQueryParameters::GetTriangulationState()
    {
    return _GetTriangulationState();
    }

void IMrDTMQueryParameters::SetTriangulationState(bool pi_isReturnedDataTriangulated)
    {
    _SetTriangulationState(pi_isReturnedDataTriangulated);
    }

Bentley::GeoCoordinates::BaseGCSPtr IMrDTMQueryParameters::GetSourceGCS()
    {
    return _GetSourceGCS();
    }

Bentley::GeoCoordinates::BaseGCSPtr IMrDTMQueryParameters::GetTargetGCS()
    {
    return _GetTargetGCS();
    }

void IMrDTMQueryParameters::SetGCS(Bentley::GeoCoordinates::BaseGCSPtr& sourceGCSPtr, 
                                   Bentley::GeoCoordinates::BaseGCSPtr& targetGCSPtr)
    {
    _SetGCS(sourceGCSPtr, targetGCSPtr);
    }
        
long IMrDTMQueryParameters::GetEdgeOptionTriangulationParam()
    {
    return _GetEdgeOptionTriangulationParam();
    }
    
double IMrDTMQueryParameters::GetMaxSideLengthTriangulationParam()
    {
    return _GetMaxSideLengthTriangulationParam();
    }
    
void IMrDTMQueryParameters::SetEdgeOptionTriangulationParam(long edgeOption)
    {
    _SetEdgeOptionTriangulationParam(edgeOption);
    }
    
void IMrDTMQueryParameters::SetMaxSideLengthTriangulationParam(double maxSideLength)
    {
    _SetMaxSideLengthTriangulationParam(maxSideLength);
    }
            
double IMrDTMQueryParameters::GetToleranceTriangulationParam()
    {
    return P2P_TOLERANCE;
    }
   
  
/*==================================================================*/  
/*                   IMrDTMFullResolutionQueryParams                */
/*==================================================================*/
IMrDTMFullResolutionQueryParams::IMrDTMFullResolutionQueryParams() 
    {
    }

IMrDTMFullResolutionQueryParams::~IMrDTMFullResolutionQueryParams() 
    {
    }

IMrDTMFullResolutionQueryParamsPtr IMrDTMFullResolutionQueryParams::CreateParams()
    {
    return new MrDTMFullResolutionQueryParams();
    }
   
size_t IMrDTMFullResolutionQueryParams::GetMaximumNumberOfPoints()
    {
    return _GetMaximumNumberOfPoints();            
    }

void IMrDTMFullResolutionQueryParams::SetMaximumNumberOfPoints(size_t maximumNumberOfPoints)
    {
    _SetMaximumNumberOfPoints(maximumNumberOfPoints);
    }

bool IMrDTMFullResolutionQueryParams::GetReturnAllPtsForLowestLevel()
    {
    return _GetReturnAllPtsForLowestLevel();
    }
  
void IMrDTMFullResolutionQueryParams::SetReturnAllPtsForLowestLevel(bool returnAllPts)
    {
    _SetReturnAllPtsForLowestLevel(returnAllPts);
    }

/*==================================================================*/
/*                   IMrDTMFullResolutionLinearQueryParams          */
/*==================================================================*/   
IMrDTMFullResolutionLinearQueryParams::IMrDTMFullResolutionLinearQueryParams() 
    {
    }
        
IMrDTMFullResolutionLinearQueryParams::~IMrDTMFullResolutionLinearQueryParams() 
    {
    }

IMrDTMFullResolutionLinearQueryParamsPtr IMrDTMFullResolutionLinearQueryParams::CreateParams()
    {
    return new MrDTMFullResolutionLinearQueryParams();
    }

size_t IMrDTMFullResolutionLinearQueryParams::GetMaximumNumberOfPointsForLinear()
    {
    return _GetMaximumNumberOfPointsForLinear();
    }

int IMrDTMFullResolutionLinearQueryParams::SetMaximumNumberOfPointsForLinear(size_t maximumNumberOfPointsForLinear)
    {
    return _SetMaximumNumberOfPointsForLinear(maximumNumberOfPointsForLinear);
    }

void IMrDTMFullResolutionLinearQueryParams::SetUseDecimation(bool useDecimation)
    {
    _SetUseDecimation(useDecimation);
    }
   
bool IMrDTMFullResolutionLinearQueryParams::GetUseDecimation()
    {
    return _GetUseDecimation();
    }

void IMrDTMFullResolutionLinearQueryParams::SetCutLinears(bool cutLinears)
    {
    _SetCutLinears(cutLinears);
    }

bool IMrDTMFullResolutionLinearQueryParams::GetCutLinears()
    {
    return _GetCutLinears();
    }

void IMrDTMFullResolutionLinearQueryParams::SetAddLinears(const bool addLinears)
    {
    _SetAddLinears(addLinears);
    }

bool IMrDTMFullResolutionLinearQueryParams::GetAddLinears()
    {
    return _GetAddLinears();
    }

const std::vector<int>& IMrDTMFullResolutionLinearQueryParams::GetFilteringFeatureTypes(bool& doIncludeFilteringFeatureTypes)
    {
    return _GetFilteringFeatureTypes(doIncludeFilteringFeatureTypes);
    }

//When no feature type is specified all feature types are returned.
int IMrDTMFullResolutionLinearQueryParams::SetFilteringFeatureTypes(const std::vector<int>& filteringFeatureTypes, bool doIncludeFilteringFeatures)
    {
    return _SetFilteringFeatureTypes(filteringFeatureTypes, doIncludeFilteringFeatures);
    }

void IMrDTMFullResolutionLinearQueryParams::SetIncludeFilteringFeatureTypes(const bool& doIncludeFilteringFeatures)
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
/*                   IMrDTMViewDependentQueryParams                 */
/*==================================================================*/   
IMrDTMViewDependentQueryParams::IMrDTMViewDependentQueryParams() 
    {
    }

IMrDTMViewDependentQueryParams::~IMrDTMViewDependentQueryParams() 
    {
    }

double IMrDTMViewDependentQueryParams::GetMinScreenPixelsPerPoint() const
    {
    return _GetMinScreenPixelsPerPoint();
    }

bool IMrDTMViewDependentQueryParams::GetUseSameResolutionWhenCameraIsOff() const
    {
    return _GetUseSameResolutionWhenCameraIsOff();
    }

bool IMrDTMViewDependentQueryParams::GetUseSplitThresholdForLevelSelection() const
    {
    return _GetUseSplitThresholdForLevelSelection();
    }

bool IMrDTMViewDependentQueryParams::GetUseSplitThresholdForTileSelection() const
    {
    return _GetUseSplitThresholdForTileSelection();
    }

const double* IMrDTMViewDependentQueryParams::GetRootToViewMatrix() const
    {
    return _GetRootToViewMatrix();
    }                           
                        
void IMrDTMViewDependentQueryParams::SetMinScreenPixelsPerPoint(double minScreenPixelsPerPoint)
    {
    _SetMinScreenPixelsPerPoint(minScreenPixelsPerPoint);
    }

void IMrDTMViewDependentQueryParams::SetRootToViewMatrix(const double rootToViewMatrix[][4])
    {
    _SetRootToViewMatrix(rootToViewMatrix);
    }

void IMrDTMViewDependentQueryParams::SetUseSameResolutionWhenCameraIsOff(bool useSameResolution)
    {
    _SetUseSameResolutionWhenCameraIsOff(useSameResolution);
    }

void IMrDTMViewDependentQueryParams::SetUseSplitThresholdForLevelSelection(bool useSplitThreshold)
    {
    _SetUseSplitThresholdForLevelSelection(useSplitThreshold);
    }

void IMrDTMViewDependentQueryParams::SetUseSplitThresholdForTileSelection(bool useSplitThreshold)
    {
    _SetUseSplitThresholdForTileSelection(useSplitThreshold);
    }

IMrDTMViewDependentQueryParamsPtr IMrDTMViewDependentQueryParams::CreateParams()
    {
    return new MrDTMViewDependentQueryParams();
    }

/*==================================================================*/
/*                   IMrDTMFixResolutionIndexQueryParams            */
/*==================================================================*/   
IMrDTMFixResolutionIndexQueryParams::IMrDTMFixResolutionIndexQueryParams() 
    {
    }

IMrDTMFixResolutionIndexQueryParams::~IMrDTMFixResolutionIndexQueryParams() 
    {
    }

int IMrDTMFixResolutionIndexQueryParams::GetResolutionIndex() const
    {
    return _GetResolutionIndex();
    }
                            
void IMrDTMFixResolutionIndexQueryParams::SetResolutionIndex(int resolutionIndex)
    {
    _SetResolutionIndex(resolutionIndex);
    }

IMrDTMFixResolutionIndexQueryParamsPtr IMrDTMFixResolutionIndexQueryParams::CreateParams()
    {
    return new MrDTMFixResolutionIndexQueryParams();
    }

/*==================================================================*/
/*                   IMrDTMFixResolutionMaxPointsQueryParams        */
/*==================================================================*/   
IMrDTMFixResolutionMaxPointsQueryParams::IMrDTMFixResolutionMaxPointsQueryParams() 
    {
    }

IMrDTMFixResolutionMaxPointsQueryParams::~IMrDTMFixResolutionMaxPointsQueryParams() 
    {
    }

__int64 IMrDTMFixResolutionMaxPointsQueryParams::GetMaxNumberPoints()
    {
    return _GetMaxNumberPoints();
    }

void IMrDTMFixResolutionMaxPointsQueryParams::SetMaximumNumberPoints(__int64 maxNumberPoints)
    {
    _SetMaximumNumberPoints(maxNumberPoints);
    }

IMrDTMFixResolutionMaxPointsQueryParamsPtr IMrDTMFixResolutionMaxPointsQueryParams::CreateParams()
    {
    return new MrDTMFixResolutionMaxPointsQueryParams();
    }
    
/*==================================================================*/
/*                   IMrDTMQueryAllLinearsQueryParams               */
/*==================================================================*/  
IMrDTMQueryAllLinearsQueryParams::IMrDTMQueryAllLinearsQueryParams() 
    {
    }

IMrDTMQueryAllLinearsQueryParams::~IMrDTMQueryAllLinearsQueryParams() 
    {
    }

void IMrDTMQueryAllLinearsQueryParams::SetFeatures (const list<HFCPtr<HVEDTMLinearFeature>>& features)
    {
    _SetFeatures(features);
    }
        
list<IMrDTMFeaturePtr> IMrDTMQueryAllLinearsQueryParams::GetFeatures()
    {
    return _GetFeatures();
    }

IMrDTMQueryAllLinearsQueryParamsPtr IMrDTMQueryAllLinearsQueryParams::CreateParams()
    {
    return new MrDTMQueryAllLinearsQueryParams();
    }
    
/*==================================================================*/
/*        QUERY PARAMETERS SECTION - END                            */
/*==================================================================*/

/*==================================================================*/
/*        QUERY SECTION - BEGIN                                     */
/*==================================================================*/

/*----------------------------------------------------------------------------+
|IMrDTMQuery::Query
+----------------------------------------------------------------------------*/
int IMrDTMQuery::Query(BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr&            dtmPtr,
                       const DPoint3d*                  pQueryShapePts,
                       int                              nbQueryShapePts,
                       const IMrDTMQueryParametersPtr&  mrDTMQueryParamsPtr) const
    {
    return _Query(dtmPtr, pQueryShapePts, nbQueryShapePts, mrDTMQueryParamsPtr);
    }

/*----------------------------------------------------------------------------+
|IMrDTMQuery::GetNbClip
+----------------------------------------------------------------------------*/
int IMrDTMQuery::GetNbClip() const
    {
    return _GetNbClip();
    }

/*----------------------------------------------------------------------------+
|IMrDTMQuery::GetClip
+----------------------------------------------------------------------------*/
/*
int IMrDTMQuery::GetClip(DPoint3d*& clipPointsP,
                         int&       numberOfPoints,
                         bool&      isClipMask,
                         int        clipInd) const
    {
    return _GetClip(clipPointsP, numberOfPoints, isClipMask, clipInd);
    }
*/
/*----------------------------------------------------------------------------+
|IMrDTMQuery::AddClip
+----------------------------------------------------------------------------*/
int IMrDTMQuery::AddClip(DPoint3d* clipPointsP,
                             int   numberOfPoints,
                             bool  isClipMask)
    {
    return _AddClip(clipPointsP, numberOfPoints, isClipMask);
    }

/*----------------------------------------------------------------------------+
|IMrDTMQuery::RemoveAllClip
+----------------------------------------------------------------------------*/
int IMrDTMQuery::RemoveAllClip()
    {
    return _RemoveAllClip();
    }

/*----------------------------------------------------------------------------+
|IMrDTMQuery::GetReprojectionQueryInterface
+----------------------------------------------------------------------------*/
IMrDTMQueryPtr MrDTMQuery::GetReprojectionQueryInterface(IMrDTMPtr        mrDtmToQueryPtr,
                                                         DTMQueryType     queryType,
                                                         DTMQueryDataType queryDataType,
                                                         const GCS&         sourceGCS,
                                                         const GCS&         targetGCS,
                                                         const DRange3d&  extentInTargetGCS)
    {
    IMrDTMQueryPtr mrDtmQueryPtr;



    if ((sourceGCS.IsNull() == false) || (targetGCS.IsNull() == false))
        {
        mrDtmQueryPtr = mrDtmToQueryPtr->GetQueryInterface(queryType, queryDataType);

        if (mrDtmQueryPtr != 0)
            {
            mrDtmQueryPtr = new MrDTMReprojectionQuery(mrDtmQueryPtr,
                                                       sourceGCS,
                                                       targetGCS,
                                                       extentInTargetGCS);
            }
        }

    return mrDtmQueryPtr;
    }

/*----------------------------------------------------------------------------+
|MrDTMQuery Method Definition Section - Begin
+----------------------------------------------------------------------------*/
MrDTMQuery::MrDTMQuery()
    {    
    }

MrDTMQuery::~MrDTMQuery()
    {
    }

/*----------------------------------------------------------------------------+
|MrDTMQuery::CreateClipShape
| Protected... Updates the internal class m_clipShapePtr to reflect the result
| of companding all clips set in the m_clips member (added using AddClips())
| The order of the content of the clips is important and is processed in the order
| they were set.
| The given range is used for two purposes. First it indicates the limit of the
| DTM and the clip shapes are clipped upon this extent. Secondly, it serves as
| the base shape in the event only mask type clips are defined.
+----------------------------------------------------------------------------*/
HFCPtr<HVEShape> MrDTMQuery::CreateClipShape(DRange3d& spatialIndexRange) const
    {
    return CreateShapeFromClips(spatialIndexRange, m_clips);
    }

/*----------------------------------------------------------------------------+
|MrDTMQuery::CreateClipShape
| Protected... Updates the internal class m_clipShapePtr to reflect the result
| of companding all clips set in the m_clips member (added using AddClips())
| The order of the content of the clips is important and is processed in the order
| they were set.
| The given areaShape is used for two purposes. First it indicates the limit of the
| DTM and the clip shapes are clipped upon this areaShape. Secondly, it serves as
| the base shape in the event only mask type clips are defined.
+----------------------------------------------------------------------------*/
HFCPtr<HVEShape> MrDTMQuery::CreateClipShape(HFCPtr<HVEShape> areaShape) const
    {
    return CreateShapeFromClips(areaShape, m_clips);
    }

int MrDTMQuery::AddLinears(const DTMPtr&                      dtmPtr,
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
                                                                               
int MrDTMQuery::_GetNbClip() const
    {
    return m_clips == 0 ? 0 : (int)m_clips->GetNbClips();
    }

int MrDTMQuery::_AddClip(DPoint3d* clipPointsP,
                         int       numberOfPoints,
                         bool      isClipMask)
    {
    assert(clipPointsP != 0 && numberOfPoints > 0);

    if (m_clips == 0)
        {
        m_clips = IMrDTMClipContainer::Create();
        }

    IMrDTMClipInfoPtr clipInfoPtr(IMrDTMClipInfo::Create(clipPointsP, numberOfPoints, isClipMask));

    m_clips->AddClip(clipInfoPtr);
    
    return 0;
    }

int MrDTMQuery::_RemoveAllClip()
    {
    m_clips = 0;
    return SUCCESS;
    }
    
// Provide default implementation of the Query method
int MrDTMQuery::_Query(BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr&   dtmPtr, 
					   const DPoint3d*                  pClipShapePts, 
					   int                              nbClipShapePts, 
					   const IMrDTMQueryParametersPtr&  mrDTMQueryParamsPtr) const 
{
	return 0;
}

/*----------------------------------------------------------------------------+
|MrDTMQuery Method Definition Section - End
+----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------+
|MrDTMFullResolutionLinearQuery Method Definition Section - Begin
+----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------+
|MrDTMFullResolutionLinearQuery::MrDTMFullResolutionLinearQuery
+----------------------------------------------------------------------------*/
MrDTMFullResolutionLinearQuery::MrDTMFullResolutionLinearQuery(HFCPtr<HGFFeatureIndex<HFCPtr<HVEDTMLinearFeature>,
                                                                      HGF3DCoord<double>,
                                                                      YProtFeatureExtentType>> mrDTMLinearIndexPtr)
    {
    m_mrDTMLinearIndexPtr = mrDTMLinearIndexPtr;
    }

/*----------------------------------------------------------------------------+
|MrDTMFullResolutionLinearQuery::~MrDTMFullResolutionLinearQuery
+----------------------------------------------------------------------------*/
MrDTMFullResolutionLinearQuery::~MrDTMFullResolutionLinearQuery()
    {
    }

/*----------------------------------------------------------------------------+
|MrDTMFullResolutionLinearQuery::Query
+----------------------------------------------------------------------------*/
int MrDTMFullResolutionLinearQuery::_Query(BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr&            dtmPtr,
                                           const DPoint3d*                  pQueryShapePts,
                                           int                              nbQueryShapePts,
                                           const IMrDTMQueryParametersPtr&  mrDTMQueryParamsPtr) const
    {
    assert(dynamic_cast<IMrDTMFullResolutionLinearQueryParams*>(mrDTMQueryParamsPtr.get()) != 0);

    int                               status;
    list<HFCPtr<HVEDTMLinearFeature>> featureList;

    YProtFeatureExtentType contentExtent(m_mrDTMLinearIndexPtr->GetContentExtent());
    YProtFeatureExtentType queryExtent;

    if (pQueryShapePts != 0)
        {
        //MS It would be cleaner to have GetExtentFromClipShape templated but eventually there should be only one extent.
        queryExtent = GetExtentFromClipShape<YProtFeatureExtentType>(pQueryShapePts,
            nbQueryShapePts,
            ExtentOp<YProtFeatureExtentType>::GetZMin(contentExtent),
            ExtentOp<YProtFeatureExtentType>::GetZMax(contentExtent));
        }
    else
        {
        queryExtent.Set(ExtentOp<YProtFeatureExtentType>::GetXMin(contentExtent),
            ExtentOp<YProtFeatureExtentType>::GetYMin(contentExtent),
            ExtentOp<YProtFeatureExtentType>::GetZMin(contentExtent),
            ExtentOp<YProtFeatureExtentType>::GetXMax(contentExtent),
            ExtentOp<YProtFeatureExtentType>::GetYMax(contentExtent),
            ExtentOp<YProtFeatureExtentType>::GetZMax(contentExtent));
        }


    // If there are clips defined ... we should certainly intersect the clips extents with the query extent.
    if (GetNbClip() > 0)
        {
        DRange3d limitRange;
        limitRange.low.x = ExtentOp<YProtFeatureExtentType>::GetXMin(queryExtent);
        limitRange.low.y = ExtentOp<YProtFeatureExtentType>::GetYMin(queryExtent);
        limitRange.high.x = ExtentOp<YProtFeatureExtentType>::GetXMax(queryExtent);
        limitRange.high.y = ExtentOp<YProtFeatureExtentType>::GetYMax(queryExtent);

        HFCPtr<HVEShape> clipShapePtr = CreateClipShape(limitRange);

        if ((clipShapePtr != NULL) && (!clipShapePtr->IsEmpty()))
            {
            HFCPtr<HVEShape> queryExtentShapeP = new HVEShape(limitRange.low.x, limitRange.low.y, limitRange.high.x, limitRange.high.y, clipShapePtr->GetCoordSys());
            clipShapePtr->Intersect(*queryExtentShapeP);
            HGF2DExtent shapeExtent = clipShapePtr->GetExtent();
            queryExtent.SetXMin(shapeExtent.GetXMin());
            queryExtent.SetYMin(shapeExtent.GetYMin());
            queryExtent.SetXMax(shapeExtent.GetXMax());
            queryExtent.SetYMax(shapeExtent.GetYMax());
            }
        }

    try
        {
        m_mrDTMLinearIndexPtr->GetIn(queryExtent, featureList);

        status = SUCCESS;
        }
    catch (...)
        {
        status = ERROR;
        }

    if ((status == SUCCESS) && (dtmPtr == 0))
        {
        status = CreateBcDTM(dtmPtr);
        }

    if (status == SUCCESS)
        {        
        IMrDTMFullResolutionLinearQueryParams* queryParams(dynamic_cast<IMrDTMFullResolutionLinearQueryParams*>(mrDTMQueryParamsPtr.get()));

        bool doIncludeFilteringFeatureTypes;
            
        const std::vector<int>& filteringFeatureTypes = queryParams->GetFilteringFeatureTypes(doIncludeFilteringFeatureTypes);        

        //Filter the features based on their type
        if (filteringFeatureTypes.size() > 0)
            {            
            std::vector<int>::const_iterator foundFeatureType;
           
            list<HFCPtr<HVEDTMLinearFeature>>::iterator featureIter(featureList.begin());            
            
            while (featureIter != featureList.end())
                {
                foundFeatureType = std::find (filteringFeatureTypes.begin(), filteringFeatureTypes.end(), (int)(*featureIter)->GetFeatureType());

                if (foundFeatureType == filteringFeatureTypes.end() && doIncludeFilteringFeatureTypes)
                    {
                    featureIter = featureList.erase(featureIter);
                    }                
                else
                if (foundFeatureType != filteringFeatureTypes.end() && !doIncludeFilteringFeatureTypes)
                    {
                    featureIter = featureList.erase(featureIter);
                    }
                else
                    {
                    featureIter++;
                    }                                
                }                                                
            }        

        if (GetLinearsForPresentationModeCallback() == 0)
            {
            if (queryParams->GetCutLinears())
                {
                HFCPtr<HVE2DPolygonOfSegments> queryPolyLine;

                if (pQueryShapePts != 0)
                    {
                    int nPts(nbQueryShapePts);

                    // Allow extra space to make sure the first point is equal to the last point in the array
                    if (!(HDOUBLE_EQUAL_EPSILON(pQueryShapePts[0].x, pQueryShapePts[nbQueryShapePts-1].x) && HDOUBLE_EQUAL_EPSILON(pQueryShapePts[0].y, pQueryShapePts[nbQueryShapePts-1].y)))
                        nPts++;

                    HArrayAutoPtr<double> tmpPts (new double[nPts * 2]);

                    // Convert to DPoint2d
                    for(int i = 0; i < nbQueryShapePts; i ++)
                        {
                        tmpPts[i * 2]     = pQueryShapePts[i].x;
                        tmpPts[i * 2 + 1] = pQueryShapePts[i].y;
                        }

                    // Recopy last point to close the shape
                    if (nPts > nbQueryShapePts)
                        {
                        tmpPts[(nPts-1) * 2]     = pQueryShapePts[0].x;
                        tmpPts[(nPts-1) * 2 + 1] = pQueryShapePts[0].y;
                        }


                    HFCPtr<HGF2DCoordSys> pCoordSys(new HGF2DCoordSys());
                    queryPolyLine = new HVE2DPolygonOfSegments(nPts * 2, tmpPts, pCoordSys);
                    }
                else
                    {
                    int nPts(5);
                    HArrayAutoPtr<double> tmpPts (new double[nPts * 2]);

                    tmpPts[0] = contentExtent.GetXMin();
                    tmpPts[1] = contentExtent.GetYMin();
                    tmpPts[2] = contentExtent.GetXMin();
                    tmpPts[3] = contentExtent.GetYMax();
                    tmpPts[4] = contentExtent.GetXMax();
                    tmpPts[5] = contentExtent.GetYMax();
                    tmpPts[6] = contentExtent.GetXMax();
                    tmpPts[7] = contentExtent.GetYMin();
                    tmpPts[8] = tmpPts[0];
                    tmpPts[9] = tmpPts[1];

                    HFCPtr<HGF2DCoordSys> pCoordSys(new HGF2DCoordSys());
                    queryPolyLine = new HVE2DPolygonOfSegments(nPts * 2, tmpPts, pCoordSys);
                    }

                list<HFCPtr<HVEDTMLinearFeature>> cutFeatureList;

                CutLinears(featureList, cutFeatureList, queryPolyLine);
                status = AddLinears(dtmPtr, cutFeatureList, queryParams->GetMaximumNumberOfPointsForLinear(), queryParams->GetUseDecimation());
                }
            else if (queryParams->GetAddLinears())
                {                  
                status = AddLinears(dtmPtr, featureList, queryParams->GetMaximumNumberOfPointsForLinear(), queryParams->GetUseDecimation());
                }
            else
                {
                MrDTMQueryAllLinearsQueryParams* allLinearQueryParams(dynamic_cast<MrDTMQueryAllLinearsQueryParams*>(queryParams));

                allLinearQueryParams->SetFeatures(featureList);
                }
            }
        else
            {
            HArrayAutoPtr<HFCPtr<HVEDTMLinearFeature>> tempFeatureList(new HFCPtr<HVEDTMLinearFeature>[featureList.size()]);

            list<HFCPtr<HVEDTMLinearFeature>>::const_iterator featureListIter(featureList.begin());
            list<HFCPtr<HVEDTMLinearFeature>>::const_iterator featureListIterEnd(featureList.end());

            HFCPtr<HVEDTMLinearFeature>* tempFeatureListP = tempFeatureList.get();

            while (featureListIter != featureListIterEnd)
                {
                *tempFeatureListP = *featureListIter;

                tempFeatureListP++;
                featureListIter++;
                }

            status = GetLinearsForPresentationModeCallback()(dtmPtr, (DTMLinearFeature*)tempFeatureList.get(), (uint32_t)featureList.size(), queryParams->GetMaximumNumberOfPointsForLinear());
            }
        }

    if ((status == SUCCESS) &&
        ((mrDTMQueryParamsPtr == 0) || (mrDTMQueryParamsPtr->GetTriangulationState() == true)))
        {
        if ((m_clips != 0) && (m_clips->GetNbClips() > 0))
            {
            status = TriangulateDTM(dtmPtr, mrDTMQueryParamsPtr);

            DRange3d range;
            range.low.x = contentExtent.GetXMin();
            range.low.y = contentExtent.GetYMin();
            range.high.x = contentExtent.GetXMax();
            range.high.y = contentExtent.GetYMax();

            // Set clip shape to DTM
            SetClipsToDTM(dtmPtr, range, m_clips);
            }

        status = TriangulateDTM(dtmPtr, mrDTMQueryParamsPtr);
        }

    return status;
    }

/*----------------------------------------------------------------------------+
|MrDTMFullResolutionLinearQuery Method Definition Section - End
+----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------+
|MrDTMReprojectionQuery Method Definition Section - Begin
+----------------------------------------------------------------------------*/
MrDTMReprojectionQuery::MrDTMReprojectionQuery(IMrDTMQueryPtr         originalQueryPtr,
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

MrDTMReprojectionQuery::~MrDTMReprojectionQuery()
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
|MrDTMReprojectionQuery::_Query
| Performs a query (the wrapped query) then reprojects result from the source DTM
| geographic coordinate system to the effective DTM element geographic coordinate
| system.
+----------------------------------------------------------------------------*/
int MrDTMReprojectionQuery::_Query(BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr&            dtmPtr,
                                   const DPoint3d*                  pQueryShapePts,
                                   int                              nbQueryShapePts,
                                   const IMrDTMQueryParametersPtr&  mrDTMQueryParamsPtr) const
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

    const bool triangulationState = mrDTMQueryParamsPtr->GetTriangulationState();

    //MS It would be better to have some cloning function here and not modify mrDTMQueryParamsPtr.
    mrDTMQueryParamsPtr->SetTriangulationState(false);

    mrDTMQueryParamsPtr->SetGCS((BaseGCSPtr&)m_sourceGCS.GetGeoRef().GetBasePtr(),
        (BaseGCSPtr&)m_targetGCS.GetGeoRef().GetBasePtr());

    bool isQueryDone = false;

   if ((dtmPtr != 0) && (dtmPtr->GetBcDTM()->GetTinHandle()->numPoints > 0))
        {
        DTMPtr tempDTMPtr;

        status = m_originalQueryPtr->Query(tempDTMPtr, &clipShapeInSourceGCS[0], (int)clipShapeInSourceGCSPtCount, mrDTMQueryParamsPtr);

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
        status = m_originalQueryPtr->Query(dtmPtr, &clipShapeInSourceGCS[0], (int)clipShapeInSourceGCSPtCount, mrDTMQueryParamsPtr);

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
            status = TriangulateDTM(dtmPtr, mrDTMQueryParamsPtr);

            // Set clip shape to DTM
            SetClipsToDTM(dtmPtr, m_extentInTargetGCS, m_clips);
            }

        // Triangulate
        status = TriangulateDTM(dtmPtr, mrDTMQueryParamsPtr);    
        }

    mrDTMQueryParamsPtr->SetTriangulationState(triangulationState);

    return status;
    }

/*----------------------------------------------------------------------------+
|MrDTMReprojectionQuery::_AddClip
| Performs required reprojection to the data source geographic coordinate system.
| and adds the result clip for the wrapped query
+----------------------------------------------------------------------------*/
int MrDTMReprojectionQuery::_AddClip(DPoint3d* clipPointsP,
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
    MrDTMQuery::_AddClip(clipPointsP, numberOfPoints, isClipMask);


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
|MrDTMReprojectionQuery::ReprojectDTMObject
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
int MrDTMReprojectionQuery::ReprojectDTMObject(BC_DTM_OBJ*                   dtmObjP,
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
|MrDTMReprojectionQuery Method Definition Section - End
+----------------------------------------------------------------------------*/

/*==================================================================*/
/*        QUERY SECTION - END                                       */
/*==================================================================*/
END_BENTLEY_MRDTM_NAMESPACE