/*--------------------------------------------------------------------------------------+
|
|     $Source: PointCloudSchema/PointCloudGcsFacility.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct PointCloudGcsFacility
    {           
    private:
        enum WktFlavor
            {
            WktFlavor_Oracle9 = 1,
            WktFlavor_Autodesk,
            WktFlavor_OGC,
            WktFlavor_End,
            };

        static GeoCoordinates::BaseGCS::WktFlavor   GetWKTFlavor(WStringP wktStrWithoutFlavor, WStringCR wktStr);
        static bool                                 MapWktFlavorEnum(GeoCoordinates::BaseGCS::WktFlavor& baseGcsWktFlavor, PointCloudGcsFacility::WktFlavor wktFlavor);
        static GeoCoordinates::BaseGCSPtr           CreateGcsFromWkt(WStringCR spatialReferenceWkt);
        static BentleyStatus                        ComputeLocalTransform(DRange3dCR range, DgnGCSR src, DgnGCSR dst, TransformR approxTransform);
    public:      
        
        static BentleyStatus ComputeSceneToWorldTransform(TransformR sceneToWorld, WStringCR spatialReferenceWkt, DRange3dCR range, DgnDbR dgnDb);
    };


END_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE
