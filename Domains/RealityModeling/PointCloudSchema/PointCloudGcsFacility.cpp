/*--------------------------------------------------------------------------------------+
|
|     $Source: PointCloudSchema/PointCloudGcsFacility.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <PointCloudSchemaInternal.h>
#include "PointCloudGcsFacility.h"

USING_NAMESPACE_BENTLEY_POINTCLOUDSCHEMA

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
GeoCoordinates::BaseGCSPtr PointCloudGcsFacility::CreateGcsFromWkt(WStringCR spatialReferenceWkt)
    {
    if (spatialReferenceWkt.empty())
        return nullptr;

    StatusInt warning;
    WString   warningErrorMsg;
        
    WString wktWithoutFlavor;
    GeoCoordinates::BaseGCS::WktFlavor wktFlavor = PointCloudGcsFacility::GetWKTFlavor(&wktWithoutFlavor, spatialReferenceWkt);

    // Create a GCS from the point cloud Wkt string
    GeoCoordinates::BaseGCSPtr pSrcBaseGcs = GeoCoordinates::BaseGCS::CreateGCS();
    if (!pSrcBaseGcs.IsValid())
        return nullptr;

    pSrcBaseGcs->InitFromWellKnownText(&warning, &warningErrorMsg, wktFlavor, wktWithoutFlavor.GetWCharCP());
    if (pSrcBaseGcs.IsNull() || !pSrcBaseGcs->IsValid())
        return nullptr;

    pSrcBaseGcs->SetReprojectElevation(true);

    return pSrcBaseGcs;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Mathieu.St-Pierre   12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
GeoCoordinates::BaseGCS::WktFlavor PointCloudGcsFacility::GetWKTFlavor(WString* wktStrWithoutFlavor, const WString& wktStr)
    {
    PointCloudGcsFacility::WktFlavor wktFlavor = PointCloudGcsFacility::WktFlavor_Oracle9;

    int64_t charInd = wktStr.size() - 1;
    for (charInd = wktStr.size() - 1; charInd >= 0; charInd--)
        {
        if (wktStr[(size_t)charInd] == L']')
            {
            break;
            }
        else
        if (((short)wktStr[(size_t)charInd] >= 1) || ((short)wktStr[(size_t)charInd] < PointCloudGcsFacility::WktFlavor_End))
            {
            wktFlavor = (PointCloudGcsFacility::WktFlavor)wktStr[(size_t)charInd];            
            }
        }

    if (wktStrWithoutFlavor != 0)
        {   
        *wktStrWithoutFlavor = wktStr.substr(0, (size_t)(charInd + 1));
        }

    GeoCoordinates::BaseGCS::WktFlavor baseGcsWktFlavor;

    bool result = PointCloudGcsFacility::MapWktFlavorEnum(baseGcsWktFlavor, wktFlavor);

    assert(result == true);
        
    return baseGcsWktFlavor;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Mathieu.St-Pierre   12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointCloudGcsFacility::MapWktFlavorEnum(GeoCoordinates::BaseGCS::WktFlavor& baseGcsWktFlavor, PointCloudGcsFacility::WktFlavor wktFlavor)
    {
    //Temporary use numeric value until the basegcs enum match the csmap's one.
    switch(wktFlavor)
        {
        case PointCloudGcsFacility::WktFlavor_Oracle9 :
            baseGcsWktFlavor = GeoCoordinates::BaseGCS::wktFlavorOracle9;
            break;

        case PointCloudGcsFacility::WktFlavor_Autodesk :
            baseGcsWktFlavor = GeoCoordinates::BaseGCS::wktFlavorAutodesk;
            break;

        case PointCloudGcsFacility::WktFlavor_OGC :
            baseGcsWktFlavor = GeoCoordinates::BaseGCS::wktFlavorOGC;
            break;

        default : 
            return false;
        }

    return true;   
    }     

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PointCloudGcsFacility::ComputeLocalTransform(DRange3dCR range, DgnGCSR src, DgnGCSR dst, TransformR approxTransform)
    {
    DPoint3d rangePts[2];
    rangePts[0] = range.low;
    rangePts[1] = range.high;
    
    // Compute the extent to compute the linear transform
    DPoint3d extent;
    extent.DifferenceOf (rangePts[1], rangePts[0]);

    // Compute a linear transform that approximate the reprojection transformation.
    StatusInt status = src.GetLocalTransform(&approxTransform, rangePts[0], &extent, true/*doRotate*/, true/*doScale*/, dst);

    // 0 == SUCCESS, 1 == Warning, 2 == Severe Warning,
    // Negative values are severe errors.
    if (status == 0 || status == 1)
        return SUCCESS;

    return ERROR;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2016
//----------------------------------------------------------------------------------------
BentleyStatus PointCloudGcsFacility::ComputeSceneToWorldTransform(TransformR sceneToWorld, WStringCR spatialReferenceWkt, DRange3dCR range, DgnDbR dgnDb)
    {
    //  *** Not required UOR_PER_METER == 1
    //     Transform sceneToUor;
    //     sceneToUor.InitIdentity();
    //     sceneToUor.ScaleMatrixColumns(UOR_PER_METER, UOR_PER_METER, UOR_PER_METER);
    sceneToWorld.InitIdentity();

    if (spatialReferenceWkt.empty())
        return SUCCESS;

    // Scene GCS
    GeoCoordinates::BaseGCSPtr pSceneGcs = PointCloudGcsFacility::CreateGcsFromWkt(spatialReferenceWkt);
    if (pSceneGcs.IsNull() || !pSceneGcs->IsValid())
        return SUCCESS; // Assumed to be coincident.

    // Dgn GCS
    DgnGCSP pDstGcs = dgnDb.Units().GetDgnGCS();
    if (pDstGcs == NULL || !pDstGcs->IsValid())
        return SUCCESS; // Assumed to be coincident.    

    // POINTCLOUD_WIP_GR06_GCS - Dont create DgnGCS; keep the base, because PC exposes a baseGCS, not a DgnGCS
    // Review the need to transform range with UnitsFromMeters if we stop using DgnGcs.
    DgnGCSPtr pSrcGcs = DgnGCS::CreateGCS(pSceneGcs.get(), dgnDb);

    // If the wkt are equals, nothing to do
    if (pSrcGcs->IsEquivalent(*pDstGcs))
        return SUCCESS;

    bool originalElevationValue = pDstGcs->SetReprojectElevation(true);

    BentleyStatus status = PointCloudGcsFacility::ComputeLocalTransform(range, *pSrcGcs, *pDstGcs, sceneToWorld);

    // Restore original value
    pDstGcs->SetReprojectElevation(originalElevationValue);

    return status;
    }
