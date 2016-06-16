/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/cppwrappers/DTM.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "TerrainModel/Core/IDTM.h"
#include "bcDTMImpl.h"
#include <string.h>
BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sylvain.Pucci   08/10
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t IDTM::GetPointCount ()
    {
    return _GetPointCount();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt IDTM::GetBoundary (BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPointArray& ret)
    {
    return _GetBoundary (ret);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt IDTM::CalculateSlopeArea (double& flatArea, double& slopeArea, DPoint3dCP pts, int numPoints)
    {
    return _CalculateSlopeArea (flatArea, slopeArea, pts, numPoints);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Elenie.Godzaridis 03/16
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt IDTM::CalculateSlopeArea(double& flatArea, double& slopeArea, DPoint3dCP pts, int numPoints, DTMAreaValuesCallback progressiveCallback, DTMCancelProcessCallback isCancelledCallback)
    {
    return _CalculateSlopeArea(flatArea, slopeArea, pts, numPoints, progressiveCallback, isCancelledCallback);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sylvain.Pucci   08/10
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMDraping* IDTM::GetDTMDraping ()
    {
    return _GetDTMDraping ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sylvain.Pucci   08/10
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMDrainage* IDTM::GetDTMDrainage ()
    {
    return _GetDTMDrainage ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sylvain.Pucci   08/10
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMContouring* IDTM::GetDTMContouring ()
    {
    return _GetDTMContouring ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Elenie.Godzaridis  1/16
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMVolume* IDTM::GetDTMVolume ()
    {
    return _GetDTMVolume ();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daryl.Holmwood   01/11
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt IDTM::GetTransformDTM (BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr& transformedDTM, TransformCR transformation)
    {
    return _GetTransformDTM (transformedDTM, transformation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daryl.Holmwood   02/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool IDTM::GetTransformation (TransformR transformation)
    {
    return _GetTransformation (transformation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt IDTMDraping::DrapePoint (double* elevation, double* slope, double* aspect, DPoint3d triangle[3], int* drapedType, DPoint3dCR point)
    {
    return _DrapePoint(elevation, slope, aspect, triangle, drapedType, point);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt IDTMDraping::DrapeLinear (DTMDrapedLinePtr& ret, DPoint3dCP pts, int numPoints)
    {
    return _DrapeLinear(ret, pts, numPoints);
    }


bool IDTMDraping::ProjectPoint(DPoint3dR pointOnDTM, DMatrix4dCR w2vMap, DPoint3dCR testPoint)
    {
    return _ProjectPoint(pointOnDTM, w2vMap, testPoint);
    }

bool IDTMDraping::DrapeAlongVector(DPoint3d* endPt, double *slope, double *aspect, DPoint3d triangle[3], int *drapedType, DPoint3dCR point, double directionOfVector, double slopeOfVector)
    {
    return _DrapeAlongVector(endPt, slope, aspect, triangle, drapedType, point, directionOfVector, slopeOfVector);
    }


DTMStatusInt IDTMDrainage::GetDescentTrace (DTMDrainageFeaturePtr& ret, DPoint3dCR pt, double maxpondDepth)
    {
    return _GetDescentTrace (ret, pt, maxpondDepth);
    }

DTMStatusInt IDTMDrainage::GetAscentTrace (DTMDrainageFeaturePtr& ret, DPoint3dCR pt, double maxpondDepth)
    {
    return _GetAscentTrace (ret, pt, maxpondDepth);
    }

DTMStatusInt IDTMDrainage::TraceCatchmentForPoint (DTMDrainageFeaturePtr& ret, DPoint3dCR pt, double maxpondDepth)
    {
    return _TraceCatchmentForPoint (ret, pt, maxpondDepth);
    }

DTMStatusInt IDTMContouring::ContourAtPoint (DTMPointArray& ret, DPoint3dCR pt,double contourInterval, DTMContourSmoothing smoothOption, double smoothFactor, int smoothDensity, const DTMFenceParams& fence )
    {
    return _ContourAtPoint (ret, pt, contourInterval, smoothOption, smoothFactor, smoothDensity, fence);
    }

DTMStatusInt IDTMContouring::ContourAtPoint (DTMPointArray& ret, DPoint3dCR pt, double contourInterval, DTMContourSmoothing smoothOption, double smoothFactor, int smoothDensity)
    {
    return _ContourAtPoint (ret, pt, contourInterval, smoothOption, smoothFactor, smoothDensity);
    }

int IDTMDrainageFeature::GetNumParts()
    {
    return _GetNumParts();
    }

bool IDTMDrainageFeature::GetIsPond (int index)
    {
    return _GetIsPond (index);
    }

DTMStatusInt IDTMDrainageFeature::GetPoints (DTMPointArray& ret, int index)
    {
    return _GetPoints (ret, index);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Elenie.Godzaridis 1/16
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt IDTMVolume::ComputeCutFillVolume(double* cut, double* fill, double* volume, PolyfaceHeaderCP mesh)
    {
    return _ComputeCutFillVolume(cut, fill, volume, mesh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Elenie.Godzaridis 1/16
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt IDTMVolume::ComputeCutFillVolumeClosed(double* cut, double* fill, double* volume, PolyfaceHeaderCP mesh)
    {
    return _ComputeCutFillVolumeClosed(cut, fill, volume, mesh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Elenie.Godzaridis 1/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool IDTMVolume::RestrictVolumeToRegion(uint64_t regionId)
    {
    return _RestrictVolumeToRegion(regionId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Elenie.Godzaridis 1/16
+---------------+---------------+---------------+---------------+---------------+------*/
void IDTMVolume::RemoveAllRestrictions()
    {
    return _RemoveAllRestrictions();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt IDTM::GetRange (DRange3dR range)
    {
    return _GetRange (range);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu.St-Pierre 04/16
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM::ExportToGeopakTinFile(WCharCP fileNameP)
    {
    return _ExportToGeopakTinFile(fileNameP);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
BcDTMP BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM::GetBcDTM()
    {
    return _GetBcDTM();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt IDTMDrapedLinePoint::GetPointCoordinates (DPoint3d& coordP) const
    {
    return _GetPointCoordinates (coordP);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
double IDTMDrapedLinePoint::GetDistanceAlong () const
    {
    return _GetDistanceAlong ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
DTMDrapedLineCode IDTMDrapedLinePoint::GetCode () const
    {
    return _GetCode ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt IDTMDrapedLine::GetPointByIndex(DTMDrapedLinePointPtr& ret, unsigned int index) const
    {
    return _GetPointByIndex (ret, index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt IDTMDrapedLine::GetPointByIndex(DPoint3d* ptP, double* distanceP, DTMDrapedLineCode *codeP, unsigned int index) const
    {
    return _GetPointByIndex (ptP, distanceP, codeP, index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
unsigned int IDTMDrapedLine::GetPointCount() const
    {
    return _GetPointCount ();
    }

END_BENTLEY_TERRAINMODEL_NAMESPACE
