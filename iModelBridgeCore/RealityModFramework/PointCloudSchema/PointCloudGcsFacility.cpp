/*--------------------------------------------------------------------------------------+
|
|     $Source: PointCloudSchema/PointCloudGcsFacility.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <PointCloudSchemaInternal.h>
#include "PointCloudGcsFacility.h"

USING_NAMESPACE_BENTLEY_POINTCLOUDSCHEMA

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCSPtr PointCloudGcsFacility::CreateGcsFromWkt(WStringCR spatialReferenceWkt, DgnDbR dgnDb, bool reprojectElevation)
    {
    DgnGCSPtr sceneGcs;
//    WString spatialReferenceWktStr(spatialReferenceWkt);

    if (!spatialReferenceWkt.empty())
        {
        StatusInt warning;
        WString   warningErrorMsg;
        
        WString            wktWithoutFlavor;
        GeoCoordinates::BaseGCS::WktFlavor wktFlavor = PointCloudGcsFacility::GetWKTFlavor(&wktWithoutFlavor, spatialReferenceWkt);

        // Create a GCS from the point cloud Wkt string
        GeoCoordinates::BaseGCSPtr pSrcBaseGcs = GeoCoordinates::BaseGCS::CreateGCS();
        if(pSrcBaseGcs.IsValid())
            {
            pSrcBaseGcs->InitFromWellKnownText(&warning, &warningErrorMsg, wktFlavor, wktWithoutFlavor.GetWCharCP());
            if(pSrcBaseGcs->IsValid())
                // POINTCLOUD_WIP_GR06_GCS - Dont create DgnGCS; keep the base, because PC exposes a baseGCS, not a DgnGCS
                //                           Also, bug with InitFromWellKnownText (does not accept ...PROJECTION["Lambert Conformal Conic, Two Standard Parallels"]... and others; returned GCS always seems invalid) AR looks at that
                sceneGcs = DgnGCS::CreateGCS(pSrcBaseGcs.get(), dgnDb);
            }
        }

    if (sceneGcs.IsNull() || !sceneGcs->IsValid())
        return NULL;

    sceneGcs->SetReprojectElevation(reprojectElevation);

    return sceneGcs;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Mathieu.St-Pierre   12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
GeoCoordinates::BaseGCS::WktFlavor PointCloudGcsFacility::GetWKTFlavor(WString* wktStrWithoutFlavor, const WString& wktStr)
    {
    PointCloudGcsFacility::WktFlavor wktFlavor = PointCloudGcsFacility::WktFlavor_Oracle9;

    size_t charInd = wktStr.size() - 1;    

    for (charInd = wktStr.size() - 1; charInd >= 0; charInd--)
        {
        if (wktStr[charInd] == L']')
            {
            break;
            }
        else
        if (((short)wktStr[charInd] >= 1) || ((short)wktStr[charInd] < PointCloudGcsFacility::WktFlavor_End))
            {
            wktFlavor = (PointCloudGcsFacility::WktFlavor)wktStr[charInd];            
            }
        }

    if (wktStrWithoutFlavor != 0)
        {   
        *wktStrWithoutFlavor = wktStr.substr(0, charInd + 1);
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
    extent.differenceOf(&rangePts[1], &rangePts[0]);

    // Compute a linear transform that approximate the reprojection transformation.
    StatusInt status = src.GetLocalTransform(&approxTransform, rangePts[0], &extent, true/*doRotate*/, true/*doScale*/, dst);

    // 0 == SUCCESS, 1 == Warning, 2 == Severe Warning,
    // Negative values are severe errors.
    if (status == 0 || status == 1)
        return SUCCESS;

    return ERROR;
    }

//----------------------------------------------------------------------------------------
// Get transformation from native point cloud coordinates to UOR. The transformation required
// for the reprojection to the local coordinate system is included in the output Transform.
// 
// @bsimethod                                                       Eric.Paquet     2/2015
//----------------------------------------------------------------------------------------
BentleyStatus PointCloudGcsFacility::GetTransformToUor(TransformR transform, WString wktStringGeoreference, DRange3dCR rangeUOR, DgnDbR dgnDb)
    {
    // Transformation to UOR
    Transform sceneToUor;
    sceneToUor.initIdentity();
    sceneToUor.ScaleMatrixColumns(UOR_PER_METER, UOR_PER_METER, UOR_PER_METER);

    // Include transformation for the coordinate system if required
    Transform reproTrf;
    if (SUCCESS != GetLocalTransformForReprojection(reproTrf, wktStringGeoreference, rangeUOR, dgnDb))
        {
        // No reprojection to the local coordinate system occurred.
        reproTrf.initIdentity();
        }

    transform.productOf(&reproTrf, &sceneToUor);
    return BSISUCCESS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     2/2015
//----------------------------------------------------------------------------------------
BentleyStatus PointCloudGcsFacility::GetLocalTransformForReprojection(TransformR transform, WString wktStringGeoreference, DRange3dCR rangeUOR, DgnDbR dgnDb)
    {
    transform.initIdentity();

    // Must be able to create a valid GCS
    DgnGCSPtr pDstGcs(dgnDb.Units().GetDgnGCS());

    if (pDstGcs == NULL || !pDstGcs->IsValid())
        return ERROR;

    // Must be able to create a valid GCS
    DgnGCSPtr pSrcGcs = PointCloudGcsFacility::CreateGcsFromWkt(wktStringGeoreference, dgnDb, true);
    if (pSrcGcs.IsNull() || !pSrcGcs->IsValid())
        return ERROR;

    // If the wkt are equals, nothing to do
    if (pSrcGcs->IsEquivalent(*pDstGcs))
        return ERROR;

    BentleyStatus status = SUCCESS;
    bool originalElevationValue = pDstGcs->SetReprojectElevation(true);
    if(SUCCESS != PointCloudGcsFacility::ComputeLocalTransform(rangeUOR, *pSrcGcs, *pDstGcs, transform))
        status = ERROR;

    // Restore original value
    pDstGcs->SetReprojectElevation(originalElevationValue);

    return status;
    }
