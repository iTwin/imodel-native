/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/IGeoCoordServices.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! Status values returned by IGeoCoordinateServices methods.
enum GeoReferenceStatus
    {
    GEOREF_STATUS_Success                = 0,
    GEOREF_STATUS_ReprojectDisabled      = GEOREFERENCE_ERROR_BASE,
    GEOREF_STATUS_BadDgnModel            = GEOREFERENCE_ERROR_BASE + 1,
    GEOREF_STATUS_MasterHasNoGCS         = GEOREFERENCE_ERROR_BASE + 2,
    GEOREF_STATUS_AttachmentHasNoGCS     = GEOREFERENCE_ERROR_BASE + 3,
    GEOREF_STATUS_BadArg                 = GEOREFERENCE_ERROR_BASE + 4,
    GEOREF_STATUS_NoLinearTransform      = GEOREFERENCE_ERROR_BASE + 5,
    };

/*---------------------------------------------------------------------------------**//**
* This interface defines a "Geo Coordinate Reference" object. There can only be one
* geo coordinate reference object active at a time. 
*
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct          IGeoCoordinateServices
{
virtual WCharCP                             GetGCSName (WStringR gcsName, DgnGCSP sourceGCS) = 0;
virtual WCharCP                             GetDescription (WStringR gcsDescription, DgnGCSP sourceGCS) = 0;
virtual GeoReferenceStatus                  ReprojectUorPoints (DgnDbR source, DgnDbR target, DPoint3dP outUors, DPoint3dCP inUors, int numPoints) = 0;

virtual GeoReferenceStatus                  ReprojectUorPoints (DgnGCSP sourceGCS, DgnGCSP targetGCS, DPoint3dP outUors, DPoint3dCP inUors, int numPoints) = 0;

//! Get the DgnGCS for the specified project.
//! @return the DgnGCS for this project or NULL if the project is not geo-located
//! @remarks Do not call delete on the returned point
virtual DgnGCS*                             GetGCSFromProject (DgnDbR) = 0;

//! Returns true if the project has an assigned GCS.
virtual bool                                HasGCS (DgnDbR) = 0;

//! Returns true if a reprojection operation is required to convert UORs from sourceGCS to targetGCS.
virtual bool                                RequiresReprojection (DgnGCSP sourceGCS, DgnGCSP targetGCS) = 0;

virtual bool                                GetUnitDefinition (DgnGCSP sourceGCS, UnitDefinitionR unitDef, StandardUnit&  standardUnitNumber) = 0;

/*---------------------------------------------------------------------------------**//**
* Calculates the design coordinates (UORS) of the input Longitude/Latitude/Elevation point.
* @param    outUors         OUT     The calculated design coordinates.
* @param    inLatLong       IN      The longitude,latitude,elevation in the datum of this GCS.
* @return   SUCCESS or a CS_MAP error code if any of the points could not be reprojected.
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual  BentleyStatus UorsFromLatLong
(
DPoint3dR               outUors,
GeoPointCR              inLatLong,
DgnGCS&                 gcs
) = 0;

/*---------------------------------------------------------------------------------**//**
* Calculates the longitude, latitude, and elevation from design coordinates (UORS).
* @param    outLatLong      OUT     The calculated longitude,latitude,elevation in the datum of this GCS.
* @param    inUors          IN      The input design coordinates.
* @return   SUCCESS or a CS_MAP error code if any of the points could not be reprojected.
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual  BentleyStatus LatLongFromUors
(
GeoPointR               outLatLong,
DPoint3dCR              inUors,
DgnGCS&                 gcs
) = 0;
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
