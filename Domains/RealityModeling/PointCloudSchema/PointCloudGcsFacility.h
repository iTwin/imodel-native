/*--------------------------------------------------------------------------------------+
|
|     $Source: PointCloudSchema/PointCloudGcsFacility.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
            static BentleyStatus                                    GetLocalTransformForReprojection(TransformR transform, WString wktStringGeoreference, DRange3dCR rangeUOR, DgnDbR dgnDb);

    public:

        enum WktFlavor
            {
            WktFlavor_Oracle9 = 1, 
            WktFlavor_Autodesk,             
            WktFlavor_OGC,             
            WktFlavor_End, 
            };

        static GeoCoordinates::BaseGCS::WktFlavor                   GetWKTFlavor(WStringP wktStrWithoutFlavor, const WString& wktStr);       
        static bool                                                 MapWktFlavorEnum(GeoCoordinates::BaseGCS::WktFlavor& baseGcsWktFlavor, PointCloudGcsFacility::WktFlavor wktFlavor);

        POINTCLOUDSCHEMA_EXPORT static DgnGCSPtr                    CreateGcsFromWkt(WStringCR spatialReferenceWkt, DgnDbR dgnDb, bool reprojectElevation);
        POINTCLOUDSCHEMA_EXPORT static BentleyStatus                ComputeLocalTransform(DRange3dCR range, DgnGCSR src, DgnGCSR dst, TransformR approxTransform);
        POINTCLOUDSCHEMA_EXPORT static BentleyStatus                GetTransformToUor(TransformR transform, WString wktStringGeoreference, DRange3dCR rangeUOR, DgnDbR dgnDb);
    };


END_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE
