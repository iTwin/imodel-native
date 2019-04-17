/*--------------------------------------------------------------------------------------+
|    $RCSfile: ScalableMeshUtilityFunctions.h,v $
|   $Revision: 1.15 $
|       $Date: 2013/03/27 15:53:36 $
|     $Author: Jean-Francois.Cote $
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
|                                                                        |
|    ScalableMeshFileCreator.h                              (C) Copyright 2001.        |
|                                                BCIVIL Corporation.        |
|                                                All Rights Reserved.    |
|                                                                       |
+----------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <TerrainModel/TerrainModel.h>
#include <TerrainModel/Core/bcDTMClass.h>

#include <ScalableMesh/IScalableMeshQuery.h>
#include <ScalableMesh/IScalableMeshClipContainer.h>


USING_NAMESPACE_BENTLEY_SCALABLEMESH

BENTLEY_SM_EXPORT int GetShapeInFrontOfProjectivePlane(std::vector<DPoint3d>&       shapeInFrontOfProjectivePlane, 
                                                 double&                 ratioShapeInFrontToTile,
                                                 const std::vector<DPoint3d>& tileborderPoints,
                                                 const double            rootToViewMatrix[][4]);

BENTLEY_SM_EXPORT bool GetVisibleAreaForView(DPoint3d**   fencePt, 
                                            int&           nbPts, 
                                            const DPoint3d viewBox[],                                         
                                            DRange3d&      dtmRange, 
                                            DRange3d&      dtmIntersectionRange);
/*
static HFCPtr<HVE2DPolygonOfSegments> ComputeConvexHull    (const DPoint3d*      inputPts,
                                                            size_t          inputPtsQty)
*/


BENTLEY_SM_EXPORT int CreateBcDTM(BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr& dtmPtr);

BENTLEY_SM_EXPORT int SetClipsToDTM (BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr& dtmPtr,
                               const DRange3d&                      dtmRange,
                               const IScalableMeshClipContainerPtr&        clips);

BENTLEY_SM_EXPORT int SetClipsToDTM (BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr& dtmPtr,
                               const DRange3d&                      dtmRange,
                               const std::vector<DPoint3d>&              regionPoints,
                               const IScalableMeshClipContainerPtr&        clips);

BENTLEY_SM_EXPORT int TriangulateDTM(BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr&                     dtmPtr, 
                               const BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshQueryParametersPtr& scmQueryParamsPtr);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AlainRobert  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BENTLEY_SM_EXPORT bool GCSMathematicalDomainsOverlap(BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& sourceGCSPtr,
                                               BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCSPtr);

typedef std::vector<DPoint3d> MaskPoints;
typedef std::list<MaskPoints> MaskList;

BENTLEY_SM_EXPORT bool AddExtentAndTestMatch(bool&           shouldAddMatchedCachedTile,
                                       MaskList&       maskList,
                                       bool            reset,                                        
                                       const DRange2d& extentToCover,
                                       const DRange2d& matchedCachedTileExtent);

class DTMLinearFeature;

typedef int (*addLinearsForPresentationModeFP)(const BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr&  dtmPtr,                                                 
                                               DTMLinearFeature*            linearList,
                                               unsigned int                 nbLinearListElems,                                               
                                               size_t                       maxNumberOfPoints);
                                
addLinearsForPresentationModeFP GetLinearsForPresentationModeCallback();

BENTLEY_SM_EXPORT bool SetLinearsForPresentationModeCallback(addLinearsForPresentationModeFP callbackFP);

BENTLEY_SM_EXPORT void GetCoverageTerrainAbsFileName(BeFileName& coverageFileName, const WString& baseExtraFilesPath, const Utf8String& coverageName);

BENTLEY_SM_EXPORT void StitchSegmentsAtJunctions(bvector<bvector<DPoint3d>>& polylines, const bvector<DSegment3d>& segments);

#ifdef SCALABLE_MESH_ATP

BENTLEY_SM_EXPORT double GetGroundDetectionDuration();
BENTLEY_SM_EXPORT void   SetGroundDetectionDuration(double t);
BENTLEY_SM_EXPORT void   AddGroundDetectionDuration(double t);

#endif
