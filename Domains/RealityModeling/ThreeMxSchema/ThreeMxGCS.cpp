/*-------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/ThreeMxGCS.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ThreeMxSchemaInternal.h"

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_THREEMX_SCHEMA



//----------------------------------------------------------------------------------------
// @bsimethod                                                      Ray.Bentley     09/2015
//----------------------------------------------------------------------------------------
BentleyStatus ThreeMxGCS::GetProjectionTransform (TransformR transform, S3SceneInfoCR sceneInfo, DgnDbR db, DRange3dCR range)
    {
    DgnGCSPtr databaseGCS  = db.Units().GetDgnGCS();

    if (! databaseGCS.IsValid())
        return ERROR;

    if (3 == sceneInfo.SRSOrigin.size())
        transform = Transform::From (DPoint3d::FromXYZ (sceneInfo.SRSOrigin[0], sceneInfo.SRSOrigin[1], sceneInfo.SRSOrigin[2]));

    if (sceneInfo.SRS.empty())
        {
        transform = Transform::FromIdentity();
        return SUCCESS;         // No GCS...
        }

    int                     epsgCode;
    double latitude, longitude;
    WString                 warningMsg;
    StatusInt               status, warning;
    DRange3d                sourceRange;

    DgnGCSPtr    acute3dGCS = DgnGCS::CreateGCS (db);

    if (1 == sscanf (sceneInfo.SRS.c_str(), "EPSG:%d", &epsgCode))
        status = acute3dGCS->InitFromEPSGCode (&warning, &warningMsg, epsgCode);
    else if (2 == sscanf (sceneInfo.SRS.c_str(), "ENU:%lf,%lf", &latitude, &longitude))
        {
        // ENU specification does not impose any projection method so we use the first azimuthal available using values that will
        // mimick the intent (North is Y positive, no offset)
        // Note that we could have injected the origin here but keeping it in the transform as for other GCS specs
        if (latitude < 90.0 && latitude > -90.0 && longitude < 180.0 && longitude > -180.0)
            status = acute3dGCS->InitAzimuthalEqualArea(&warningMsg, L"WGS84", L"METER", longitude, latitude, 0.0, 1.0, 0.0, 0.0, 1);
        else
            status = ERROR;
        }
    else
        status = acute3dGCS->InitFromWellKnownText(&warning, &warningMsg, DgnGCS::wktFlavorEPSG, WString (sceneInfo.SRS.c_str(), false).c_str());

    if (SUCCESS != status)
        {
        BeAssert (false && warningMsg.c_str());
        return ERROR;
        }

    transform.Multiply (sourceRange, range);
    
    DPoint3d        extent;
    Transform       localTransform;

    extent.DifferenceOf (sourceRange.high, sourceRange.low);

    // Compute a linear transform that approximate the reprojection transformation.
    status = acute3dGCS->GetLocalTransform(&localTransform, sourceRange.low, &extent, true/*doRotate*/, true/*doScale*/, *databaseGCS);

    // 0 == SUCCESS, 1 == Wajrning, 2 == Severe Warning,  Negative values are severe errors.
    if (status == 0 || status == 1)
        {        
        transform = Transform::FromProduct (localTransform, transform);

        DPoint3d uorScales(DPoint3d::From(1, 1, 1));
        databaseGCS->UorsFromCartesian(uorScales, uorScales);
        Transform uorsTransform(Transform::FromScaleFactors(uorScales.x, uorScales.y, uorScales.z));        

        transform = Transform::FromProduct (uorsTransform, transform);        

        return SUCCESS;
        }

    return ERROR;

    }
