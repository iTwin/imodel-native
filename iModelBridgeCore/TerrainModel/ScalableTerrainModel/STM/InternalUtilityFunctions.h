/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/STM/InternalUtilityFunctions.h $
|    $RCSfile: InternalUtilityFunctions.h,v $
|   $Revision: 1.7 $
|       $Date: 2012/06/27 14:06:54 $
|     $Author: Chantal.Poulin $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <ScalableTerrainModel/IMrDTMClipContainer.h>
#include <ScalableTerrainModel/IMrDTMQuery.h>

HFCPtr<HVE2DShape> CreateShapeFromPoints (const DPoint3d* points, size_t numberOfPoints, HFCPtr<HGF2DCoordSys> coordSys);

HFCPtr<HVEShape> CreateShapeFromClips (const DRange3d&               spatialIndexRange,
                                       const IMrDTMClipContainerPtr& clips);

HFCPtr<HVEShape> CreateShapeFromClips (const HFCPtr<HVEShape>        areaShape,
                                       const IMrDTMClipContainerPtr& clips);

int CutLinears(list<HFCPtr<HVEDTMLinearFeature>>& linearList, 
               list<HFCPtr<HVEDTMLinearFeature>>& cutLinearList, 
               HFCPtr<HVE2DPolygonOfSegments> queryPolyLine);

HFCPtr<HVE2DShape> GetGCSDomainsIntersection (Bentley::GeoCoordinates::BaseGCSPtr& firstGCSPtr, 
                                              Bentley::GeoCoordinates::BaseGCSPtr& secondGCSPtr, 
                                              HFCPtr<HGF2DCoordSys> latitudeLongitudeCoordSys);

void GetReprojectedBox (Bentley::GeoCoordinates::BaseGCSPtr& sourceGCSPtr,
                        Bentley::GeoCoordinates::BaseGCSPtr& targetGCSPtr,
                        DPoint3d                             boxPoints[],
                        DPoint3d                             reprojectedBoxPoints[]);

StatusInt GetReprojectedBoxDomainLimited (Bentley::GeoCoordinates::BaseGCSPtr& targetGCSPtr,
                                          Bentley::GeoCoordinates::BaseGCSPtr& sourceGCSPtr,
                                          DPoint3d    boxPoints[],
                                          DPoint3d    reprojectedBoxPoints[],
                                          DRange3d    additionalSourceExtent,
                                          HFCPtr<HVE2DShape>    queryShape);

StatusInt ReprojectPoint (Bentley::GeoCoordinates::BaseGCSPtr& sourceGCSPtr,
                          Bentley::GeoCoordinates::BaseGCSPtr& targetGCSPtr,
                          const DPoint3d& inPoint,
                          DPoint3d& outPoint);

StatusInt ReprojectRangeDomainLimited (DRange3d& reprojectedRange,
                                       const DRange3d& initialRange,
                                       Bentley::GeoCoordinates::BaseGCSPtr& sourceGCS,
                                       Bentley::GeoCoordinates::BaseGCSPtr& targetGCS);

HFCPtr<HVE2DShape> ReprojectShapeDomainLimited (Bentley::GeoCoordinates::BaseGCSPtr& sourceGCSPtr, 
                                                Bentley::GeoCoordinates::BaseGCSPtr& targetGCSPtr,  
                                                const DPoint3d*   pi_pSourcePt,
                                                size_t  pi_SourcePtQty);

int SetClipToDTM (BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr& dtmPtr,
                  const DRange3d&                spatialIndexRange,
                  const HVE2DShape&              shape);

