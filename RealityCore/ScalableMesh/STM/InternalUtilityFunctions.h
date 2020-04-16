/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include <ScalableMesh/IScalableMeshClipContainer.h>
#include <ScalableMesh/IScalableMeshQuery.h>
#include "ImagePPHeaders.h" 

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

void CalcNormals (DVec3d**      calculatedNormals,                  
                  const DVec3d& viewNormalParam, 
                  size_t        nbPoints, 
                  DPoint3d*     pPoints, 
                  size_t        nbFaceIndexes, 
                  int32_t*        pFaceIndexes);

BENTLEY_SM_EXPORT HFCPTR<HVE2DSHAPE> CreateShapeFromPoints (const DPoint3d* points, size_t numberOfPoints, HFCPTR<HGF2DCOORDSYS> coordSys);

HFCPTR<HVESHAPE> CreateShapeFromClips (const DRange3d&               spatialIndexRange,
                                       const IScalableMeshClipContainerPtr& clips);

HFCPTR<HVESHAPE> CreateShapeFromClips (HFCPTR<HVESHAPE>        areaShape,
                                       const IScalableMeshClipContainerPtr& clips);

HFCPTR<HVE2DSHAPE> GetGCSDomainsIntersection (BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& firstGCSPtr,
                                              BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& secondGCSPtr, 
	HFCPTR<HGF2DCOORDSYS> latitudeLongitudeCoordSys);

void GetReprojectedBox (BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& sourceGCSPtr,
                        BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCSPtr,
                        DPoint3d                             boxPoints[],
                        DPoint3d                             reprojectedBoxPoints[]);

StatusInt GetReprojectedBoxDomainLimited (BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCSPtr,
                                          BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& sourceGCSPtr,
                                          DPoint3d    boxPoints[],
                                          DPoint3d    reprojectedBoxPoints[],
                                          DRange3d    additionalSourceExtent,
	                                      HFCPTR<HVE2DSHAPE>    queryShape);

StatusInt ReprojectPoint (BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& sourceGCSPtr,
                          BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCSPtr,
                          const DPoint3d& inPoint,
                          DPoint3d& outPoint);

StatusInt ReprojectRangeDomainLimited (DRange3d& reprojectedRange,
                                       const DRange3d& initialRange,
                                       BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& sourceGCS,
                                       BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCS);

HFCPTR<HVE2DSHAPE> ReprojectShapeDomainLimited (BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& sourceGCSPtr,
                                                BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCSPtr,  
                                                const DPoint3d*   pi_pSourcePt,
                                                size_t  pi_SourcePtQty);

BENTLEY_SM_EXPORT int SetClipToDTM (BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr& dtmPtr,
                  const DRange3d&                spatialIndexRange,
                  const HVE2DSHAPE&              shape);

int AddClipToDTM(BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr&           dtmPtr,
                 const HVE2DSHAPE& shape);

struct BENTLEY_SM_EXPORT PtToPtConverter
    {        
    DPoint3d operator () (const DPoint3d& inputPt) const;

    DPoint3d operator () (const HGF3DCOORD<double>& inputPt) const;

    static void Transform(DPoint3d* ptsOut, const DPoint3d* ptsIn, size_t nbPts);
    };

double ComputeMinEdgeLength(const DPoint3d* points, size_t ptNum, const int32_t* idx, size_t idxNum);
BENTLEY_SM_EXPORT inline bool IsUrl(WCharCP filename);
void SimplifyPolygonToMinEdge(double minEdge, bvector<DPoint3d>& poly);

BENTLEY_SM_EXPORT bool MergeFeatures(bvector<bvector<DPoint3d>> features, CurveVectorPtr& result, bool isClosedFeature);
BENTLEY_SM_EXPORT int    ResolveCrossingVoidIslands(bvector<bvector<DPoint3d>>& polylines, bvector<DTMFeatureType>& types, const bvector<uint32_t>& idOfSurroundingFeature, bvector<uint64_t>& ids);
BENTLEY_SM_EXPORT bool ClipIntersectsBox(const ClipVectorPtr& cp, DRange3d ext, Transform tr);
BENTLEY_SM_EXPORT void PlaneSetFromPolygon(bvector<DPoint3d> const& polygon, ClipPlaneSet*& planeSet);
BENTLEY_SM_EXPORT int AddToBCDtm(BC_DTM_OBJ* dtmObjP, bvector<DPoint3d>& points, CurveVectorPtr nonHullFeatures, bvector< std::pair<DTMFeatureType, DTMFeatureId>>& nonHullFeatureTypes, CurveVectorPtr hullFeatures);
END_BENTLEY_SCALABLEMESH_NAMESPACE