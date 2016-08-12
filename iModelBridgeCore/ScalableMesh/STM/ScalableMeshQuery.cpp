/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshQuery.cpp $
|    $RCSfile: ScalableMeshQuery.cpp,v $
|   $Revision: 1.41 $
|       $Date: 2012/11/29 17:30:37 $
|     $Author: Mathieu.St-Pierre $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <ScalableMeshPCH.h>
#include "ImagePPHeaders.h"
//#define GPU
#undef static_assert
#include <ppl.h>
#ifdef GPU
#include <amp.h>
#endif

// NEEDS_WORK_SM : add pragma to disable wnarning for templating
#pragma warning (disable: 4250)
#pragma warning (disable: 4589)


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
#include "ScalableMeshQuery.hpp"
#include "ScalableMesh\ScalableMeshGraph.h"
#include "DrapeOnGraph.h"


//NEEDS_WORK_SM : Is there a way to avoid this mutex?
std::mutex s_createdNodeMutex;

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

#define SM_TRACE_CLIPS_GETMESH 0

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

BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr IScalableMeshQueryParameters::GetSourceGCS()
    {
    return _GetSourceGCS();
    }

BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr IScalableMeshQueryParameters::GetTargetGCS()
    {
    return _GetTargetGCS();
    }

void IScalableMeshQueryParameters::SetGCS(BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& sourceGCSPtr,
                                   BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCSPtr)
    {
    _SetGCS(sourceGCSPtr, targetGCSPtr);
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
/*        QUERY PARAMETERS SECTION - END                            */
/*==================================================================*/

/*==================================================================*/
/*        QUERY SECTION - BEGIN                                     */
/*==================================================================*/

/*----------------------------------------------------------------------------+
|IScalableMeshPointQuery::Query
+----------------------------------------------------------------------------*/
int IScalableMeshPointQuery::Query(bvector<DPoint3d>&               points,
                                   const DPoint3d*                  pQueryShapePts,
                                   int                              nbQueryShapePts,
                                   const IScalableMeshQueryParametersPtr&  scmQueryParamsPtr) const
    {
    return _Query(points, pQueryShapePts, nbQueryShapePts, scmQueryParamsPtr);
    }

/*----------------------------------------------------------------------------+
|IScalableMeshPointQuery::GetNbClip
+----------------------------------------------------------------------------*/
int IScalableMeshPointQuery::GetNbClip() const
    {
    return _GetNbClip();
    }

/*----------------------------------------------------------------------------+
|IScalableMeshPointQuery::GetClip
+----------------------------------------------------------------------------*/
/*
int IScalableMeshPointQuery::GetClip(DPoint3d*& clipPointsP,
                         int&       numberOfPoints,
                         bool&      isClipMask,
                         int        clipInd) const
    {
    return _GetClip(clipPointsP, numberOfPoints, isClipMask, clipInd);
    }
*/
/*----------------------------------------------------------------------------+
|IScalableMeshPointQuery::AddClip
+----------------------------------------------------------------------------*/
int IScalableMeshPointQuery::AddClip(DPoint3d* clipPointsP,
                             int   numberOfPoints,
                             bool  isClipMask)
    {
    return _AddClip(clipPointsP, numberOfPoints, isClipMask);
    }

/*----------------------------------------------------------------------------+
|IScalableMeshPointQuery::RemoveAllClip
+----------------------------------------------------------------------------*/
int IScalableMeshPointQuery::RemoveAllClip()
    {
    return _RemoveAllClip();
    }

/*----------------------------------------------------------------------------+
|IScalableMeshPointQuery::GetReprojectionQueryInterface
+----------------------------------------------------------------------------*/
IScalableMeshPointQueryPtr ScalableMeshPointQuery::GetReprojectionQueryInterface(IScalableMeshPtr       scmToQueryPtr,
                                                                                 ScalableMeshQueryType  queryType,                                                         
                                                                                 const GCS&             sourceGCS,
                                                                                 const GCS&             targetGCS,
                                                                                 const DRange3d&        extentInTargetGCS)
    {
    IScalableMeshPointQueryPtr scmQueryPtr;

    if ((sourceGCS.IsNull() == false) || (targetGCS.IsNull() == false))
        {
        scmQueryPtr = scmToQueryPtr->GetQueryInterface(queryType);

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
|ScalableMeshPointQuery Method Definition Section - Begin
+----------------------------------------------------------------------------*/
ScalableMeshPointQuery::ScalableMeshPointQuery()
    {
    }

ScalableMeshPointQuery::~ScalableMeshPointQuery()
    {
    }

/*----------------------------------------------------------------------------+
|ScalableMeshPointQuery::CreateClipShape
| Protected... Updates the internal class m_clipShapePtr to reflect the result
| of companding all clips set in the m_clips member (added using AddClips())
| The order of the content of the clips is important and is processed in the order
| they were set.
| The given range is used for two purposes. First it indicates the limit of the
| DTM and the clip shapes are clipped upon this extent. Secondly, it serves as
| the base shape in the event only mask type clips are defined.
+----------------------------------------------------------------------------*/
HFCPtr<HVEShape> ScalableMeshPointQuery::CreateClipShape(DRange3d& spatialIndexRange) const
    {
    return CreateShapeFromClips(spatialIndexRange, m_clips);
    }

/*----------------------------------------------------------------------------+
|ScalableMeshPointQuery::CreateClipShape
| Protected... Updates the internal class m_clipShapePtr to reflect the result
| of companding all clips set in the m_clips member (added using AddClips())
| The order of the content of the clips is important and is processed in the order
| they were set.
| The given areaShape is used for two purposes. First it indicates the limit of the
| DTM and the clip shapes are clipped upon this areaShape. Secondly, it serves as
| the base shape in the event only mask type clips are defined.
+----------------------------------------------------------------------------*/
HFCPtr<HVEShape> ScalableMeshPointQuery::CreateClipShape(HFCPtr<HVEShape> areaShape) const
    {
    return CreateShapeFromClips(areaShape, m_clips);
    }

// If the segment is not NULL and if the segment intersect the query polyline or if is the segment is inside the query polyline, add the extremity points
int ScalableMeshPointQuery::_GetNbClip() const
    {
    return m_clips == 0 ? 0 : (int)m_clips->GetNbClips();
    }

int ScalableMeshPointQuery::_AddClip(DPoint3d* clipPointsP,
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

int ScalableMeshPointQuery::_RemoveAllClip()
    {
    m_clips = 0;
    return SUCCESS;
    }  

// Provide default implementation of the Query method
int ScalableMeshPointQuery::_Query(bvector<DPoint3d>&               points,
                                   const DPoint3d*                  pClipShapePts,
                                   int                              nbClipShapePts,
                                   const IScalableMeshQueryParametersPtr&  scmQueryParamsPtr) const
    {
    return 0;
    }

/*----------------------------------------------------------------------------+
|ScalableMeshPointQuery Method Definition Section - End
+----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------+
|ScalableMeshReprojectionQuery Method Definition Section - Begin
+----------------------------------------------------------------------------*/
ScalableMeshReprojectionQuery::ScalableMeshReprojectionQuery(IScalableMeshPointQueryPtr         originalQueryPtr,
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

    SMStatus reprojCreateStatus;
    m_targetToSourceReproj = REPROJECTION_FACTORY.Create(m_targetGCS, m_sourceGCS, 0, reprojCreateStatus);
    assert(SMStatus::S_SUCCESS == reprojCreateStatus);

    m_sourceToTargetReproj = REPROJECTION_FACTORY.Create(m_sourceGCS, m_targetGCS, 0, reprojCreateStatus);
    assert(SMStatus::S_SUCCESS == reprojCreateStatus);
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

    if (SMStatus::S_SUCCESS != reprojectionFunctionP->Reproject(pts, (int)numPoints, pts))
        {
        status = BSIERROR;
        }

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
    ScalableMeshPointQuery::_AddClip(clipPointsP, numberOfPoints, isClipMask);


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
IScalableMeshMeshPtr IScalableMeshMesh::Create(size_t nbPoints, DPoint3d* points, size_t nbFaceIndexes, const int32_t* faceIndexes, size_t normalCount, DVec3d* pNormal, int32_t* pNormalIndex, size_t uvCount, DVec2d* pUv, int32_t* pUvIndex)
    {
    return new ScalableMeshMesh(nbPoints, points, nbFaceIndexes, faceIndexes, normalCount, pNormal, pNormalIndex, uvCount, pUv, pUvIndex);
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

size_t IScalableMeshMesh::GetNbFaces() const
    {
    return _GetNbFaces();
    }

DTMStatusInt IScalableMeshMesh::GetAsBcDTM(BcDTMPtr& bcdtm)
    {
    return _GetAsBcDTM(bcdtm);
    }

DTMStatusInt IScalableMeshMesh::GetBoundary(bvector<DPoint3d>& pts)
    {
    return _GetBoundary(pts);
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

ScalableMeshTexturePtr ScalableMeshTexture::Create(RefCountedPtr<SMMemoryPoolBlobItem<Byte>>& pTextureData)
    {
    return new ScalableMeshTexture(pTextureData);
    }

const Byte* IScalableMeshTexture::GetData() const
    {
    return _GetData();
    }

Point2d IScalableMeshTexture::GetDimension() const
    {
    return _GetDimension();
    }

size_t IScalableMeshTexture::GetSize() const
    {
    return _GetSize();
    }

size_t IScalableMeshTexture::GetNOfChannels() const
    {
    return _GetSize() / (_GetDimension().x * _GetDimension().y);
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
            m_polyfaceQueryCarrier = new PolyfaceQueryCarrier (3, false/*twoSided*/, m_nbFaceIndexes, m_nbPoints, m_points, m_faceIndexes, m_nbPoints, m_pNormalAuto, m_faceIndexes, m_uvCount, m_pUv, m_pUvIndex);
            }
        else
            m_polyfaceQueryCarrier = new PolyfaceQueryCarrier(3, false/*twoSided*/, m_nbFaceIndexes, m_nbPoints, m_points, m_faceIndexes, m_normalCount, m_pNormal, m_pNormalIndex, m_uvCount, m_pUv, m_pUvIndex);
        }

//    m_polyfaceQueryCarrier = new PolyfaceQueryCarrier(3, false/*twoSided*/, m_nbFaceIndexes, m_nbPoints, m_points, m_faceIndexes, m_normalCount, m_pNormal, m_pNormalIndex, m_uvCount, m_pUv, m_pUvIndex);

    return m_polyfaceQueryCarrier;
    }

size_t ScalableMeshMesh::_GetNbPoints() const
    {
    return m_nbPoints;
    }

size_t ScalableMeshMesh::_GetNbFaces() const
    {
    return m_nbFaceIndexes/3;
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

bool PointProjectsToTriangle2d(DPoint3d& point, DPoint3d* triangle)
    {
    DPoint2d tri[3] = { DPoint2d::From(triangle[0].x, triangle[0].y), DPoint2d::From(triangle[1].x, triangle[1].y), DPoint2d::From(triangle[2].x, triangle[2].y) };
    DPoint2d ptA = DPoint2d::From(point.x, point.y);
    return bsiDPoint2d_isPointInConvexPolygon(&ptA, tri, 3, 1);
    }

bool ScalableMeshMesh::_FindTriangleForProjectedPoint(int* outTriangle, DPoint3d& point, bool use2d) const
    {
    if (m_nbPoints < 3 || m_nbFaceIndexes < 3) return false;
    volatile double maxParam = -DBL_MAX;
    volatile bool canContinue = true;
#pragma omp parallel for firstprivate(use2d)
    for (int i = 0; i < m_nbFaceIndexes; i += 3)
        {
        if (canContinue)
            {
            DPoint3d pts[3];
            pts[0] = m_points[m_faceIndexes[i] - 1];
            pts[1] = m_points[m_faceIndexes[i + 1] - 1];
            pts[2] = m_points[m_faceIndexes[i + 2] - 1];
            DPoint3d bary, projectedPt;
            double param=0;
            bool doIntersect = true;
            if (use2d)
                {
                double minX = pts[0].x, minY = pts[0].y, maxX = pts[0].x, maxY = pts[0].y;
                minX = pts[1].x > pts[2].x ? (minX > pts[2].x ? pts[2].x : (minX > pts[1].x ? pts[1].x : minX)) : (minX > pts[1].x ? pts[1].x : (minX > pts[2].x ? pts[2].x : minX));
                minY = pts[1].y > pts[2].y ? (minY > pts[2].y ? pts[2].y : (minY > pts[1].y ? pts[1].y : minY)) : (minY > pts[1].y ? pts[1].y : (minY > pts[2].y ? pts[2].y : minY));
                maxX = pts[1].x < pts[2].x ? (maxX < pts[2].x ? pts[2].x : (maxX < pts[1].x ? pts[1].x : maxX)) : (maxX < pts[1].x ? pts[1].x : (maxX < pts[2].x ? pts[2].x : maxX));
                maxY = pts[1].y < pts[2].y ? (maxY < pts[2].y ? pts[2].y : (maxY < pts[1].y ? pts[1].y : maxY)) : (maxY < pts[1].y ? pts[1].y : (maxY < pts[2].y ? pts[2].y : maxY));
                if (minX > point.x || minY > point.y || maxX < point.x || maxY < point.y) doIntersect = false;
                }
            if (doIntersect)
                {
                DRay3d ray = DRay3d::FromOriginAndVector(point, DVec3d::From(0, 0, -1));
                bool intersectTri = false;
                if (use2d)
                    {
                    intersectTri = PointProjectsToTriangle2d(point, pts);
                    }
                else
                    intersectTri = bsiDRay3d_intersectTriangle(&ray, &projectedPt, &bary, &param, pts) && bary.x >= -1.0e-6f
                    && bary.x <= 1.0&& bary.y >= -1.0e-6f && bary.y <= 1.0 && bary.z >= -1.0e-6f && bary.z <= 1.0 && param > maxParam;
                if (intersectTri)
                    {
                    outTriangle[0] = m_faceIndexes[i];
                    outTriangle[1] = m_faceIndexes[i + 1];
                    outTriangle[2] = m_faceIndexes[i + 2];
                    maxParam = param;
                    if (use2d) canContinue = false;
                    }
                }
            }
        //if (maxParam > -DBL_MAX) return true;
        }
    if (maxParam > -DBL_MAX) return true;
    return false;
    }

ScalableMeshMesh::ScalableMeshMesh(size_t nbPoints, DPoint3d* points, size_t nbFaceIndexes, const int32_t* faceIndexes, size_t normalCount, DVec3d* pNormal, int32_t* pNormalIndex, size_t uvCount, DVec2d* pUv, int32_t* pUvIndex)
    {
    m_nbPoints = nbPoints;
    m_points   = new DPoint3d[nbPoints];
    memcpy(m_points, points, sizeof(DPoint3d) * nbPoints);

  //  CheckFaceIndexes (nbFaceIndexes, faceIndexes, nbPoints);
    m_nbFaceIndexes = nbFaceIndexes;
    if (nbFaceIndexes > 0)
        {
        m_faceIndexes = new int32_t[nbFaceIndexes];
        memcpy(m_faceIndexes, faceIndexes, sizeof(int32_t) * nbFaceIndexes);
        }
    else
        {
        m_faceIndexes = nullptr;
        }
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

    if (uvCount > 0)
    {
        m_uvCount = uvCount;
        m_pUv = new DVec2d[uvCount];
        memcpy(m_pUv, pUv, sizeof(DVec2d) * uvCount);
        m_pUvIndex = new int32_t[nbFaceIndexes];
        memcpy(m_pUvIndex, pUvIndex, sizeof(int32_t) * nbFaceIndexes);
    }
    else
        {
        m_pUv = nullptr;
        m_pUvIndex = nullptr;
        m_uvCount = 0;
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
    m_pUv = 0;
    m_pUvIndex = 0;
    m_uvCount = 0;
    }

ScalableMeshMesh::~ScalableMeshMesh()
    {
    if(m_points != nullptr) delete [] m_points;
    if (m_faceIndexes != nullptr) delete[] m_faceIndexes;
    if (m_pNormal != nullptr) delete[] m_pNormal;
    if (m_pNormalIndex != nullptr) delete[] m_pNormalIndex;
    if (m_pUv != nullptr) delete[] m_pUv;
    if (m_pUvIndex != nullptr) delete[] m_pUvIndex;
    if (m_pNormalAuto != nullptr)
        delete[] m_pNormalAuto;
    if (m_polyfaceQueryCarrier != 0)
        {
        delete m_polyfaceQueryCarrier;
        }
    }

int ScalableMeshMesh::AppendMesh(size_t nbPoints, DPoint3d* points, size_t nbFaceIndexes, const int32_t* faceIndexes, size_t normalCount, DVec3d* pNormal, int32_t* pNormalIndex, size_t uvCount, const DPoint2d* pUv, const int32_t* pUvIndex)
    {
    //NEEDS_WORK_SM - Not Supported yet
    assert(normalCount == 0 && pNormal == 0 && pNormalIndex == 0);

    int status = ERROR;
    DPoint3d* newPoints = m_points;
    int32_t* newfaceIndexes = m_faceIndexes;
    if (nbPoints > 0)
        {
        //CheckFaceIndexes (nbFaceIndexes, faceIndexes, nbPoints);

        newPoints = new DPoint3d[nbPoints + m_nbPoints];
        memcpy(&newPoints[m_nbPoints], points, sizeof(DPoint3d) * nbPoints);

        if (m_nbPoints > 0)
            {
            memcpy(newPoints, m_points, sizeof(DPoint3d) * m_nbPoints);
            delete[] m_points;
            }
        }
    if (nbFaceIndexes > 0)
        {
        newfaceIndexes = new int32_t[m_nbFaceIndexes + nbFaceIndexes];

        memcpy(&newfaceIndexes[m_nbFaceIndexes], faceIndexes, sizeof(int32_t) * nbFaceIndexes);

        /*if (m_nbPoints > 0)
            {
            for (size_t faceIndIter = m_nbFaceIndexes; faceIndIter < m_nbFaceIndexes + nbFaceIndexes; faceIndIter++)
                {
                newfaceIndexes[faceIndIter] += (int32_t)m_nbPoints;
                }
            }*/

        if (m_nbFaceIndexes > 0)
            {
            memcpy(newfaceIndexes, m_faceIndexes, sizeof(int32_t) * m_nbFaceIndexes);
            delete[] m_faceIndexes;
            }

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

        if (uvCount > 0)
        {
            DVec2d* newUv = new DVec2d[uvCount + m_uvCount];
            /*for (size_t i = 0; i < uvCount; i++)
                pUv[i] = DVec2d::From(1, 0);*/
            memcpy(&newUv[m_uvCount], pUv, sizeof(DVec2d)*uvCount);

            if (m_uvCount > 0)
            {
                memcpy(newUv, m_pUv, sizeof(DVec2d)*m_uvCount);
                delete[] m_pUv;
            }


            int32_t* newUvIndex = new int32_t[m_nbFaceIndexes + nbFaceIndexes];

            memcpy(&newUvIndex[m_nbFaceIndexes], pUvIndex, sizeof(int32_t) * nbFaceIndexes);

            /*if (m_nbPoints > 0)
            {
                for (size_t faceIndIter = m_nbFaceIndexes; faceIndIter < m_nbFaceIndexes + nbFaceIndexes; faceIndIter++)
                {
                    newUvIndex[faceIndIter] += (int32_t)m_nbPoints;
                }
            }*/

            if (m_nbFaceIndexes > 0)
            {
                memcpy(newUvIndex, m_pUvIndex, sizeof(int32_t) * m_nbFaceIndexes);
                delete[] m_pUvIndex;
            }
            /*int32_t* newUvIndex = new int32_t[nbFaceIndexes + m_nbFaceIndexes];

            if (m_uvCount > 0)
            {
                for (size_t newFaceIndIter = m_nbFaceIndexes, faceIndIter = 0; newFaceIndIter < m_nbFaceIndexes + nbFaceIndexes; newFaceIndIter++, faceIndIter++)
                {
                    newUvIndex[newFaceIndIter] = pUvIndex[faceIndIter] + (int32_t)m_uvCount;// -1;
                }
            }
            else
            {
                for (int i = 0; i < nbFaceIndexes; i++)
                    newUvIndex[m_nbFaceIndexes + i] = pUvIndex[i] -1;
                //memcpy(&newUvIndex[m_nbFaceIndexes], pUvIndex, sizeof(int32_t)*nbFaceIndexes);
            }

            if (m_nbFaceIndexes > 0)
            {
                memcpy(newUvIndex, m_pUvIndex, sizeof(int32_t) * m_nbFaceIndexes);
                /*for (int i = 0; i < m_nbFaceIndexes; i++)
                    newUvIndex[i] = m_pUvIndex[0];// -1;*/
    /*            delete[] m_pUvIndex;
            }
            m_uvCount = uvCount + m_uvCount;
            m_pUv = newUv;
            m_pUvIndex = newUvIndex;*/
            m_pUv = newUv;
            m_pUvIndex = newUvIndex;
            
        }
//        delete[] m_pUvIndex;

        m_points = newPoints;
        m_nbPoints = nbPoints + m_nbPoints;
        m_faceIndexes = newfaceIndexes;
        m_nbFaceIndexes = nbFaceIndexes + m_nbFaceIndexes;
        // test UV
//        m_pUvIndex = newfaceIndexes;
        m_uvCount = m_uvCount + uvCount;

        if (m_polyfaceQueryCarrier != 0)
            {
            delete m_polyfaceQueryCarrier;
            m_polyfaceQueryCarrier = 0;
            }

        status = SUCCESS;

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

size_t s_nGetDTMs=0;
size_t s_nMissedDTMs=0;

int bcdtmObject_triangulateStmTrianglesDtmObjectOld
(
BC_DTM_OBJ *dtmP,//  Pointer To DTM Object
const DPoint3d* ptsList,
int numPts,
const int* indexList,
int numIndices
)
    {
    int ret = DTM_SUCCESS, dbg = DTM_TRACE_VALUE(0), cdbg = DTM_CHECK_VALUE(0), tdbg = DTM_TIME_VALUE(0);
    long n, /*dtmFeature, numTrgPts, stmPoints[4],*/ startTime = bcdtmClock();
    long index, mid, bottom, top;
    bvector<DPoint3d> originalPts, dtmPts;
    dtmP->ppTol = dtmP->plTol = 1e-8;
    bcdtmObject_storeDtmFeatureInDtmObject(dtmP, DTMFeatureType::RandomSpots, dtmP->nullUserTag, 1, &dtmP->nullFeatureId, ptsList, numPts);
    // Log Arguments
    if (dbg)
        {
        bcdtmWrite_message(0, 0, 0, "Triangulating STM Triangles");
        bcdtmWrite_message(0, 0, 0, "dtmP  = %p", dtmP);
        }

    // Check For Valid DTM Object
    if (bcdtmObject_testForValidDtmObject(dtmP)) goto errexit;

    // Check For DTM Data State
    if (dtmP->dtmState != DTMState::Data)
        {
        bcdtmWrite_message(1, 0, 0, "Method Requires Untriangulated DTM");
        goto errexit;
        }

 
    // Process For Triangulation
    if (dbg) bcdtmWrite_message(0, 0, 0, "Processing For Triangulation ** Number Of DTM Points = %8ld", dtmP->numPoints);
    if (bcdtmObject_processForTriangulationDtmObject(dtmP))goto errexit;
    if (dbg) bcdtmWrite_message(0, 0, 0, "Processing For Triangulation Completed ** Number Of DTM Points = %8ld", dtmP->numPoints);

    // Allocate Memory For Nodes Array
    if (bcdtmObject_allocateNodesMemoryDtmObject(dtmP)) goto errexit;

    // Initialise Circular List Parameters
    dtmP->cListPtr = 0;
    dtmP->cListDelPtr = dtmP->nullPtr;
    dtmP->numSortedPoints = dtmP->numPoints;

    //  Allocate Circular List Memory For Dtm Object
    if (dbg) bcdtmWrite_message(0, 0, 0, "Allocating Circular List Memory For Dtm Object");
    if (bcdtmObject_allocateCircularListMemoryDtmObject(dtmP)) goto errexit;


    for (int i = 0; i < dtmP->numPoints-1; ++i)
        {
        if (((*(dtmP)->pointsPP) + i + 1)->x == ((*(dtmP)->pointsPP) + i)->x) assert(((*(dtmP)->pointsPP) + i + 1)->y >= ((*(dtmP)->pointsPP) + i)->y);
        else assert(((*(dtmP)->pointsPP) + i + 1)->x > ((*(dtmP)->pointsPP) + i)->x);
        dtmPts.push_back(*((*(dtmP)->pointsPP) + i));
        }
    dtmPts.push_back(*((*(dtmP)->pointsPP) + dtmP->numPoints - 1));
    originalPts.resize(numPts);
    memcpy(&originalPts[0], ptsList, numPts*sizeof(DPoint3d));
    std::sort(originalPts.begin(), originalPts.end(), [] (const DPoint3d&a, const DPoint3d&b) { return b.x - a.x > 1e-8 || (fabs(a.x - b.x) < 1e-8  &&b.y - a.y > 1e-8); });
    auto newIt = std::unique(originalPts.begin(), originalPts.end(), [] (const DPoint3d&a, const DPoint3d&b) { return fabs(a.x - b.x) < 1e-8 && fabs(a.y - b.y) < 1e-8; });
    originalPts.resize(newIt - originalPts.begin());
    std::sort(dtmPts.begin(), dtmPts.end(), [] (const DPoint3d&a, const DPoint3d&b) { return b.x - a.x > 1e-8 || (fabs(a.x - b.x) < 1e-8  &&b.y - a.y > 1e-8); });
    newIt = std::unique(dtmPts.begin(), dtmPts.end(), [] (const DPoint3d&a, const DPoint3d&b) { return fabs(a.x - b.x) < 1e-8 && fabs(a.y - b.y) < 1e-8; });
    dtmPts.resize(newIt - dtmPts.begin());
    assert(dtmPts.size() == originalPts.size());

    if (dtmPts.size() == originalPts.size())
        {
        for (size_t i = 0; i < dtmPts.size(); ++i) assert(fabs(dtmPts[i].x - originalPts[i].x) < 1e-8 && fabs(dtmPts[i].y - originalPts[i].y) < 1e-8);
        }

    // Create Circular List From STM Triangles
    if (dbg) bcdtmWrite_message(0, 0, 0, "Creating Circular List From STM Triangles");
    for (int idx = 0; idx < numIndices; idx += 3)
        {
        DPoint3d tri[3] = { ptsList[indexList[idx] - 1], ptsList[indexList[idx + 1] - 1], ptsList[indexList[idx + 2] - 1] };
        int stmPoints[3];
        //  Get Point Numbers For STM Triangle
        for (size_t i = 0; i < 3; ++i)
            {
            // Binary Search To Find Related DTM Point Number
            index = -1;
            bottom = 0;
            top = dtmP->numPoints - 1;
            if (fabs(tri[i].x - pointAddrP(dtmP, bottom)->x) < 1e-8 && fabs(tri[i].y - pointAddrP(dtmP, bottom)->y) < 1e-8) index = bottom;
            else if (fabs(tri[i].x -pointAddrP(dtmP, top)->x) < 1e-8    && fabs(tri[i].y - pointAddrP(dtmP, top)->y) < 1e-8) index = top;
            else
                {
                while (top - bottom > 1)
                    {
                    mid = (top + bottom) / 2;
                    if (fabs(tri[i].x - pointAddrP(dtmP, mid)->x) < 1e-8 && fabs(tri[i].y - pointAddrP(dtmP, mid)->y) < 1e-8) top = bottom = index = mid;
                    else if (tri[i].x >  pointAddrP(dtmP, mid)->x || (tri[i].x == pointAddrP(dtmP, mid)->x && tri[i].y > pointAddrP(dtmP, mid)->y)) bottom = mid;
                    else top = mid;
                    }
                }

            // Check Point Was Found
            if (index == -1)
                {
                bcdtmWrite_message(1, 0, 0, "Cannot Find DTM Point Number For STM Triangle Vertex");
                goto errexit;
                }
            stmPoints[i] = index;

            //  Check Point Found Has The Correct Coordinates
            if (cdbg)
                {
                if (tri[i].x != pointAddrP(dtmP, stmPoints[i])->x || tri[i].y != pointAddrP(dtmP, stmPoints[i])->y)
                    {
                    bcdtmWrite_message(2, 0, 0, "Incorrect Point Association");
                    goto errexit;
                    }
                }
            }

        // Insert Lines Into Circular List

        for (n = 0; n < 3; ++n)
            {
            if (!bcdtmList_testLineDtmObject(dtmP, stmPoints[n], stmPoints[(n + 1)%3]))
                {
                if (bcdtmList_insertLineDtmObject(dtmP, stmPoints[n], stmPoints[(n + 1) % 3])) goto errexit;
                }
            }
        }
    dtmP->dtmState = DTMState::Tin;
    dtmP->numTriangles = numIndices / 3;

    // Check Triangulated STM Triangles
    if (cdbg)
        {
        if (dbg) bcdtmWrite_message(0, 0, 0, "Checking DTM Validity");
        if (bcdtmCheck_trianglesDtmObject(dtmP))
            {
            bcdtmWrite_message(1, 0, 0, "DTM Invalid");
            goto errexit;
            }
        if (dbg) bcdtmWrite_message(0, 0, 0, "Checking DTM Valid");
        }

    // Log Triangulation Statistics
    if (dbg == 1) bcdtmObject_reportStatisticsDtmObject(dtmP);

    // Log Triangualtion Times
    if (tdbg) bcdtmWrite_message(0, 0, 0, "Time To Triangulate %8ld STM Triangles = %8.3lf Seconds", dtmP->numFeatures, bcdtmClock_elapsedTime(bcdtmClock(), startTime));

    // Clean Up
cleanup:

    // Return

    if (dbg && ret == DTM_SUCCESS) bcdtmWrite_message(0, 0, 0, "Triangulating STM Triangles Completed");
    if (dbg && ret != DTM_SUCCESS) bcdtmWrite_message(0, 0, 0, "Triangulating STM Triangles Error");
    return (ret);

    // Error Exit

errexit:
    if (ret == DTM_SUCCESS) ret = DTM_ERROR;
    goto cleanup;
    }

DTMStatusInt ScalableMeshMesh::_GetBoundary(bvector<DPoint3d>& pts)
    {
    return DTM_ERROR;
    }

DTMStatusInt ScalableMeshMesh::_GetAsBcDTM(BcDTMPtr& bcdtm)
    { 
    BC_DTM_OBJ* bcDtmP = 0;
    int dtmCreateStatus = bcdtmObject_createDtmObject(&bcDtmP);
    if (dtmCreateStatus == 0)
        {
        bcdtm = BcDTM::CreateFromDtmHandle(*bcDtmP);
        }
    else return DTM_ERROR;

    DPoint3d triangle[4];


    bvector<int> indices;
    bvector<DPoint3d> pts;
    bvector<int> destIdx(m_nbPoints);
    std::fill_n(destIdx.begin(), destIdx.size(), -1);
    for (unsigned int t = 0; t < m_nbFaceIndexes; t += 3)
        {
        for (int i = 0; i < 3; i++)
            triangle[i] = m_points[m_faceIndexes[i + t] - 1];

        triangle[3] = triangle[0];

        //colinearity test
        if (triangle[0].AlmostEqualXY(triangle[1]) || triangle[1].AlmostEqualXY(triangle[2]) || triangle[2].AlmostEqualXY(triangle[0])) continue;
        DSegment3d triSeg = DSegment3d::From(triangle[0], triangle[1]);
        double param;
        DPoint3d closestPt;
        triSeg.ProjectPointXY(closestPt, param, triangle[2]);
        if (closestPt.AlmostEqualXY(triangle[2])) continue;
        if (destIdx[m_faceIndexes[t] - 1] == -1)
            {
            pts.push_back(m_points[m_faceIndexes[t] - 1]);
            destIdx[m_faceIndexes[t] - 1] = (int)pts.size() - 1;
            }
        if (destIdx[m_faceIndexes[t+1] - 1] == -1)
            {
            pts.push_back(m_points[m_faceIndexes[t+1] - 1]);
            destIdx[m_faceIndexes[t+1] - 1] = (int)pts.size() - 1;
            }
        if (destIdx[m_faceIndexes[t + 2] - 1] == -1)
            {
            pts.push_back(m_points[m_faceIndexes[t + 2] - 1]);
            destIdx[m_faceIndexes[t + 2] - 1] = (int)pts.size() - 1;
            }
        if (destIdx[m_faceIndexes[t + 2] - 1] == destIdx[m_faceIndexes[t + 1] - 1] || destIdx[m_faceIndexes[t + 2] - 1] == destIdx[m_faceIndexes[t] - 1] || destIdx[m_faceIndexes[t + 1] - 1] == destIdx[m_faceIndexes[t] - 1]) continue;
        indices.push_back(destIdx[m_faceIndexes[t] - 1]);
        if (bsiGeom_getXYPolygonArea(&triangle[0], 3) < 0)
            {
            indices.push_back(destIdx[m_faceIndexes[t + 2] - 1]);
            indices.push_back(destIdx[m_faceIndexes[t + 1] - 1]);
            }
        else
            {
            indices.push_back(destIdx[m_faceIndexes[t + 1] - 1]);
            indices.push_back(destIdx[m_faceIndexes[t + 2] - 1]);
            }
        }
    int status = bcdtmObject_storeTrianglesInDtmObject(bcdtm->GetTinHandle(), DTMFeatureType::GraphicBreak, &pts[0], (int)pts.size(), &indices[0], (int)indices.size() / 3);

    volatile bool dbg = false;
    if (dbg)
        {
        for (auto& idx : indices) idx += 1;
        size_t nIndices = indices.size();
        WString nameBefore = WString(L"E:\\output\\scmesh\\2016-06-03\\") + L"fpostgetmesh_";
        DRange3d range;
        bcdtm->GetRange(range);
        nameBefore.append(to_wstring(range.low.x).c_str());
        nameBefore.append(L"_");
        nameBefore.append(to_wstring(range.low.y).c_str());
        nameBefore.append(L".m");
        FILE* meshBeforeClip = _wfopen(nameBefore.c_str(), L"wb");
        fwrite(&m_nbPoints, sizeof(size_t), 1, meshBeforeClip);
        fwrite(m_points, sizeof(DPoint3d), m_nbPoints, meshBeforeClip);
        fwrite(&nIndices, sizeof(size_t), 1, meshBeforeClip);
        fwrite(&indices[0], sizeof(int32_t), nIndices, meshBeforeClip);
        fclose(meshBeforeClip);
        }
    assert(status == SUCCESS);

    status = bcdtmObject_triangulateStmTrianglesDtmObject(bcdtm->GetTinHandle());
    assert(status == SUCCESS);

    return status == SUCCESS? DTM_SUCCESS : DTM_ERROR;
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
                double param = -DBL_MAX;
                if (edgeSegment.Intersect(intersectPts[nOfIntersects], param, cuttingPlane) && param > -1.0e-5 && param <= 1.0)
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

const Byte* ScalableMeshTexture::_GetData() const
    {
    return m_textureData;
    }

size_t ScalableMeshTexture::_GetSize() const
    {
    return m_dataSize;
    }

Point2d ScalableMeshTexture::_GetDimension() const
    {
    return m_dimension;
    }

ScalableMeshTexture::ScalableMeshTexture(RefCountedPtr<SMMemoryPoolBlobItem<Byte>>& pPoolBlobItem)
    : m_texturePtr(pPoolBlobItem)
    {
    
    if (m_texturePtr.IsValid() && m_texturePtr->GetSize() > 0)
        {        
        int dimensionX;
        memcpy_s(&dimensionX, sizeof(int), m_texturePtr->GetData(), sizeof(int));
        m_dimension.x = dimensionX;
        int dimensionY;
        memcpy_s(&dimensionY, sizeof(int), (int*)m_texturePtr->GetData() + 1, sizeof(int));
        m_dimension.y = dimensionY;
        memcpy_s(&m_nbChannels, sizeof(int), (int*)m_texturePtr->GetData() + 2, sizeof(int));
        m_dataSize = m_dimension.x * m_dimension.y * m_nbChannels;

        m_textureData = m_texturePtr->GetData() + sizeof(int) * 3;
        } 
    else
        {
        m_dimension.x = m_dimension.y = 0;
        m_dataSize = 0;
        }
    }

ScalableMeshTexture::~ScalableMeshTexture()
    {    
    }

DTMStatusInt ScalableMeshMeshWithGraph::_GetBoundary(bvector<DPoint3d>& pts)
    {
    bvector<bvector<DPoint3d>> bound;
    GetGraphExteriorBoundary(m_graphData, bound, m_points);
    if (bound.size() == 0) return DTM_ERROR;
    else if (bound.size() == 1) pts = bound[0];
    else
        {
        bmap<double, size_t> largestPolys;
        for (auto& poly : bound)
            {
            DRange3d range = DRange3d::From(poly);
            largestPolys.insert(make_bpair(range.XLength() * range.YLength(), &poly - &bound.front()));
            }
        pts = bound[largestPolys.rbegin()->second];
        }
    return DTM_SUCCESS;
    }

int ScalableMeshMeshWithGraph::_ProjectPolyLineOnMesh(DPoint3d& endPt, bvector<bvector<DPoint3d>>& projectedPoints, const DPoint3d* points, int nPts, int* segment, const MTGNodeId triangleEdge, DPoint3d startPt) const
    {
    if (triangleEdge == -1) return ERROR;
    MTGNodeId edge = triangleEdge;
   ScalableMeshGraphDraping draping(m_graphData, m_is3d);
   draping.SetPolyLine(points, nPts);
   draping.SetVertices(m_points, m_nbPoints);
    if (!draping.FollowPolylineOnGraph(projectedPoints, endPt, edge, segment, startPt)) return ERROR;
    //if (!FollowPolylineOnGraph(m_graphData, endPt, projectedPoints, m_points, edge, segment, points, nPts, startPt, m_nbPoints, m_is3d)) return ERROR;
    return SUCCESS;
    }

int ScalableMeshMeshWithGraph::_ProjectPolyLineOnMesh(DPoint3d& endPt, bvector<bvector<DPoint3d>>& projectedPoints, const DPoint3d* points, int nPts, int* segment, const int* triangle, DPoint3d startPt, MTGNodeId& lastEdge) const
    {
    MTGNodeId triangleStartEdge = FindFaceInGraph(m_graphData, triangle[0], triangle[1], triangle[2]);
    if (triangleStartEdge == -1) return ERROR;
    ScalableMeshGraphDraping draping(m_graphData, m_is3d);
    draping.SetPolyLine(points, nPts);
    draping.SetVertices(m_points, m_nbPoints);
    if (!draping.FollowPolylineOnGraph(projectedPoints, endPt, triangleStartEdge, segment, startPt)) return ERROR;
    //if (!FollowPolylineOnGraph(m_graphData, endPt, projectedPoints, m_points, triangleStartEdge, segment, points, nPts, startPt, m_nbPoints, m_is3d)) return ERROR;
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
/*    //we find a close point based on point sort order 
    auto it = std::lower_bound(&m_points[0], &m_points[0] + m_nbPoints, point, [] (const DPoint3d&i, const DPoint3d&j)
        {
        if (i.x <j.x) return true;
        else if (i.x ==j.x && i.y < j.y) return true;
        else if (i.x == j.x && i.y == j.y && i.z < j.z) return true;
        return false;
        });
    std::set<MTGNodeId> visitedEdges;
    std::queue<MTGNodeId> toLookUpEdges;
    size_t nNodesInGraph = (size_t)(m_graphData)->GetNodeIdCount();
    MTGNodeId edgeId = (MTGNodeId)nNodesInGraph / 2;
    int upper = (MTGNodeId)nNodesInGraph;
    int lower = 0;
    int minV = (it - &m_points[0]) + 1;
    bool found = false;
    while (!found)
        {
        if (edgeId == -1 || edgeId == (MTGNodeId)nNodesInGraph) break;
        int v = -1;
        m_graphData->TryGetLabel(edgeId, 0, v);
        if (v < minV)
            {
            lower = edgeId;
            if (edgeId == (MTGNodeId)nNodesInGraph - 1) edgeId = (MTGNodeId)nNodesInGraph;
            else edgeId = lower + (upper - lower) / 2;
            continue;
            }
        else if (v > minV)
            {
            upper = edgeId;
            if (edgeId == 1) edgeId = 0;
            else if (edgeId == 0) edgeId = -1;
            else
                {
                edgeId = lower + (upper - lower) / 2;
                }
            continue;
            }
        found = true;
        toLookUpEdges.push(edgeId);
        }
    visitedEdges.insert(edgeId);
    while (!toLookUpEdges.empty())
        {
        MTGNodeId edgeId = toLookUpEdges.front();
        toLookUpEdges.pop();
        MTGARRAY_VERTEX_LOOP(id, m_graphData, edgeId)
            {
            if (id != edgeId && visitedEdges.count(id) == 0)
                {
                toLookUpEdges.push(id);
                visitedEdges.insert(id);
                }
            }
        MTGARRAY_END_VERTEX_LOOP(id, m_graphData, edgeId)
            if (FastCountNodesAroundFace(m_graphData, edgeId) > 3) continue;
        if (visitedEdges.count(m_graphData->EdgeMate(m_graphData->FSucc(edgeId))) == 0)
            {
            toLookUpEdges.push(m_graphData->EdgeMate(m_graphData->FSucc(edgeId)));
            visitedEdges.insert(m_graphData->EdgeMate(m_graphData->FSucc(edgeId)));
            }
        if (visitedEdges.count(m_graphData->EdgeMate(m_graphData->FSucc(m_graphData->FSucc(edgeId)))) == 0)
            {
            toLookUpEdges.push(m_graphData->EdgeMate(m_graphData->FSucc(m_graphData->FSucc(edgeId))));
            visitedEdges.insert(m_graphData->EdgeMate(m_graphData->FSucc(m_graphData->FSucc(edgeId))));
            }
        DPoint3d pts[3];
        int labels[3] = { -1, -1, -1 };
        m_graphData->TryGetLabel(edgeId, 0, labels[0]);
        m_graphData->TryGetLabel(m_graphData->FSucc(edgeId), 0, labels[1]);
        m_graphData->TryGetLabel(m_graphData->FSucc(m_graphData->FSucc(edgeId)), 0, labels[2]);
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
        bool intersectTri = false;
        if (use2d)
            {
            intersectTri = PointProjectsToTriangle2d(point, pts);
            }
        else
            intersectTri = bsiDRay3d_intersectTriangle(&ray, &projectedPt, &bary, &param, pts) && bary.x >= -1.0e-6f
            && bary.x <= 1.0&& bary.y >= -1.0e-6f && bary.y <= 1.0 && bary.z >= -1.0e-6f && bary.z <= 1.0 && param > maxParam;
        if (intersectTri)
            {
            outTriangle = edgeId;
            maxParam = param;
            if (use2d)
                {
                return true;
                }
            }
        }*/

    
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
        double param=0;
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
        bool intersectTri = false;
        if (use2d)
            {
            intersectTri = PointProjectsToTriangle2d(point, pts);
            }
        else
            intersectTri = bsiDRay3d_intersectTriangle(&ray, &projectedPt, &bary, &param, pts) && bary.x >= -1.0e-6f
            && bary.x <= 1.0&& bary.y >= -1.0e-6f && bary.y <= 1.0 && bary.z >= -1.0e-6f && bary.z <= 1.0 && param > maxParam;
        if (intersectTri)
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



const char* s_path = "E:\\output\\scmesh\\2015-11-27\\";
size_t s_nClips = 0;
     

void ScalableMeshMesh::ApplyClipMesh(const DifferenceSet& d)
    {
#if SM_TRACE_CLIPS_GETMESH
    std::multimap < int32_t, bpair<std::array<int32_t, 3>, std::array<DPoint2d, 3>>> originalUvs;
    if (m_uvCount > 0 && m_pUvIndex && d.addedFaces.size() >= 3)
        {
        Utf8String nameBeforeClips = Utf8String(s_path)+"tilebeforeclips_";
        nameBeforeClips.append(std::to_string(s_nClips).c_str());
        nameBeforeClips.append(".txt");
        std::ofstream stats;
        stats.open(nameBeforeClips.c_str(), std::ios_base::trunc);
        stats << " N POINTS "+std::to_string(m_nbPoints) << std::endl;
        stats << " N UVS "+std::to_string(m_uvCount) << std::endl;
        for(size_t i =0; i < m_nbFaceIndexes; i+=3)
            {
            stats << "TRIANGLE " + std::to_string(i / 3)+"\n";
            std::string s;
            print_polygonarray(s, " POINT ", &m_points[m_faceIndexes[i] - 1], 1);
            print_polygonarray(s, " POINT ", &m_points[m_faceIndexes[i+1] - 1], 1);
            print_polygonarray(s, " POINT ", &m_points[m_faceIndexes[i+2] - 1], 1);
            stats << s;
            stats << " UVS :" << std::endl;
            stats << std::to_string(m_pUv[m_pUvIndex[i]-1].x)+" " +std::to_string(m_pUv[m_pUvIndex[i]-1].y) << std::endl;
            stats << std::to_string(m_pUv[m_pUvIndex[i+1]-1].x)+" " +std::to_string(m_pUv[m_pUvIndex[i+1]-1].y) << std::endl;
            stats << std::to_string(m_pUv[m_pUvIndex[i+2]-1].x)+" " +std::to_string(m_pUv[m_pUvIndex[i+2]-1].y) << std::endl;
            std::array<int32_t, 3> face = { m_faceIndexes[i], m_faceIndexes[i + 1], m_faceIndexes[i + 2] };
            std::array<DPoint2d, 3> uvCoords = { m_pUv[m_pUvIndex[i]-1], m_pUv[m_pUvIndex[i + 1]-1], m_pUv[m_pUvIndex[i + 2]-1] };
            if (m_pUv[m_pUvIndex[i]-1].x > 1+1e-4 || m_pUv[m_pUvIndex[i]-1].x < 0-1e-4 || m_pUv[m_pUvIndex[i + 1]-1].x > 1+1e-4 || m_pUv[m_pUvIndex[i + 1]-1].x < 0-1e-4 || m_pUv[m_pUvIndex[i + 2]-1].x > 1+1e-4 || m_pUv[m_pUvIndex[i + 2]-1].x < 0-1e-4)
                stats << "WRONG INDEX -- INDICES " + std::to_string(m_pUvIndex[i]-1) + " " + std::to_string(m_pUvIndex[i + 1]-1) + " " + std::to_string(m_pUvIndex[i + 2]-1) << std::endl;
            originalUvs.insert(make_pair(m_faceIndexes[i], make_bpair(face, uvCoords)));
            originalUvs.insert(make_pair(m_faceIndexes[i+1], make_bpair(face, uvCoords)));
            originalUvs.insert(make_pair(m_faceIndexes[i+2], make_bpair(face, uvCoords)));
            }
        stats.close();
    }
#endif
  
#if SM_TRACE_CLIPS_GETMESH
    if (m_uvCount > 0 && m_pUvIndex && d.addedFaces.size() >= 3)
        {
        Utf8String nameAfterClips = Utf8String(s_path) + "tileafterclips_";
        nameAfterClips.append(std::to_string(s_nClips).c_str());
        nameAfterClips.append(".txt");
        std::ofstream stats2;
        stats2.open(nameAfterClips.c_str(), std::ios_base::trunc);
        stats2 << " N POINTS " + std::to_string(m_nbPoints) << std::endl;
        stats2 << " N UVS " + std::to_string(m_uvCount) << std::endl;
        for (size_t i = 0; i < m_nbFaceIndexes; i += 3)
            {
            stats2 << "TRIANGLE " + std::to_string(i / 3) + "\n";
            std::string s;
            print_polygonarray(s, " POINT ", &m_points[m_faceIndexes[i] - 1], 1);
            print_polygonarray(s, " POINT ", &m_points[m_faceIndexes[i + 1] - 1], 1);
            print_polygonarray(s, " POINT ", &m_points[m_faceIndexes[i + 2] - 1], 1);
            stats2 << s;
            stats2 << " UVS :" << std::endl;
            stats2 << std::to_string(m_pUv[m_pUvIndex[i]-1].x) + " " + std::to_string(m_pUv[m_pUvIndex[i]-1].y) << std::endl;
            stats2 << std::to_string(m_pUv[m_pUvIndex[i + 1]-1].x) + " " + std::to_string(m_pUv[m_pUvIndex[i + 1]-1].y) << std::endl;
            stats2 << std::to_string(m_pUv[m_pUvIndex[i + 2]-1].x) + " " + std::to_string(m_pUv[m_pUvIndex[i + 2]-1].y) << std::endl;

            if (originalUvs.count(m_faceIndexes[i]) > 0)
                {
                auto range = originalUvs.equal_range(m_faceIndexes[i]);
                for (auto it = range.first; it != range.second; ++it)
                    {
                    auto face = it->second.first;
                    auto uvCoords = it->second.second;
                    if ((face[0] == m_faceIndexes[i] || face[1] == m_faceIndexes[i] || face[2] == m_faceIndexes[i]) &&
                        (face[0] == m_faceIndexes[i + 1] || face[1] == m_faceIndexes[i + 1] || face[2] == m_faceIndexes[i + 1]) &&
                        (face[0] == m_faceIndexes[i + 2] || face[1] == m_faceIndexes[i + 2] || face[2] == m_faceIndexes[i + 2]))
                        {
                        if ((fabs(uvCoords[0].Distance(m_pUv[m_pUvIndex[i]-1])) < 1e-3 || fabs(uvCoords[1].Distance(m_pUv[m_pUvIndex[i]-1])) < 1e-3 ||fabs(uvCoords[2].Distance(m_pUv[m_pUvIndex[i]-1])) < 1e-3) &&
                            (fabs(uvCoords[0].Distance(m_pUv[m_pUvIndex[i+1]-1])) < 1e-3  || fabs(uvCoords[1].Distance(m_pUv[m_pUvIndex[i+1]-1])) < 1e-3 || fabs(uvCoords[2].Distance(m_pUv[m_pUvIndex[i+1]-1])) < 1e-3) &&
                            (fabs(uvCoords[0].Distance(m_pUv[m_pUvIndex[i+2]-1])) < 1e-3  || fabs(uvCoords[1].Distance(m_pUv[m_pUvIndex[i+2]-1])) < 1e-3 || fabs(uvCoords[2].Distance(m_pUv[m_pUvIndex[i+2]-1])) < 1e-3))
                            {

                            }
                        else
                            {
                            stats2 << " DIFFERENCE !" << std::endl;
                            stats2 << " STORED UVS " << std::endl;
                            stats2 << std::to_string(uvCoords[0].x) + " " + std::to_string(uvCoords[0].y) << std::endl;
                            stats2 << std::to_string(uvCoords[1].x) + " " + std::to_string(uvCoords[1].y) << std::endl;
                            stats2 << std::to_string(uvCoords[2].x) + " " + std::to_string(uvCoords[2].y) << std::endl;
                            break;
                            }
                        }
                    }              
                    
                }
            }
        stats2.close();
        s_nClips++;
        }
#endif
    }

//=======================================================================================
// @description Recomputes UVs based on interpolating coordinates within the node extent.
//              This assumes that texture is square and covers the node entirely.
//              This function is used to recompute UVs after clipping when dataset has photos
//              draped on. See SMMeshIndex::TextureFromRaster, ScalableMeshNode::_GetMesh.
// @bsimethod                                                   Elenie.Godzaridis 11/15
//=======================================================================================
void ScalableMeshMesh::RecalculateUVs(DRange3d& nodeRange)
    {
    delete[] m_pUvIndex;
    m_pUvIndex = new int32_t[m_nbFaceIndexes];
    memcpy(m_pUvIndex, m_faceIndexes, m_nbFaceIndexes*sizeof(int32_t));
    for (size_t i = 0; i < m_nbFaceIndexes; ++i)
        {
        if (m_pUvIndex[i] - 1 >= m_uvCount) m_uvCount = m_pUvIndex[i];
        }
    delete[] m_pUv;
    m_pUv = new DPoint2d[m_uvCount];
    for (size_t i = 0; i < m_uvCount; ++i)
        {
        m_pUv[i].x = (m_points[i].x - nodeRange.low.x) / (nodeRange.XLength());
        m_pUv[i].y = (m_points[i].y - nodeRange.low.y) / (nodeRange.YLength());
        }
    }

ScalableMeshMeshWithGraph::ScalableMeshMeshWithGraph(size_t nbPoints, DPoint3d* points, size_t nbFaceIndexes, int32_t* faceIndexes, size_t normalCount, DVec3d* pNormal, int32_t* pNormalIndex, MTGGraph* pGraph, bool is3d, size_t uvCount, DVec2d* pUv, int32_t* pUvIndex)
    : ScalableMeshMesh(nbPoints, points, nbFaceIndexes, faceIndexes, normalCount, pNormal, pNormalIndex, uvCount, pUv, pUvIndex)
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

ScalableMeshMeshWithGraphPtr ScalableMeshMeshWithGraph::Create(size_t nbPoints, DPoint3d* points, size_t nbFaceIndexes, int32_t* faceIndexes, size_t normalCount, DVec3d* pNormal, int32_t* pNormalIndex, MTGGraph* pGraph, bool is3d, size_t uvCount, DVec2d* pUv, int32_t* pUvIndex)
    {
    return new ScalableMeshMeshWithGraph(nbPoints, points, nbFaceIndexes, faceIndexes, normalCount, pNormal, pNormalIndex, pGraph, is3d, uvCount, pUv, pUvIndex);
    }

ScalableMeshMeshWithGraphPtr ScalableMeshMeshWithGraph::Create(MTGGraph* pGraph, bool is3d)
 {
    return new ScalableMeshMeshWithGraph(pGraph, is3d);
 }


IScalableMeshMeshQueryParams::IScalableMeshMeshQueryParams()
    {}

IScalableMeshMeshQueryParams::~IScalableMeshMeshQueryParams()
    {}

BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr IScalableMeshMeshQueryParams::GetSourceGCS()
    {
    return _GetSourceGCS();
    }

BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr IScalableMeshMeshQueryParams::GetTargetGCS()
    {
    return _GetTargetGCS();
    }

size_t IScalableMeshMeshQueryParams::GetLevel()
    {
    return _GetLevel();
    }

void IScalableMeshMeshQueryParams::SetGCS(BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& sourceGCSPtr,
                                   BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCSPtr)
    {
    _SetGCS(sourceGCSPtr, targetGCSPtr);
    }

void IScalableMeshMeshQueryParams::SetLevel(size_t depth)
    {
    _SetLevel(depth);
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

bool IScalableMeshViewDependentMeshQueryParams::IsProgressiveDisplay() const 
    {
    return _IsProgressiveDisplay();
    }

void IScalableMeshViewDependentMeshQueryParams::SetViewBox(const DPoint3d viewBox[])
    {
    _SetViewBox(viewBox);
    }

double IScalableMeshViewDependentMeshQueryParams::GetMinScreenPixelsPerPoint() const
    {
    return _GetMinScreenPixelsPerPoint();
    }

StopQueryCallbackFP IScalableMeshViewDependentMeshQueryParams::GetStopQueryCallback() const
    {
    return _GetStopQueryCallback();
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

void IScalableMeshViewDependentMeshQueryParams::SetProgressiveDisplay(bool isProgressiveDisplay)
    {
    _SetProgressiveDisplay(isProgressiveDisplay);
    }

void IScalableMeshViewDependentMeshQueryParams::SetRootToViewMatrix(const double rootToViewMatrix[][4])
    {
    _SetRootToViewMatrix(rootToViewMatrix);
    }

StatusInt IScalableMeshViewDependentMeshQueryParams::SetStopQueryCallback(StopQueryCallbackFP stopQueryCallbackFP)
    {
    return _SetStopQueryCallback(stopQueryCallbackFP);
    }

void IScalableMeshViewDependentMeshQueryParams::SetViewClipVector(ClipVectorPtr& viewClipVector)
    {
    _SetViewClipVector(viewClipVector);
    }

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
    int32_t const* pPointIndex = (int32_t*)meshFacesP;
    size_t normalCount = numMeshPts;
    DVec3dCP  pNormal = (DVec3dCP)meshVectorsP;
    int32_t const* pNormalIndex = (int32_t*)meshFacesP;
    */
    if (s_passNormal)
        {
        *meshPtr = IScalableMeshMesh::Create(numMeshPts, meshPtsP, numMeshFaces, (int32_t*)meshFacesP, numMeshPts, (DVec3d*)meshVectorsP, (int32_t*)meshFacesP, 0, 0, 0);
        }
    else
        {
        *meshPtr = IScalableMeshMesh::Create(numMeshPts, meshPtsP, numMeshFaces, (int32_t*)meshFacesP, 0, 0, 0, 0, 0, 0);
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


bool IScalableMeshMeshFlags::ShouldLoadTexture() const
    {
    return _ShouldLoadTexture();
    }

bool IScalableMeshMeshFlags::ShouldLoadGraph() const
    {
    return _ShouldLoadGraph();
    }

void IScalableMeshMeshFlags::SetLoadTexture(bool loadTexture) 
    {
    _SetLoadTexture(loadTexture);
    }

void IScalableMeshMeshFlags::SetLoadGraph(bool loadGraph) 
    {
    _SetLoadGraph(loadGraph);
    }

IScalableMeshMeshFlagsPtr IScalableMeshMeshFlags::Create()
    {
    return new ScalableMeshMeshFlags();
    }

IScalableMeshMeshFlagsPtr IScalableMeshMeshFlags::Create(bool shouldLoadTexture, bool shouldLoadGraph)
    {
    IScalableMeshMeshFlagsPtr smMeshFlag(new ScalableMeshMeshFlags());
    smMeshFlag->SetLoadTexture(shouldLoadTexture);
    smMeshFlag->SetLoadGraph(shouldLoadGraph);
    return smMeshFlag;
    }

bool ScalableMeshMeshFlags::_ShouldLoadTexture() const
    {
    return m_loadTexture;
    }

bool ScalableMeshMeshFlags::_ShouldLoadGraph() const
    {
    return m_loadGraph;
    }

void ScalableMeshMeshFlags::_SetLoadTexture(bool loadTexture)
    {
    m_loadTexture = loadTexture;
    }

void ScalableMeshMeshFlags::_SetLoadGraph(bool loadGraph)
    {
    m_loadGraph = loadGraph;
    }

bool IScalableMeshNode::ArePoints3d() const
    {
    return _ArePoints3d();
    }

BcDTMPtr IScalableMeshNode::GetBcDTM() const
    {
    return _GetBcDTM();
    }

bool IScalableMeshNode::ArePointsFullResolution() const
    {
    return _ArePointsFullResolution();
    }

IScalableMeshMeshPtr IScalableMeshNode::GetMesh(IScalableMeshMeshFlagsPtr& flags) const
    {
    return _GetMesh(flags);
    }  

IScalableMeshMeshPtr IScalableMeshNode::GetMeshUnderClip(IScalableMeshMeshFlagsPtr& flags, uint64_t clip) const
    {
    return _GetMeshUnderClip(flags, clip);
    }

IScalableMeshMeshPtr IScalableMeshNode::GetMeshByParts(bset<uint64_t>& clipsToShow) const
    {
    return _GetMeshByParts(clipsToShow);
    }

IScalableMeshTexturePtr IScalableMeshNode::GetTexture() const
    {
    return _GetTexture();
    }

bool IScalableMeshNode::IsTextured() const
    {
    return _IsTextured();
    }

//Gets neighbors by relative position. For example, neighbor (-1, 0, 0) shares the node's left face. (1,1,0) shares the node's top-right diagonal. 
bvector<IScalableMeshNodePtr>  IScalableMeshNode::GetNeighborAt( char relativePosX, char relativePosY, char relativePosZ) const
    {
    return _GetNeighborAt(relativePosX, relativePosY, relativePosZ);
    }

bvector<IScalableMeshNodePtr>  IScalableMeshNode::GetChildrenNodes() const
    {
    return _GetChildrenNodes();
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

void IScalableMeshNode::LoadNodeHeader() const 
    {
    return _LoadHeader();
    }

void IScalableMeshNode::ApplyAllExistingClips() const
    {
    return _ApplyAllExistingClips();
    }

void     IScalableMeshNode::RefreshMergedClip() const
    {
    return _RefreshMergedClip();
    }

bool     IScalableMeshNode::AddClip(uint64_t id, bool isVisible) const
    {
    return _AddClip(id, isVisible);
    }


bool     IScalableMeshNode::ModifyClip(uint64_t id,  bool isVisible) const
    {
    return _ModifyClip(id,  isVisible);
    }

bool     IScalableMeshNode::DeleteClip(uint64_t id, bool isVisible) const
    {
    return _DeleteClip(id,isVisible);
    }

bool IScalableMeshNode::HasClip(uint64_t clipId) const
    {
    return _HasClip(clipId);
    }

bool IScalableMeshNode::IsClippingUpToDate() const
    {
    return _IsClippingUpToDate();
    }

void IScalableMeshNode::GetSkirtMeshes(bvector<PolyfaceHeaderPtr>& meshes) const
    {
    return _GetSkirtMeshes(meshes);
    }

bool IScalableMeshNode::RunQuery(ISMPointIndexQuery<DPoint3d, DRange3d>& query, bvector<IScalableMeshNodePtr>& nodes) const
    {
    return _RunQuery(query,nodes);
    }

bool IScalableMeshNode::RunQuery(ISMPointIndexQuery<DPoint3d, DRange3d>& query) const
    {
    return _RunQuery(query);
    }

int IScalableMeshNodeRayQuery::Query(IScalableMeshNodePtr&                               nodePtr,
                              const DPoint3d*                           pTestPt,
                              const DPoint3d*                              pClipShapePts,
                              int                                          nbClipShapePts,
                              const IScalableMeshNodeQueryParamsPtr& scmQueryParamsPtr) const
    {
    return _Query(nodePtr, pTestPt, pClipShapePts, nbClipShapePts, scmQueryParamsPtr);
    }

int IScalableMeshNodeRayQuery::Query(bvector<IScalableMeshNodePtr>&                               nodesPtr,
                                     const DPoint3d*                           pTestPt,
                                     const DPoint3d*                              pClipShapePts,
                                     int                                          nbClipShapePts,
                                     const IScalableMeshNodeQueryParamsPtr& scmQueryParamsPtr) const
    {
    return _Query(nodesPtr, pTestPt, pClipShapePts, nbClipShapePts, scmQueryParamsPtr);
    }
  

StatusInt  IScalableMeshNodeEdit::AddMesh(DPoint3d* vertices, size_t nVertices, int32_t* indices, size_t nIndices)
    {
    return _AddMesh(vertices, nVertices, indices, nIndices);
    }

StatusInt IScalableMeshNodeEdit::AddTexturedMesh(bvector<DPoint3d>& vertices, bvector<bvector<int32_t>>& pointsIndices, bvector<DPoint2d>& uv, bvector<bvector<int32_t>>& uvIndices, size_t nTexture)
    {
    return _AddTexturedMesh(vertices, pointsIndices, uv, uvIndices, nTexture);
    }

StatusInt  IScalableMeshNodeEdit::AddTextures(bvector<Byte>& data, bool sibling)
    {
    return _AddTextures(data, sibling);
    }

StatusInt  IScalableMeshNodeEdit::SetNodeExtent(DRange3d& extent)
    {
    return _SetNodeExtent(extent);
    }

StatusInt  IScalableMeshNodeEdit::SetArePoints3d(bool arePoints3d)
    {
    return _SetArePoints3d(arePoints3d);
    }

StatusInt  IScalableMeshNodeEdit::SetContentExtent(DRange3d& extent)
    {
    return _SetContentExtent(extent);
    }

/*=========================IScalableMeshCachedDisplayNode===============================*/
StatusInt IScalableMeshCachedDisplayNode::GetCachedMesh(SmCachedDisplayMesh*& cachedMesh) const
    {
    return _GetCachedMesh(cachedMesh);
    }

StatusInt IScalableMeshCachedDisplayNode::GetCachedTexture(SmCachedDisplayTexture*& cachedTexture) const
    {
    return _GetCachedTexture(cachedTexture);
    }
    
/*==================================================================*/
/*        3D MESH RELATED CODE - END                                */
/*==================================================================*/


/*==================================================================*/
/*        TEMPLATES - DECLARATION - BEGIN                           */
/*==================================================================*/


template class ScalableMeshFixResolutionViewPointQuery<DPoint3d>;

template class ScalableMeshFullResolutionPointQuery<DPoint3d>;

template class ScalableMeshViewDependentPointQuery<DPoint3d>;

template class ScalableMeshFixResolutionViewPointQuery<DPoint3d>;

template class ScalableMeshViewDependentMeshQuery<DPoint3d>;

template class ScalableMeshFullResolutionMeshQuery<DPoint3d>;

template class ScalableMeshReprojectionMeshQuery<DPoint3d>;

template class ScalableMeshNodeRayQuery<DPoint3d>;

template class ScalableMeshNodePlaneQuery<DPoint3d>;

template class ScalableMeshNode<DPoint3d>;

template class ScalableMeshCachedMeshNode<DPoint3d>;

template class ScalableMeshCachedDisplayNode<DPoint3d>;

template class ScalableMeshNodeEdit<DPoint3d>;

template class ScalableMeshNodeWithReprojection<DPoint3d>;

// reactivate warning
//#pragma warning (restore: 4250)

/*==================================================================*/
/*        TEMPLATES - DECLATION - END                               */
/*==================================================================*/



END_BENTLEY_SCALABLEMESH_NAMESPACE
