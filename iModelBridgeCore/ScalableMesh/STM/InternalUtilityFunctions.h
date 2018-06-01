/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/InternalUtilityFunctions.h $
|    $RCSfile: InternalUtilityFunctions.h,v $
|   $Revision: 1.7 $
|       $Date: 2012/06/27 14:06:54 $
|     $Author: Chantal.Poulin $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <ScalableMesh/IScalableMeshClipContainer.h>
#include <ScalableMesh/IScalableMeshQuery.h>
#include "ImagePPHeaders.h" 


void CalcNormals (DVec3d**      calculatedNormals,                  
                  const DVec3d& viewNormalParam, 
                  size_t        nbPoints, 
                  DPoint3d*     pPoints, 
                  size_t        nbFaceIndexes, 
                  int32_t*        pFaceIndexes);

BENTLEY_SM_EXPORT ImagePP::HFCPtr<ImagePP::HVE2DShape> CreateShapeFromPoints (const DPoint3d* points, size_t numberOfPoints, ImagePP::HFCPtr<ImagePP::HGF2DCoordSys> coordSys);

ImagePP::HFCPtr<ImagePP::HVEShape> CreateShapeFromClips (const DRange3d&               spatialIndexRange,
                                       const IScalableMeshClipContainerPtr& clips);

ImagePP::HFCPtr<ImagePP::HVEShape> CreateShapeFromClips (const ImagePP::HFCPtr<ImagePP::HVEShape>        areaShape,
                                       const IScalableMeshClipContainerPtr& clips);

ImagePP::HFCPtr<ImagePP::HVE2DShape> GetGCSDomainsIntersection (BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& firstGCSPtr, 
                                              BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& secondGCSPtr, 
                                              ImagePP::HFCPtr<ImagePP::HGF2DCoordSys> latitudeLongitudeCoordSys);

void GetReprojectedBox (BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& sourceGCSPtr,
                        BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCSPtr,
                        DPoint3d                             boxPoints[],
                        DPoint3d                             reprojectedBoxPoints[]);

StatusInt GetReprojectedBoxDomainLimited (BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCSPtr,
                                          BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& sourceGCSPtr,
                                          DPoint3d    boxPoints[],
                                          DPoint3d    reprojectedBoxPoints[],
                                          DRange3d    additionalSourceExtent,
                                          ImagePP::HFCPtr<ImagePP::HVE2DShape>    queryShape);

StatusInt ReprojectPoint (BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& sourceGCSPtr,
                          BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCSPtr,
                          const DPoint3d& inPoint,
                          DPoint3d& outPoint);

StatusInt ReprojectRangeDomainLimited (DRange3d& reprojectedRange,
                                       const DRange3d& initialRange,
                                       BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& sourceGCS,
                                       BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCS);

ImagePP::HFCPtr<ImagePP::HVE2DShape> ReprojectShapeDomainLimited (BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& sourceGCSPtr, 
                                                BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCSPtr,  
                                                const DPoint3d*   pi_pSourcePt,
                                                size_t  pi_SourcePtQty);

BENTLEY_SM_EXPORT int SetClipToDTM (BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr& dtmPtr,
                  const DRange3d&                spatialIndexRange,
                  const ImagePP::HVE2DShape&              shape);

int AddClipToDTM(BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr&           dtmPtr,
                 const ImagePP::HVE2DShape& shape);

struct BENTLEY_SM_EXPORT PtToPtConverter
    {        
    DPoint3d operator () (const DPoint3d& inputPt) const;

    DPoint3d operator () (const ImagePP::HGF3DCoord<double>& inputPt) const;

    static void Transform(DPoint3d* ptsOut, const DPoint3d* ptsIn, size_t nbPts);
    };

BENTLEY_SM_EXPORT bool IsUrl(WCharCP filename);
