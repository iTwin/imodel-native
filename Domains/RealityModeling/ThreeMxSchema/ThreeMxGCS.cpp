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
        return SUCCESS;         // No GCS...

    int                     epsgCode;
    WString                 warningMsg;
    StatusInt               status, warning;
    DRange3d                sourceRange;

    DgnGCSPtr    acute3dGCS = DgnGCS::CreateGCS (db);

    if (1 == sscanf (sceneInfo.SRS.c_str(), "EPSG:%d", &epsgCode))
        status = acute3dGCS->InitFromEPSGCode (&warning, &warningMsg, epsgCode);
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
        return SUCCESS;
        }

    return ERROR;

    }
